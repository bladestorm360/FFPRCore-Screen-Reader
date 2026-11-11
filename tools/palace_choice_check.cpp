// check-sources: text.cpp
// Actual native choice observer and story-controller integration, no game run.
#define FFPR_PALACE_REAL
#define main retained_story_runtime_main
#include "ff6_story_runtime_check.cpp"
#undef main
#include "../src/games/ff6/palace/choices.cpp"
#include <numeric>
namespace palace_fixture {
struct Counter {int32_t clock=1,address=2;};
struct Operands {void* strings=nullptr;};
struct Mnemonic {void* opcode=nullptr;void* operands=nullptr;};
struct Core {void* counter=nullptr;void* instructions=nullptr;void* current=nullptr;};
int originals=0,local_reads=0,cases=0,failures=0;
void check(bool pass,const char* name){++cases;if(!pass){++failures;std::printf("FAIL %s\n",name);}}
int original(void*,const void*){++originals;return 17;}
fixture::RoofName key(std::wstring_view s){fixture::RoofName out;out.length=int(s.size());std::copy(s.begin(),s.end(),out.chars);out.chars[s.size()]=0;return out;}
struct NativeQuestion {
    std::array<fixture::RoofName,6> keys;
    fixture::RoofName msg=key(L"Msg"),wait=key(L"Wait"),select=key(L"Select"),prompt;
    std::vector<void*> options=std::vector<void*>(6),message=std::vector<void*>(1),empty;
    Operands choice_args{&options},msg_args{&message},empty_args{&empty};
    Mnemonic before{&msg,&msg_args},pause{&wait,&empty_args},current{&select,&choice_args};
    std::vector<void*> instructions{&before,&pause,&current};Counter pc;
    Core core{&pc,&instructions,&current};
    void set(std::initializer_list<std::wstring_view> values,std::wstring_view preceding={}) {
        for(size_t i=0;i<6;++i){keys[i]=key(L"");options[i]=nullptr;}
        size_t i=0;for(const auto value:values){keys[i]=key(value);options[i]=&keys[i];++i;}
        prompt=key(preceding);message[0]=&prompt;pc={1,2};core.current=&current;
    }
    void observe(){check(ffpr::ff6::palace::d_select(&core,nullptr)==17,"native Select result is preserved");}
};
}
namespace field::map_state {
std::optional<int> local_value(int map,int index){fixture::native();++palace_fixture::local_reads;return map==fixture::map?fixture::flags.local[size_t(index)]:std::nullopt;}
}
int main(){
    // The retained controller runs with unavailable bindings first; its older
    // guides retain their exact behavior and do not acquire a generic choice.
    if(retained_story_runtime_main())return 1;
    using namespace palace_fixture;using namespace ffpr::ff6::palace;
    fixture::offsets["<programCounter>k__BackingField"]=offsetof(Core,counter);
    fixture::offsets["<clockCount>k__BackingField"]=offsetof(Counter,clock);
    fixture::offsets["<currentAddress>k__BackingField"]=offsetof(Counter,address);
    fixture::offsets["mnemonics"]=offsetof(Core,instructions);
    fixture::offsets["currentInstruction"]=offsetof(Core,current);
    fixture::offsets["mnemonic"]=offsetof(Mnemonic,opcode);
    fixture::offsets["operands"]=offsetof(Mnemonic,operands);
    fixture::offsets["sValues"]=offsetof(Operands,strings);
    check(install_choices()==1,"all named native bindings install one Select observation");o_select=&original;
    fixture::flags.scenario.fill(0);fixture::flags.local.fill(0);fixture::map=177;fixture::choice=true;fixture::ready=false;
    events::fire(events::Kind::MapChanged);NativeQuestion q;
    constexpr std::wstring_view ids[9][3]={
      {L"E0502_00_177_c_01",L"E0502_00_177_c_02",L"E0502_00_177_c_03"},
      {L"E0502_01_177_c_01",L"E0502_01_177_c_02",L"E0502_01_177_c_03"},
      {L"E0502_02_177_c_01",L"E0502_02_177_c_02",L"E0502_02_177_c_03"},
      {L"E0502_03_177_c_01",L"E0502_03_177_c_02",L"E0502_03_177_c_03"},
      {L"E0502_04_177_c_01",L"E0502_04_177_c_02",L"E0502_04_177_c_03"},
      {L"E0502_06_177_c_01",L"E0502_06_177_c_02",L""},
      {L"E0502_07_177_c_01",L"E0502_07_177_c_02",L""},
      {L"E0502_08_177_c_01",L"E0502_08_177_c_02",L""},
      {L"E0504_00_177_c_01",L"E0504_00_177_c_02",L"E0504_00_177_c_03"}};
    constexpr int correct[]{2,0,1,1,0,0,0,0,1};
    const auto ordinal=[](int n){return n==1?L"Choose the first option.":n==2?L"Choose the second option.":L"Choose the third option.";};
    for(int menu=0;menu<9;++menu){const int count=menu>=5&&menu<=7?2:3;std::array<int,3> order{0,1,2};
        do {
            if(count==2)q.set({ids[menu][order[0]],ids[menu][order[1]]});
            else q.set({ids[menu][order[0]],ids[menu][order[1]],ids[menu][order[2]]},menu==4?L"E0502_04_177_a_01":L"");
            q.observe();int expected=0;for(int i=0;i<count;++i)if(order[size_t(i)]==correct[menu])expected=i+1;
            check(choice_available()&&choice_instruction()==ordinal(expected),"actual reordered menu yields its native best answer");
        }while(std::next_permutation(order.begin(),order.begin()+count));
    }
    for(int first=1;first<=3;++first){q.set({ids[4][2],ids[4][0],ids[4][1]},L"E0502_00_177_a_19");q.observe();fixture::flags.local[4]=first;
        check(choice_instruction()==ordinal(first==1?2:first==2?3:1),"remembered question uses actual first-asked local4 and current option order");}
    for(int mask=0;mask<8;++mask){for(int i=1;i<=3;++i)fixture::flags.local[size_t(i)]=(mask>>(i-1))&1;
        q.set({ids[4][0],ids[4][1],ids[4][2]},L"E0502_04_177_a_01");q.observe();int first=1;while(first<=3&&(mask&(1<<(first-1))))++first;
        check(choice_instruction()==ordinal(first==4?1:first),"ask picks a genuinely unasked question for all eight histories");
        q.set({ids[5][0],ids[5][1]});q.observe();check(choice_instruction()==ordinal(mask==7?2:1),"More exits only after all three distinct questions");}
    q.set({ids[4][0],ids[4][1],ids[4][2]});q.observe();check(!choice_available(),"identical question options without their actual prompt never invent asking versus remembering");
    for(auto [prompt,expected]:{std::pair{L"E0504_02_177_a_01",1},std::pair{L"E0504_01_177_a_01",1},std::pair{L"E0505_00_177_a_01",1}}){q.set({},prompt);q.observe();check(choice_instruction()==ordinal(expected),"empty-ID native Yes/No identifies the exact current question");}
    for(int done:{0,1}){fixture::flags.scenario[256]=done;q.set({},L"E0504_03_177_a_01");q.observe();check(choice_instruction()==ordinal(done?1:2),"resume declines until the real optional battle receipt");}
    fixture::flags.scenario[256].reset();check(choice_instruction()==L"Story guidance is waiting for current game state.","unknown battle receipt cannot resume early");
    fixture::map=176;events::fire(events::Kind::MapChanged);q.set({},L"E0474_00_999_a_25");q.observe();
    check(choice_instruction()==ordinal(2),"early tour exit returns to unvisited soldiers");
    for(int i=230;i<=253;++i)fixture::flags.scenario[size_t(i)]=1;
    check(choice_instruction()==ordinal(1),"early finish is advised only after all24 native receipts");
    fixture::flags.scenario[241].reset();check(choice_instruction()==L"Story guidance is waiting for current game state.","unknown soldier receipt does not certify the full tour");
    fixture::nav_guidance=false;const int before_reads=fixture::scalar_reads;check(choice_instruction()==L"Navigation puzzle guidance is off"&&fixture::scalar_reads==before_reads,"disabled tour type reads no flags and reveals no answer");fixture::nav_guidance=true;
    fixture::map=177;events::fire(events::Kind::MapChanged);q.set({ids[0][0],ids[0][1],ids[0][2]});q.observe();
    fixture::other_guidance=false;const int before_locals=local_reads;check(choice_instruction()==L"Non-navigation puzzle guidance is off"&&local_reads==before_locals,"disabled banquet type reads no locals and reveals no answer");fixture::other_guidance=true;
    const auto requests=fixture::requests,reads=fixture::state_reads;ffpr::ff6::story::next(true);
    check(fixture::speech.back()==L"Choose the third option."&&fixture::requests==requests&&fixture::state_reads==reads,"Shift+G owns current banquet advice during blocked field input without route or snapshot");
    const int copied=fixture::string_reads;q.pc.clock=2;
    const auto began=std::chrono::steady_clock::now();for(int i=0;i<10000;++i)d_select(&q.core,nullptr);
    const double waiting_ms=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-began).count();
    check(fixture::string_reads==copied,"ten thousand waiting ticks copy no strings or arrays");
    q.pc={1,-1};q.observe();check(!choice_available(),"invalid native PC retires the prior question");
    q.pc={1,2};q.core.current=&q.before;q.observe();check(!choice_available(),"current-instruction mismatch cannot certify a question");q.core.current=&q.current;
    q.set({ids[0][0],ids[0][0],ids[0][2]});q.observe();check(!choice_available(),"duplicate menu keys fail closed");
    q.set({ids[0][0],ids[0][1],ids[0][2]});q.keys[0].length=32;q.observe();check(!choice_available(),"overlong source key cannot leave a partial answer");
    q.set({ids[0][0],ids[0][1],ids[0][2]});q.options.resize(5);q.observe();check(!choice_available(),"truncated native option array fails closed");q.options.resize(6);
    q.set({ids[0][0],ids[0][1],ids[0][2]});q.observe();events::fire(events::Kind::MapChanging);check(!choice_available(),"map lifetime retires native question ownership");
    fixture::map=1;events::fire(events::Kind::MapChanged);const int outside=fixture::string_reads;d_select(&q.core,nullptr);
    check(fixture::string_reads==outside&&!choice_available(),"unrelated maps perform no question capture");
    check(fixture::wrong_owner==0&&originals>=10000,"all observation is owner-only and preserves every original Select call");
    std::printf("Palace choice:%d assertions/%d failures;10000 native waiting detours %.3fms aggregate, throttled fixture not game responsiveness\n",cases,failures,waiting_ms);return failures?1:0;
}
