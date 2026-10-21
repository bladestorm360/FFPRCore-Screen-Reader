#include "text_util.h"
#include <windows.h>
#include <cstdio>
#include <cstdarg>
#include <locale.h>
#include <array>
#include <atomic>
#include <algorithm>
#include "log.h"
#include "timing.h"

// ---------------------------------------------------------------------------
// Win32 file I/O ONLY — deliberately no CRT stdio.
//
// The first version of this file opened the log with _wfopen_s(..., L"w, ccs=UTF-8") and
// then wrote to it with narrow fprintf. That combination puts the stream in Unicode
// orientation and then violates it, which trips the CRT's invalid-parameter handler ->
// __fastfail -> STATUS_STACK_BUFFER_OVERRUN (0xC0000409). Inside a DLL injected into
// someone else's process that does not throw an error, it TERMINATES THE GAME. It did:
// the game refused to launch, with VERSION.dll named as the faulting module and a 3-byte
// log containing nothing but the BOM.
//
// CreateFileW/WriteFile have no orientation, no locale, and no invalid-parameter handler.
// We format into a fixed buffer, convert to UTF-8 ourselves, and write bytes. Nothing in
// this file can abort the process.
// ---------------------------------------------------------------------------

namespace {

_locale_t g_utf8 = nullptr;   // see init()

HANDLE           g_file  = INVALID_HANDLE_VALUE;
SRWLOCK          g_lock = SRWLOCK_INIT;
std::atomic<bool> g_ready{false}, g_stopping{false};
HANDLE           g_wake = nullptr, g_worker = nullptr;
struct TimingLine {
    timing::Kind kind{};
    char name[96]{}, detail[96]{}, parent[96]{};
    uint64_t frame=0,span=0,parent_span=0,count=0,stamp=0;
    int64_t started=0,elapsed=0,self=0,cpu=-1;
    uintptr_t code=0;
    DWORD thread=0;
    int value=0;
};
struct Line { DWORD size = 0; char bytes[2048]{}; bool is_timing=false; TimingLine timing; };
constexpr size_t kQueueSize = 2048;
std::array<Line, kQueueSize> g_queue;
size_t g_head = 0, g_count = 0;
std::atomic<unsigned> g_dropped{0};
size_t           g_bytes = 0;
wchar_t          g_base[MAX_PATH]{};
// Writer-owned health counters survive a failed file handle. Recovery appends to
// the same part rather than truncating the evidence already on disk.
uint64_t g_started = 0, g_written = 0, g_failed_writes = 0, g_lost_bytes = 0, g_total_dropped = 0;
DWORD g_last_error = 0;
unsigned g_part = 0;
constexpr DWORD kHealthInterval = 10000;

// Preserve a valid UTF-8 prefix and make diagnostic truncation visible.
void mark_truncated(char* bytes, size_t capacity) {
    constexpr char marker[] = " [truncated]";
    size_t end = (std::min)(strlen(bytes), capacity - sizeof(marker));
    while (end && (static_cast<unsigned char>(bytes[end]) & 0xC0) == 0x80) --end;
    memcpy(bytes + end, marker, sizeof(marker));
}
void write_bytes(const char* data, int len);

void build_path(wchar_t* out, size_t n, int index)
{
    if (index == 0) _snwprintf_s(out, n, _TRUNCATE, L"%s.log", g_base);
    else            _snwprintf_s(out, n, _TRUNCATE, L"%s.%d.log", g_base, index);
}

void rotate()
{
    wchar_t from[MAX_PATH], to[MAX_PATH];
    build_path(to, MAX_PATH, log_::kKeep);
    DeleteFileW(to);
    for (int i = log_::kKeep - 1; i >= 0; --i) {
        build_path(from, MAX_PATH, i);
        build_path(to,   MAX_PATH, i + 1);
        MoveFileExW(from, to, MOVEFILE_REPLACE_EXISTING);
    }
}

void open_current(bool append = false)
{
    wchar_t path[MAX_PATH];
    build_path(path, MAX_PATH, 0);
    // FILE_SHARE_DELETE as well as READ: without it, a rename or delete of the file we hold
    // open FAILS, so a rotation while the game is running would move some of the numbered
    // logs and silently skip others (that is why a ffpr_screen_reader.2.log went missing). Sharing
    // delete also lets the user copy or clear the log without closing the game.
    g_file = CreateFileW(path, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_DELETE, nullptr,
                         append ? OPEN_ALWAYS : CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (g_file == INVALID_HANDLE_VALUE) { ++g_failed_writes; g_last_error = GetLastError(); return; }
    if (append) {
        LARGE_INTEGER zero{}, end{};
        if (!SetFilePointerEx(g_file, zero, &end, FILE_END)) {
            ++g_failed_writes; g_last_error = GetLastError(); CloseHandle(g_file); g_file = INVALID_HANDLE_VALUE; return;
        }
        g_bytes = static_cast<size_t>(end.QuadPart);
        return;
    }
    g_bytes = 0;
    if (g_file != INVALID_HANDLE_VALUE) {
        const unsigned char bom[3] = { 0xEF, 0xBB, 0xBF };
        write_bytes(reinterpret_cast<const char*>(bom), 3);
        char identity[160];
        const int n = _snprintf_s(identity, sizeof(identity), _TRUNCATE,
            "[log] session pid=%lu start-tick=%llu part=%u\r\n", GetCurrentProcessId(), g_started, g_part);
        if (n > 0) write_bytes(identity, n);
    }
}

void write_bytes(const char* data, int len)
{
    if (len <= 0) return;
    if (g_file == INVALID_HANDLE_VALUE) { g_lost_bytes += len; return; }
    DWORD total = 0;
    DWORD failure = ERROR_WRITE_FAULT;
    // WriteFile may succeed with a short write. Retry only the remaining suffix;
    // bound attempts even on a malfunctioning file/device. This is worker-only.
    for (unsigned attempt = 0; total < static_cast<DWORD>(len) && attempt < 4; ++attempt) {
        DWORD wrote = 0;
        const bool ok = WriteFile(g_file, data + total, static_cast<DWORD>(len) - total, &wrote, nullptr) != FALSE;
        if (!ok || !wrote || wrote > static_cast<DWORD>(len) - total) {
            failure = ok ? ERROR_WRITE_FAULT : GetLastError(); break;
        }
        total += wrote;
    }
    g_bytes += total;
    if (total != static_cast<DWORD>(len)) {
        ++g_failed_writes; g_lost_bytes += len - total;
        g_last_error = failure;
        CloseHandle(g_file); g_file = INVALID_HANDLE_VALUE;
        return;
    }
    ++g_written;
    // NO FlushFileBuffers here. WriteFile has already handed the bytes to the kernel's file
    // cache, and a crash of THIS process (the only crash the mod can cause) never loses
    // them - only a kernel crash or a power cut could. The flush that used to sit on this
    // line forced a synchronous disk write per log line, 15-30 ms each on the game thread:
    // every battle cursor move logged two lines and stalled the game for two frames, and a
    // route's `smooth=28ms` stage timing was the flush of the log line inside it, not the
    // smoothing (2026-09-25 logs, nav_open_issues.md section 6). Every hook logs, so the
    // logger must cost microseconds, not milliseconds.

    if (g_bytes >= log_::kMaxBytes) {
        CloseHandle(g_file);
        g_file = INVALID_HANDLE_VALUE;
        rotate();
        ++g_part;
        open_current();
    }
}

// Producers never take a disk/rotation lock, wait for the writer, or perform I/O.
// A full/contended diagnostic queue drops logs, never game input or speech.
void emit_utf8(const char* utf8_line)
{
    char out[2048]{};
    DWORD ms = GetTickCount();
    int n = _snprintf_s(out, sizeof(out), _TRUNCATE, "[%7u.%03u] %s\r\n",
                        ms / 1000, ms % 1000, utf8_line);
    if (n < 0) {
        // _TRUNCATE returns -1 with a usable prefix. Retain it with an explicit
        // marker, stopping before a partial UTF-8 code point instead of dropping
        // the entire diagnostic (especially long CJK/braille text).
        constexpr char marker[] = " [truncated]\r\n";
        size_t end = (std::min)(strlen(out), sizeof(out) - sizeof(marker));
        while (end && (static_cast<unsigned char>(out[end]) & 0xC0) == 0x80) --end;
        memcpy(out + end, marker, sizeof(marker)); n = static_cast<int>(end + sizeof(marker) - 1);
    }
    if (n <= 0 || !g_ready.load(std::memory_order_acquire)) return;
    if (!TryAcquireSRWLockExclusive(&g_lock)) { ++g_dropped; return; }
    if (g_count < kQueueSize) {
        auto& line = g_queue[(g_head + g_count++) % kQueueSize];
        line.is_timing=false;
        line.size = static_cast<DWORD>(n);
        memcpy(line.bytes, out, n);
    } else ++g_dropped;
    ReleaseSRWLockExclusive(&g_lock);
    SetEvent(g_wake);
}

void timing_text(char (&out)[96],const char* text) {
    size_t i=0;if(text)for(;i<sizeof(out)-1&&text[i];++i)out[i]=text[i];out[i]=0;
}
void emit_timing(const timing::Event& event) {
    if(!g_ready.load(std::memory_order_acquire))return;
    if(!TryAcquireSRWLockExclusive(&g_lock)){++g_dropped;return;}
    if(g_count<kQueueSize) {
        auto& line=g_queue[(g_head+g_count++)%kQueueSize];line.is_timing=true;
        auto& row=line.timing;
        row.kind=event.kind;row.frame=event.frame;row.span=event.span;row.parent_span=event.parent_span;
        row.count=event.count;row.stamp=GetTickCount64();row.started=event.started;
        row.elapsed=event.elapsed;row.self=event.self;row.cpu=event.cpu;row.code=event.code;row.thread=event.thread;row.value=event.value;
        timing_text(row.name,event.name);timing_text(row.detail,event.detail);timing_text(row.parent,event.parent);
    } else ++g_dropped;
    ReleaseSRWLockExclusive(&g_lock);
    SetEvent(g_wake);
}
void write_timing(const TimingLine& row) {
    // Logger worker only: no formatting, module lookup or I/O on the producer.
    char bytes[2048];const double ms=1000.0/timing::frequency;
    int size=0;
    if(row.kind==timing::Kind::Summary) {
        size=_snprintf_s(bytes,sizeof(bytes),_TRUNCATE,
            "[%7llu.%03llu] [Timing] summary %s: calls=%llu observed=%.3fms exclusive=%.3fms max=%.3fms thread=%lu frame=%llu\r\n",
            row.stamp/1000,row.stamp%1000,row.name,row.count,row.elapsed*ms,row.self*ms,row.code*ms,row.thread,row.frame);
    } else if(row.kind==timing::Kind::Marker) {
        size=_snprintf_s(bytes,sizeof(bytes),_TRUNCATE,
            "[%7llu.%03llu] [Timing] context %s value=%d thread=%lu frame=%llu\r\n",
            row.stamp/1000,row.stamp%1000,row.name,row.value,row.thread,row.frame);
    } else if(row.kind==timing::Kind::Gap) {
        size=_snprintf_s(bytes,sizeof(bytes),_TRUNCATE,
            "[%7llu.%03llu] [Timing] input interval %.3fms owner-cpu=%.3fms thread=%lu frame=%llu; includes native engine and unobserved time; cpu=-1 if unavailable\r\n",
            row.stamp/1000,row.stamp%1000,row.elapsed*ms,row.cpu<0?-1.0:row.cpu/10000.0,row.thread,row.frame);
    } else {
        const auto base=timing::game_base.load(),extent=timing::game_size.load();
        const bool game_code=row.code>=base&&row.code-base<extent;
        const char* kind=row.kind==timing::Kind::Mod?"mod":row.kind==timing::Kind::Native?"native-read":"native-original";
        size=_snprintf_s(bytes,sizeof(bytes),_TRUNCATE,
            "[%7llu.%03llu] [Timing] slow %s %s%s%s: %.3fms exclusive=%.3fms owner-cpu=%.3fms qpc=%.6f thread=%lu frame=%llu span=%llu parent=%llu/%s code=%s0x%llX\r\n",
            row.stamp/1000,row.stamp%1000,kind,row.name,row.detail[0]?".":"",row.detail,
            row.elapsed*ms,row.self*ms,row.cpu<0?-1.0:row.cpu/10000.0,static_cast<double>(row.started)/timing::frequency,
            row.thread,row.frame,row.span,row.parent_span,row.parent,game_code?"GameAssembly+":"",static_cast<unsigned long long>(game_code?row.code-base:row.code));
    }
    if(size>0)write_bytes(bytes,size);
}

DWORD WINAPI writer(void*)
{
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
    rotate();
    open_current();
    auto health = [] {
        if (g_file == INVALID_HANDLE_VALUE) open_current(true);
        char line[512];
        const int n = _snprintf_s(line, sizeof(line), _TRUNCATE,
            "[log] health pid=%lu session=%llu part=%u uptime-ms=%llu frame=%llu written=%llu dropped=%llu write-failures=%llu lost-bytes=%llu last-error=%lu\r\n",
            GetCurrentProcessId(), g_started, g_part, GetTickCount64() - g_started,
            timing::frame.load(std::memory_order_relaxed), g_written, g_total_dropped,
            g_failed_writes, g_lost_bytes, g_last_error);
        if (n > 0) write_bytes(line, n);
    };
    health();
    uint64_t next_health = GetTickCount64() + kHealthInterval;
    for (;;) {
        const DWORD wake = WaitForSingleObject(g_wake, kHealthInterval);
        if (wake != WAIT_OBJECT_0 && wake != WAIT_TIMEOUT) break;
        for (;;) {
            Line line;
            AcquireSRWLockExclusive(&g_lock);
            const bool have = g_count != 0;
            if (have) {
                line = g_queue[g_head];
                g_head = (g_head + 1) % kQueueSize;
                --g_count;
            }
            ReleaseSRWLockExclusive(&g_lock);
            if (!have) break;
            if(line.is_timing)write_timing(line.timing);
            else write_bytes(line.bytes, static_cast<int>(line.size));
            // Sustained diagnostics must not postpone health/recovery forever.
            if (GetTickCount64() >= next_health) { health(); next_health = GetTickCount64() + kHealthInterval; }
        }
        const unsigned lost = g_dropped.exchange(0);
        if (lost) {
            g_total_dropped += lost;
            char line[128];
            const int n = _snprintf_s(line, sizeof(line), _TRUNCATE,
                "[log] %u diagnostic lines dropped; producer never waits for disk\r\n", lost);
            if (n > 0) write_bytes(line, n);
        }
        if (GetTickCount64() >= next_health) { health(); next_health = GetTickCount64() + kHealthInterval; }
        if (g_stopping.load(std::memory_order_acquire)) break;
    }
    health();
    if (g_file != INVALID_HANDLE_VALUE) { CloseHandle(g_file); g_file = INVALID_HANDLE_VALUE; }
    return 0;
}

} // namespace

namespace log_ {

void init()
{
    if (g_ready) return;

    GetModuleFileNameW(nullptr, g_base, MAX_PATH);
    if (wchar_t* slash = wcsrchr(g_base, L'\\')) *(slash + 1) = 0;
    text_util::append(g_base, MAX_PATH, L"ffpr_screen_reader", _TRUNCATE);

    // %ls / %S in a NARROW printf converts through the C locale, and a wide character the
    // locale cannot represent (the en dash in "Kingdom of Caldisla – Town") makes the whole
    // call fail: the line came out EMPTY, and every route diagnostic on the world map was a
    // blank line (2026-09-22). Formatting through a UTF-8 locale keeps them. The CRT is
    // static (/MT), so this locale object is ours alone and never touches the game's.
    g_utf8 = _create_locale(LC_ALL, ".UTF8");
    g_started = GetTickCount64();
    g_wake = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if (!g_wake) return;
    g_worker = CreateThread(nullptr, 0, writer, nullptr, 0, nullptr);
    if (!g_worker) { CloseHandle(g_wake); g_wake = nullptr; return; }
    g_ready.store(true, std::memory_order_release);
    timing::enable(&emit_timing);

    // Name the host process in the header. Only the game boots the mod now (see
    // host_is_game in dllmain.cpp), but if a log ever turns up that was written by something
    // else, it should say so itself rather than being mistaken for the run you just played.
    wchar_t exe[MAX_PATH]{};
    GetModuleFileNameW(nullptr, exe, MAX_PATH);
    const wchar_t* slash = wcsrchr(exe, L'\\');
    char           host[64]{};
    WideCharToMultiByte(CP_UTF8, 0, slash ? slash + 1 : exe, -1, host, sizeof(host) - 1,
                        nullptr, nullptr);

    SYSTEMTIME st{};
    GetLocalTime(&st);
    write("=== FFPR native screen reader - %04d-%02d-%02d %02d:%02d:%02d - %s pid=%lu ===",
          st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond,
          host[0] ? host : "?", GetCurrentProcessId());
    write("[Timing] automatic diagnostics: slow scopes >=5ms, input gaps >=50ms, observed-scope summaries every5s; selected/input-interval owner CPU time (-1 unmeasured); bounded queue, no producer waits");
}

void write(const char* fmt, ...)
{
    if (!g_ready) return;
    char buf[1024];
    va_list ap;
    va_start(ap, fmt);
    int n = g_utf8 ? _vsnprintf_s_l(buf, sizeof(buf), _TRUNCATE, fmt, g_utf8, ap)
                   : _vsnprintf_s(buf, sizeof(buf), _TRUNCATE, fmt, ap);
    if (n < 0 && buf[0] == 0) _snprintf_s(buf, sizeof(buf), _TRUNCATE, "(unformattable line: %s)", fmt);
    else if (n < 0) mark_truncated(buf, sizeof(buf));
    va_end(ap);
    emit_utf8(buf);

}

void writeW(const wchar_t* prefix, const wchar_t* text)
{
    if (!g_ready) return;
    wchar_t combined[1024];
    const int combined_size = _snwprintf_s(combined, sizeof(combined) / sizeof(combined[0]), _TRUNCATE, L"%s%s",
                 prefix ? prefix : L"", text ? text : L"");
    if (combined_size < 0) {
        constexpr wchar_t marker[] = L" [truncated]";
        size_t end = (sizeof(combined) - sizeof(marker)) / sizeof(wchar_t);
        if (end && text_util::high_surrogate(combined[end - 1])) --end;
        memcpy(combined + end, marker, sizeof(marker));
    }

    // A UTF-16 unit can require three UTF-8 bytes (four per surrogate pair).
    // Conversion must fit before emit_utf8 applies its marked byte limit.
    char utf8[4096];
    int n = WideCharToMultiByte(CP_UTF8, 0, combined, -1, utf8, sizeof(utf8), nullptr, nullptr);
    if (n > 0) emit_utf8(utf8);   // n includes the terminator; emit_utf8 uses strlen semantics

}

void shutdown()
{
    // Explicit offline-host teardown only. DllMain intentionally does not call this.
    // Keep immutable locale/event storage alive for any producer already formatting.
    timing::sink.store(nullptr,std::memory_order_release);
    if (!g_ready.exchange(false)) return;
    g_stopping.store(true, std::memory_order_release);
    SetEvent(g_wake);
    WaitForSingleObject(g_worker, INFINITE);
    CloseHandle(g_worker); g_worker = nullptr;
}

} // namespace log_
