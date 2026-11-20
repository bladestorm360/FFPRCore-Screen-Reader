namespace {
struct Ff6Step {
    int receipt;
    Objective target;
    int skip_after = -1;
};

constexpr Id ff6_id(int flag) { return static_cast<Id>(static_cast<int>(Id::Ff6Scenario) + flag); }

constexpr Ff6Step ff6_contact(int flag, std::wstring_view label, int map, int entity,
                              Type type = Type::NPC, Kind kind = Kind::Interact, int skip_after = -1)
{
    return {flag, {ff6_id(flag), label, map, {entity, -1, -1}, kind, type}, skip_after};
}

constexpr Ff6Step ff6_event(int flag, std::wstring_view label, int map, int entity,
                            int second = -1, int third = -1)
{
    return {flag, {ff6_id(flag), label, map, {entity, second, third}, Kind::Infer, Type::Event}};
}

constexpr Ff6Step ff6_spot(int flag, std::wstring_view label, int map, baked_route::Cell cell)
{
    return {flag, {ff6_id(flag), label, map, {-1, -1, -1}, Kind::Spot, Type::PointIn, cell}};
}

constexpr auto kMoogleDefense = ff6_contact(14, L"Defend Terra with Locke and the moogles", 318, 9);
constexpr auto kLethe = ff6_event(50, L"Ride the Lethe River and defeat Ultros", 345, 12);
constexpr auto kLockeChoice = ff6_contact(51, L"Choose Locke's scenario", 584, 62);
constexpr auto kBanonChoice = ff6_contact(65, L"Choose Banon and Terra's scenario", 584, 58);
constexpr auto kSabinChoice = ff6_contact(103, L"Choose Sabin's scenario", 584, 61);
constexpr auto kLightPath = ff6_event(68, L"Follow the light through the Narshe mines", 320, 17);
constexpr auto kTrain = ff6_event(123, L"Board and stop the Phantom Train", 355, 1);
constexpr auto kTrainExit = ff6_contact(124, L"Speak to Cyan to leave the Phantom Train", 372, 1);
constexpr auto kTrench = ff6_event(135, L"Find the helmet and take the Serpent Trench to Nikeah", 378, 5);
constexpr Objective kBuyMeat{ff6_id(126), L"Buy Dried Meat in Mobliz", 87, {14, -1, -1},
    Kind::Interact, Type::NPC, {}, L"Buy Dried Meat from the item shop."};
constexpr Objective kFeedGau{ff6_id(592), L"Give Gau Dried Meat on the Veldt", 1, {-1, -1, -1},
    Kind::Spot, Type::PointIn, baked_route::Cell{1, 220, 116},
    L"Win a battle on the Veldt. When Gau appears, use Dried Meat on him."};
constexpr auto kNarsheDefense = ff6_contact(138, L"Protect Banon and defeat Kefka at Narshe", 19, 25);
constexpr auto kOpera = ff6_contact(177, L"Perform at the Opera House and stop Ultros", 112, 21);
constexpr auto kSealedGate = ff6_event(225, L"Open the Sealed Gate", 162, 19, 18, 20);
constexpr auto kPalace = ff6_event(258, L"Attend the Imperial Palace banquet", 165, 27);
constexpr auto kBurningHouse = ff6_contact(275, L"Rescue Relm from the burning house", 180, 95);
constexpr auto kEsperCave = ff6_event(281, L"Find the espers in the cave west of Thamasa", 422, 12);
constexpr auto kFloatingEntrance = ff6_contact(286, L"Fly to the Floating Continent", 119, 9, Type::Entity);
constexpr auto kFloatingEscape = ff6_event(297, L"Defeat Ultima Weapon and escape the Floating Continent", 631, 11);
constexpr auto kSolitaryIsland = ff6_contact(306, L"Help Cid and leave the Solitary Island", 191, 7);
constexpr auto kGerad = ff6_contact(336, L"Follow Gerad and rescue Figaro Castle", 217, 3);
// The Falcon descends at a native vehicle action pad. It uses the same
// landed-action check as the ordinary world-map destination for the tower.
constexpr auto kTowerEntrance = ff6_contact(473, L"Fly the Falcon to Kefka's Tower", 2, 162,
    Type::TransportationEventAction, Kind::Infer);
constexpr auto kFinal = ff6_contact(591, L"Reach Kefka with all three parties and defeat him", 570, 15,
    Type::SwitchEvent, Kind::Spot);

constexpr std::array kNarsheOpening{
    ff6_event(1, L"Break through the first Narshe patrol", 3, 177),
    ff6_event(2, L"Break through the next Narshe patrol", 3, 169, 172),
    ff6_event(3, L"Approach the Narshe mines", 3, 162),
    ff6_event(4, L"Enter the Narshe mines", 3, 155),
    ff6_event(5, L"Advance through the mine guard line", 18, 25),
    ff6_event(6, L"Reach the new mine shaft", 18, 22),
    ff6_event(8, L"Approach the last mine guard post", 632, 13),
    ff6_event(9, L"Defeat Ymir at the frozen esper", 632, 11),
    ff6_event(10, L"Follow Terra after her encounter with the esper", 633, 3),
    ff6_contact(11, L"Speak to Arvis", 17, 15),
    ff6_event(12, L"Escape the search party through the mines", 3, 145),
    ff6_event(13, L"Flee the guards in the mine passage", 637, 14),
    kMoogleDefense
};

constexpr std::array kReturners{
    ff6_contact(15, L"Meet Edgar at Figaro Castle", 34, 19),
    ff6_contact(17, L"Meet Kefka at Figaro Castle", 34, 18),
    ff6_contact(18, L"Ask Kefka about the Imperial campaign", 21, 277),
    ff6_contact(19, L"Ask Kefka about Figaro", 21, 278),
    ff6_contact(20, L"Hear why Kefka has come", 21, 279),
    ff6_contact(21, L"Follow Locke to Terra's room", 21, 304),
    ff6_spot(22, L"Enter Terra's guest room to rest", 28, {1, 103, 29}),
    ff6_contact(23, L"Respond to the fire at Figaro Castle", 30, 27),
    ff6_contact(25, L"Escape from Figaro Castle with Edgar", 21, 262),
    ff6_contact(26, L"Ask the guard to open the cave to South Figaro", 328, 1),
    ff6_event(40, L"Find Vargas on Mount Kolts", 343, 1, 10),
    ff6_contact(41, L"Defeat Vargas and reunite with Sabin", 343, 2),
    ff6_contact(42, L"Meet Banon at the Returners' Hideout", 71, 8),
    ff6_contact(43, L"Speak to Locke about the Returners", 68, 1),
    ff6_contact(44, L"Speak to Sabin about Edgar", 67, 32),
    ff6_contact(45, L"Speak to Edgar before deciding", 71, 11),
    ff6_contact(46, L"Ask the soldier where Banon went", 67, 29),
    ff6_contact(47, L"Tell Banon whether you will join the Returners", 66, 1),
    kLethe
};

constexpr std::array kLockeScenario{
    ff6_contact(54, L"Use Steal to take a soldier's uniform", 38, 108),
    ff6_contact(55, L"Take the cider from the merchant", 43, 7),
    ff6_contact(56, L"Give the cider to the old man", 61, 1),
    ff6_contact(57, L"Give the password to the boy downstairs", 60, 1),
    ff6_event(58, L"Find the secret passage in the mansion", 48, 1, 21),
    ff6_contact(59, L"Free Celes from the dungeon", 50, 11),
    ff6_contact(60, L"Take the Old Clock Key", 50, 6),
    ff6_contact(61, L"Wind the clock to open the passage", 52, 1, Type::Entity),
    ff6_event(62, L"Continue through the secret passage with Celes", 55, 1),
    ff6_event(63, L"Investigate the noise in the cave", 626, 2),
    ff6_event(64, L"Defeat the Tunnel Armor", 626, 3)
};

constexpr std::array kBanonScenario{
    ff6_event(66, L"Approach the Narshe guards", 3, 135, 134, 186),
    ff6_contact(67, L"Twist the stone to open the mine entrance", 3, 132, Type::Entity),
    kLightPath,
    ff6_event(69, L"Bring Banon to Arvis", 17, 13)
};

constexpr std::array kSabinCamp{
    ff6_event(72, L"Reach the Imperial camp outside Doma", 350, 13, 14, 15),
    ff6_contact(81, L"Defeat the Imperial commander as Cyan", 75, 69),
    ff6_event(84, L"Approach General Leo's camp", 350, 25),
    ff6_event(85, L"Follow Kefka through the Imperial camp", 350, 29),
    ff6_contact(86, L"Confront Kefka in the camp", 350, 30),
    ff6_event(88, L"Follow Kefka toward the river", 350, 38),
    ff6_event(89, L"Reach the king of Doma", 76, 1),
    ff6_event(94, L"Help Cyan fight the Imperial soldiers", 350, 77, 78, 102),
    ff6_event(95, L"Escape in Magitek armor with Cyan", 350, 81, 82),
    ff6_event(96, L"Break through the Imperial camp exit", 350, 85),
    ff6_event(97, L"Leave the Imperial camp for the forest", 350, 88, 87, 89)
};

constexpr std::array kSabinVeldt{
    ff6_event(124, L"Approach Baren Falls", 375, 43, 42, 44),
    ff6_event(125, L"Jump down Baren Falls", 375, 27, 26, 28),
    Ff6Step{126, kBuyMeat},
    kTrench
};

constexpr std::array kBalanceJourney{
    ff6_contact(137, L"Form the parties for the defense of Narshe", 19, 23),
    kNarsheDefense,
    ff6_contact(142, L"Take Figaro Castle to Kohlingen", 23, 1, Type::NPC, Kind::Interact, 148),
    ff6_contact(148, L"Find Terra at the top of Zozo", 383, 141),
    ff6_contact(149, L"Speak to Ramuh in Zozo", 395, 1),
    ff6_contact(150, L"Take Ramuh's magicite", 395, 21, Type::AnimEntity),
    ff6_contact(151, L"Take Cait Sith's magicite", 395, 22, Type::AnimEntity),
    ff6_contact(152, L"Take Siren's magicite", 395, 23, Type::AnimEntity),
    ff6_contact(153, L"Take Kirin's magicite", 395, 24, Type::AnimEntity),
    ff6_contact(155, L"Regroup with the others outside the room", 395, 26),
    ff6_contact(156, L"Meet the Impresario in Jidoor", 611, 7),
    ff6_contact(157, L"Read the Impresario's letter", 611, 1, Type::AnimEntity),
    kOpera,
    ff6_contact(178, L"Win Setzer's help aboard the Blackjack", 120, 9),
    ff6_contact(180, L"Talk to the Returner to get into the factory", 145, 3),
    ff6_event(183, L"Follow Kefka into the Magitek Factory", 397, 32, 31, 33),
    ff6_contact(184, L"Defeat Ifrit and Shiva", 398, 13),
    ff6_contact(189, L"Take Ifrit's magicite", 398, 11, Type::AnimEntity),
    ff6_contact(190, L"Take Shiva's magicite", 398, 1, Type::AnimEntity),
    ff6_contact(191, L"Defeat Number 024", 402, 1),
    ff6_contact(192, L"Release the espers from the capsules", 403, 7, Type::Entity),
    ff6_contact(193, L"Operate the factory's emergency switch", 403, 3, Type::Entity),
    ff6_contact(194, L"Speak to Cid to start the mine cart escape", 404, 1),
    ff6_event(195, L"Reach the Blackjack after escaping the factory", 143, 54, 53, 55),
    ff6_contact(196, L"Return to Terra with Maduin's magicite", 155, 32),
    ff6_contact(199, L"Continue Maduin's memory at the gate", 161, 8),
    ff6_contact(203, L"Speak to Terra after her memory returns", 161, 3),
    ff6_event(207, L"Report to the Returners in Narshe", 3, 126, 127),
    kSealedGate,
    ff6_event(226, L"Leave the Sealed Gate and return to the Blackjack", 162, 15, 16, 17),
    kPalace,
    ff6_contact(263, L"Meet General Leo at the Albrook docks", 131, 5),
    ff6_contact(264, L"Rest at the Albrook inn before departure", 130, 9, Type::ShopNPC),
    ff6_contact(266, L"Speak to Terra during the voyage", 623, 16),
    ff6_contact(267, L"Speak to General Leo near Crescent Island", 623, 3),
    ff6_contact(268, L"Disembark with General Leo at Crescent Island", 623, 19),
    ff6_contact(271, L"Ask Strago about the espers in Thamasa", 185, 11),
    ff6_contact(272, L"Rest at the inn in Thamasa", 184, 11),
    kBurningHouse,
    ff6_event(276, L"Speak to Shadow after rescuing Relm", 185, 10),
    kEsperCave,
    ff6_contact(282, L"Confront Kefka in Thamasa", 180, 50),
    kFloatingEntrance,
    kFloatingEscape
};

constexpr std::array kRuinJourney{
    kSolitaryIsland,
    kGerad,
    ff6_contact(339, L"Recruit Setzer in Kohlingen", 263, 1),
    ff6_event(340, L"Enter Darill's Tomb with Setzer", 455, 17),
    ff6_contact(341, L"Operate the first sluice in Darill's Tomb", 458, 1, Type::Entity),
    ff6_contact(342, L"Raise the water in Darill's Tomb", 459, 1, Type::Entity),
    ff6_contact(343, L"Open the passage past the turtle", 463, 24, Type::Entity),
    ff6_contact(345, L"Ride the turtle across the flooded chamber", 463, 1, Type::Entity),
    ff6_contact(352, L"Defeat Dullahan at Darill's tombstone", 468, 4, Type::Entity),
    ff6_event(353, L"Follow Setzer to the Falcon", 468, 5),
    kTowerEntrance,
    kFinal
};

Stage ff6_next(const Ff6Receipts& scenario, std::span<const Ff6Step> steps)
{
    for (const auto& step : steps) {
        if (step.skip_after >= 0 && scenario[step.skip_after] == 1) continue;
        if (!scenario[step.receipt]) return {};
        if (scenario[step.receipt] == 0) return {step.target};
    }
    return {};
}

bool in_range(int map, int first, int last) { return map >= first && map <= last; }

bool ff6_guide_for(Id id, int map)
{
    if (id == kMoogleDefense.target.id) return map == 318;
    if (id == kLethe.target.id) return map == 345 || map == 346 || map == 348;
    if (id == ff6_id(40) || id == ff6_id(41)) return in_range(map, 334, 343);
    if (in_range(static_cast<int>(id), static_cast<int>(ff6_id(54)), static_cast<int>(ff6_id(64))))
        return in_range(map, 38, 65) || in_range(map, 626, 630);
    if (id == kLightPath.target.id || id == ff6_id(69)) return map == 320;
    if (id == kTrain.target.id || id == kTrainExit.target.id) return in_range(map, 355, 372) || map == 624;
    if (id == kTrench.target.id) return in_range(map, 377, 382) || map == 96;
    if (id == kNarsheDefense.target.id) return map == 19;
    if (id == kOpera.target.id) return in_range(map, 112, 118) || map == 576;
    if (id == ff6_id(180)) return map == 143;
    if (id == kSealedGate.target.id) return in_range(map, 405, 411);
    if (id == kPalace.target.id) return in_range(map, 165, 179);
    if (id == kBurningHouse.target.id) return in_range(map, 412, 420);
    if (id == kEsperCave.target.id) return in_range(map, 421, 430);
    if (id == kFloatingEscape.target.id) return map == 431 || map == 631 || map == 432;
    if (id == kSolitaryIsland.target.id) return map == 190 || in_range(map, 191, 193) || map == 597;
    if (id == kGerad.target.id)
        return in_range(map, 214, 251) || in_range(map, 435, 441) || in_range(map, 447, 454);
    if (in_range(static_cast<int>(id), static_cast<int>(ff6_id(341)), static_cast<int>(ff6_id(353))))
        return in_range(map, 456, 468);
    if (id == kFinal.target.id) return in_range(map, 542, 571) || map == 590 || map == 595 || map == 596;
    return false;
}

Stage ff6_scenarios(const Ff6Receipts& scenario, int map)
{
    if (map == 584) {
        if (scenario[64] == 0) return {kLockeChoice.target};
        if (scenario[69] == 0) return {kBanonChoice.target};
        if (scenario[135] == 0) return {kSabinChoice.target};
        return {};
    }
    // Native selection flags are independent and persist after that branch.
    // A completed branch cannot claim the origin of the next one.
    if (scenario[51] == 1 && scenario[64] == 0) return ff6_next(scenario, kLockeScenario);
    if (scenario[65] == 1 && scenario[69] == 0) return ff6_next(scenario, kBanonScenario);
    if (scenario[103] != 1 || scenario[135] != 0 || !scenario[97] || !scenario[123]) return {};
    if (scenario[97] == 0) return ff6_next(scenario, kSabinCamp);
    if (scenario[123] == 0) return {kTrain.target};
    if (map == 372) return {kTrainExit.target};
    return ff6_next(scenario, kSabinVeldt);
}
}

