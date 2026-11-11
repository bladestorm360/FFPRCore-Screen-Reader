// check-sources:
// Actual retained UTF-8 decoder versus prepared immutable FFIII strings.
// The reference strategy is selected only by this offline translation unit.
#include "../src/core/field/baked_map.cpp"
#include <chrono>
#include <cstdio>
#include <fstream>
int main() {
    using namespace field::baked;
    int failures=0;size_t reads=0,bytes=0;
    const auto expect=[&](bool ok,const char* why){if(!ok){++failures;std::printf("FAIL %s\n",why);}};
    for(int n=1;n<=6;++n) {
        const auto path="data/ff"+std::to_string(n)+"/maps.bin";
        std::ifstream f(path,std::ios::binary);const std::string blob(std::istreambuf_iterator<char>(f),{});
        const auto began=std::chrono::steady_clock::now();expect(load(blob.data(),blob.size()),"real bake loads");
        const auto loaded=std::chrono::steady_clock::now();auto* prepared=g_strings;
        expect((prepared!=nullptr)==(n==3),"only FFIII enables the scoped decoded identity table");
        for(uint32_t i=0;i<g_header->strings_count;++i) {
            const auto ready=str(i);g_strings=nullptr;const auto reference=str(i);g_strings=prepared;
            expect(ready==reference,"every string preserves exact original UTF-8 decoding");
            if(prepared)bytes+=ready.size()*sizeof(wchar_t)+sizeof(std::wstring);
        }
        expect(str(UINT32_MAX).empty(),"invalid string index remains empty");
        if(prepared) {
            std::vector<uint32_t> names;const auto* world=submap_by_map_id(1);
            for(const auto& object:grids(*world).objects)names.push_back(object.name);
            volatile size_t checksum=0;
            const auto run=[&] {for(int pass=0;pass<10;++pass)for(const auto id:names){const auto text=str(id);checksum=checksum+text.size();}};
            g_strings=nullptr;const auto before=std::chrono::steady_clock::now();run();const auto reference_end=std::chrono::steady_clock::now();
            g_strings=prepared;run();const auto ready_end=std::chrono::steady_clock::now();reads=names.size()*10;
            std::printf("FFIII %zu actual world identity reads: original%.3fms prepared%.3fms; full load%.3fms owned table%zu bytes checksum%zu\n",reads,
                std::chrono::duration<double,std::milli>(reference_end-before).count(),std::chrono::duration<double,std::milli>(ready_end-reference_end).count(),
                std::chrono::duration<double,std::milli>(loaded-began).count(),bytes,size_t(checksum));
        }
    }
    std::printf("Baked strings: all6 exact parity, %d failures\n",failures);return failures?1:0;
}
