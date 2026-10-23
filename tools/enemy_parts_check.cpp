// Actual native identity adapter and part policy; native owners are fixtures.
#include "../src/core/battle/enemy_parts.cpp"
#include <cstdio>
#include <cstring>
namespace fixture {game::Id game=game::Id::FF6;struct Enemy{void* master=nullptr;};int monster=0;int id(void* p,const void*){return *static_cast<int*>(p);} }
namespace game {Id current(){return fixture::game;} }
namespace ref {
Class::Class(const char* n,const char* c,uint8_t g,const char* a):ns(n),name(c),games(g),assembly(a){}
Field::Field(Class& c,const char* n,const char* a):cls(c),name(n),alt(a){}
Method::Method(Class& c,const char* n,int a):cls(c),name(n),argc(a){}
int32_t Field::get(){return offsetof(fixture::Enemy,master);}
void* Method::ptr(){return reinterpret_cast<void*>(&fixture::id);}il2cpp::Method* Method::get(){return nullptr;}
void call_failed_note(const char*){std::abort();}
}
namespace modtext {std::wstring T(std::wstring_view value){return std::wstring(value.substr(11));} }
int main() {
    using namespace ffpr::battle::enemy_parts;int failures=0;
    const auto expect=[&](bool ok,const char* why){if(!ok){++failures;std::printf("FAIL %s\n",why);}};
    fixture::Enemy enemy{&fixture::monster};
    for(const auto& part:parts) {
        fixture::monster=part.id;fixture::game=static_cast<game::Id>(part.game);
        expect(qualify(&enemy,L"Boss",part.message)==std::wstring(L"Boss (")+modtext::T(part.label)+L")","each reviewed sprite identity has its own part name");
        expect(qualify(&enemy,L"Boss",L"different message")==L"Boss","master identity cannot label a mismatched or transformed name");
        for(int n=1;n<=6;++n)if(n!=part.game)expect(!descriptor(n,part.id,part.message),"IDs never transfer between games");
    }
    fixture::game=game::Id::FF6;fixture::monster=257;
    expect(qualify(&enemy,L"Ymir",L"MSG_MON_NAME_254")==L"Ymir (Shell)","Ymir257 is the actual shell sprite");
    fixture::monster=309;expect(qualify(&enemy,L"Ymir",L"MSG_MON_NAME_254")==L"Ymir (Head)","Ymir309 is the independently targetable head");
    fixture::game=game::Id::FF5;fixture::monster=388;
    expect(qualify(&enemy,L"Neo Exdeath",L"MSG_MON_NAME_182")==L"Neo Exdeath (Lower right)","Neo Exdeath anatomy follows the actual part script, not formation slot order");
    fixture::monster=511;expect(qualify(&enemy,L"Neo Exdeath",L"MSG_MON_NAME_182")==L"Neo Exdeath","invisible Neo Exdeath script dummy receives no invented anatomy");
    fixture::monster=1;expect(qualify(&enemy,L"Soldier",L"MSG_MON_NAME_01")==L"Soldier","ordinary enemies keep native names");
    enemy.master=nullptr;expect(qualify(&enemy,L"Boss",L"MSG_MON_NAME_254")==L"Boss","missing native master keeps native name");
    std::printf("Boss part identity checks: %d failures\n",failures);return failures?1:0;
}
