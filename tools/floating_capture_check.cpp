// check-sources: field/baked_map.cpp field/baked_router.cpp field/map_state_program.cpp field/field_refs.cpp field/entity_filter.cpp field/save_points.cpp field/navigation_labels.cpp field/npc_names.cpp json.cpp text.cpp
// Real FFIII border identities through the actual native observation and planner.
// Fake managed objects stay on the owner; all native power/selectors are reread.
#define main retained_floating_capture_main
#include "route_worker_check.cpp"
#undef main
int main() {
    using namespace field;using namespace baked_nav;
    const auto read=[](const char* path){std::ifstream f(path,std::ios::binary);return std::string(std::istreambuf_iterator<char>(f),{});};
    const auto bytes=read("data/ff3/maps.bin");
    if(!baked::load(bytes.data(),bytes.size())||!json::parse(read("data/ff3/mapstate.json"),fixture::records))return 2;
    fixture::game_id=game::Id::FF3;fixture::capture_transport=fixture::capture_border=fixture::capture_entity_alive=true;
    fixture::capture_ready=true;fixture::current_map=1;
    const auto* world=baked::submap_by_map_id(1);const auto& grid=baked::grids(*world);
    map_state::Flags flags;flags.scenario.fill(1);flags.global.fill(1);flags.treasure.fill(1);
    int failures=0,actors=0;double fresh_ms=0;const auto expect=[&](bool ok,const char* why){if(!ok){++failures;std::printf("FAIL %s\n",why);}};
    RouteSnapshot snapshot(map_state::Resolver(flags,fixture::records));snapshot.map_id=1;snapshot.submap=world;
    for(const int value:{-1,0,1}) {
        auto observed=flags;if(value<0)observed.scenario.fill({});else observed.scenario.fill(value);
        map_state::Resolver resolver(observed,fixture::records);
        const auto full=resolver.transitions(*world),doors=resolver.transitions(*world,true);
        bool same=true;for(int layer=0;layer<3;++layer)for(size_t i=0;i<static_cast<size_t>(grid.width)*grid.height;++i) {
            const int a=(*full)[layer].empty()?0:(*full)[layer][i];
            const int b=(*doors)[layer].empty()?0:(*doors)[layer][i];same&=(a&1)==b;
        }
        expect(same,"doors-only projection preserves every original native-door bit for unknown/clear/set flags");
    }
    struct NativeName {uint8_t header[16]{};int32_t length=0;wchar_t text[128]{};} name;
    fixture::transport_property.native_name=&name;
    fixture::TransportArray array;array.count=1;
    fixture::TransportList list;list.items=&array;list.count=1;fixture::transport_property.targets=&list;
    const auto started=std::chrono::steady_clock::now();
    for(const auto& object:grid.objects)if(const auto* rule=air_transitions::match(3,1,object)) {
        ++actors;const auto raw=baked::object_identifier(object);name.length=int(raw.size());std::copy(raw.begin(),raw.end(),name.text);
        fixture::transport_property.type=2;fixture::transport_property.id=object.entity_id();fixture::transport_property.script=rule->script;
        array.values[0]=8;fixture::transport_actor.power=true;fixture::transport_actor.cache_enable=16;
        const int engine_before=fixture::engine_reads;
        const auto read_started=std::chrono::steady_clock::now();
        const bool captured=capture_fixed_border(&fixture::transport_actor,snapshot);
        fresh_ms+=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-read_started).count();
        expect(captured,"real border actor uses exact native observation");
        const auto& ready=snapshot.obstacles.back();
        expect(ready.fixed_border==&object&&ready.cell.x==object.tx&&ready.cell.y==object.ty&&ready.trigger&&ready.transportation_ids==(uint64_t(1)<<8),"live power, exact cell/identity and current selector survive capture");
        expect(fixture::engine_reads==engine_before&&ready.raw_name.empty(),"qualified capture makes no Unity transform/name call or text copy");
        fixture::transport_actor.power=false;array.values[0]=11;
        expect(capture_fixed_border(&fixture::transport_actor,snapshot)&&!snapshot.obstacles.back().trigger&&snapshot.obstacles.back().transportation_ids==(uint64_t(1)<<11),"same actor's changed native power/selectors are not cached");
        list.count=0;expect(capture_fixed_border(&fixture::transport_actor,snapshot)&&snapshot.obstacles.back().transportation_restricted&&!snapshot.obstacles.back().transportation_ids,"empty native selector permits no vehicle");
        list.count=1;fixture::transport_property.targets=nullptr;
        expect(capture_fixed_border(&fixture::transport_actor,snapshot)&&!snapshot.obstacles.back().transportation_restricted,"null native selector remains unrestricted");fixture::transport_property.targets=&list;
        fixture::transport_actor.cache_enable=1;
        expect(!capture_fixed_border(&fixture::transport_actor,snapshot),"foreign native cache owner retains full ordinary live observation");fixture::transport_actor.cache_enable=16;
        ++fixture::transport_property.script;expect(!capture_fixed_border(&fixture::transport_actor,snapshot),"changed script cannot borrow a static border's geometry");--fixture::transport_property.script;
        name.text[0]=L'?';expect(!capture_fixed_border(&fixture::transport_actor,snapshot),"different native name cannot borrow a border identity");
    }
    expect(actors==1524,"every actual Floating Continent border actor is checked");
    snapshot.map_id=4;expect(!capture_fixed_border(&fixture::transport_actor,snapshot),"surface world does not enter this capture path");
    snapshot.map_id=1;fixture::game_id=game::Id::FF6;expect(!capture_fixed_border(&fixture::transport_actor,snapshot),"other games retain ordinary observation");
    std::printf("Floating border capture: %d actors, %.3fms aggregate fresh capture, %.3fms complete owner regression episode, %d engine reads, %d failures\n",actors,fresh_ms,std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-started).count(),fixture::engine_reads,failures);
    return failures?1:0;
}
