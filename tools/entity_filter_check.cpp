// check-sources: field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp json.cpp text.cpp
#include "../src/core/field/entity_filter.h"
#include <cstdio>
#include <thread>
#include <chrono>
#include <fstream>
namespace field::baked_nav {
std::optional<AirTransitionTarget> air_transition_target(ExitSnapshot&,const std::optional<MapGoal>&,const std::optional<MapGoal>&,std::function<bool()>,bool,bool) { return {}; }
std::optional<AirTransitionTarget> nautilus_surface_target(ExitSnapshot&,std::function<bool()>,bool) { return {}; }

std::vector<int> exit_distances(ExitSnapshot&,const std::vector<ExitTarget>&,std::function<bool()>) { return {}; }
std::vector<bool> story_gate_mask(ExitSnapshot&,const std::vector<StoryTarget>& targets,std::function<bool()>) { return std::vector<bool>(targets.size(),false); }
}
int main() {
    using namespace field; using namespace filter_worker;
    int failures = 0;
    auto expect = [&](bool ok, const char* reason) { if (!ok) { ++failures; std::printf("FAIL %s\n", reason); } };
    FilterInput input; input.sort = true; input.player = {};
    auto row = [&](Kind kind, Category category, std::wstring label, float distance, int floor = 0) {
        FilterRow value; value.index = input.rows.size(); value.kind = kind; value.category = category;
        value.name = label; value.label = npc_names::parse(label); value.position = {distance, 0, 0};
        value.numbering_position = value.position; value.remote_map = floor; value.cell_x = static_cast<int>(distance);
        input.rows.push_back(std::move(value));
    };
    row(Kind::NPC, Category::NPCs, L"7: Soldier", 2);
    row(Kind::NPC, Category::NPCs, L"Soldier２", 8);
    row(Kind::NPC, Category::NPCs, L"⑥ Old Man", 5);
    row(Kind::NPC, Category::NPCs, L"Soldier1", 1, 50);
    row(Kind::NPC, Category::NPCs, L"Soldier2", 2, 50);
    row(Kind::MapExit, Category::MapExits, L"Exit", 4); input.rows.back().destination = 100;
    row(Kind::MapExit, Category::MapExits, L"Exit", 6); input.rows.back().destination = 100;
    row(Kind::Event, Category::Events, L"Pad", 7); input.rows.back().pad = L"one";
    row(Kind::Event, Category::Events, L"Pad", 3); input.rows.back().pad = L"one";
    row(Kind::Event, Category::Events, L"Layer", 9); input.rows.back().event_type = L"ToLayer";
    Filtered result;
    {
        FilterInput books;books.category=Category::Objects;books.sort=true;
        for(int i=0;i<12;++i){FilterRow value;value.index=books.rows.size();value.kind=Kind::Event;value.category=Category::Objects;
            value.name=i==9?L"Switch":L"Book";value.label={value.name,-1};value.remote_map=i>=10?50:0;
            value.position=value.numbering_position={static_cast<float>(i),0,0};value.cell_x=i;books.rows.push_back(value);}
        FilterRow npc; npc.index=books.rows.size();npc.kind=Kind::NPC;npc.category=Category::NPCs;npc.name=L"Book";npc.label={L"Book",-1};books.rows.push_back(npc);
        auto numbered=build(books);
        expect(numbered.names[0]==L"Book1"&&numbered.names[8]==L"Book9"&&numbered.names[9]==L"Switch",
            "nine repeated interactive objects get suffixes while a unique object keeps its name");
        expect(numbered.names[10]==L"Book1"&&numbered.names[11]==L"Book2"&&numbered.names[12]==L"Book",
            "object numbering is independent per map and separate from an identically named NPC");
        books.rows[0].story_gated=true;books.player={8,0,0};numbered=build(books);
        expect(numbered.names[8]==L"Book9"&&numbered.order.front()==8,"object numbering survives reachability pruning and distance resorting");
        numbered=build(books);expect(numbered.names[8]==L"Book9","repeated worker builds never accumulate numbering suffixes");
    }
    std::thread worker([&] { result = build(input); }); worker.join();
    expect(result.names[0] == L"Soldier2" && result.names[1] == L"Soldier1" && result.names[2] == L"Old Man",
        "NPC source ordinals number exact tags before distance filtering");
    expect(result.names[3] == L"Soldier1" && result.names[4] == L"Soldier2", "remote floor numbering is independent");
    expect(result.order.size() == 10 && result.order.back() == 4, "default grouping preserves all rows and remote order");
    expect(result.exits.size() == 2, "unfiltered live exit cache includes both exits");
    input.pads = input.exits = input.layer = true; input.rows[1].selected = true;
    result = build(input);
    expect(result.order.size() == 7 && result.restored >= 0 && result.order[result.restored] == 1,
        "existing pad/exit/layer guards and selected identity survive worker sort");
    expect(result.exits.size() == 2, "navigation filters do not remove wall-tone exit locations");
    input.rows[5].route_distance=-1; input.rows[6].route_distance=20;
    result=build(input);
    expect(std::find(result.order.begin(),result.order.end(),6)!=result.order.end() &&
        std::find(result.order.begin(),result.order.end(),5)==result.order.end(),"blocked nearest tile cannot prune a reachable sibling exit");
    input.rows[5].route_distance=50; input.rows[6].route_distance=20; input.sort=false;
    result=build(input);
    expect(std::find(result.order.begin(),result.order.end(),6)!=result.order.end(),"grouping chooses shortest usable route independently of sort order");
    input.world=true; input.exits=false; input.rows[5].route_distance=-1;
    result=build(input);
    expect(std::find(result.order.begin(),result.order.end(),5)==result.order.end() && result.exits.size()==1,
        "unreachable world trigger is pruned from list and audio exit cache with grouping off");
    input.world=false; input.exits=true; input.sort=true; input.rows[5].route_distance=input.rows[6].route_distance=-1;
    {
        FilterInput all;all.world=true;all.reachable_only=false;
        for(int i=0;i<3;++i) {FilterRow row;row.index=i;row.kind=Kind::MapExit;row.category=Category::MapExits;
            row.destination=i==2?9:8;row.route_distance=-1;row.story_gated=true;all.rows.push_back(row);}
        auto view=build(all);
        expect(view.order.size()==3&&view.exits.size()==3,"Current map retains unreachable and story-gated world destinations with grouping off");
        all.exits=true;view=build(all);
        expect(view.order.size()==2,"Current map may group physical sibling exits while retaining each inaccessible destination");
        all.reachable_only=true;view=build(all);
        expect(view.order.empty(),"reachable modes retain the existing inaccessible-destination policy");
    }
    input.category = Category::NPCs; result = build(input);
    expect(result.order.size() == 5 && result.names[0] == L"Soldier2", "category filter does not renumber groups");
    input.rows[1].selected = false; input.selected_has = true; input.selected_remote = 50;
    input.selected_x = 2; input.selected_y = 0; result = build(input);
    expect(result.restored >= 0 && result.order[result.restored] == 4, "recreated remote row restores floor/cell selection");
    input.continued_goal = true; input.continued_map = 50; input.continued_x = 1;
    result = build(input);
    expect(result.continued >= 0 && result.order[result.continued] == 3,
        "pending destination is matched on the worker independently of ordinary selection");
    input.continued_map = input.current_map = 12; input.continued_x = 8;
    input.rows[1].cell_valid = true; result = build(input);
    expect(result.continued >= 0 && result.order[result.continued] == 1, "live destination uses copied verified cell");
    input.rows[1].cell_valid = false; result = build(input);
    expect(result.continued == -1, "invalid native cell capture cannot match a live destination");
    const auto categories = build_categories(input);
    for (int category = 0; category < kCategoryCount; ++category) {
        input.category = static_cast<Category>(category);
        const auto expected = build(input);
        const auto& actual = categories.views[category];
        expect(actual.order == expected.order && actual.restored == expected.restored && actual.continued == expected.continued,
            "prepared category matches the independent native-policy filter and selection");
    }
    {
        FilterInput variants;variants.current_map=50;variants.selected_has=true;variants.selected_map=50;
        variants.selected_entity_id=32;variants.selected_category=Category::Events;variants.selected_raw=L"original";
        variants.continued_goal=true;variants.continued_map=50;variants.continued_entity_id=32;variants.continued_raw=L"original";
        FilterRow first;first.index=0;first.kind=Kind::Event;first.category=Category::Events;first.entity_id=32;
        first.remote_map=50;first.raw_name=L"replacement";
        auto original=first;original.index=1;original.raw_name=L"original";
        variants.rows={first,original};
        auto view=build(variants);
        expect(view.restored>=0&&view.order[view.restored]==1&&view.continued>=0&&view.order[view.continued]==1,
            "collocated story variants retain their exact developer identity across floor restoration");
        variants.rows.pop_back();view=build(variants);
        expect(view.restored==-1&&view.continued==-1,"a missing objective never rebinds to a collocated replacement story variant");
    }
    {
        FilterInput chests;chests.current_map=183;chests.selected_has=true;chests.selected_map=183;
        chests.selected_entity_id=0;chests.selected_category=Category::Chests;chests.selected_x=35;chests.selected_y=18;
        FilterRow opened;opened.index=0;opened.kind=Kind::TreasureChest;opened.category=Category::Chests;
        opened.entity_id=1;opened.name=L"Treasure Chest";opened.cell_valid=true;opened.cell_x=34;opened.cell_y=18;
        auto target=opened;target.index=1;target.entity_id=0;target.cell_x=35;chests.rows={opened,target};
        auto view=build(chests);expect(view.restored>=0&&view.order[view.restored]==1,"native chest ID zero stays selected after battle re-sort");
        chests.rows.pop_back();view=build(chests);expect(view.restored==-1,"missing unopened chest never substitutes an opened chest");
        chests.selected_entity_id=-1;chests.selected_name=L"Treasure Chest";
        view=build(chests);expect(view.restored==-1,"type-only chest names are never a restoration identity fallback");
    }
    {
        // Same-floor browsing has no pending cross-map goal. A distance flip
        // must not discard the physical exit already selected by the player.
        FilterInput same; same.current_map=343; same.exits=true;
        same.selected_has=true; same.selected_map=343; same.selected_category=Category::MapExits;
        same.selected_entity_id=0; same.selected_raw=L"target"; same.selected_x=4; same.selected_y=6;
        FilterRow a; a.index=0; a.kind=Kind::MapExit; a.category=Category::MapExits;
        a.entity_id=0; a.raw_name=L"target"; a.cell_valid=true; a.cell_x=4; a.cell_y=6;
        a.destination=1; a.route_distance=20; a.selected=true;
        auto b=a; b.index=1; b.entity_id=1; b.raw_name=L"sibling"; b.cell_x=5; b.route_distance=1; b.selected=false;
        same.rows={a,b}; auto view=build(same);
        expect(view.order.size()==1 && view.restored==0 && view.order[0]==0, "same-map selected exit survives a distance flip");
        same.rows[0].selected=false; view=build(same);
        expect(view.order.size()==1 && view.restored==0 && view.order[0]==0, "recreated native exit restores exact ID zero and cell");
        same.selected_raw=L"wrong"; view=build(same);
        expect(view.order.size()==1 && view.order[0]==1 && view.restored==-1, "raw mismatch cannot pin an exit");
        same.selected_raw=L"target"; same.selected_has=false; view=build(same);
        expect(view.order.size()==1 && view.order[0]==1, "cleared selection restores nearest grouping");
        same.selected_has=true; same.rows[0].available=false; view=build(same);
        expect(view.order.size()==1 && view.order[0]==1 && view.restored==-1, "unavailable selection cannot pin");
        same.rows[0].available=true; same.rows[0].story_gated=true; view=build(same);
        expect(view.order.size()==1 && view.order[0]==1 && view.restored==-1, "story-gated selection cannot pin");
        same.rows[0].story_gated=false; same.rows[0].route_distance=-1; same.world=true; view=build(same);
        expect(view.order.size()==1 && view.order[0]==1 && view.restored==-1, "unreachable world selection cannot pin");
        same.world=false; view=build(same);
        expect(view.order.size()==1 && view.order[0]==0 && view.restored==0, "blocked indoor selection remains for explanation");
        same.continued_goal=true; same.continued_map=343; same.continued_x=5; same.continued_y=6;
        same.continued_entity_id=1; same.continued_raw=L"sibling"; view=build(same);
        expect(view.order.size()==1 && view.order[0]==1 && view.continued==0, "pending goal takes priority over selected sibling");
    }
    {
        // A remote objective must remain the sole representative after it becomes live.
        FilterInput handoff; handoff.current_map=80; handoff.exits=true;
        handoff.continued_goal=true; handoff.continued_map=77; handoff.continued_x=4;
        handoff.continued_y=6; handoff.continued_entity_id=0; handoff.continued_raw=L"target";
        FilterRow target; target.index=0; target.kind=Kind::MapExit; target.category=Category::MapExits;
        target.remote_map=77; target.cell_x=4; target.cell_y=6; target.entity_id=0;
        target.raw_name=L"target"; target.destination=1; target.route_distance=1;
        auto sibling=target; sibling.index=1; sibling.cell_x=5; sibling.entity_id=1;
        sibling.raw_name=L"sibling"; sibling.route_distance=10; handoff.rows={target,sibling};
        auto remote=build(handoff);
        expect(remote.continued>=0 && remote.order[remote.continued]==0 && remote.order.size()==1,
            "remote grouped objective initially retained");
        handoff.current_map=77; handoff.rows[0].remote_map=handoff.rows[1].remote_map=0;
        handoff.rows[0].cell_valid=handoff.rows[1].cell_valid=true;
        handoff.rows[0].route_distance=20; handoff.rows[1].route_distance=1;
        auto live=build(handoff); auto views=build_categories(handoff);
        expect(live.continued>=0 && live.order[live.continued]==0 && live.order.size()==1,
            "grouped live handoff preserves exact objective despite closer sibling");
        expect(views.views[0].continued>=0 && views.views[0].order[views.views[0].continued]==0 &&
            views.views[3].continued>=0 && views.views[3].order[views.views[3].continued]==0 &&
            views.audio_order.size()==2, "category publication preserves objective and physical audio exits");
        handoff.sort=true; live=build(handoff);
        expect(live.continued>=0 && live.order[live.continued]==0, "sorting cannot displace the objective");
        handoff.continued_goal=false; live=build(handoff);
        expect(live.order.size()==1 && live.order[0]==1 && live.continued==-1, "clearing objective restores nearest grouping");
        handoff.continued_goal=true; handoff.exits=false; live=build(handoff);
        expect(live.order.size()==2 && live.continued>=0 && live.order[live.continued]==0, "grouping disabled preserves both exits");
        handoff.exits=true; handoff.rows[0].available=false; live=build(handoff);
        expect(live.order.size()==1 && live.order[0]==1 && live.continued==-1, "unavailable objective cannot bind sibling");
        handoff.rows[0].available=true; handoff.rows[0].story_gated=true; live=build(handoff);
        expect(live.order.size()==1 && live.order[0]==1 && live.continued==-1, "story-hidden objective cannot bind sibling");
        handoff.rows[0].story_gated=false; handoff.continued_entity_id=2; live=build(handoff);
        expect(live.order.size()==1 && live.order[0]==1 && live.continued==-1, "wrong native ID cannot bind sibling including ID zero");
        handoff.continued_entity_id=0; handoff.continued_raw=L"wrong"; live=build(handoff);
        expect(live.order.size()==1 && live.order[0]==1 && live.continued==-1, "wrong raw identity cannot bind sibling");
        handoff.continued_raw=L"target"; handoff.world=true; handoff.rows[0].route_distance=-1; live=build(handoff);
        expect(live.order.size()==1 && live.order[0]==1 && live.continued==-1, "unreachable world objective leaves usable sibling browsable");
        handoff.world=false; live=build(handoff);
        expect(live.order.size()==1 && live.order[0]==0 && live.continued==0, "blocked indoor objective remains available for explanation");
        handoff.cancelled=[] { return true; }; live=build(handoff);
        expect(live.order.empty(), "cancelled grouped work produces no result");
    }
    {
        // Identity and geometry come from the actual FFIII Gysahl town bake.
        // baked::load borrows the backing bytes for subsequent checks.
        std::ifstream f("data/ff3/maps.bin",std::ios::binary); static const std::string bytes{std::istreambuf_iterator<char>(f),{}};
        expect(baked::load(bytes.data(),bytes.size()), "FFIII Gysahl bake loads");
        const auto* sm=baked::submap_by_map_id(77); const baked::Object *a=nullptr,*b=nullptr;
        if(sm) for(const auto& object:baked::grids(*sm).objects) {
            if(object.entity_id()==6 && object.tx==16 && object.ty==31 && object.a==1) a=&object;
            if(object.entity_id()==13 && object.tx==17 && object.ty==31 && object.a==1) b=&object;
        }
        expect(a && b, "Gysahl boundary sibling identities and cells exist in bake");
        if(a && b) {
            FilterInput actual;actual.exits=true;actual.current_map=80;actual.continued_goal=true;actual.continued_map=77;
            actual.continued_x=a->tx;actual.continued_y=a->ty;actual.continued_entity_id=a->entity_id();actual.continued_raw=baked::object_identifier(*a);
            FilterRow one;one.index=0;one.kind=Kind::MapExit;one.category=Category::MapExits;one.remote_map=77;
            one.cell_x=a->tx;one.cell_y=a->ty;one.entity_id=a->entity_id();one.raw_name=baked::object_identifier(*a);one.destination=1;one.route_distance=1;
            auto two=one;two.index=1;two.cell_x=b->tx;two.cell_y=b->ty;two.entity_id=b->entity_id();two.raw_name=baked::object_identifier(*b);two.route_distance=2;
            actual.rows={one,two};actual.current_map=77;actual.rows[0].remote_map=actual.rows[1].remote_map=0;
            actual.rows[0].cell_valid=actual.rows[1].cell_valid=true;actual.rows[0].route_distance=20;actual.rows[1].route_distance=1;
            auto result=build_categories(actual);
            expect(result.views[3].order.size()==1 && result.views[3].continued==0 && result.views[3].order[0]==0 && result.audio_order.size()==2,
                "real Gysahl boundary objective survives controlled distance flip");
        }
    }
    // Large owned lists are prepared once, not recaptured/renamed for each key.
    FilterInput large;
    large.sort = true;
    for (size_t i = 0; i < 10000; ++i) {
        FilterRow value; value.index = i; value.kind = Kind::MapExit;
        value.category = Category::MapExits; value.name = L"Exit";
        value.position = {static_cast<float>(i % 256), static_cast<float>(i / 256), 0};
        large.rows.push_back(std::move(value));
    }
    const auto started = std::chrono::steady_clock::now();
    const auto prepared = build_categories(large);
    const auto elapsed = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - started).count();
    expect(prepared.views[0].order.size() == 10000 && prepared.views[3].order.size() == 10000,
        "large-list categories retain every entity without a per-frame entry cap");
    std::printf("Prepared 10,000 rows / six categories: %.3f ms (offline computation)\n", elapsed);
    int checks=0;
    large.cancelled=[&] { return ++checks>12000; };
    const auto retired=build_categories(large);
    expect(retired.views[0].order.empty()&&checks==12001,
        "obsolete destination refresh exits during row preparation/sort without finishing all six views");
    const auto cancel_start=std::chrono::steady_clock::now();
    int retired_count=0;
    for(int iteration=0;iteration<100;++iteration) {
        int checks_this=0; large.cancelled=[&] { return ++checks_this>100; };
        if(build_categories(large).views[0].order.empty() && checks_this>=101) ++retired_count;
    }
    const auto cancel_ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-cancel_start).count();
    expect(retired_count==100, "rapid obsolete work is cancelled on every iteration");
    std::printf("100 rapid cancellations, 10,000 owned rows: %.3f ms aggregate (offline worker)\n",cancel_ms);
    {
        const auto read=[](const char* path){std::ifstream f(path,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
        static const auto bytes=read("data/ff1/maps.bin");expect(baked::load(bytes.data(),bytes.size()),"real FF1 entry-guard bake loads");
        json::Value records;expect(json::parse(read("data/ff1/mapstate.json"),records),"real FF1 entry-guard records load");
        const auto* sm=baked::submap_by_map_id(1);expect(sm!=nullptr,"real world event floor exists");
        if(sm) {
            const baked::Object* trigger=nullptr;for(const auto& object:baked::grids(*sm).objects)
                if(object.entity_id()==89&&object.type==baked::ObjType::Event)trigger=&object;
            expect(trigger!=nullptr,"native conditional one-use event exists");
            if(trigger)for(int profile=0;profile<3;++profile) {
                map_state::Flags flags;if(profile<2){for(auto& v:flags.scenario)v=profile;for(auto& v:flags.treasure)v=profile;for(auto& v:flags.global)v=profile;}
                FilterInput stateful;stateful.current_map=1;stateful.state=std::make_shared<map_state::Resolver>(flags,records);
                FilterRow row;row.index=0;row.kind=Kind::Event;row.category=Category::Events;row.entity_id=89;row.raw_name=baked::object_identifier(*trigger);
                row.position={(trigger->tx-sm->width*.5f)*16+8,(sm->height*.5f-trigger->ty)*16-8,0};stateful.rows={row};
                const auto view=build_categories(stateful);
                expect(view.views[0].order.empty()==(profile==1),"actual list worker retires the native used-event branch while preserving unused/unknown states");
            }
        }
    }
    {
        FilterInput split;
        for(int i=0;i<4;++i) {
            FilterRow object;object.index=i;object.kind=Kind::Event;object.category=i%2?Category::Events:Category::Objects;
            object.event_type=i%2?L"Event":L"Interactive Object";object.remote_map=i>=2?98:0;object.selected=i==2;
            split.rows.push_back(object);
        }
        const auto views=build_categories(split);
        expect(views.views[0].order.size()==4&&views.views[7].order==std::vector<size_t>({0,2})&&
            views.views[4].order==std::vector<size_t>({1,3})&&views.views[7].restored==1,
            "Objects and Events have separate current/remote views while All and selected identity are retained");
    }
    {
        FilterInput physical;
        for(int i=0;i<3;++i){FilterRow row;row.index=i;row.kind=Kind::MapExit;row.category=Category::MapExits;row.destination=1;row.route_distance=i+1;physical.rows.push_back(row);}
        physical.exits=false;auto ungrouped=build_categories(physical);physical.exits=true;auto grouped=build_categories(physical);
        expect(ungrouped.views[0].order.size()==3&&grouped.views[0].order.size()==1&&ungrouped.audio_order==grouped.audio_order&&grouped.audio_order.size()==3,
            "soundscape physical registry is independent from the pathfinder Group exits toggle");
    }
    std::printf("Entity filter: owned worker sorting, stable NPC groups, inclusion guards, selection: %d failures\n", failures);
    return failures ? 1 : 0;
}
