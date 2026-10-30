// check-sources: field/map_state_program.cpp field/baked_map.cpp field/baked_router.cpp json.cpp text.cpp
// Real extracted initializer/bake consistency, without loading a game or a native hook.
#include "field/map_state.h"
#include <cstdio>
#include <fstream>
#include <iterator>
#include <string>
#include <chrono>
using namespace field;
int failures = 0;
void expect(bool pass, const char* message) { if (!pass) { ++failures; std::printf("FAIL %s\n", message); } }
std::string read(const std::string& path)
{
    std::ifstream f(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>()};
}
int main()
{
    {
        json::Value sample;map_state::Flags unknown;
        expect(json::parse(R"({"code":[[2,10,0,0,-1,1,3],[1,"optional"],[3,4],[0],[1,"guaranteed"],[6]]})",sample),"partial-state fixture parses");
        auto projected=map_state::project(sample,unknown);
        expect(!projected.known&&projected.groups==std::set<std::wstring>{L"guaranteed"},"unknown local keeps only groups activated on every complete branch");
        expect(json::parse(R"({"code":[[1,"earlier"],[9],[6]]})",sample),"unsupported fixture parses");
        projected=map_state::project(sample,unknown);
        expect(!projected.known&&projected.groups.empty(),"unsupported instructions or incomplete exploration certify no partial facts");
    }
    for (int n = 1; n <= 6; ++n) {
        const std::string root = "data/ff" + std::to_string(n) + "/";
        const std::string bytes = read(root + "maps.bin");
        expect(baked::load(bytes.data(), bytes.size()), "matching bake loads");
        const auto decoding_started = std::chrono::steady_clock::now();
        std::vector<const baked::Grids*> decoded;
        for (int i = 0; i < baked::submap_count(); ++i) decoded.push_back(&baked::grids(*baked::submap_at(i)));
        for (int i = 0; i < baked::submap_count(); ++i)
            expect(decoded[static_cast<size_t>(i)] == &baked::grids(*baked::submap_at(i)),
                   "prepared grids retain immutable cache identities");
        const double decode_ms = std::chrono::duration<double, std::milli>(
            std::chrono::steady_clock::now() - decoding_started).count();
        json::Value records;
        expect(json::parse(read(root + "mapstate.json"), records), "navigation decisions parse");
        int event_guards=0;
        for(int i=0;i<baked::submap_count();++i) {
            const auto& sm=*baked::submap_at(i);const auto* record=records.get(baked::name_of(sm));
            const auto* guards=record?record->get(L"events"):nullptr;if(!guards)continue;
            for(const auto& guard:guards->arr) {
                ++event_guards;expect(guard.get(L"code")->arr.size()<=256,"event predicate retains only bounded native entry control flow");
                const baked::Object* object=nullptr;
                for(const auto& row:baked::grids(sm).objects)
                    if(static_cast<int>(row.type)==guard.get(L"type")->n&&row.entity_id()==guard.get(L"id")->n&&
                        baked::object_identifier(row)==guard.get(L"raw")->s&&baked::str(row.group)==guard.get(L"group")->s&&
                        row.layer==guard.get(L"cell")->arr[0].n&&row.tx==guard.get(L"cell")->arr[1].n&&row.ty==guard.get(L"cell")->arr[2].n)object=&row;
                expect(object!=nullptr,"every native entry guard has its exact original bake identity");if(!object)continue;
                // A destination-scoped predicate controls its exact companion
                // arc, not consumption of the native physical door. Preserve
                // the original identity assertion above for every predicate.
                if(const auto* destination=guard.get(L"destination")) {
                    object=nullptr;
                    for(const auto& row:baked::grids(sm).objects)
                        if(row.type==baked::ObjType::Warp&&row.entity_id()==guard.get(L"id")->n&&
                           baked::object_identifier(row)==guard.get(L"raw")->s&&baked::str(row.group)==guard.get(L"group")->s&&
                           row.layer==guard.get(L"cell")->arr[0].n&&row.tx==guard.get(L"cell")->arr[1].n&&row.ty==guard.get(L"cell")->arr[2].n&&
                           row.a==destination->arr[0].n&&row.b==destination->arr[1].n)object=&row;
                    expect(object!=nullptr,"each destination guard has its exact real companion arc");if(!object)continue;
                }
                for(int profile=0;profile<3;++profile) {
                    map_state::Flags copied;if(profile<2){for(auto& v:copied.scenario)v=profile;for(auto& v:copied.treasure)v=profile;for(auto& v:copied.global)v=profile;}
                    map_state::Resolver state(copied,records);const auto availability=map_state::project(guard,copied);
                    expect(state.consumed(sm,*object)==(availability.known&&availability.group.empty()),"entry guards retire an event only when all native prefix paths skip every action");
                    if(profile==2)expect(!state.consumed(sm,*object),"unknown persistent flags cannot consume a potentially actionable event");
                }
            }
        }
        std::printf("FF%d: %d exact native entry guards checked in known/unknown flag states\n",n,event_guards);
        map_state::Flags flags;
        for (auto& v : flags.scenario) v = 0;
        for (auto& v : flags.treasure) v = 0;
        for (auto& v : flags.global) v = 0;
        map_state::Resolver resolver(flags, records);
        if(n==3) {
            const auto* world=baked::submap_by_map_id(1);const auto& grid=baked::grids(*world);
            const baked::Object* tunnel=nullptr;for(const auto& o:grid.objects)if(o.type==baked::ObjType::Event&&o.entity_id()==2367)tunnel=&o;
            expect(tunnel!=nullptr,"reported Tozus tunnel trigger has its exact real world identity");
            if(tunnel)for(int state:{-1,0,1}){
                auto form=flags;if(state>=0)form.party_mini=state;
                map_state::Resolver current(form,records);
                expect(current.consumed(*world,*tunnel)==(state==0),"Tozus event is inactive only when all-member Mini is known false; unknown remains conservative");
            }
            auto post=flags;post.scenario[16]=1;
            for(int id:{23,24,25,26,27,35}) {
                const auto* floor=baked::submap_by_map_id(id);const auto* row=records.get(baked::name_of(*floor));
                const auto selection=map_state::project(*row,post);
                expect(selection.groups.count(L"ev_e_0017")!=0,"Sasune post-curse NPC group survives its visual mist call and unrelated unknown companion branch");
            }
            const auto* mine=baked::submap_by_map_id(159);
            expect(mine!=nullptr,"Mythril Mines native floor is baked");
            if(mine) {
                const baked::Object* wall=nullptr;const baked::Object* lever=nullptr;
                for(const auto& o:baked::grids(*mine).objects) {
                    if(o.type==baked::ObjType::Entity&&o.entity_id()==10)wall=&o;
                    if(o.type==baked::ObjType::Entity&&o.entity_id()==8)lever=&o;
                }
                expect(wall&&lever,"Mythril wall and switch retain distinct native identities");
                if(wall&&lever) {
                    for(int phase=0;phase<3;++phase) {
                        auto state=flags;state.scenario[16]=phase>=1;state.scenario[180]=phase==2;
                        map_state::Resolver mine_state(state,records);
                        expect(mine_state.active(*mine,*wall)==(phase<2),"mine wall persists until its own switch flag");
                        expect(mine_state.active(*mine,*lever)==(phase==1),"mine switch only exists in the native enabled story phase");
                        baked_route::Request path;path.submap=mine;path.start={1,10,20};path.goals={{1,10,7}};
                        map_state::add_obstacles(mine_state,*mine,path);
                        expect(baked_route::find(path).found==(phase==2),"B2 stays gated until the actual wall is removed");
                        if(phase<2) {
                            auto ungated=path;ungated.blocked.clear();
                            expect(baked_route::find(ungated).found,"only the mine wall closes the baked B2 route");
                        }
                        if(phase==1) {
                            path.goals=baked_route::goals_for(path,{lever->layer,lever->tx,lever->ty},baked_route::GoalKind::Interact).cells;
                            expect(baked_route::find(path).found,"enabled mine switch is reachable before opening B2");
                        }
                    }
                }
            }
        }
        if(n==6) {
            auto state=flags;state.scenario[13]=1;
            const auto defense=map_state::project(*records.get(L"Map_30011_2"),state);
            expect(defense.known&&defense.groups.count(L"ev_e_0018")&&defense.groups.count(L"ev_e_0609"),
                "native SetEntities preserves the normal exit group alongside the Terra defense group");
            const auto& cave=*records.get(L"Map_30781_1");
            state=flags;state.current_map_id=498;state.current_area_id=static_cast<int>(cave.get(L"area_id")->n);
            for(auto& v:state.area)v=0;
            const auto initial=map_state::project(cave,state);
            expect(initial.known&&initial.groups.count(L"ev_e_0806")&&initial.groups.count(L"ev_e_0799")&&initial.groups.count(L"ev_e_0808"),
                "Phoenix Cave independently retains the twin-switch gate, drain lever and bridge event groups");
            state.area[7]=1;
            const auto rocks=map_state::project(cave,state);
            expect(rocks.known&&rocks.groups.count(L"ev_e_0801")&&rocks.groups.count(L"ev_e_0806"),
                "native area locals add the rock crossing without dropping other puzzle groups");
            for(int index=1;index<=7;++index) {
                state.area.fill(0);state.area[static_cast<size_t>(index)]=1;
                const auto held=map_state::project(cave,state);state.area[static_cast<size_t>(index)]=0;
                const auto released=map_state::project(cave,state);
                expect(held.known&&released.known&&(held.entities!=released.entities||held.groups!=released.groups),
                    "every Phoenix temporary plate changes projected geometry again on its native release");
            }
            state.current_map_id=499;state.scenario[406]=1;
            map_state::Resolver alternate(state,records);
            const auto* second=baked::submap_by_map_id(499);bool checked_show=false,checked_hide=false;
            for(const auto& o:baked::grids(*second).objects)if(o.type==baked::ObjType::Event) {
                if(o.entity_id()==55){expect(alternate.active(*second,o),"lower rock state exposes the alternate upper control");checked_show=true;}
                if(o.entity_id()==58){expect(!alternate.active(*second,o),"native Hide also applies to a physical Event actor");checked_hide=true;}
            }
            expect(checked_show&&checked_hide,"both real rock controls are identified");
            state.current_area_id+=1000;
            expect(!map_state::project(cave,state).known,"another area's local variables cannot certify Phoenix Cave state");
        }
        int known = 0, unknown = 0, solid = 0, known_chests = 0, unknown_chests = 0, zero_chests = 0;
        for (int i = 0; i < baked::submap_count(); ++i) {
            const auto& sm = *baked::submap_at(i);
            resolver.known(sm) ? ++known : ++unknown;
            baked_route::Request request;
            request.submap = &sm;
            map_state::add_obstacles(resolver, sm, request);
            for (const auto& o : baked::grids(sm).objects) {
                if (o.type == baked::ObjType::TreasureBox) {
                    const auto opened = resolver.treasure_opened(sm, o);
                    if (opened) {
                        ++known_chests;
                        expect(!*opened, "zero saved flag means unopened chest");
                        const auto* record = records.get(baked::name_of(sm));
                        const auto* chests = record ? record->get(L"treasures") : nullptr;
                        bool matched = false;
                        if (chests) for (const auto& row : chests->arr) {
                            const auto& r = row.arr;
                            if (r.size() < 3 || static_cast<int>(r[0].n) != o.entity_id() || r[1].s != baked::str(o.group)) continue;
                            const int bit = static_cast<int>(r[2].n);
                            if (bit < 0 || bit >= 1024) continue;
                            if(bit==0)++zero_chests;
                            auto changed = flags; changed.treasure[static_cast<size_t>(bit)] = 1;
                            map_state::Resolver collected(changed, records);
                            expect(collected.treasure_opened(sm, o) == std::optional<bool>(true),
                                   "matching copied saved bit means opened chest on any floor");
                            changed.treasure[static_cast<size_t>(bit)].reset();
                            map_state::Resolver missing(changed, records);
                            expect(!missing.treasure_opened(sm, o).has_value(),
                                   "unavailable saved bit remains unknown rather than unopened");
                            matched = true; break;
                        }
                        expect(matched, "known chest state is backed by matching entity and story-group flag record");
                    } else ++unknown_chests;
                }
                if (!resolver.active(sm, o, true)) continue;
                if ((o.type != baked::ObjType::NPC && o.type != baked::ObjType::ShopNPC) || !(o.d & 1)) continue;
                ++solid;
                bool blocked = false;
                const auto effective=resolver.effective(sm,o);
                for (const auto& c : request.blocked) if (c.x == effective.tx && c.y == effective.ty) blocked = true;
                expect(blocked, "every applicable solid NPC blocks future-floor routes");
            }
        }
        if (n == 2) {
            const auto* room = baked::submap_by_name(L"Map_30051_8");
            expect(room != nullptr, "Dreadnought engine approach exists");
            if (room) {
                baked_route::Request req; req.submap = room;
                for (const auto& o : baked::grids(*room).objects) {
                    if (o.type == baked::ObjType::PointIn && o.a == 1) req.start = {o.layer,o.tx,o.ty};
                    if (o.type == baked::ObjType::GotoMap && o.a == 175) req.goals = {{o.layer,o.tx,o.ty}};
                }
                map_state::add_obstacles(resolver, *room, req);
                const auto before = baked_route::find(req);
                expect(before.found, "projected Dreadnought route uses the captain detour");
                const auto after = baked_route::find(req);
                expect(before.cells == after.cells, "a battle leaves permanent captain obstacles intact");
            }
        }
        expect(known_chests > 0, "each game has verified saved chest flags");
        expect(unknown_chests==0,"every extracted real treasure flag, including zero, has a cross-floor mapping");
        expect(zero_chests==(n==2?0:n==5?4:1),"all eight real flag-zero chest objects retain unopened/opened/unknown-bit tests across all six games");
        std::printf("FF%d: %d saved chest mappings verified; %d unmapped chests remain unknown\n", n, known_chests, unknown_chests);
        std::printf("FF%d: %d known, %d conservative maps; %d solid NPC checks; all grids decoded %.1f ms offline\n",
                    n, known, unknown, solid, decode_ms);
    }
    std::printf("%s: %d failures\n", failures ? "FAIL" : "PASS", failures);
    return failures ? 1 : 0;
}
