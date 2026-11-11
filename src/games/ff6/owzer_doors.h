#pragma once
namespace ffpr::ff6::story {
// Exact native one-second door producer. The caller owns intent/cancellation;
// this observer copies no pointer and neither changes nor polls the timer.
bool install_owzer_doors(bool(*wanted)(),void(*opened)());
}
