// Contact order follows the three world epochs. Once the native plot moves
// the party to a later world, an earlier world's missed advice is obsolete.
namespace {
struct Ff5Step {
    int receipt;
    Objective target;
    int skip_after = -1;
};
constexpr Id ff5_id(int flag) { return static_cast<Id>(static_cast<int>(Id::Ff5Scenario) + flag); }
constexpr Ff5Step ff5_contact(int flag, std::wstring_view label, int map, int entity,
                             Type type = Type::NPC, Kind kind = Kind::Interact, int skip_after = -1)
{
    return {flag, {ff5_id(flag), label, map, {entity, -1, -1}, kind, type}, skip_after};
}
constexpr Ff5Step ff5_event(int flag, std::wstring_view label, int map, int entity, int skip_after = -1)
{
    return ff5_contact(flag, label, map, entity, Type::Event, Kind::Infer, skip_after);
}
constexpr Ff5Step ff5_warp(int flag, std::wstring_view label, int map, int entity, Kind kind = Kind::Infer)
{
    return ff5_contact(flag, label, map, entity, Type::Warp, kind);
}
constexpr Ff5Step ff5_spot(int flag, std::wstring_view label, int map, baked_route::Cell cell)
{
    return {flag, {ff5_id(flag), label, map, {-1, -1, -1}, Kind::Spot, Type::PointIn, cell}};
}
constexpr Ff5Step kBoko{4, {ff5_id(4), L"Return to Boko", 189, {12,-1,-1}, Kind::Interact,
                              Type::NPC, baked_route::Cell{1,16,21}}};
constexpr std::array kFirstWorld{
    ff5_event(1, L"Approach the Tycoon meteorite", 189, 11),
    ff5_event(2, L"Rescue Lenna from the goblins", 189, 8),
    ff5_contact(3, L"Find Galuf at the meteorite", 189, 15),
    kBoko,
    ff5_warp(5, L"Ride Boko into the Western Valley", 1, 17),
    ff5_event(417, L"Jump the gaps with Boko", 577, 15, 6),
    ff5_event(418, L"Defeat the first goblin group", 577, 13, 6),
    ff5_event(419, L"Defeat the second goblin group", 577, 14, 6),
    ff5_warp(6, L"Leave the valley with Lenna and Galuf", 577, 24),
    ff5_warp(7, L"Enter the cave to the Pirates' Hideout", 191, 8),
    ff5_event(8, L"Find the hidden switch in the cave", 193, 8),
    ff5_event(9, L"Follow the ship through the cave", 194, 7),
    ff5_event(10, L"Enter the Pirates' Hideout", 195, 24, 12),
    ff5_contact(12, L"Board the pirates' ship", 198, 23, Type::Entity),
    ff5_event(14, L"Speak to the guards at the Wind Shrine", 202, 19),
    ff5_contact(15, L"Defeat Wing Raptor", 205, 17, Type::AnimEntity),
    ff5_event(16, L"Reach the Wind Crystal", 209, 6),
    ff5_event(20, L"Find Zok's house in Tule", 30, 64, 21),
    ff5_event(21, L"Get the canal key from Zok", 39, 14),
    ff5_warp(22, L"Return to the ship at Tule", 30, 58),
    ff5_contact(24, L"Open Torna Canal and face Karlabos", 210, 4, Type::Entity),
    ff5_event(25, L"Continue into the Ship Graveyard", 210, 3),
    ff5_event(26, L"Cross the flooded passage", 213, 7, 29),
    ff5_event(27, L"Rest aboard the wrecked ship", 220, 9, 29),
    ff5_contact(28, L"Open the passage through the wrecks", 211, 17, Type::Entity),
    ff5_contact(29, L"Defeat Siren in the Ship Graveyard", 211, 109, Type::Entity),
    ff5_contact(30, L"Ask about Walse in Carwen", 40, 88, Type::NPC, Kind::Interact, 32),
    ff5_contact(31, L"Ask about the wind drake in Carwen", 47, 6, Type::NPC, Kind::Interact, 32),
    ff5_event(32, L"Defeat Magissa and Forza on North Mountain", 232, 1),
    ff5_warp(33, L"Help the wind drake on North Mountain", 233, 1),
    ff5_warp(37, L"Speak to the king of Walse", 55, 20),
    ff5_event(38, L"Defeat Garula at the Water Crystal", 243, 19),
    ff5_contact(423, L"Take the Berserker shard", 243, 30, Type::AnimEntity, Kind::Interact, 39),
    ff5_contact(424, L"Take the Red Mage shard", 243, 31, Type::AnimEntity, Kind::Interact, 39),
    ff5_contact(425, L"Take the Summoner shard", 243, 32, Type::AnimEntity, Kind::Interact, 39),
    ff5_contact(426, L"Take the Time Mage shard", 243, 33, Type::AnimEntity, Kind::Interact, 39),
    ff5_contact(427, L"Take the Mystic Knight shard", 243, 22, Type::AnimEntity, Kind::Interact, 39),
    ff5_contact(39, L"Speak to the wounded king of Walse", 57, 7),
    ff5_warp(40, L"Use the Walse meteorite to reach Karnak", 246, 2),
    ff5_contact(41, L"Visit the weapon shop in Karnak", 88, 20),
    ff5_contact(42, L"Speak to Cid in the Karnak jail", 101, 2),
    ff5_event(43, L"Approach Karnak Castle", 95, 99, 44),
    ff5_warp(44, L"Enter the Fire-Powered Ship with Cid", 250, 28),
    ff5_event(45, L"Defeat Liquid Flame", 271, 1),
    ff5_event(46, L"Approach the Fire Crystal", 104, 21),
    ff5_contact(245, L"Fight through the Karnak courtyard", 95, 29, Type::Event, Kind::Spot, 47),
    ff5_warp(47, L"Escape Karnak Castle before the explosion", 95, 55, Kind::Transition),
    ff5_contact(48, L"Obtain Ifrit in the Library of the Ancients", 279, 4, Type::Entity),
    ff5_event(49, L"Defeat Byblos at the library", 283, 7),
    ff5_contact(50, L"Speak to Mid at the library", 283, 1),
    ff5_event(51, L"Find Cid in the Karnak pub", 92, 33),
    ff5_event(52, L"Have Cid and Mid repair the ship", 250, 8),
    ff5_contact(53, L"Board the repaired Fire-Powered Ship", 250, 27),
    ff5_event(54, L"Approach the harbor in Crescent", 123, 209),
    ff5_event(55, L"Investigate the sinking ship in Crescent", 123, 44),
    ff5_contact(56, L"Ask the scholar about black chocobos", 130, 21, Type::NPC, Kind::Interact, 58),
    ff5_contact(57, L"Speak to the teacher in the village", 131, 3, Type::NPC, Kind::Interact, 58),
    ff5_contact(58, L"Meet the black chocobo in the forest", 129, 111),
    ff5_event(59, L"Defeat the Sandworm and cross the desert", 286, 65),
    ff5_event(60, L"Enter Gohn", 287, 5),
    ff5_warp(61, L"Follow King Tycoon through Gohn", 287, 10),
    ff5_event(62, L"Reach the warp device beneath Gohn", 289, 8),
    ff5_contact(63, L"Use the control at the Catapult entrance", 292, 11, Type::Entity),
    ff5_event(64, L"Find the airship in the Catapult", 295, 8),
    ff5_contact(65, L"Use the airship to leave the Catapult", 298, 5),
    ff5_event(66, L"Approach the rising Ronka Ruins", 1, 132),
    ff5_event(67, L"Ask Cid and Mid about reaching Ronka", 296, 45),
    ff5_event(68, L"Return to the Tycoon meteorite with Galuf", 189, 7),
    ff5_contact(69, L"Collect the adamantite", 190, 4, Type::AnimEntity),
    ff5_event(70, L"Bring adamantite to Cid and Mid", 296, 22),
    ff5_spot(71, L"Raise the airship to Ronka's altitude", 1, {1,62,158}),
    ff5_contact(72, L"Defeat the Soul Cannon", 299, 10, Type::Entity),
    ff5_contact(73, L"Reach King Tycoon in the Ronka Ruins", 305, 1),
    ff5_contact(76, L"Read Cid's note at the Catapult", 293, 32, Type::Entity),
    ff5_event(77, L"Meet Cid at the Tycoon meteorite", 189, 27),
    ff5_contact(78, L"Visit the Walse meteorite with Cid", 244, 5),
    ff5_contact(79, L"Meet Cid and Mid at the Karnak meteorite", 249, 5),
    ff5_contact(80, L"Clear the monster from the Karnak meteorite", 247, 11, Type::Entity),
    ff5_contact(81, L"Meet Cid and Mid at the last meteorite", 307, 26),
    ff5_event(82, L"Clear the last meteorite", 309, 3),
    ff5_warp(83, L"Follow Galuf to the second world", 1, 138)
};
constexpr std::array kSecondWorld{
    ff5_spot(84, L"Use a tent or cottage on the island", 12, {1,206,110}),
    ff5_event(85, L"Rescue the party from Exdeath's prison", 316, 24),
    ff5_event(86, L"Fight through the Big Bridge", 320, 8),
    ff5_event(87, L"Defeat Gilgamesh on the Big Bridge", 322, 8),
    ff5_event(88, L"Cross the rest of the Big Bridge", 320, 2),
    ff5_contact(89, L"Find the inn in Regole", 146, 6, Type::NPC, Kind::Interact, 90),
    ff5_contact(90, L"Follow the moogle in the forest", 324, 3),
    ff5_event(91, L"Reach the fallen moogle", 325, 43),
    ff5_event(92, L"Rescue the moogle", 325, 44),
    ff5_event(93, L"Enter the Moogle Village", 136, 11, 94),
    ff5_contact(94, L"Speak to the rescued moogle", 136, 29),
    ff5_contact(95, L"Meet Krile in the Moogle Village", 136, 37),
    ff5_contact(96, L"Ask Krile about the wind drake", 167, 161),
    ff5_contact(97, L"Ask the Bal guards to open the gate", 167, 172),
    ff5_warp(98, L"Enter Quelb on the way to Drakenvale", 12, 17),
    ff5_event(99, L"Speak to Kelger in Quelb", 165, 1),
    ff5_event(100, L"Defeat the dragon grass and collect it", 342, 1),
    ff5_event(101, L"Return to the closed gates of Bal", 167, 187),
    ff5_event(102, L"Visit Krile in the castle", 175, 53),
    ff5_contact(103, L"Heal and board the wind drake", 167, 201),
    ff5_event(104, L"Fly to the island where Ghido lives", 12, 66),
    ff5_contact(105, L"Speak to the guards at Castle Surgate", 153, 16, Type::Entity),
    ff5_contact(106, L"Land the wind drake on Xezat's fleet", 12, 71, Type::TransportationEventAction, Kind::Infer),
    ff5_event(107, L"Rest below deck on Xezat's ship", 351, 1),
    ff5_contact(108, L"Meet Xezat on deck during the attack", 347, 1),
    ff5_contact(109, L"Defeat Gilgamesh on Xezat's ship", 347, 34),
    ff5_contact(110, L"Take Xezat's submarine", 604, 8),
    ff5_contact(112, L"Follow Xezat into the Barrier Tower", 353, 10),
    ff5_event(113, L"Take the Whisperweed at the Barrier Tower", 354, 11),
    ff5_event(114, L"Defeat the guardian at the Barrier Tower", 362, 11),
    ff5_contact(115, L"Destroy the barrier generator", 367, 10, Type::Entity),
    ff5_warp(116, L"Escape the Barrier Tower", 353, 16),
    ff5_event(117, L"Meet Ghido and receive the Guardian Branch", 375, 1),
    ff5_event(118, L"Enter the Great Forest of Moore", 377, 45),
    ff5_event(119, L"Reach the burning part of the forest", 377, 41),
    ff5_event(120, L"Reach the Guardian Tree", 383, 19),
    ff5_event(121, L"Confront Exdeath inside the Guardian Tree", 384, 8),
    ff5_contact(122, L"Speak to Krile after the battle", 384, 9),
    ff5_event(123, L"Return to Castle Exdeath", 319, 1),
    ff5_event(124, L"Face Gilgamesh in Castle Exdeath", 403, 12),
    ff5_event(125, L"Defeat Exdeath in his castle", 406, 1)
};
constexpr std::array kMergedWorld{
    ff5_contact(127, L"Speak to Krile in Tycoon", 70, 166),
    ff5_warp(129, L"Leave Tycoon Castle with Krile", 70, 168),
    ff5_warp(130, L"Find Boko near the Pirates' Hideout", 191, 14),
    ff5_warp(131, L"Cross the western valley with Boko", 407, 2),
    ff5_warp(132, L"Escape the pit with Faris", 408, 20),
    ff5_event(133, L"Meet Ghido beneath the merged world", 630, 10),
    ff5_event(134, L"Find the Guardian Tree in the merged world", 409, 16),
    ff5_contact(136, L"Open the Pyramid of Moore", 410, 17, Type::Entity),
    ff5_contact(137, L"Retrieve the first tablet in the Pyramid", 426, 3, Type::Entity),
    ff5_warp(138, L"Rescue Lenna at the Guardian Tree", 409, 18),
    // The first boarding of ship 17 runs the Void montage and restores the
    // airship. Its final scene opens the actual Rift contact group.
    Ff5Step{139, {ff5_id(139), L"Board the ship to recover the airship", 19, {-1, -1, -1},
                  Kind::Spot, Type::PointIn, baked_route::Cell{1,87,105}, {}, 17}},
    ff5_event(141, L"Enter the Island Shrine", 427, 1),
    ff5_contact(142, L"Retrieve the second tablet in the Island Shrine", 442, 1, Type::Entity),
    ff5_warp(143, L"Split the party at Fork Tower", 443, 1),
    ff5_warp(144, L"Climb the first tower and prepare for both battles", 444, 1),
    ff5_contact(145, L"Claim Holy and Flare at the same time", 452, 1, Type::Entity),
    ff5_event(146, L"Have Cid upgrade the airship at the Catapult", 292, 12),
    ff5_event(147, L"Enter the Great Sea Trench", 462, 7),
    ff5_contact(148, L"Retrieve the third tablet in the Great Sea Trench", 474, 1, Type::Entity),
    ff5_event(149, L"Enter Istory Falls", 476, 1),
    ff5_contact(150, L"Retrieve the fourth tablet at Istory Falls", 484, 4, Type::Entity),
    ff5_warp(153, L"Fly into the Interdimensional Rift", 19, 112, Kind::AirshipTransition)
};
constexpr std::array kRift{
    ff5_event(154, L"Proceed through the Rift desert", 525, 10),
    ff5_contact(156, L"Examine the book and defeat Apanda", 551, 27, Type::Entity),
    ff5_contact(157, L"Defeat Catastrophe in the Rift castle", 560, 40),
    ff5_event(158, L"Defeat Halicarnassus", 563, 1),
    ff5_event(159, L"Defeat Twintania", 553, 39),
    ff5_event(161, L"Face Exdeath and Neo Exdeath", 569, 3)
};
}

Stage ff5_stage(const Ff5Receipts& scenario)
{
    if (scenario[161] == 1) return {{}, {}, true};
    if (scenario[0] != 1 || !scenario[83] || !scenario[125] || !scenario[153]) return {};
    std::span<const Ff5Step> journey = kFirstWorld;
    if (scenario[153] == 1) journey = kRift;
    else if (scenario[125] == 1) journey = kMergedWorld;
    else if (scenario[83] == 1) journey = kSecondWorld;
    for (const auto& step : journey) {
        if (step.skip_after >= 0 && scenario[step.skip_after] == 1) continue;
        if (!scenario[step.receipt]) return {};
        if (scenario[step.receipt] == 1) continue;
        return {step.target};
    }
    return {};
}