Stage ff6_stage(const Ff6Receipts& scenario, int origin_map, std::optional<bool> has_dried_meat)
{
    if (scenario[591] == 1) return {{}, {}, true};
    if (scenario[0] != 1 || !scenario[50] || !scenario[136] || !scenario[297] || !scenario[473]) return {};
    Stage stage;
    if (scenario[297] == 1) {
        // A resumed Tower save skips the reunion and the earlier Falcon trip.
        if (scenario[473] == 1)
            stage = {ff6_guide_for(kFinal.target.id, origin_map) ? kFinal.target : kTowerEntrance.target};
        else
            stage = ff6_next(scenario, kRuinJourney);
    } else if (scenario[136] == 1) {
        stage = ff6_next(scenario, kBalanceJourney);
    } else if (scenario[50] == 1) {
        stage = ff6_scenarios(scenario, origin_map);
    } else if (scenario[14] == 1) {
        stage = ff6_next(scenario, kReturners);
    } else {
        stage = ff6_next(scenario, kNarsheOpening);
    }
    if (stage.objective) {
        if (stage.objective->id == kBuyMeat.id && has_dried_meat == true) stage.objective = kFeedGau;
        stage.local_guide = ff6_guide_for(stage.objective->id, origin_map);
    }
    return stage;
}
