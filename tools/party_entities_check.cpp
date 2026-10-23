// Native collection boundary, including FF6's separate parked-party ownership.
#include "../src/core/field/party_entities.cpp"
#include <cstdio>
#include <cstring>
#include <vector>
namespace fixture {
struct Array {std::vector<void*> items;};struct Controller {Array* parties=nullptr;};struct Map {Controller* controller=nullptr;};
game::Id game=game::Id::FF6;Array array;Controller controller{&array};Map map{&controller};
int reads=0;void* party=reinterpret_cast<void*>(10);void* other=reinterpret_cast<void*>(11);
}
namespace game {Id current(){return fixture::game;}}
namespace ref {
Class::Class(const char*n,const char*c,uint8_t g,const char*a):ns(n),name(c),games(g),assembly(a){}
Field::Field(Class&c,const char*n,const char*a):cls(c),name(n),alt(a){}
int32_t Field::get(){++fixture::reads;return 0;}
bool Class::is(void* p){++fixture::reads;return std::strcmp(name,"FieldPartyEntity")==0&&(p==fixture::party||p==fixture::other);}
}
namespace field {
namespace r {ref::Class kFieldMap{"Last.Map","FieldMap"};ref::Field fFieldController{kFieldMap,"fieldController"};}
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
    std::printf("Native parked-party collection: %d failures\n",failures);return failures?1:0;
}
