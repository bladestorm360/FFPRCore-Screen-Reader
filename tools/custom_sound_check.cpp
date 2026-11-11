// check-sources: audio/cue_bank.cpp audio/soundscape.cpp audio/hrtf.cpp audio/legacy_cues.cpp audio/tone_generator.cpp audio/custom_cues.cpp
// Actual loader, settings capture, scheduler and PCM render; no game/device use.
#define main retained_soundscape_main
#include "soundscape_check.cpp"
#undef main
#include <map>
namespace fixture {std::map<std::string,int> custom_values;}
namespace settings {
bool get_bool(const char* key,bool fallback){const auto i=fixture::custom_values.find(key);return i==fixture::custom_values.end()?fallback:i->second!=0;}
int get_int(const char* key,int fallback){const auto i=fixture::custom_values.find(key);return i==fixture::custom_values.end()?fallback:i->second;}
}
int main() {
 using namespace sound;int failures=retained_soundscape_main(0,nullptr);
 const auto expect=[&](bool pass,const char* message){if(!pass){++failures;std::printf("FAIL %s\n",message);}};
 auto defaults=cues::load(std::filesystem::current_path().wstring());
 runtime::nav_enabled=true;runtime::wall_mask=0;runtime::counter_enabled=false;
 const auto npc=size_t(cues::Id::NPC),wall=size_t(cues::Id::WallNorth),counter=size_t(cues::Id::Counter);
 auto bank=defaults;
 for(const auto i:{npc,wall,counter}) {
  bank[i].original=defaults[i].mono;bank[i].mono.assign(88,4000);bank[i].custom=true;
 }
 runtime::Player custom_player;custom_player.bank=bank;custom_player.prepared=true;custom_player.options=fixture::options();
 custom_player.options.custom.cues[npc].gain=.5f;
 custom_player.start(16,cues::Id::NPC,1,0,1,1,false,42);
 expect(custom_player.voices[16].reference.mono.empty()&&custom_player.voices[16].gain==.5f,"selected custom PCM has its own gain");
 auto off=fixture::options();off.custom.enabled=false;
 custom_player.configure(off);custom_player.start(16,cues::Id::NPC,1,0,1,1,false,42);
 runtime::Player original;original.bank=defaults;original.prepared=true;original.options=fixture::options();
 original.start(16,cues::Id::NPC,1,0,1,1,false,42);
 std::vector<int16_t> actual(882),wanted(882);
 custom_player.render(actual.data(),441);original.render(wanted.data(),441);
 expect(actual==wanted,"global off renders the retained generated entity default sample exactly");
 auto one_off=fixture::options();one_off.custom.cues[npc].enabled=false;one_off.custom.cues[npc].gain=0;
 custom_player.configure(one_off);custom_player.start(16,cues::Id::NPC,1,0,1,1,false,42);
 expect(custom_player.voices[16].reference.mono==defaults[npc].mono&&custom_player.voices[16].gain==1,"individual off restores default rather than muting it with custom gain");
 const auto dll=std::filesystem::absolute("third_party/openal-soft/soft_oal.dll").wstring();
 expect(custom_player.hrtf.open(dll,bank),"custom/default switching uses actual HRTF loopback");
 custom_player.start(16,cues::Id::NPC,1,1,1,1,false,42);
 expect(custom_player.voices[16].reference.mono==defaults[npc].mono,"HRTF fallback owns retained default PCM");
 custom_player.render(actual.data(),441);
 expect(fixture::energy(actual,0)>0&&fixture::energy(actual,1)>0,"retained default is audible through HRTF");
 auto hrtf_custom=one_off;hrtf_custom.custom.cues[npc].enabled=true;hrtf_custom.custom.cues[npc].gain=.25f;
 custom_player.configure(hrtf_custom);custom_player.start(16,cues::Id::NPC,1,1,1,1,false,42);
 expect(custom_player.voices[16].reference.mono.empty()&&custom_player.voices[16].gain==.25f,"HRTF custom selection keeps private gain");
 custom_player.stop_all();custom_player.hrtf.open(L"",bank);
 runtime::Player walls;walls.bank=bank;walls.prepared=true;walls.options=fixture::options();
 walls.options.custom.cues[wall].loop=false;walls.options.custom.cues[wall].repeat_ms=200;
 runtime::wall_mask=1;walls.maintain(10);
 expect(walls.voices[2].on&&!walls.voices[2].loop,"custom wall can ping without looping");
 walls.render(actual.data(),441);walls.maintain(10.19);
 expect(!walls.voices[2].on,"custom wall does not repeat before200ms");
 walls.maintain(10.201);expect(walls.voices[2].on,"custom wall repeats at its configured interval");
 auto looped=walls.options;looped.custom.cues[wall].loop=true;walls.configure(looped);walls.maintain(11);
 expect(walls.voices[2].on&&walls.voices[2].loop,"wall loop restores continuous file playback");
 auto default_wall=walls.options;default_wall.custom.cues[wall].enabled=false;default_wall.custom.cues[wall].gain=0;default_wall.custom.cues[wall].loop=false;
 walls.configure(default_wall);walls.maintain(11.01);
 expect(walls.voices[2].on&&walls.voices[2].loop&&!walls.voices[2].reference.mono.empty(),"disabled wall custom restores generated loop independently of private gain and period");
 runtime::wall_mask=0;walls.maintain(11.1);expect(!walls.voices[2].on,"leaving wall stops custom loop");
 walls.options.custom.cues[counter].repeat_ms=500;runtime::counter_enabled=true;walls.maintain(12);
 expect(walls.voices[8].on&&!walls.voices[8].loop,"custom EXP counter has optional timed ticks");
 walls.render(actual.data(),441);walls.maintain(12.49);expect(!walls.voices[8].on,"custom counter respects500ms interval");
 walls.maintain(12.501);expect(walls.voices[8].on,"custom counter resumes at configured interval");
 runtime::counter_enabled=false;walls.maintain(12.6);expect(!walls.voices[8].on,"native EXP completion stops timed custom counter");
 runtime::Player entities;entities.bank=bank;entities.bank[npc].mono.assign(cues::rate,4000);entities.prepared=true;entities.options=fixture::options();
 entities.options.custom.cues[npc].repeat_ms=200;
 const double at=runtime::now();auto snapshot=std::make_unique<scape::Snapshot>();snapshot->epoch=runtime::epoch();snapshot->captured_at=at;
 snapshot->rows.push_back({42,scape::Kind::NPC,1,1});entities.accept(std::move(snapshot),at);entities.maintain(at+.01);
 entities.maintain(at+.22);size_t voices=0;for(const auto& voice:entities.voices)if(voice.on&&voice.id==42)++voices;
 expect(voices==1&&entities.deferred>0,"short custom interval never overlaps one long emitter clip");
 for(int n=0;n<51;++n)entities.render(actual.data(),441);
 entities.maintain(at+1.1);voices=0;for(const auto& voice:entities.voices)if(voice.on&&voice.id==42)++voices;
 expect(voices==1,"due emitter repeats after long clip completes");
 const auto& keys=custom::keys(cues::Id::NPC);runtime::available_custom=uint32_t(1)<<npc;
 fixture::custom_values[keys.repeat]=1;expect(custom::override_ms(cues::Id::NPC)==200,"native timing has200ms minimum");
 fixture::custom_values[keys.repeat]=9000;expect(custom::override_ms(cues::Id::NPC)==5000,"native timing has5s maximum");
 fixture::custom_values[keys.repeat]=999;expect(custom::capture().cues[npc].repeat_ms==900,"native and menu timing use100ms increments");
 fixture::custom_values[keys.enabled]=0;expect(custom::override_ms(cues::Id::NPC)==0,"disabled custom sound never changes generated cadence");
 fixture::custom_values[keys.enabled]=1;runtime::available_custom=0;expect(custom::override_ms(cues::Id::NPC)==0,"missing custom file keeps original timing");
 fixture::custom_values[keys.volume]=25;fixture::custom_values[std::string("HRTF.")+keys.volume]=75;
 fixture::custom_values["HRTF"]=0;expect(custom::capture().cues[npc].gain==.5f,"custom stereo volume keeps its own saved bank");
 fixture::custom_values["HRTF"]=1;expect(custom::capture().cues[npc].gain==1.5f,"custom HRTF volume keeps its own saved bank");
 fixture::custom_values["CustomSounds"]=0;expect(!custom::capture().enabled,"global custom toggle is captured without file reads");
 fixture::custom_values["CustomSounds"]=1;
 const auto began=std::chrono::steady_clock::now();float checksum=0;
 for(int n=0;n<10000;++n)checksum+=custom::capture().cues[npc].gain;
 const double us=std::chrono::duration<double,std::micro>(std::chrono::steady_clock::now()-began).count()/10000;
 expect(checksum>0,"bounded native custom settings capture remains observable");
 std::printf("Custom capture18: %.3f us mean,10000 calls, fixture settings under resource cap; game latency not measured\n",us);
 runtime::wall_mask=0;runtime::counter_enabled=false;runtime::available_custom=0;
 std::printf("Custom sound controls/runtime: %d failures\n",failures);return failures?1:0;
}
