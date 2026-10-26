// Native collection boundary, including FF6's separate parked-party ownership.
#include "../src/core/field/party_entities.cpp"
#include <cstdio>
#include <cstring>
#include <vector>
namespace fixture {
struct Array {std::vector<void*> items;};struct Controller {Array* parties=nullptr;void* companion=nullptr;};struct Map {Controller* controller=nullptr;};
game::Id game=game::Id::FF6;Array array;Controller controller{&array};Map map{&controller};
int reads=0;void* party=reinterpret_cast<void*>(10);void* other=reinterpret_cast<void*>(11);
struct Property {int asset=0;} property;
struct Companion {Property* property=&fixture::property;} companion;
struct CompanionData {int asset=34;} saved;
struct SaveManager {CompanionData* companion=&saved;} save;
bool manager_ready=true;
void* save_instance(const void*){++reads;return manager_ready?&save:nullptr;}
}
namespace game {Id current(){return fixture::game;}}
namespace ref {
Class::Class(const char*n,const char*c,uint8_t g,const char*a):ns(n),name(c),games(g),assembly(a){}
Field::Field(Class&c,const char*n,const char*a):cls(c),name(n),alt(a){}
int32_t Field::get(){++fixture::reads;return std::strcmp(name,"companionEntity")==0?offsetof(fixture::Controller,companion):0;}
Method::Method(Class&c,const char*n,int a):cls(c),name(n),argc(a){}
void* Method::ptr(){return std::strcmp(name,"get_Instance")==0?reinterpret_cast<void*>(&fixture::save_instance):nullptr;}
il2cpp::Method* Method::get(){return nullptr;}
void call_failed_note(const char*){std::abort();}
bool Class::is(void* p){++fixture::reads;if(std::strcmp(name,"FieldCompanionEntity")==0)return p==&fixture::companion;if(std::strcmp(name,"SaveCompanionEntityData")==0)return p==&fixture::saved;return std::strcmp(name,"FieldPartyEntity")==0&&(p==fixture::party||p==fixture::other);}
}
namespace field {
namespace r {ref::Class kFieldMap{"Last.Map","FieldMap"};ref::Field fFieldController{kFieldMap,"fieldController"},fEntityProperty{kFieldMap,"property"};}
std::wstring translate_entity_name(const std::wstring& raw){return raw;}
void* field_map(){++fixture::reads;return &fixture::map;}
}
namespace unity {
int array_length(void* p){++fixture::reads;return p?static_cast<int>(static_cast<fixture::Array*>(p)->items.size()):0;}
void* array_at(void* p,int i){++fixture::reads;return static_cast<fixture::Array*>(p)->items.at(static_cast<size_t>(i));}
}
int main(){
    namespace parties=field::party_entities;int failures=0;
    const auto expect=[&](bool ok,const char* why){if(!ok){++failures;std::printf("FAIL %s\n",why);}};
    fixture::array.items={fixture::party,nullptr,fixture::other};int slot=0;void* entity=nullptr;
    expect(parties::next_native(&fixture::controller,slot,entity)&&entity==fixture::party,"first separate parked actor is captured");
    expect(parties::next_native(&fixture::controller,slot,entity)&&!entity,"empty active-party slot is not invented");
    expect(parties::next_native(&fixture::controller,slot,entity)&&entity==fixture::other,"third slot is preserved rather than compressed");
    expect(!parties::next_native(&fixture::controller,slot,entity),"bounded collection terminates");
    expect(parties::native_slot(fixture::party)==0&&parties::native_slot(fixture::other)==2,"native identity determines stable corps numbering");
    fixture::array.items={nullptr,fixture::party,fixture::other};expect(parties::native_slot(fixture::party)==1,"a fresh observation follows native ownership changes");
    fixture::array.items.push_back(nullptr);slot=0;expect(!parties::next_native(&fixture::controller,slot,entity)&&parties::native_slot(fixture::party)<0,"unexpected native count is rejected before iteration");
    fixture::game=game::Id::FF5;const int before=fixture::reads;
    expect(!parties::next_native(&fixture::controller,slot,entity)&&!parties::is_native(fixture::party)&&parties::native_slot(fixture::party)<0&&fixture::reads==before,"other games never resolve FF6-only fields or classes");
    fixture::controller.companion=&fixture::companion;
    for(int n=1;n<=6;++n){fixture::game=static_cast<game::Id>(n);expect(parties::is_companion(&fixture::companion)&&!parties::is_companion(fixture::party),"native follower type is separate from parked corps in every game");expect(parties::companion_name(&fixture::companion)==(n==3?L"シド":L""),"only the matching game's verified companion IDs supply names");}
    fixture::game=game::Id::FF3;for(const auto& value:std::vector<std::pair<int,std::wstring>>{{33,L"サラ"},{34,L"シド"},{35,L"デッシュ"},{36,L"エリア"},{37,L"アルス"},{38,L"ドーガ"},{39,L"ウネ"},{41,L"グツコー"}}){fixture::saved.asset=value.first;expect(parties::companion_name(&fixture::companion)==value.second,"synthetic PropertyNpc.AssetId zero still names every companion from the actual native base-identity source");}
    fixture::property.asset=34;fixture::saved.asset=999;expect(parties::companion_name(&fixture::companion).empty(),"unrecognized saved companion cannot borrow a property/stale identity");
    fixture::saved.asset=35;fixture::controller.companion=nullptr;expect(parties::companion_name(&fixture::companion).empty(),"a former or unowned companion cannot borrow the current identity");fixture::controller.companion=&fixture::companion;
    fixture::manager_ready=false;expect(parties::companion_name(&fixture::companion).empty(),"missing save manager stays unknown");fixture::manager_ready=true;fixture::save.companion=nullptr;expect(parties::companion_name(&fixture::companion).empty(),"missing companion data stays unknown");fixture::save.companion=&fixture::saved;
    std::printf("Native parked-party collection: %d failures\n",failures);return failures?1:0;
}
