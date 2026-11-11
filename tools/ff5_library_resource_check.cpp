// check-sources: field/baked_map.cpp field/baked_router.cpp field/entity_translator.cpp field/npc_names.cpp json.cpp text.cpp ../games/ff5/library_model.cpp ../games/ff5/catapult_model.cpp ../games/ff5/stone_model.cpp ../games/ff5/book_model.cpp
// Shipped owned policy/catalogs and the real worker-safe entity translator.
#include "../src/games/ff5/library_model.h"
#include "../src/games/ff5/catapult_model.h"
#include "../src/games/ff5/stone_model.h"
#include "../src/games/ff5/book_model.h"
#include "../src/core/field/entity_translator.h"
#include "../src/core/modtext.h"
#include <cstdio>
#include <fstream>
#include <thread>
namespace fixture {
json::Value puzzles,translations,catalog,common;
thread_local std::wstring locale=L"en";
int failures=0,profiles=0,catapult_profiles=0,stone_profiles=0,book_profiles=0;
std::string read(const char* p){std::ifstream f(p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
void check(bool value,const char* why){if(!value){++failures;if(failures<25)std::printf("FAIL Library resource %s\n",why);}}
std::wstring lookup(std::wstring_view key){
 for(const auto* file:{&catalog,&common})if(const auto* row=file->get(std::wstring(key)))if(const auto* value=row->get(locale))return value->s;
 ++failures;return L"[missing translation]";
}
ff5::library::Snapshot observation(const json::Value& row){
 ff5::library::Snapshot s;s.locale=locale;s.player={1,0,0};int map=0,bits=0,hidden=0;
 std::swscanf(row.get(L"key")->s.c_str(),L"%d:%d:%d",&map,&bits,&hidden);s.map=map;
 for(size_t i=0;i<s.values.size();++i)s.values[i]=(bits>>i)&1;
 for(size_t i=0;i<ff5::library::controls(map).size();++i)s.hidden[i]=bool((hidden>>i)&1);
 return s;
}
}
namespace data {bool game_data_ready(){return true;}const json::Value& game_json(const char*){return fixture::translations;}}
namespace log_ {void write(const char*,...){} }
namespace field {
std::wstring current_map_name(){std::abort();}
std::wstring safe_event_label(const std::wstring& raw,const std::wstring&,int,int,int){return raw;}
}
namespace modtext {
LocaleScope::LocaleScope(std::wstring s){previous_=fixture::locale;fixture::locale=std::move(s);}
LocaleScope::~LocaleScope(){fixture::locale=*previous_;}
const std::wstring& lang(){return fixture::locale;}
std::wstring T(std::wstring_view key){return fixture::lookup(key);}
std::wstring TF(std::wstring_view key,std::initializer_list<std::wstring> args){auto out=T(key);int i=0;for(const auto& arg:args){const auto token=L"{"+std::to_wstring(i++)+L"}";size_t at=0;while((at=out.find(token,at))!=out.npos){out.replace(at,token.size(),arg);at+=arg.size();}}return out;}
}
int main(){
 using namespace fixture;using namespace ff5::library;
 const auto bytes=read("data/ff5/maps.bin");
 if(!field::baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff5/puzzles.json"),puzzles)||
    !json::parse(read("data/ff5/translation.json"),translations)||!json::parse(read("data/ff5/mod_text.json"),catalog)||
    !json::parse(read("data/common/mod_text.json"),common))return 2;
 field::prepare_entity_translations();
 std::thread worker([]{
  const auto& rows=puzzles.get(L"library_policy")->get(L"rows")->arr;
  check(rows.size()==188,"exact qualified observation count");
  for(const auto* language:{L"en",L"ja",L"fr",L"it",L"de",L"es",L"ko",L"zht",L"zhc",L"ru",L"th",L"pt"}){
   locale=language;
   for(int state=0;state<8;++state){ff5::book::Snapshot s;s.map=160;s.locale=locale;s.values.fill(0);s.hidden.fill(false);s.gate_hidden=false;
    if(state<3)s.values[size_t(3+state)]=1;
    if(state==4)s.values[0]=s.values[1]=s.values[2]=1;
    if(state==5)s.hidden[0]=true;if(state==6)s.map=159;if(state==7)s.values[0]=1;
    const auto advice=ff5::book::plan(s,puzzles);++book_profiles;check(advice.supported&&!advice.unknown&&!advice.targets.empty(),"Surgate every decision branch has localized advice");
    for(const auto& target:advice.targets)check(!target.instruction.empty()&&target.instruction!=L"[missing translation]"&&target.goal.avoid_other_events,"Surgate actual shelf/reset/return/librarian instruction is localized");
   }
   for(int state=0;state<9;++state){ff5::stone::Snapshot s;s.map=state<7?371:372;s.locale=locale;s.values.fill(0);s.stairs_hidden=state==8;
    if(state==1||state==4){s.values[2]=s.values[5]=1;}
    if(state==2||state==3){s.values[0]=s.values[2]=1;}
    if(state==3||state==4||state==5||state==6){s.values[6]=1;}
    if(state==5){s.values[2]=s.values[3]=1;}if(state==6)s.map=370;
    const auto advice=ff5::stone::plan(s,puzzles);++stone_profiles;check(advice.supported&&!advice.unknown&&!advice.targets.empty(),"Ghido every decision branch has localized advice");
    for(const auto& t:advice.targets)check(!t.instruction.empty()&&t.instruction!=L"[missing translation]"&&t.goal.avoid_other_events,"Ghido bounded safe target has a complete catalog instruction");
   }
   for(size_t stage=0;stage<5;++stage){ff5::catapult::Snapshot s;s.map=293;s.locale=locale;s.values.fill(0);s.hidden.fill(false);
    for(size_t i=0;i<stage;++i)s.values[i]=1;const auto result=ff5::catapult::plan(s,puzzles);++catapult_profiles;
    check(result.supported&&!result.unknown&&!result.finished&&!result.targets.empty(),"Catapult every stage produces localized advice");
    if(!result.targets.empty()){
     const auto& target=result.targets.front();check(target.goal.entity_id==ff5::catapult::controls(293)[stage].id&&target.goal.avoid_other_events,"Catapult native stage selects exact safe control");
     const auto& key=puzzles.get(L"catapult_guide")->get(L"steps")->arr[stage].get(L"text")->s;
     check(target.instruction==catalog.get(key)->get(language)->s&&!target.instruction.empty(),"Catapult instruction resolves the actual game-language catalog");
     const auto& entry=result.targets.back();const auto& entry_key=puzzles.get(L"catapult_guide")->get(L"entry_step")->get(L"text")->s;
     check(entry.goal.entity_id==17&&entry.kind==field::TargetKind::Spot&&entry.instruction==catalog.get(entry_key)->get(language)->s,"Catapult native entry/No instruction is localized in every language");
    }
   }
   const auto mid=field::translate_entity_name_snapshot(L"ミド");
   check(mid==translations.get(L"ミド")->get(language)->s,"central Mid name uses this game's actual language table");
   for(const auto& row:rows){
    auto s=observation(row);const auto advice=plan(s,puzzles);++profiles;
    check(advice.supported&&!advice.finished&&!advice.unknown&&!advice.targets.empty(),"every original policy observation produces a bounded plan");
    for(const auto& target:advice.targets){
     check(target.goal.avoid_other_events&&!target.instruction.empty()&&target.instruction.find(L'{')==std::wstring::npos,"safe target has complete localized instruction");
     if(s.map==283&&target.goal.entity_id==1)check(target.instruction==modtext::TF(L"Talk to {0}.",{mid}),"Mid continuation is localized through actual translator");
    }
   }
  }
  locale=L"en";auto s=observation(rows.front());
  const auto unknown=[&](const Snapshot& input,const json::Value& resource,const char* why){const auto result=plan(input,resource);check(result.supported&&result.unknown&&!result.finished&&result.targets.empty()&&result.note==modtext::T(L"Navigation data is loading"),why);};
  for(size_t i=0;i<s.values.size();++i){auto missing=s;missing.values[i].reset();unknown(missing,puzzles,"unknown scenario receipt remains loading");}
  for(size_t i=0;i<controls(s.map).size();++i){auto missing=s;missing.hidden[i].reset();unknown(missing,puzzles,"unknown layout control remains loading");}
  unknown(s,json::Value{},"absent resource remains loading");
  auto broken=puzzles;broken.obj[L"library_policy"].obj[L"version"].n=2;unknown(s,broken,"unknown policy version remains loading");
  broken=puzzles;auto& duplicated=broken.obj[L"library_policy"].obj[L"rows"].arr;duplicated.push_back(duplicated.front());unknown(s,broken,"duplicate observation is rejected");
  broken=puzzles;broken.obj[L"library_policy"].obj[L"rows"].arr.front().obj[L"targets"].arr.front().arr[0].n=4097;unknown(s,broken,"out-of-bound actor rejected");
  broken=puzzles;broken.obj[L"library_policy"].obj[L"rows"].arr.front().obj[L"targets"].arr.front().arr[2].n=.5;unknown(s,broken,"fractional policy cell rejected");
  broken=puzzles;auto& over=broken.obj[L"library_policy"].obj[L"rows"].arr.front().obj[L"targets"].arr;over.resize(33,over.front());unknown(s,broken,"over-budget target list rejected");
  s.cancelled=[]{return true;};auto cancelled=plan(s,puzzles);check(cancelled.targets.empty()&&!cancelled.finished,"cancelled model returns no stale advice");
 Snapshot end;end.locale=L"en";end.map=272;end.values[2]=1;check(plan(end,puzzles).finished,"final receipt works without rereading retired controls");
  end.map=283;end.values.fill(0);end.values[0]=end.values[1]=end.values[3]=end.values[6]=1;
  const auto mid=plan(end,puzzles);check(!mid.finished&&!mid.targets.empty()&&mid.targets.front().goal.entity_id==1,"Byblos alone leaves the actual Mid operation");
 });worker.join();
 std::printf("FFV resources: Library%d/Catapult%d/Ghido%d/Surgate%d plans/12 languages; malformed, unknown, cancellation, translated Mid and final receipt; failures%d\n",profiles,catapult_profiles,stone_profiles,book_profiles,failures);
 return failures?1:0;
}
