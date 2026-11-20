// Included by storyline.cpp. Each step owns an exact native contact and
// saved outcome. Scripted transport uses the current visible vehicle rules.
namespace {
struct Ff4Step {
    int receipt;
    Objective target;
    int skip_after = -1;
};
constexpr Ff4Step ff4_contact(int flag, std::wstring_view label, int map, int entity,
                             Type type = Type::NPC, Kind kind = Kind::Interact, int skip_after = -1)
{
    const auto id = static_cast<Id>(static_cast<int>(Id::Ff4Scenario) + flag);
    return {flag, {id, label, map, {entity, -1, -1}, kind, type}, skip_after};
}
constexpr Ff4Step ff4_event(int flag, std::wstring_view label, int map, int entity, int skip_after = -1)
{
    return ff4_contact(flag, label, map, entity, Type::Event, Kind::Infer, skip_after);
}
constexpr Ff4Step ff4_transfer(int flag, std::wstring_view label, int map, int entity, Kind kind = Kind::Infer)
{
    return ff4_contact(flag, label, map, entity, Type::Warp, kind);
}
// After Edge's parents, the floor initializer moves actor 10 from its
// off-map staging tile to Bottom (21,16). Live routing still checks its identity.
constexpr Ff4Step kRubicante{
    53, {static_cast<Id>(static_cast<int>(Id::Ff4Scenario) + 53), L"Defeat Rubicante", 239,
         {10, -1, -1}, Kind::Interact, Type::NPC, baked_route::Cell{0,21,16}}
};
constexpr std::array kFf4Journey{
    ff4_event(1, L"Meet Rosa in Baron Castle", 9, 7, 3),
    ff4_event(2, L"Meet Cid in Baron Castle", 4, 76, 3),
    ff4_contact(3, L"Rest in Cecil's room", 14, 5),
    ff4_event(6, L"Defeat the Mist Dragon", 162, 22),
    ff4_event(7, L"Enter Mist with Kain", 67, 33),
    ff4_event(8, L"Take Rydia to the inn in Kaipo", 32, 40),
    ff4_event(9, L"Find Rosa in Kaipo", 37, 14),
    ff4_contact(10, L"Meet Tellah in the Underground Waterway", 163, 20),
    ff4_event(12, L"Approach the underground waterfall", 170, 5),
    ff4_contact(13, L"Defeat Octomammoth", 172, 12),
    ff4_event(14, L"Approach Damcyan after the attack", 1, 160),
    ff4_event(15, L"Find Edward in Damcyan", 41, 31),
    ff4_event(16, L"Defeat the Antlion and obtain the Sand Ruby", 177, 11),
    ff4_contact(17, L"Use the Sand Ruby on Rosa", 37, 9),
    ff4_event(18, L"Reach the ice wall on Mount Hobs", 178, 8),
    ff4_event(19, L"Help Yang defeat Mom Bomb", 180, 6),
    ff4_event(20, L"Speak to the king and defend Fabul", 47, 31),
    ff4_transfer(21, L"Arrange passage with the king of Fabul", 49, 10),
    ff4_event(22, L"Depart from Fabul's harbor", 1, 148),
    ff4_contact(23, L"Speak to the elder of Mysidia", 58, 19),
    ff4_event(24, L"Enter Mount Ordeals", 182, 25),
    ff4_event(25, L"Meet Tellah on Mount Ordeals", 183, 5),
    ff4_event(26, L"Defeat Scarmiglione on Mount Ordeals", 185, 9),
    ff4_event(27, L"Defeat Scarmiglione again", 185, 5),
    ff4_contact(28, L"Enter the mirror chamber and become a paladin", 185, 4, Type::Entity),
    ff4_contact(29, L"Return to the elder of Mysidia", 58, 15),
    ff4_event(30, L"Meet Yang at the Baron inn", 66, 5),
    ff4_contact(31, L"Use the Baron Key to open the waterway", 25, 19),
    ff4_event(32, L"Defeat Baigan in Baron Castle", 5, 24),
    ff4_contact(33, L"Confront Cagnazzo in the throne room", 8, 7),
    ff4_event(34, L"Receive the Twin Harp from Edward", 100, 22),
    ff4_event(100, L"Enter the magnetic cavern without metal equipment", 192, 8, 97),
    ff4_contact(97, L"Challenge the Dark Elf with the Twin Harp", 201, 5),
    ff4_contact(36, L"Defeat the Dark Elf", 201, 4),
    ff4_contact(98, L"Take the Earth Crystal", 201, 3, Type::AnimEntity),
    ff4_event(37, L"Thank Edward for his help", 100, 10, 38),
    ff4_event(38, L"Return the Earth Crystal to Troia", 98, 36),
    ff4_contact(39, L"Travel to the Tower of Zot with Kain", 1, 135, Type::Entity),
    ff4_event(40, L"Defeat the Magus Sisters", 206, 29),
    ff4_event(41, L"Confront Golbez in the Tower of Zot", 208, 12),
    ff4_contact(42, L"Speak to Kain and rescue Rosa", 208, 10),
    ff4_event(43, L"Defeat Barbariccia", 209, 35),
    ff4_contact(44, L"Use the Magma Rock in the Agart well", 109, 3, Type::Entity),
    ff4_contact(45, L"Fly the Enterprise down into the underworld", 1, 1030, Type::TransportationEventAction, Kind::Infer),
    ff4_event(46, L"Defend the Crystal in the Dwarven Castle", 120, 17),
    ff4_event(47, L"Defeat Dr. Lugae in the Tower of Babel", 225, 6),
    ff4_contact(273, L"Use Dr. Lugae's Key at the cannon room", 220, 28, Type::Entity, Kind::Interact, 48),
    ff4_event(48, L"Stop the super cannon with Yang", 221, 2),
    ff4_event(49, L"Escape the Tower of Babel", 210, 10),
    ff4_contact(50, L"Ask the Baron engineers to upgrade the Enterprise", 4, 58),
    ff4_event(51, L"Meet Edge in the Cave of Eblan", 228, 19),
    ff4_event(99, L"Enter the Tower of Babel with Edge", 234, 9, 52),
    ff4_event(52, L"Approach Edge's parents", 239, 15),
    kRubicante,
    ff4_transfer(54, L"Follow the lower passage in the Tower of Babel", 321, 6),
    ff4_transfer(55, L"Take the Falcon", 242, 14),
    ff4_event(56, L"Receive Luca's Necklace from King Giott", 120, 16),
    ff4_event(57, L"Have Cid prepare the Falcon for lava", 126, 11),
    ff4_contact(58, L"Use Luca's Necklace at the Sealed Cavern", 268, 6, Type::Entity),
    ff4_contact(59, L"Retrieve the Dark Crystal", 290, 4, Type::AnimEntity),
    ff4_event(60, L"Defeat the Demon Wall", 289, 5),
    ff4_event(61, L"Leave the Sealed Cavern with Kain", 268, 5),
    ff4_event(62, L"Have Cid fit the Falcon's drill", 120, 15),
    ff4_contact(63, L"Drill to the surface with the Falcon", 2, 52, Type::TransportationEventAction, Kind::Infer),
    ff4_event(64, L"Raise the Lunar Whale in Mysidia", 57, 45),
    ff4_contact(108, L"Use the Lunar Whale crystal to reach the Moon", 357, 1, Type::Warp, Kind::Interact, 65),
    ff4_event(65, L"Meet FuSoYa at the Crystal Palace", 153, 12),
    ff4_contact(66, L"Use the Lunar Whale crystal to return to the surface", 161, 14, Type::AnimEntity),
    ff4_event(67, L"Defeat the archfiends inside the Giant of Babel", 248, 5),
    ff4_event(68, L"Destroy the Giant of Babel's CPU", 249, 3),
    ff4_transfer(69, L"Enter the Lunar Subterrane from the Crystal Palace", 154, 13),
    ff4_event(70, L"Face Zeromus", 317, 3)
};
}

Stage ff4_stage(const Ff4Receipts& scenario)
{
    if (scenario[70] == 1) return {{}, {}, true};
    if (scenario[0] != 1) return {};
    for (const auto& step : kFf4Journey) {
        if (step.skip_after >= 0 && scenario[step.skip_after] == 1) continue;
        if (!scenario[step.receipt]) return {};
        if (scenario[step.receipt] == 1) continue;
        return {step.target};
    }
    return {};
}
