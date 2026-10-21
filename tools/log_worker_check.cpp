// Real logger with a blocked fake file write; no game DLL or screen reader is loaded.
#include <windows.h>
#include <atomic>
#include <cstdio>
#include <thread>
#include <string>
namespace fixture {
HANDLE blocked, release, idle_health;
DWORD producer_thread = GetCurrentThreadId();
std::atomic<bool> wrong_thread{false};
std::string captured;
bool fail_next = false, short_next = false;
BOOL WINAPI write_file(HANDLE, LPCVOID bytes, DWORD count, LPDWORD written, LPOVERLAPPED) {
    if (GetCurrentThreadId() == producer_thread) wrong_thread = true;
    SetEvent(blocked);
    WaitForSingleObject(release, INFINITE);
    if (fail_next) { fail_next = false; *written = 0; SetLastError(ERROR_DISK_FULL); return FALSE; }
    *written = short_next ? count / 2 : count;
    short_next = false;
    captured.append(static_cast<const char*>(bytes), *written);
    if (std::string_view(static_cast<const char*>(bytes), *written).find("frame=4321") != std::string_view::npos) SetEvent(idle_health);
    return TRUE;
}
}
#define WriteFile fixture::write_file
#include "../src/core/log.cpp"
#undef WriteFile
int main() {
    fixture::blocked = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    fixture::release = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    fixture::idle_health = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    log_::init();
    int failures = 0;
    auto expect = [&](bool ok, const char* message) { if (!ok) { ++failures; std::printf("FAIL %s\n", message); } };
    expect(WaitForSingleObject(fixture::blocked, 5000) == WAIT_OBJECT_0, "writer reaches a blocked file write");
    // The writer is parked outside the queue lock. Preserve these records before
    // intentionally filling the queue, so the Unicode assertions do not depend
    // on the documented policy of dropping a contended diagnostic.
    log_::write("long-narrow: %s", std::string(3000, 'a').c_str());
    log_::write("long-formatted-wide: %ls", std::wstring(900, L'\u65e5').c_str());
    log_::writeW(L"long-wide: ", std::wstring(1000, L'\u65e5').c_str());
    log_::writeW(L"long-wide-ascii: ", std::wstring(2000, L'x').c_str());
    HANDLE done = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    std::thread producer([&] {
        for (int i = 0; i < 6000; ++i) log_::write("diagnostic %d", i);
        log_::writeW(L"speech: ", L"Japanese \u65e5\u672c\u8a9e");
        SetEvent(done);
    });
    // Event evidence, independent of a timing benchmark: all producers finish
    // while disk remains blocked, even when the fixed diagnostic queue is full.
    expect(WaitForSingleObject(done, 5000) == WAIT_OBJECT_0, "producers never wait for disk or log rotation");
    expect(g_dropped > 0 && g_count <= kQueueSize, "full queue remains bounded and records diagnostic loss");
    expect(!fixture::wrong_thread && GetThreadPriority(g_worker) == THREAD_PRIORITY_BELOW_NORMAL, "file I/O is confined to the lower-priority logger worker");
    SetEvent(fixture::release);
    producer.join();
    // Wait only in this offline test until the writer has passed its initial
    // health record, then change the frame for the idle periodic-health check.
    const auto deadline = GetTickCount64() + 5000;
    for (;;) {
        AcquireSRWLockExclusive(&g_lock); const auto pending = g_count; ReleaseSRWLockExclusive(&g_lock);
        if (pending < kQueueSize / 2 || GetTickCount64() >= deadline) break;
        std::this_thread::yield();
    }
    timing::frame.store(4321);
    expect(WaitForSingleObject(fixture::idle_health, 15000) == WAIT_OBJECT_0,
           "idle logger writes periodic native-frame health without a producer wake");
    log_::shutdown();
    expect(g_count == 0, "explicit host teardown drains the logger queue");
    expect(fixture::captured.find("long-narrow:") != std::string::npos,
           "long narrow diagnostic retains its prefix");
    expect(fixture::captured.find("long-formatted-wide:") != std::string::npos,
           "long formatted Unicode diagnostic retains its prefix");
    expect(fixture::captured.find("long-wide:") != std::string::npos && fixture::captured.find("[truncated]") != std::string::npos,
           "long wide diagnostic retains a marked prefix");
    const auto ascii_start = fixture::captured.find("long-wide-ascii:");
    const auto ascii_end = fixture::captured.find('\n', ascii_start);
    expect(ascii_start != std::string::npos && fixture::captured.substr(ascii_start, ascii_end - ascii_start).find("[truncated]") != std::string::npos,
           "wide-buffer truncation is marked even when the remaining ASCII fits the UTF-8 output");
    expect(fixture::captured.find("[log] health pid=") != std::string::npos &&
           fixture::captured.find("frame=4321") != std::string::npos &&
           fixture::captured.find("write-failures=0") != std::string::npos,
           "session health identifies the run, native frame progress and file loss counters");
    // Exercise the same writer primitives without a ten-second test delay.
    // File operations still run on a dedicated thread, with no game loaded.
    bool short_ok = false, failure_ok = false, recovery_ok = false, rotation_ok = false;
    const auto old_failures = g_failed_writes;
    std::thread recovery([&] {
        open_current(true);
        fixture::short_next = true;
        const auto start = fixture::captured.size();
        write_bytes("partial-write\r\n", 15);
        short_ok = fixture::captured.substr(start) == "partial-write\r\n" && g_failed_writes == old_failures;
        fixture::fail_next = true;
        write_bytes("failed\r\n", 8);
        failure_ok = g_file == INVALID_HANDLE_VALUE && g_failed_writes == old_failures + 1 &&
                     g_last_error == ERROR_DISK_FULL && g_lost_bytes >= 8;
        open_current(true); write_bytes("recovered\r\n", 11);
        recovery_ok = g_file != INVALID_HANDLE_VALUE && fixture::captured.ends_with("recovered\r\n");
        const auto part = g_part;
        g_bytes = log_::kMaxBytes - 1;
        write_bytes("rotate\r\n", 8);
        rotation_ok = g_part == part + 1 && fixture::captured.find("part=" + std::to_string(g_part)) != std::string::npos;
        CloseHandle(g_file); g_file = INVALID_HANDLE_VALUE;
    });
    recovery.join();
    expect(short_ok, "successful short writes retain their suffix without duplication");
    expect(failure_ok, "disk failure retires the handle and records error and lost bytes");
    expect(recovery_ok, "failed log handle can reopen in append mode and resume diagnostics");
    expect(rotation_ok, "size rotation writes the same session identity and a new part number");
    expect(MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, fixture::captured.data(),
                              static_cast<int>(fixture::captured.size()), nullptr, 0) > 0,
           "all retained diagnostic text remains valid UTF-8 after truncation");
    CloseHandle(done); CloseHandle(fixture::blocked); CloseHandle(fixture::release); CloseHandle(fixture::idle_health);
    std::printf("%s: nonblocking producers, bounded queue, periodic health, Unicode/truncation, short-write/failure recovery; %d failures\n", failures ? "FAIL" : "PASS", failures);
    return failures ? 1 : 0;
}
