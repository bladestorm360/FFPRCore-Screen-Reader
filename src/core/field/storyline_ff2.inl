// Included by storyline.cpp. This is the recommended native FFII quest
// order. Scenario receipts and learned keywords are independent banks.
namespace {
struct Ff2Step {
    int receipt;
    Objective target;
    bool keyword = false;
    int skip_after = -1;
};

constexpr Ff2Step ff2_contact(int flag, std::wstring_view label, int map, int entity,
                             Type type = Type::NPC, Kind kind = Kind::Interact, int skip_after = -1)
{
    return {flag, {static_cast<Id>(static_cast<int>(Id::Ff2Scenario) + flag), label, map,
                  {entity, -1, -1}, kind, type}, false, skip_after};
}
constexpr Ff2Step ff2_learn(int content_id, std::wstring_view label, int map, int entity, int skip_after)
{
    const auto id = static_cast<Id>(static_cast<int>(Id::Ff2Keyword) + content_id - kFirstFf2Keyword);
    return {content_id, interact(id, label, map, entity), true, skip_after};
}

constexpr std::array kFf2Journey{
    ff2_learn(917, L"Learn Wild Rose from Hilda", 6, 49, 4),
    ff2_contact(4, L"Tell the Fynn barkeep Wild Rose", 26, 41),
    ff2_contact(5, L"Speak to Scott in the secret room", 27, 12),
    ff2_contact(6, L"Show Scott's ring to Hilda", 6, 49, Type::NPC, Kind::Interact, 8),
    ff2_learn(918, L"Learn Mythril from Hilda", 6, 49, 8),
    ff2_contact(8, L"Ask Hilda about Mythril", 6, 49),
    ff2_contact(9, L"Recruit Minwu in Altair", 6, 29),
    ff2_contact(11, L"Free Paul and the captives at Semitt Falls", 132, 11),
    ff2_contact(12, L"Defeat the sergeant at Semitt Falls", 135, 8),
    ff2_contact(13, L"Take the mythril from the chest", 135, 6, Type::TreasureBox),
    ff2_contact(14, L"Give the mythril to Tobul", 13, 9),
    ff2_learn(919, L"Learn Dreadnought from Hilda", 6, 5, 15),
    ff2_contact(15, L"Ask the Bafsk agent about Dreadnought", 67, 68),
    ff2_contact(16, L"Reach the Dreadnought launch site", 137, 13),
    ff2_contact(17, L"Take the pass from the chest", 138, 7, Type::TreasureBox, Kind::Interact, 33),
    ff2_contact(19, L"Return to Hilda after the launch", 326, 44),
    ff2_learn(921, L"Learn Airship from Cid", 116, 7, 21),
    ff2_learn(923, L"Ask Cid about Airship and learn Sunfire", 116, 7, 21),
    ff2_learn(922, L"Ask the king about Sunfire and learn Goddess's Bell", 328, 10, 21),
    ff2_contact(21, L"Ask Josef about Goddess's Bell", 60, 21),
    ff2_contact(22, L"Ask Josef to open the hidden storeroom", 118, 14, Type::Entity),
    ff2_contact(23, L"Take the snowcraft from the chest", 119, 4, Type::TreasureBox),
    ff2_contact(24, L"Defeat the adamantoise in Snow Cavern", 147, 15),
    ff2_contact(25, L"Take the Goddess's Bell from the wall", 147, 14, Type::Entity),
    ff2_contact(26, L"Face Borghen on the way out", 139, 12),
    ff2_contact(27, L"Use the Goddess's Bell at Kashuan Keep", 149, 30, Type::Entity),
    ff2_contact(28, L"Recruit Gordon at Kashuan Keep", 156, 12),
    ff2_contact(29, L"Defeat Red Soul", 150, 9),
    ff2_contact(30, L"Take Egil's Torch", 150, 6, Type::TreasureBox),
    ff2_contact(31, L"Use Egil's Torch to collect Sunfire", 149, 56, Type::Entity),
    ff2_contact(32, L"Leave Kashuan Keep with the Sunfire", 149, 3, Type::GotoMap, Kind::Transition),
    ff2_contact(33, L"Get past the Dreadnought guard", 164, 42),
    ff2_contact(100, L"Open the captive cell's iron bars", 169, 28, Type::AnimEntity, Kind::Interact, 34),
    ff2_contact(34, L"Rescue Hilda and Cid from the Dreadnought", 169, 17),
    ff2_contact(35, L"Use the Sunfire on the Dreadnought engine", 175, 12, Type::Entity),
    ff2_contact(36, L"Visit the king in Altair", 325, 35, Type::Event, Kind::Infer),
    ff2_learn(926, L"Learn Dragoons from Gordon", 326, 54, 39),
    ff2_contact(39, L"Arrange passage to Deist with Leila", 112, 54),
    ff2_contact(40, L"Board Leila's ship", 1, 175, Type::Entity),
    ff2_contact(41, L"Meet the child at Castle Deist", 176, 32),
    ff2_contact(42, L"Speak to the child again", 176, 32),
    ff2_learn(927, L"Learn Wyverns from the child's mother", 179, 28, 44),
    ff2_contact(43, L"Get the pendant in Deist Cavern", 184, 32, Type::Entity),
    ff2_contact(44, L"Ask the wyvern about Wyverns", 180, 6),
    ff2_contact(45, L"Defeat the chimeras by the Life Spring", 187, 34),
    ff2_contact(46, L"Place the wyvern egg in the Life Spring", 187, 41, Type::Entity),
    ff2_contact(47, L"Report to Hilda in Altair", 329, 5),
    ff2_contact(48, L"Enter the Coliseum and defeat the behemoth", 191, 19, Type::Event, Kind::Infer),
    ff2_contact(49, L"Examine the Coliseum cell gate", 192, 29, Type::Entity),
    ff2_contact(50, L"Rescue Hilda from the Coliseum", 195, 8, Type::Entity),
    ff2_contact(51, L"Meet Leila at the rebel camp", 31, 98),
    ff2_contact(52, L"Retake Castle Fynn from Gottos", 43, 25),
    ff2_learn(924, L"Learn Mysidia from Hilda", 43, 23, 56),
    ff2_learn(930, L"Ask Hilda about Mysidia and learn Mask", 43, 23, 56),
    ff2_learn(931, L"Ask Hilda about Mask and learn Ekmet Teloess", 43, 23, 56),
    ff2_contact(56, L"Use Ekmet Teloess at the secret wall", 43, 22, Type::Entity),
    ff2_contact(57, L"Take the White Mask beneath Castle Fynn", 75, 7, Type::TreasureBox),
    ff2_contact(58, L"Place the White Mask on the Mysidia statue", 98, 4, Type::Entity),
    ff2_contact(59, L"Defeat the guardian of the Black Mask", 205, 9),
    ff2_contact(60, L"Take the Black Mask", 205, 7, Type::TreasureBox),
    ff2_contact(61, L"Use the Black Mask on the doppelganger", 207, 28),
    ff2_contact(62, L"Take the crystal rod", 209, 8, Type::TreasureBox),
    ff2_contact(63, L"Sail to Mysidian Tower", 1, 108, Type::Warp, Kind::Infer),
    ff2_contact(64, L"Recruit Ricard inside Leviathan", 229, 27),
    ff2_contact(65, L"Defeat the Roundworm", 230, 10),
    ff2_contact(66, L"Board the ship to escape Leviathan", 230, 3, Type::Warp),
    ff2_contact(67, L"Enter Mysidian Tower with the crystal rod", 231, 100, Type::AnimEntity),
    ff2_contact(68, L"Defeat the Fire Gigas", 261, 11),
    ff2_contact(69, L"Defeat the Ice Gigas", 264, 11),
    ff2_contact(70, L"Defeat the Thunder Gigas", 235, 11),
    ff2_contact(71, L"Meet Minwu at the seal", 240, 9),
    ff2_contact(72, L"Receive the Ultima Tome", 241, 19, Type::AnimEntity),
    ff2_learn(928, L"Learn Cyclone from Hilda", 43, 92, 73),
    ff2_contact(73, L"Use the pendant at the Mirror Chamber", 41, 5, Type::Entity),
    ff2_contact(74, L"Ride the wyvern into the Cyclone", 1, 324, Type::Warp),
    ff2_contact(75, L"Defeat the Emperor in the Cyclone", 273, 5, Type::Event, Kind::Infer),
    ff2_contact(76, L"Visit Cid at Paul's house", 21, 30),
    ff2_contact(77, L"Land the airship at Castle Palamecia", 1, 49, Type::TransportationEventAction, Kind::Infer),
    ff2_contact(78, L"Use the Castle Palamecia pit", 280, 3, Type::Warp, Kind::Infer),
    ff2_contact(79, L"Confront Leon at Castle Palamecia", 278, 8),
    ff2_learn(929, L"Learn Jade Passage from Hilda", 43, 89, 80),
    ff2_contact(80, L"Enter the Jade Passage", 1, 48, Type::Warp),
    ff2_contact(81, L"Enter Pandaemonium from the Jade Passage", 292, 3, Type::Warp, Kind::Transition),
    ff2_contact(82, L"Face the Emperor in Pandaemonium", 316, 46)
};
}

Stage ff2_stage(const Ff2Receipts& scenario, const Ff2Keywords& learned)
{
    if (scenario[82] == 1) return {{}, {}, true};
    if (scenario[0] != 1) return {};
    for (const auto& step : kFf2Journey) {
        if (step.skip_after >= 0 && scenario[step.skip_after] == 1) continue;
        if (step.keyword) {
            const auto word = learned[step.receipt - kFirstFf2Keyword];
            if (!word) return {};
            if (*word) continue;
        } else {
            if (!scenario[step.receipt]) return {};
            if (scenario[step.receipt] == 1) continue;
        }
        std::optional<Objective> advice;
        if (!step.keyword && step.receipt == 11)
            advice = interact(Id::Ff2Advice, L"Ask Josef about Mythril", 60, 26);
        return {step.target, advice};
    }
    return {};
}
