// check-sources: text.cpp audio/custom_cues.cpp
// Actual native settings storage/capture and snapshot writer; fake disk only.
#define main retained_settings_worker_main
#include "settings_worker_check.cpp"
#undef main
#include "../src/core/audio/custom_cues.h"
#include <chrono>
namespace sound::runtime {uint32_t custom_available_mask(){return (uint32_t(1)<<sound::cues::count)-1;}}
int main() {
 int failures=retained_settings_worker_main();
 const auto expect=[&](bool ok,const char* why){if(!ok){++failures;std::printf("FAIL %s\n",why);}};
 for(size_t n=0;n<sound::cues::count;++n) {
  const auto& k=sound::custom::keys(sound::cues::Id(n));
  settings::set_bool(k.enabled,true);settings::set_int(k.volume,25);
  settings::set_int(k.repeat,700);settings::set_bool(k.loop,false);
 }
 settings::set_bool("CustomSounds",true);
 const auto captured=sound::custom::capture();
 expect(captured.enabled&&captured.cues[size_t(sound::cues::Id::NPC)].repeat_ms==700,"production settings decode custom capture");
 double total_us=0,peak_us=0;float sum=0;
 for(int n=0;n<10000;++n) {
  const auto began=std::chrono::steady_clock::now();sum+=sound::custom::capture().cues[10].gain;
  const auto us=std::chrono::duration<double,std::micro>(std::chrono::steady_clock::now()-began).count();total_us+=us;peak_us=std::max(peak_us,us);
 }
 expect(sum==5000,"native settings capture preserves private gain");
 fixture::blocked=CreateEventW(nullptr,TRUE,FALSE,nullptr);fixture::release=CreateEventW(nullptr,TRUE,TRUE,nullptr);
 settings::save_pending();std::thread writer([]{fixture::queued();});writer.join();
 std::map<std::string,std::string> parsed;settings::parse(fixture::contents,"",parsed);
 expect(parsed["CustomSound.npc.Interval"]=="700"&&parsed["CustomSound.npc.Volume"]=="25"&&parsed["CustomSound.wall_north.Loop"]=="false","actual writer serializes independent custom values");
 CloseHandle(fixture::blocked);CloseHandle(fixture::release);
 std::printf("Actual settings custom capture18: mean %.3f us, peak %.3f us,10000 calls; capped fixture, no game frame/audible claim\n",total_us/10000,peak_us);
 std::printf("Custom settings persistence/capture: %d failures\n",failures);return failures?1:0;
}
