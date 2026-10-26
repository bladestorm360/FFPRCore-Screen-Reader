// The shop's party panel, all six games (Last.UI.KeyInput.ShopPartyController and its
// ShopCharaStatusContentController / View, member for member in every dump.cs): the active
// party's equipment comparison for the focused item, which the game draws per character as
// an up or down arrow, an equipped mark and a "cannot equip" text. The number row (1-9) in the
// shop reads slot N: the name as rendered, then the state. Requested by the user on
// 2026-10-10; no old mod read this panel.
//
// Rendered text is read as the game shows it (the equipped mark's text, the impossible text,
// unless they are bare symbols). The arrows are icons, so their words are the mod's
// ("Stronger" / "Weaker"). Native CharacterImageController's current rendered
// animation distinguishes Can equip (win) from Can not equip (Default_00); an
// absent arrow alone cannot distinguish those states. Equipment classification
// comes from the selected Content record, not Weapon/Armor primary-key guesses.
// The panel carries no
// stat numbers: the item's own stat line is the shop's description, read by the I key.
#include <cwctype>
#include <string>
#include <vector>
#include "shop.h"
#include "shop_internal.h"
#include "../core.h"
#include "../displayed_ui.h"
#include "../hooks.h"
#include "../log.h"
#include "../modtext.h"
#include "../refs.h"
#include "../say.h"
#include "../text.h"
#include "../unity.h"
#include "../input/input.h"

