#pragma once
namespace ffpr::ff6::story {
// Actual native cycle notifications; no timer mutation or frame polling.
bool install_sealed_bridges(bool(*wanted)(),void(*pulse)(int mode,int phase));
bool sealed_bridge_phase(int& mode,int& phase); // native owner only, scalar copy
}
