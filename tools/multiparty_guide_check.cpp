// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Production story candidates and complete route consumer, real FFVI geometry
// and native-state metadata. This fixture never executes game implementation.
#define main retained_multiparty_route_main
#include "route_worker_check.cpp"
#undef main
#include "../src/games/ff6/story_model.h"
namespace field {std::wstring safe_event_label(const std::wstring& s,const std::wstring&,int,int,int){return s;}}
int main() {
    std::setvbuf(stdout,nullptr,_IONBF,0);
    using namespace field;using namespace baked_nav;
    const auto read=[](const char* p){std::ifstream f(p,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
    json::Value puzzles;const auto bytes=read("data/ff6/maps.bin");
    if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff6/mapstate.json"),fixture::records)||
       !json::parse(read("data/ff6/puzzles.json"),puzzles))return 2;
    fixture::records.obj[L"story_guides"]=*puzzles.get(L"story_guides");
    fixture::game_id=game::Id::FF6;g_loaded=g_prepared=true;
    int failures=0;
    const auto expect=[&](bool pass,const char* why){if(!pass){++failures;std::printf("FAIL %s\n",why);}};
    map_state::Flags flags;flags.scenario.fill(0);flags.treasure.fill(0);flags.global.fill(0);flags.local.fill(0);flags.area.fill(0);
    struct Pose {int map=0;Cell at;};
    std::array<Pose,3> parties;
    std::array<std::optional<MapGoal>,3> previous;
    ffpr::ff6::story::Plan last_guide;
    const auto capture=[&](RouteSnapshot& snapshot,map_state::Resolver observed) {
        // Current-floor observations are manufactured from the exact compiled
        // initializer/geometry facts. They are conservative static observations,
        // not evidence of native dynamic trigger power or displaced party poses.
        const auto& room=*snapshot.submap;
        for(const auto& original:baked::grids(room).objects) {
            if(original.type==baked::ObjType::Warp||original.type==baked::ObjType::PointIn||
               original.type==baked::ObjType::TransitionHelper||!observed.active(room,original,true))continue;
            const auto object=observed.effective(room,original);
            NativeObstacle actor;actor.id=object.entity_id();actor.type=static_cast<int>(object.type);
            actor.raw_name=baked::object_identifier(object);actor.cell={object.layer,object.tx,object.ty};
            actor.npc=object.type==baked::ObjType::NPC||object.type==baked::ObjType::ShopNPC;
            actor.chest=object.type==baked::ObjType::TreasureBox;
            actor.trigger=baked_route::step_event(object)&&!observed.consumed(room,original);
            actor.collider=object.type==baked::ObjType::CollisionEntity;actor.collision=object.a;
            actor.transportation_ids=observed.transportation_mask(room,original);
            actor.transportation_restricted=actor.transportation_ids!=~uint64_t(0);
            actor.applies=(actor.transportation_ids&baked_route::transportation_bit(1))!=0;
            actor.enabled=true;actor.solidity_known=true;actor.solid=(object.d&1)!=0;
            snapshot.obstacles.push_back(std::move(actor));
        }
    };
    const auto run=[&](int map,Cell at,int slot) {
        flags.current_map_id=map;
        const auto* room=baked::submap_by_map_id(map);
        if(const auto* record=fixture::records.get(baked::name_of(*room)))
            if(const auto* area=record->get(L"area_id"))flags.current_area_id=static_cast<int>(area->n);
        flags.global[4]=slot;
        ffpr::ff6::story::Snapshot state;state.map=map;state.player=at;state.locale=L"en";state.flags=flags;
        for(size_t i=0;i<parties.size();++i)if(parties[i].map>0) {
            state.parties[i].map=parties[i].map;
            state.parties[i].position=world_of_cell(*baked::submap_by_map_id(parties[i].map),parties[i].at);
        }
        const auto guide=ffpr::ff6::story::plan(state);
        last_guide=guide;
        if(guide.targets.empty()) {previous[slot].reset();std::printf("Guide map%d slot%d note %s\n",map,slot,text::to_utf8(guide.note).c_str());return Planned{};}
        // MSVC14.44/14.50 hang/crash on the large Flags conditional passed
        // directly by value. Choose its owned address before making the copy.
        const map_state::Flags* projection=&flags;
        if(guide.anticipated_release)projection=&*guide.anticipated_release;
        RouteSnapshot snapshot(map_state::Resolver(*projection,fixture::records));snapshot.submap=room;snapshot.map_id=map;
        snapshot.layer=at.layer;snapshot.position=world_of_cell(*room,at);snapshot.locale=L"en";snapshot.vehicle={true,1,1,{}};
        snapshot.candidates=guide.targets;
        if(previous[slot]) {
            auto pending=std::find_if(snapshot.candidates.begin(),snapshot.candidates.end(),[&](const auto& t){
                return t.goal.map_id==previous[slot]->map_id&&
                    (t.goal.entity_id>=0 ? t.goal.entity_id==previous[slot]->entity_id&&t.goal.raw_name==previous[slot]->raw_name : t.goal.cell==previous[slot]->cell);
            });
            if(pending!=snapshot.candidates.end())std::rotate(snapshot.candidates.begin(),pending,std::next(pending));
        }
        if(guide.anticipated_release)snapshot.anticipated_release=std::make_shared<const map_state::Flags>(*guide.anticipated_release);
        // Native observation precedes the release projection. Preserve those
        // observed actors, then let the production consumer reconcile them.
        capture(snapshot,map_state::Resolver(flags,fixture::records));
        for(int i=0;i<3;++i)if(i!=slot&&parties[i].map==map) {
            NativeObstacle parked;parked.party=true;parked.party_slot=i;parked.cell=parties[i].at;
            snapshot.obstacles.push_back(parked);
        }
        const auto result=plan_snapshot(snapshot);
        // Match the real guide controller: preserve the selected pending
        // objective across native floor/plate changes, until its receipt makes
        // it ineligible. Otherwise this fixture retargets a released old plate.
        if(result.path.guidance_index>=0&&result.path.guidance_index<static_cast<int>(snapshot.candidates.size()))
            previous[slot]=snapshot.candidates[static_cast<size_t>(result.path.guidance_index)].goal;
        std::printf("Guide map%d at%d,%d L%d slot%d -> %d:%d route%d next%d steps%d reason %s\n",map,at.x,at.y,at.layer,slot,
            result.goal.map_id,result.goal.entity_id,result.path.success,result.path.next_map_id,result.path.step_count,text::to_utf8(result.path.reason+L" / "+result.path.error).c_str());
        return result;
    };
    std::thread worker([&] {
        // The bridge changes this passage's PointIn on the same destination
        // map. Retaining only its earlier landing strands every later party.
        const auto* passage=baked::submap_by_map_id(565);
        for(int phase:{0,1,2}) {
            auto observed=flags;
            if(phase==2)observed.scenario[491].reset();else observed.scenario[491]=phase;
            map_state::Resolver projected(observed,fixture::records);int allowed=0;
            for(const auto& object:baked::grids(*passage).objects)
                if(object.type==baked::ObjType::Warp&&object.entity_id()==49&&!projected.consumed(*passage,object)) {
                    ++allowed;
                    if(phase!=2)expect(object.a==549&&object.b==(phase==1?6:4),
                        "Tower bridge flag chooses its exact native landing PointIn, even on the same destination map");
                }
            expect(allowed==(phase==2?2:1),"both conditional Tower landing arcs exist and unknown state preserves both possibilities");
        }
        const auto native_cell=[&](int map,int point,int source_x,int source_y,Cell cell) {
            map_state::Resolver current(flags,fixture::records);
            const auto* entry=point_in(*baked::submap_by_map_id(map),point,current);
            expect(entry!=nullptr,"native scene's actual PointIn anchor resolves its crop offset");
            if(entry){cell.x+=entry->tx-source_x;cell.y+=entry->ty-source_y;}return cell;
        };
        for(int cave_case=0;cave_case<2;++cave_case) {
        flags.scenario.fill(0);flags.treasure.fill(0);flags.global.fill(0);flags.area.fill(0);flags.local.fill(0);
        flags.scenario[557]=1; // native placement complete, sc_e_0792_2/sc_e_0966_4
        flags.scenario[965]=1; // Phoenix's native ScenarioFlag4[197]
        flags.global[5]=2;
        const auto* cave=baked::submap_by_map_id(497);
        map_state::Resolver resolver(flags,fixture::records);const auto* arrival=point_in(*cave,2,resolver);
        expect(arrival!=nullptr,"Phoenix native entrance PointIn2 resolves");
        if(arrival) {
            parties[0]={497,{arrival->layer,arrival->tx,arrival->ty}};
            // Native DisposePhoenixCave parks the second corps at12,6/Upper;
            // PointIn2's14,7 anchor supplies this bake's crop offset.
            parties[1]={497,{2,12+arrival->tx-14,6+arrival->ty-7}};
        }
        if(arrival)for(int slot=0;slot<2;++slot) {
            const auto out=run(parties[slot].map,parties[slot].at,slot);
            expect(out.path.success,"each initial Phoenix party receives a reachable guide objective");
        }
        previous={};
        if(cave_case==1) {
            // The actual premature-plate trace above reached these poses and
            // native receipts. Starting a fresh guide here must recover, not
            // tell both parties to hold forever.
            for(int bit:{399,400,403,405,407})flags.scenario[static_cast<size_t>(bit)]=1;
            flags.area[6]=1;
            parties[0]={498,{1,10,32}};parties[1]={498,{1,23,19}};
        }
        const std::map<int,int> area_controls{{108,1},{94,2},{86,3},{77,4},{61,5},{49,6},{44,7}};
        const std::map<std::pair<int,int>,int> cave_controls{{{498,56},399},{{498,40},401},{{498,41},400},
            {{498,98},403},{{499,66},405},{{499,58},406},{{499,55},407},{{499,50},408}};
        const auto leave_cave_plate=[&](const Pose& before,const Pose& after) {
            if(before.map!=498||(before.map==after.map&&before.at==after.at))return;
            for(const auto& object:baked::grids(*baked::submap_by_map_id(498)).objects) {
                if(object.type!=baked::ObjType::SwitchEvent||!(Cell{object.layer,object.tx,object.ty}==before.at))continue;
                if(const auto area=area_controls.find(object.entity_id());area!=area_controls.end())flags.area[static_cast<size_t>(area->second)]=0;
                if(object.entity_id()==40)flags.scenario[401]=0;
                if(object.entity_id()==41)flags.scenario[400]=0;
            }
        };
        int cave_actions=0,cave_hops=0;
        std::array<std::set<std::tuple<int,int,int,int,int>>,2> cave_visited;
        for(int round=0;round<48&&flags.scenario[408]!=1;++round) {
            bool progress=false;
            for(int slot=0;slot<2&&flags.scenario[408]!=1;++slot) {
                auto& party=parties[slot];
                for(int leg=0;leg<32;++leg) {
                    if(!cave_visited[slot].insert({party.map,party.at.layer,party.at.x,party.at.y,cave_actions}).second) {
                        std::printf("Phoenix repeated pose slot%d map%d at%d,%d actions%d\n",slot,party.map,party.at.x,party.at.y,cave_actions);break;
                    }
                    const auto out=run(party.map,party.at,slot);
                    if(last_guide.finished||!out.path.success||out.path.blocking_party>=0||out.path.npc_approach)break;
                    if(out.path.next_map_id>0) {
                        const auto receipt=out.pending.incoming_trap?out.pending.incoming_trap:out.pending.incoming_transition;
                        expect(receipt.has_value(),"each Phoenix transfer retains its exact native arrival");if(!receipt)break;
                        Pose after{receipt->destination,receipt->arrival};leave_cave_plate(party,after);party=after;++cave_hops;progress=true;continue;
                    }
                    if(!out.path.walk_phases.empty()&&!out.path.walk_phases.back().cells.empty()) {
                        Pose after{party.map,out.path.walk_phases.back().cells.back()};leave_cave_plate(party,after);party=after;
                    }
                    if(out.goal.entity_id<0) {
                        ++cave_actions;progress=true;break; // reviewed stop/leave waypoint; no native flag is invented
                    }
                    if(out.goal.map_id==498&&area_controls.contains(out.goal.entity_id))
                        flags.area[static_cast<size_t>(area_controls.at(out.goal.entity_id))]=1;
                    else {
                        const auto control=cave_controls.find({out.goal.map_id,out.goal.entity_id});
                        expect(control!=cave_controls.end(),"each Phoenix objective has its reviewed native effect");if(control==cave_controls.end())break;
                        flags.scenario[static_cast<size_t>(control->second)]=1;
                        if(control->second==406)flags.scenario[407]=0;
                        if(control->second==407)flags.scenario[406]=0;
                        if(control->second==399) {
                            // Drain scene visits499, returns498/PointIn1 and
                            // sets the player to19,10, preserving Ground.
                            party={498,native_cell(498,1,35,58,{1,19,10})};cave_hops+=2;
                        }
                        if(control->second==408) {
                            expect(last_guide.finish_on_exit==262,"the reunion objective arms its final native city arrival");
                            // The reunion continues584 ->267 ->266 ->262.
                            // Its last script sets408/904 after rejoining.
                            flags.scenario[904]=1;flags.scenario[557]=0;flags.global[5]=1;
                            party.map=262;cave_hops+=4;
                        }
                    }
                    if(flags.scenario[400]==1&&flags.scenario[401]==1)flags.scenario[402]=1;
                    ++cave_actions;progress=true;break;
                }
            }
            if(!progress){std::printf("Phoenix episode stalled round%d actions%d hops%d\n",round,cave_actions,cave_hops);break;}
        }
        expect(flags.scenario[408]==1,"two-party production guide reaches the native Phoenix reunion completion");
        std::printf("Phoenix episode case%d actions%d hops%d final%d\n",cave_case,cave_actions,cave_hops,flags.scenario[408].value_or(-1));
        }
        // Tower is an independent entrance fixture; preserve its already
        // qualified initial state rather than carrying cave-only puzzle state.
        flags.scenario.fill(0);flags.treasure.fill(0);flags.global.fill(0);flags.area.fill(0);flags.local.fill(0);
        flags.scenario[557]=1;flags.scenario[965]=1;previous={};
        flags.global[5]=3;
        map_state::Resolver resolver(flags,fixture::records);
        const auto* tower=baked::submap_by_map_id(542);
        for(const auto& warp:baked::grids(*tower).objects)if(warp.type==baked::ObjType::Warp&&
            (warp.entity_id()==58||warp.entity_id()==70)) {
            std::printf("Belt source%d map%d point%d\n",warp.entity_id(),warp.a,warp.b);
            const auto* arrival=point_in(*tower,warp.b,resolver);
            if(arrival)std::printf("Belt%d layer%d landing%d,%d layer%d next-layer%d\n",warp.entity_id(),warp.layer,
                arrival->tx,arrival->ty,arrival->layer,baked_route::layer_after(baked::grids(*tower),arrival->layer,arrival->tx,arrival->ty));
        }
        const auto* anchor=point_in(*tower,8,resolver);
        expect(anchor!=nullptr,"Tower left entrance PointIn8 resolves its crop offset");if(!anchor)return;
        for(int stage=0;stage<1;++stage)for(int slot=0;slot<3;++slot) {
            // Native sc_e_0966_4 ends the three drop animations in these
            // source map cells. Account for this bake's crop origin only.
            const int x[]={9,43,56},y[]={16,21,22};
            flags.scenario[480]=stage>=1?1:0;flags.scenario[475]=stage>=2?1:0;
            std::printf("Tower progression stage%d\n",stage);
            const auto out=run(542,{1,x[slot]+anchor->tx-9,y[slot]+anchor->ty-16},slot);
            if(slot==1)expect(out.path.success,"middle Tower party can advance the first switch sequence");
        }
        const int x[]={9,43,56},y[]={16,21,22};
        for(int slot=0;slot<3;++slot)parties[slot]={542,{1,x[slot]+anchor->tx-9,y[slot]+anchor->ty-16}};
        flags.scenario[480]=flags.scenario[475]=0;previous={};
        const std::map<std::pair<int,int>,int> switches{{{542,88},475},{{560,32},480},{{549,37},478},{{549,38},482},
            {{549,39},486},{{549,45},487},{{565,25},484},{{565,26},485},{{549,61},493},
            {{565,40},488},{{565,41},489},{{565,44},490},{{544,14},474},{{566,47},492},{{566,51},7},
            {{567,25},7},{{568,22},7},{{569,0},7},
            {{542,93},494},{{567,2},495},{{568,2},496},{{568,12},496},{{570,15},498},{{570,16},499},{{570,14},500}};
        const std::map<std::pair<int,int>,int> releases{{{549,37},478},{{549,38},482},
            {{565,40},488},{{565,41},489},{{565,44},490},{{570,15},498},{{570,16},499},{{570,14},500}};
        const auto release_plate=[&](const Pose& before,const Pose& after) {
            if(before.map==after.map&&before.at==after.at)return;
            for(const auto& [identity,flag]:releases)if(identity.first==before.map)
                for(const auto& object:baked::grids(*baked::submap_by_map_id(before.map)).objects)
                    if(object.type==baked::ObjType::SwitchEvent&&object.entity_id()==identity.second&&
                       Cell{object.layer,object.tx,object.ty}==before.at)flags.scenario[flag]=0;
        };
        int actions=0,hops=0;
        std::array<std::set<std::tuple<int,int,int,int,int>>,3> visited;
        for(int round=0;round<24&&!flags.scenario[501].value_or(0);++round) {
            bool progress=false;
            for(int slot=0;slot<3;++slot) {
                auto& party=parties[slot];
                for(int leg=0;leg<24;++leg) {
                    if(!visited[slot].insert({party.map,party.at.layer,party.at.x,party.at.y,actions}).second) {
                        std::printf("Tower repeated pose slot%d map%d at%d,%d actions%d\n",slot,party.map,party.at.x,party.at.y,actions);break;
                    }
                    const auto out=run(party.map,party.at,slot);
                    if(last_guide.finished)break;
                    if(!out.path.success||out.path.blocking_party>=0||out.path.npc_approach)break;
                    if(out.path.next_map_id>0) {
                        const auto receipt=out.pending.incoming_trap?out.pending.incoming_trap:out.pending.incoming_transition;
                        expect(receipt.has_value(),"each guide transition carries its exact native arrival, including manual same-floor crossings");
                        if(!receipt)break;
                        Pose after{receipt->destination,receipt->arrival};release_plate(party,after);party=after;++hops;progress=true;continue;
                    }
                    if(!out.path.walk_phases.empty()&&!out.path.walk_phases.back().cells.empty()) {
                        Pose after{party.map,out.path.walk_phases.back().cells.back()};release_plate(party,after);party=after;
                    }
                    if(out.goal.entity_id<0) {++actions;progress=true;break;}
                    auto control=switches.find({out.goal.map_id,out.goal.entity_id});
                    expect(control!=switches.end(),"arrived Tower objective has a reviewed native effect");if(control==switches.end())break;
                    if(out.goal.map_id==549&&((out.goal.entity_id==39&&flags.scenario[478]==1)||
                       (out.goal.entity_id==45&&flags.scenario[482]==1))) {
                        std::printf("Tower weight%d correctly waits for the party below\n",out.goal.entity_id);break;
                    }
                    flags.scenario[control->second]=1;++actions;progress=true;
                    // Both reviewed native weight scripts move the examining
                    // player one cell south before finishing the animation.
                    if(out.goal.map_id==549&&(out.goal.entity_id==39||out.goal.entity_id==45))++party.at.y;
                    // Exact victory scripts, pinned by the guide author:
                    // Fiend changes back from Upper to Ground at30,20;
                    // Doom ends65,14/Ground; Goddess walks to12,33.
                    if(out.goal.map_id==542&&out.goal.entity_id==93)
                        party.at=native_cell(542,8,9,16,{1,30,20});
                    if(out.goal.map_id==567&&out.goal.entity_id==2)
                        party.at=native_cell(567,2,64,10,{1,65,14});
                    if(out.goal.map_id==568&&(out.goal.entity_id==2||out.goal.entity_id==12))
                        party.at=native_cell(568,2,12,31,{party.at.layer,12,33});
                    if(flags.scenario[478]==1&&flags.scenario[482]==1)flags.scenario[483]=1;
                    if(flags.scenario[488]==1&&flags.scenario[489]==1&&flags.scenario[490]==1)flags.scenario[491]=1;
                    // Every statue victory tests this conjunction and calls
                    // sc_e_0998, which sets the shared final-passage flag497.
                    if(flags.scenario[494]==1&&flags.scenario[495]==1&&flags.scenario[496]==1)flags.scenario[497]=1;
                    if(flags.scenario[498]==1&&flags.scenario[499]==1&&flags.scenario[500]==1)flags.scenario[501]=1;
                    break;
                }
            }
            if(!progress){std::printf("Tower episode stalled round%d actions%d hops%d\n",round,actions,hops);break;}
        }
        expect(flags.scenario[501]==1,"three-party production guide reaches the native final-switch latch");
        std::printf("Tower episode actions%d hops%d final%d\n",actions,hops,flags.scenario[501].value_or(-1));
    });worker.join();
    expect(!fixture::wrong_owner,"complete multi-party model/route consumer uses no native API");
    std::printf("Multi-party guide routes: %d failures\n",failures);return failures?1:0;
}
