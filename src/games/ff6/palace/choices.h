#pragma once
#include <string>
namespace ffpr::ff6::palace {
// Native owner only. Availability is a current UI occurrence, not guide state.
bool choice_available();
bool choice_enabled();
std::wstring choice_instruction();
int install_choices();
}