namespace ffpr::shop {
namespace {

// The key-input shop (the touch shop has a ShopPartyController; the key-input one keeps the
// panel in its list window): ShopController.listController (ShopListController)
// -> listController (ShopListMainContentController) -> characterContentList.
ref::Class  kShopController{"Last.UI.KeyInput", "ShopController"};
ref::Field  fShopListController{kShopController, "listController"};
ref::Class  kShopList{"Last.UI.KeyInput", "ShopListController"};
ref::Field  fListMain{kShopList, "listController"};
ref::Class  kListMain{"Last.UI.KeyInput", "ShopListMainContentController"};
ref::Field  fCharacterContentList{kListMain, "characterContentList"};
ref::Class  kCharaStatus{"Last.UI.KeyInput", "ShopCharaStatusContentController"};
ref::Field  fCharaView{kCharaStatus, "view"};
ref::Field  fCharacterController{kCharaStatus,"characterController"};
ref::Field  fCharacterData{kCharaStatus,"<Data>k__BackingField"};
ref::Class kCharacterImage{"Last.UI","CharacterImageController"};
ref::Field fAnimation{kCharacterImage,"currentState"},fImageView{kCharacterImage,"view"};
ref::Class kCharacterImageView{"Last.UI","CharacterImageView"};
ref::Field fMainImage{kCharacterImageView,"mainImage"};
ref::Class kOwnedCharacter{"Last.Data.User","OwnedCharacterData"};
ref::Method mGetName{kOwnedCharacter,"get_Name",0};
ref::Class  kCharaView{"Last.UI.KeyInput", "ShopCharaStatusContentView"};
ref::Field  fNameText{kCharaView, "charaNameText"};
ref::Field  fUpImage{kCharaView, "stateUpImage"};
ref::Field  fDownImage{kCharaView, "stateDownImage"};
ref::Field  fEquippedImage{kCharaView, "stateEquippedImage"};
ref::Field  fEquippedText{kCharaView, "stateEquippedText"};
ref::Field  fImpossibleText{kCharaView, "stateImpossibleText"};

constexpr int kSlots = 9;

/// A UI component is drawn: alive, its object active in the hierarchy, the component enabled.
/// A bare symbol ("E", "-") is read as the game's text only when it has a letter or digit.
bool has_letter_or_digit(const std::wstring& s)
{
    for (wchar_t c : s)
        if (std::iswalnum(c)) return true;
    return false;
}

/// The open shop's list window (the owner of the party panel) while it is shown, else null.
void* party_panel()
{
    if (!is_shop_menu_active()) return nullptr;
    void* shop = cached_or_find_controller();
    if (!shop) return nullptr;
    void* list = ref::rd<void*>(shop, fShopListController);
    if (!unity::alive(list)) return nullptr;
    void* main = ref::rd<void*>(list, fListMain);
    if (!unity::alive(main) || !unity::active_in_hierarchy(main)) return nullptr;
    return main;
}

/// Slot `slot` (1-based) of the active party: "Name, Stronger" / "Name, Weaker, Equipped" /
/// "Name, Cannot equip" / "Name, No change".
void announce_slot(int slot)
{
    hooks::guarded("ShopPartyReader.AnnounceSlot", [&] {
        void* party = party_panel();
        if (!party) return;
        void* list = ref::rd<void*>(party, fCharacterContentList);
        const int count = unity::list_count(list);
        if (slot < 1 || slot > count) return;
        void* content = unity::list_at(list, slot - 1);
        if (!unity::alive(content) || !unity::active_in_hierarchy(content)) return;
        void* view = ref::rd<void*>(content, fCharaView);
        if (!view) return;

        std::vector<std::wstring> parts;
        void* portrait=ref::rd<void*>(content,fCharacterController);
        const bool portrait_visible=displayed::image_visible(ref::rd<void*>(ref::rd<void*>(portrait,fImageView),fMainImage));
        std::wstring name = displayed::text(ref::rd<void*>(view, fNameText));
        // KeyInput draws the character/job sprite instead of charaNameText.
        // Its own assigned Data identifies that visible portrait; never use a
        // different party index or disclose a character from a hidden slot.
        if(name.empty()&&portrait_visible) {
            void* data=ref::rd<void*>(content,fCharacterData);
            if(kOwnedCharacter.is(data))name=ref::call_str(mGetName,data);
        }
        if (!name.empty()) parts.push_back(name);

        const std::wstring impossible = displayed::text(ref::rd<void*>(view, fImpossibleText));
        const bool up = displayed::image_visible(ref::rd<void*>(view, fUpImage));
        const bool down = displayed::image_visible(ref::rd<void*>(view, fDownImage));
        const std::wstring equipped_text = displayed::text(ref::rd<void*>(view, fEquippedText));
        const bool equipped = !equipped_text.empty() || displayed::image_visible(ref::rd<void*>(view, fEquippedImage));

        const bool equipment=last_item_content_type()==2 || last_item_content_type()==3;
        const auto animation=equipment&&portrait_visible?ref::rd_str_bounded(portrait,fAnimation,32):std::wstring();
        const bool cannot=!impossible.empty()||animation==L"Default_00";
        if(cannot) parts.push_back(has_letter_or_digit(impossible)?impossible:T(L"Can not equip"));
        else {
            if(animation==L"win")parts.push_back(T(L"Can equip"));
            if(up)parts.push_back(T(L"Stronger"));
            else if(down)parts.push_back(T(L"Weaker"));
            else if(!equipped && animation==L"win")parts.push_back(T(L"No change"));
        }
        if (equipped) parts.push_back(has_letter_or_digit(equipped_text) ? equipped_text : T(L"Equipped"));

        if (parts.empty()) return;
        ffpr::say(text::join(parts, L", "), true);
    });
}

const char* const kActions[kSlots] = {
    "ShopPartyReader.AnnounceSlot1", "ShopPartyReader.AnnounceSlot2", "ShopPartyReader.AnnounceSlot3",
    "ShopPartyReader.AnnounceSlot4", "ShopPartyReader.AnnounceSlot5", "ShopPartyReader.AnnounceSlot6",
    "ShopPartyReader.AnnounceSlot7", "ShopPartyReader.AnnounceSlot8", "ShopPartyReader.AnnounceSlot9",
};
const char* const kDescriptions[kSlots] = {
    "Shop: party member 1, equip comparison", "Shop: party member 2, equip comparison",
    "Shop: party member 3, equip comparison", "Shop: party member 4, equip comparison",
    "Shop: party member 5, equip comparison", "Shop: party member 6, equip comparison",
    "Shop: party member 7, equip comparison", "Shop: party member 8, equip comparison",
    "Shop: party member 9, equip comparison",
};

int install()
{
    for (int i = 0; i < kSlots; ++i) {
        const int slot = i + 1;
        actions::define(kActions[i], [slot] { announce_slot(slot); });
        const auto key = static_cast<input::Key>(static_cast<int>(input::Key::Alpha1) + i);
        input::add_binding(key, input::Mod::None, input::Context::Shop, kActions[i], kDescriptions[i]);
    }
    queries::define("ShopPartyReader.IsPanelShown", [] { return party_panel() != nullptr; });
    return 0;
}

core::Install s_install{"ShopPartyReader", &install, 110};

} // namespace
} // namespace ffpr::shop
