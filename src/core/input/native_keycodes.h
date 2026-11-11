#pragma once
#include "input.h"
// Factual Unity key values agree across all six current engine declarations.
// F16–F24 have no legacy Unity equivalent and cannot collide with KeyConfig.
namespace input::native_keyboard {
inline constexpr int kUnityKeys[]={
    97,98,99,100,101,102,103,104,105,106,107,108,
    109,110,111,112,113,114,115,116,117,118,119,120,
    121,122,48,49,50,51,52,53,54,55,56,57,
    282,283,284,285,286,287,288,289,273,274,276,275,
    13,27,8,9,32,278,279,304,303,306,305,308,
    307,311,312,45,61,91,93,92,59,39,96,44,
    46,47,290,291,292,293,294,295,296,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,277,127,280,281,256,257,
    258,259,260,261,262,263,264,265,268,270,269,266,
    267,
};
static_assert(sizeof(kUnityKeys)/sizeof(kUnityKeys[0])==static_cast<int>(Key::Count));
}
