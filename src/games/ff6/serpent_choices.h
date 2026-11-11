#pragma once
#include <string>
namespace ffpr::ff6::serpent {
// Native owner only; the exact displayed arrow occurrence owns availability.
bool choice_available();
std::wstring choice_instruction();
int install_choices();
}
