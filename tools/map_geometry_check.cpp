// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp json.cpp text.cpp
// Actual production interaction goals/floods across every mapped interior bake.
// This is a survey: suspects require native/script review, not automatic walls.
#include "field/baked_router.h"
#include "field/map_state.h"
#include <windows.h>
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <filesystem>
#include <iterator>
#include <string>
#include <set>
#include <tuple>
#include <climits>
namespace b=field::baked;namespace r=field::baked_route;
std::string utf8(const std::wstring& value){if(value.empty())return {};int n=WideCharToMultiByte(CP_UTF8,0,value.data(),int(value.size()),nullptr,0,nullptr,nullptr);std::string out(n,'\0');WideCharToMultiByte(CP_UTF8,0,value.data(),int(value.size()),out.data(),n,nullptr,nullptr);return out;}
bool native_cardinal(const r::Cell& stand,const b::Object& npc,const std::vector<b::Object>& relays){
 for(int y=npc.ty;y<npc.ty+npc.th;++y)for(int x=npc.tx;x<npc.tx+npc.tw;++x){
  if(!(npc.d&2)&&stand.layer!=npc.layer)continue;
  const int dx=x-stand.x,dy=y-stand.y;if(dx==0&&dy==0)return true;if((dx==0)==(dy==0))continue;
  const int distance=std::abs(dx)+std::abs(dy);if(distance==1)return true;
  const int probe_x=stand.x+(dx>0)-(dx<0),probe_y=stand.y+(dy>0)-(dy<0);int range=1;
  for(const auto& relay:relays)if(relay.layer==stand.layer&&relay.contains(probe_x,probe_y))range+=dx?relay.tw:relay.th;
  if(distance<=range)return true;
 }
 return false;
}
// A baseline combines mutually exclusive event groups. Recheck its suspects
// using the production Resolver before mistaking inactive scenes for walls.
// These copied sample states establish witnesses, not exhaustive story validity.
struct Profile {field::map_state::Flags flags;};
std::vector<Profile> profiles(const b::Submap& sm,const json::Value& record){
 std::vector<Profile> out(2);
 for(int p=0;p<2;++p){auto& f=out[p].flags;for(auto& v:f.scenario)v=p;for(auto& v:f.treasure)v=p;for(auto& v:f.global)v=p;
  f.current_map_id=sm.first_map_id;const auto* area=record.get(L"area_id");f.current_area_id=area?int(area->n):0;}
 const auto* code=record.get(L"code");if(!code)return out;
 std::set<std::tuple<int,int,int>> tests;
 for(const auto& v:code->arr){const auto& a=v.arr;
  if(a.size()!=7||a[0].n!=2||a[4].n!=-1)continue;
  const int category=int(a[1].n),id=int(a[2].n),value=int(a[5].n);
  if(id<0||category<0||category>8)continue;
  for(int operand:{value,value==INT_MAX?value:value+1,value==INT_MIN?value:value-1})tests.emplace(category,id,operand);
 }
 for(const auto& [category,id,value]:tests){
  if(category<8&&value!=0&&value!=1)continue; // native persistent flag banks are bits
  if(out.size()>=64)break;auto f=out.front().flags;
  if(category<4&&id<256)f.scenario[category*256+id]=value;
  else if(category<8&&id<256)f.treasure[(category-4)*256+id]=value;
  else if(category==8&&id<16)f.global[id]=value;else continue;
  out.push_back({f});
 }
 return out;
}
int main(int argc,char** argv){
 const bool state_triage=argc>1&&std::string(argv[1])=="--state-triage";
 std::filesystem::create_directories("build_check/map_event_audit_20261028");
 std::ofstream out("build_check/map_event_audit_20261028/geometry_candidates.tsv");
 out<<"game\tsubmap\tid\tgroup\traw\tcell\tgoal_count\treachable_goals\tnative_cardinal_goals\tnative_reachable_goals\treason\n";
 int total_maps=0,total_npcs=0,total_counter=0,total_unreachable=0,total_unsafe=0,missing=0,wide_cases=0;
 std::ofstream relay_out("build_check/map_event_audit_20261028/missing_counter_approaches.tsv");
 relay_out<<"game\tsubmap\tnpc\tx\ty\tlayer\n";
 std::ofstream state_out;
 if(state_triage){state_out.open("build_check/map_event_audit_20261028/state_geometry.tsv");state_out<<"game\tsubmap\tid\tgroup\traw\tprofiles\tactive_profiles\treachable_profiles\tclassification\n";}
 const auto start=std::chrono::steady_clock::now();
 std::array<std::vector<char>,6> backing;
 for(int n=1;n<=6;++n){
  std::ifstream file("data/ff"+std::to_string(n)+"/maps.bin",std::ios::binary);auto& blob=backing[n-1];blob.assign(std::istreambuf_iterator<char>(file),{});
  if(!b::load(blob.data(),blob.size()))return 2;
  json::Value records;
  if(state_triage){std::ifstream state_file("data/ff"+std::to_string(n)+"/mapstate.json",std::ios::binary);std::string text{std::istreambuf_iterator<char>(state_file),{}};if(!json::parse(text,records))return 3;}
  int rooms=0,npcs=0,counter=0,unreachable=0,unsafe=0,no_seeds=0,multi_relays=0;
  for(int i=0;i<b::submap_count();++i){
   const auto& sm=*b::submap_at(i);if(sm.world||sm.first_map_id<=0||!sm.layer_mask)continue;++rooms;
   const auto& g=b::grids(sm);r::Request req;req.submap=&sm;req.passages=true;req.ignore_hazards=true;req.live_relays=true;
   std::vector<r::Cell> seeds;
   for(const auto& o:g.objects){if(o.type==b::ObjType::RelayInteraction){req.relays.push_back(o);if(o.tw>1||o.th>1)++multi_relays;}
    if(o.type==b::ObjType::PointIn)seeds.push_back({o.layer,o.tx,o.ty});}
   if(seeds.empty()){++no_seeds;continue;}
   const auto reach=r::flood(req,seeds);
   for(const auto& npc:g.objects){
    if((npc.type!=b::ObjType::NPC&&npc.type!=b::ObjType::ShopNPC)||!(npc.d&8))continue;++npcs;
    const auto goals=r::goals_for(req,{npc.layer,npc.tx,npc.ty},r::GoalKind::Interact,&npc);
    // Native SearchInteractiveArea probes the tile in front of the player.
    // Every overlapping relay adds its width/height along that facing axis.
    // Use real active-compatible groups rather than inventing linked IDs.
    auto compatible=req;compatible.relays.erase(std::remove_if(compatible.relays.begin(),compatible.relays.end(),[&](const auto& o){
      const auto group=b::str(o.group);return !group.empty()&&group!=L"entity_default"&&group!=L"default"&&o.group!=npc.group;
    }),compatible.relays.end());
    const auto actual=r::goals_for(compatible,{npc.layer,npc.tx,npc.ty},r::GoalKind::Interact,&npc);
    int bound=1;for(const auto& relay:compatible.relays)bound+=std::max(relay.tw,relay.th);bound=std::min(bound,std::max(g.width,g.height));
    static constexpr int sides[4][2]={{0,-1},{1,0},{0,1},{-1,0}};
    for(int side=0;side<4;++side)for(int distance=2;distance<=bound;++distance)for(int layer=0;layer<3;++layer){
      if(!(npc.d&2)&&layer!=npc.layer)continue;if(npc.b>0&&!(npc.b&(1<<side)))continue;
      r::Cell stand{layer,npc.tx+sides[side][0]*distance,npc.ty+sides[side][1]*distance};
      if(!r::open(compatible,g,stand)||!native_cardinal(stand,npc,compatible.relays)||r::terminal_transition(compatible,g,stand))continue;
      bool wide=false;for(const auto& relay:compatible.relays)if((relay.tw>1||relay.th>1)&&relay.layer==layer&&relay.contains(stand.x-sides[side][0],stand.y-sides[side][1]))wide=true;
      if(wide)++wide_cases;
      if(std::find(actual.cells.begin(),actual.cells.end(),stand)==actual.cells.end()){
       ++missing;relay_out<<n<<'\t'<<utf8(b::str(sm.name))<<'\t'<<npc.c<<'\t'<<stand.x<<'\t'<<stand.y<<'\t'<<layer<<'\n';
      }
    }
    int reached=0,valid=0,valid_reached=0;bool counter_ray=false;
    for(const auto& stand:goals.cells){bool okay=native_cardinal(stand,npc,req.relays);valid+=okay;reached+=reach.reachable(stand);valid_reached+=okay&&reach.reachable(stand);
     counter_ray|=okay&&(std::abs(stand.x-npc.tx)+std::abs(stand.y-npc.ty)>1);}
    if(counter_ray)++counter;
    if(!valid_reached)++unreachable;
    if(reached&&!valid_reached)++unsafe;
    if(!valid_reached||int(goals.cells.size())!=valid){
     out<<n<<'\t'<<utf8(b::str(sm.name))<<'\t'<<npc.c<<'\t'<<utf8(b::str(npc.group))<<'\t'<<utf8(b::str(npc.name))<<'\t'<<int(npc.layer)<<','<<npc.tx<<','<<npc.ty<<'\t'<<goals.cells.size()<<'\t'<<reached<<'\t'<<valid<<'\t'<<valid_reached<<'\t'<<(reached&&!valid_reached?"noncardinal_or_multicell_only":!valid_reached?"no_baseline_approach":"extra_nonnative_goals")<<'\n';
     if(state_triage){
      const auto* record=records.get(b::name_of(sm));const auto states=profiles(sm,record?*record:json::Value{});
      int active=0,arrivals=0;
      for(const auto& p:states){
       field::map_state::Resolver resolver(p.flags,records);if(!resolver.active(sm,npc))continue;++active;
       const auto target=resolver.effective(sm,npc);r::Request native=req;native.blocked.clear();native.hazards.clear();native.traps.clear();native.directional.clear();
       field::map_state::add_obstacles(resolver,sm,native,true);
       const auto destination=r::goals_for(native,{target.layer,target.tx,target.ty},r::GoalKind::Interact,&target);
       const auto traversed=r::flood(native,seeds);
       bool arrival=false;for(const auto& goal:destination.cells)if(native_cardinal(goal,target,native.relays)&&traversed.reachable(goal))arrival=true;
       arrivals+=arrival;
      }
      const bool outside=!b::in_bounds(g,npc.tx,npc.ty);
      state_out<<n<<'\t'<<utf8(b::str(sm.name))<<'\t'<<npc.c<<'\t'<<utf8(b::str(npc.group))<<'\t'<<utf8(b::str(npc.name))<<'\t'<<states.size()<<'\t'<<active<<'\t'<<arrivals<<'\t'<<(arrivals?"reachable_in_sampled_state":!active?"inactive_in_sampled_states":outside?"staged_off_grid":"needs_native_or_dynamic_context")<<'\n';
     }
    }
   }
  }
  std::printf("FF%d: %d interiors, %d actionable NPCs, %d counter approaches, %d baseline suspects (%d noncardinal/multicell-only), %d rooms without PointIn, %d multi-cell relays\n",n,rooms,npcs,counter,unreachable,unsafe,no_seeds,multi_relays);
  total_maps+=rooms;total_npcs+=npcs;total_counter+=counter;total_unreachable+=unreachable;total_unsafe+=unsafe;
 }
 std::printf("Survey: %d interiors, %d NPCs, %d counter cases, %d baseline suspects (%d unsafe-only); %.3fms offline. No native state/gates/patrol simulation; no game run.\n",total_maps,total_npcs,total_counter,total_unreachable,total_unsafe,std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count());
 std::printf("Native rectangle contract: %d real wide-counter cases, %d missing native-legal approaches\n",wide_cases,missing);
 return argc>1&&missing?1:0;
}
