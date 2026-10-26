#pragma once
#include "modtext.h"
#include "game.h"

// Shared names for actual native conditions whose master has no message key.
// Native MesIdName remains authoritative. This table names effects, never AI,
// target-selection or equipment bookkeeping flags from the common enum.
namespace condition_labels {
inline std::wstring fallback(int type) {
    switch (type) {
    case 4: return T(L"Critical");
    case 5: return T(L"KO");
    case 6: return T(L"Silence");
    case 7: return T(L"Sleep");
    case 8: return T(L"Paralysis");
    case 9: return T(L"Blind");
    case 10: return T(L"Poison");
    case 11: return T(L"Stone");
    case 12: return T(L"Confusion");
    case 13: case 604: return T(L"Invisible");
    case 14: case 33: return T(L"Blink");
    case 15: return T(L"Haste");
    case 16: return T(L"Slow");
    case 17: return T(L"Stop");
    case 19: return T(L"Protect");
    case 25: return T(L"Regen");
    case 32: return T(L"Aging");
    case 34: return T(L"Zombie");
    // FFII Imbibe applies the unnamed Salmon condition. Its native parameter
    // group raises Attack and lowers Evasion; the raw enum is not a player label.
    // Only FFII's master uses it. Keep the existing confirmed-add/removal and
    // rendered-status producers; do not infer an effect from the action name.
    case 203: return game::is(game::Id::FF2) ? T(L"Attack up, Evasion down") : std::wstring();
    // FFII's developer Venom is the persistent Poison. Its game catalog
    // overrides this key separately from type10/MSG_SYSTEM_042 (Venom).
    case 204: return T(L"Venom");
    case 205: return T(L"Amnesia"); // FF2 Forget; Fog applies this condition.
    case 401: return T(L"Mini");
    case 402: return T(L"Toad");
    case 403: return T(L"Pig");
    case 404: case 609: return T(L"Doom");
    case 405: return T(L"Gradual Petrify");
    case 406: return T(L"Curse");
    case 407: return game::is(game::Id::FF2) ? std::wstring() : T(L"Sap"); // Slip, not FF2's Double enum alias.
    case 409: return T(L"Float");
    case 410: case 206: return T(L"Berserk");
    case 412: return T(L"Shell");
    case 413: return T(L"Reflect");
    case 601: return T(L"Imp");
    case 603: return T(L"Frozen");
    default: return {};
    }
}
}
