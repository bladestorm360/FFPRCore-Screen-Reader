// check-sources: text.cpp
// Real registered command-commit/cursor/turn producers, native UI boundary fake.
#define main retained_battle_focus_main
#include "battle_focus_check.cpp"
#undef main
namespace fixture {
std::function<void()> committed_body;
void command_committed(void*,void*,const void*){++original_calls;if(committed_body)committed_body();}
}
int main(){
 using namespace fixture;namespace b=ffpr::battle;
 int failures=0;auto expect=[&](bool ok,const char* why){if(!ok){++failures;std::printf("FAIL %s\n",why);}};
 Node command,row,attack,owner,actor;List rows{&row};command.list=&rows;row.data=&attack;attack.name=L"Attack";
 actor.name=L"Rae";actor.index=1;b::o_SetCursor=&command_cursor;
 b::o_SetCommandData=+[](void*,void*,void*,void*,const void*){++fixture::original_calls;};
 const char* expected[]={"<CommandSelectingInit>b__71_0","<CommandSelectingInit>b__71_0","<CommandSelectingInit>b__71_0","<CommandSelectingInit>b__73_0","<CommandSelectingInit>b__72_0","<CommandSelectingInit>b__77_0"};
 for(int n=1;n<=6;++n){
  fixture::game=static_cast<game::Id>(n);battle_active=true;states.clear();b::command_reset_state();
  commit_dispatch=native_commit=&command_committed;commit_hook.clear();b::command_install();
  expect(commit_hook==expected[n-1],"exact game-specific native commit callback is registered");
  for(bool inline_reset:{false,true}){
   b::command_reset_state();spoken.clear();b::d_SetCommandData(&command,&actor,nullptr,nullptr,nullptr);b::d_SetCursor(&command,0,nullptr);
   expect(spoken.size()==2&&spoken[0]==L"Rae's turn"&&spoken[1].starts_with(L"Attack"),"legitimate initial Attack follows its real turn heading");
   spoken.clear();dedup::reset(b::kCommandCtx);const int calls=original_calls;
   committed_body=[&]{if(inline_reset)b::d_SetCursor(&command,0,nullptr);};
   commit_dispatch(&owner,&rows,nullptr);
   expect(original_calls==calls+1+(inline_reset?1:0),"native commit and nested reset run exactly once");
   if(!inline_reset)b::d_SetCursor(&command,0,nullptr);
   for(int i=0;i<4;++i)frame::run_tick();
   expect(spoken.empty()&&!b::g_command_turn_ready,"non-targeting commit cannot speak an orphan Attack even before a delayed handoff");
   actor.name=L"Robyn";actor.index=2;b::d_SetCommandData(&command,&actor,nullptr,nullptr,nullptr);b::d_SetCursor(&command,0,nullptr);frame::run_tick();
   expect(spoken.size()==2&&spoken[0]==L"Robyn's turn"&&spoken[1].starts_with(L"Attack"),"next character opens a fresh command window and reads initial Attack");actor.name=L"Rae";actor.index=1;
   spoken.clear();b::notify_command_submenu_active();b::d_SetCursor(&command,0,nullptr);frame::run_tick();
   expect(spoken.size()==1&&spoken[0].starts_with(L"Attack"),"a real target/submenu cancellation still reads its current command");
   spoken.clear();b::notify_command_submenu_active();b::d_SetCursor(&command,0,nullptr);committed_body={};commit_dispatch(&owner,&rows,nullptr);frame::run_tick();
   expect(spoken.empty(),"commit retires an already-deferred backout cursor read");
  }
  // The original may initialize the next actor synchronously. Closing again in
  // a postfix would erase that new actor's legitimate focus ownership.
  b::command_reset_state();b::g_command_turn_ready=true;spoken.clear();dedup::reset(b::kCommandCtx);
  committed_body=[&]{b::d_SetCursor(&command,0,nullptr);b::d_SetCommandData(&command,&actor,nullptr,nullptr,nullptr);b::d_SetCursor(&command,0,nullptr);};
  commit_dispatch(&owner,&rows,nullptr);
  expect(spoken.size()==2&&spoken[0]==L"Rae's turn"&&spoken[1].starts_with(L"Attack")&&b::g_command_turn_ready,
      "synchronous handoff preserves the new actor's turn and focus");
  b::command_reset_state();b::g_command_turn_ready=true;committed_body=[] {throw 1;};bool threw=false;
  try{commit_dispatch(&owner,&rows,nullptr);}catch(int){threw=true;}
  expect(threw&&!b::g_command_turn_ready,"native exceptions propagate without resurrecting old command ownership");committed_body={};
  std::printf("FF%d exact command-commit/cancel/turn pipeline checked\n",n);
 }
 std::printf("Command commit focus: %d failures\n",failures);return failures?1:0;
}
