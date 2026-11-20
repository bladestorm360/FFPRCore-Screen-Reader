// Included by storyline.cpp. Native story receipts are ordered by the route
// through the floating continent, surface world and World of Darkness.
namespace {
struct Ff3Step {
    int receipt;
    Objective target;
};
constexpr Ff3Step ff3_contact(int flag, std::wstring_view label, int map, int entity,
                             Type type = Type::NPC, Kind kind = Kind::Interact)
{
    const auto id = static_cast<Id>(static_cast<int>(Id::Ff3Scenario) + flag);
    return {flag, {id, label, map, {entity, -1, -1}, kind, type}};
}
constexpr Ff3Step ff3_event(int flag, std::wstring_view label, int map, int entity)
{
    return ff3_contact(flag, label, map, entity, Type::Event, Kind::Infer);
}
constexpr std::array kFf3Journey{
    ff3_event(1, L"Approach the unusual rock", 153, 13),
    ff3_contact(2, L"Examine the unusual rock", 153, 10, Type::Entity),
    ff3_event(6, L"Defeat the Land Turtle", 155, 3),
    ff3_contact(8, L"Speak to the Wind Crystal", 155, 0, Type::Entity),
    ff3_contact(9, L"Visit Elder Topapa in Ur", 12, 6),
    ff3_contact(10, L"Speak to the traveler in Kazus", 13, 49),
    ff3_contact(11, L"Speak to Cid in Kazus", 14, 18),
    ff3_contact(194, L"Board Cid's airship in the desert", 21, 3, Type::Warp),
    ff3_contact(12, L"Speak to the king of Sasune", 26, 11),
    ff3_contact(14, L"Find Princess Sara in the Sealed Cave", 163, 5),
    ff3_event(16, L"Defeat the Djinn with Sara", 164, 8),
    ff3_contact(17, L"Collect the king's reward in Sasune", 26, 3),
    ff3_event(18, L"Return to Cid after lifting the curse", 13, 48),
    ff3_contact(19, L"Reinforce the airship in Kazus", 19, 5),
    ff3_event(20, L"Ram the boulder with the reinforced airship", 1, 79),
    ff3_event(21, L"Enter Caanan with Cid", 36, 40),
    ff3_event(24, L"Approach Dragon's Peak", 1, 77),
    ff3_contact(28, L"Escape Bahamut and obtain Mini", 166, 2, Type::AnimEntity),
    ff3_contact(29, L"Enter Tozus as Mini and give the antidote", 46, 25),
    ff3_contact(30, L"Speak to the leader of the Vikings", 52, 17),
    ff3_contact(31, L"Enter the Nepto statue as Mini", 169, 8, Type::Warp),
    ff3_contact(33, L"Defeat the Giant Rat for the Nepto Eye", 173, 6),
    ff3_contact(34, L"Return the Nepto Eye to the statue", 169, 7, Type::Entity),
    ff3_contact(38, L"Receive Toad from the Gulgan", 76, 3),
    ff3_contact(39, L"Enter the Tower of Owen as Frog", 174, 10, Type::Warp),
    ff3_event(40, L"Continue through the Tower of Owen", 175, 7),
    ff3_event(41, L"Follow Desch through the tower", 178, 9),
    ff3_event(42, L"Find the way through the tower", 179, 10),
    ff3_contact(43, L"Defeat Medusa", 184, 4),
    ff3_contact(44, L"Examine the tower's flames", 184, 9, Type::Warp),
    ff3_contact(46, L"Dive into the Underground Lake as Frog", 90, 5, Type::Warp),
    ff3_contact(48, L"Recover the Horn of Ice from Gutsco", 188, 9),
    ff3_contact(49, L"Return the Horn of Ice to the dwarf", 85, 21),
    ff3_contact(50, L"Place the Horn of Ice on the altar", 85, 27, Type::Entity),
    ff3_event(51, L"Defeat the Salamander", 193, 7),
    ff3_contact(52, L"Speak to the Fire Crystal", 193, 27, Type::Entity),
    ff3_event(53, L"Hear the warning about Tokkul", 84, 8),
    ff3_contact(54, L"Enter Tokkul to reach Castle Hein", 55, 25, Type::Warp, Kind::Infer),
    ff3_contact(55, L"Speak to the captive soldier in Castle Hein", 194, 9),
    ff3_contact(56, L"Enter the escape hole as Mini", 194, 18, Type::Warp),
    ff3_contact(59, L"Defeat Hein", 205, 5),
    ff3_event(60, L"Follow the Elder Tree to the Living Woods", 91, 12),
    ff3_contact(61, L"Receive the Wheel of Time from King Argus", 70, 9),
    ff3_contact(62, L"Take the Wheel of Time to Cid", 43, 17),
    ff3_contact(63, L"Find Aria at the Wrecked Ship", 94, 9),
    ff3_contact(64, L"Collect the Water Crystal shard with Aria", 96, 5, Type::Entity),
    ff3_event(65, L"Open the Cave of Tides with Aria", 206, 38),
    ff3_event(66, L"Reach the Water Crystal and defeat Kraken", 211, 8),
    ff3_contact(68, L"Speak to the four old men in Amur", 97, 42),
    ff3_contact(69, L"Ask Gill to open the Amur sewers", 99, 6),
    ff3_event(70, L"Rescue the four old men in the sewers", 214, 12),
    ff3_event(71, L"Get the Levigrass Shoes from Delilah", 218, 6),
    ff3_contact(72, L"Defeat Goldor and free the Enterprise", 226, 4),
    ff3_contact(73, L"Fly the Enterprise to Saronia", 4, 213, Type::Warp, Kind::AirshipTransition),
    ff3_event(74, L"Help Prince Alus at the tavern", 119, 16),
    ff3_contact(75, L"Take Prince Alus to Castle Saronia", 104, 24),
    ff3_event(76, L"Collect the Nautilus from the engineers", 114, 10),
    ff3_event(77, L"Meet Doga at his manor", 131, 10),
    ff3_contact(78, L"Reach Doga's magic circle", 241, 3, Type::Warp, Kind::Infer),
    ff3_contact(79, L"Find Noah's Lute in the Temple of Time", 252, 5, Type::AnimEntity),
    ff3_contact(80, L"Wake Unei with Noah's Lute", 142, 6),
    ff3_event(81, L"Receive the Fang of Fire from Unei", 142, 7),
    ff3_event(82, L"Have Unei clear the Ancient Ruins passage", 253, 15),
    ff3_contact(83, L"Board the Invincible with Unei", 157, 7, Type::Warp),
    ff3_contact(84, L"Defeat Hecatoncheir and receive the Fang of Earth", 290, 5, Type::AnimEntity),
    ff3_contact(85, L"Return to Doga and Unei", 131, 9, Type::Warp, Kind::Infer),
    ff3_event(86, L"Face Doga and Unei", 298, 6),
    ff3_event(87, L"Receive the Syrcus Key", 298, 5),
    ff3_event(88, L"Pass the statues with the first fang", 4, 202),
    ff3_event(89, L"Pass the statues with the second fang", 4, 203),
    ff3_event(90, L"Pass the statues with the third fang", 4, 204),
    ff3_event(91, L"Pass the statues with the fourth fang", 4, 205),
    ff3_event(92, L"Defeat the guardian of the Earth Crystal", 300, 5),
    ff3_contact(93, L"Speak to the Earth Crystal", 300, 0, Type::Entity),
    ff3_contact(94, L"Use the Syrcus Key at the Crystal Tower door", 305, 29, Type::Entity),
    ff3_contact(351, L"Approach the Crystal Tower mirror", 313, 4, Type::Entity),
    ff3_contact(96, L"Defeat Xande", 314, 4),
    ff3_event(97, L"Defeat Cerberus in the World of Darkness", 327, 4),
    ff3_contact(98, L"Free the first Warrior of Dark", 327, 0, Type::Entity),
    ff3_event(99, L"Defeat the Two-Headed Dragon", 329, 4),
    ff3_contact(100, L"Free the second Warrior of Dark", 329, 18, Type::Entity),
    ff3_event(101, L"Defeat Echidna", 331, 5),
    ff3_contact(102, L"Free the third Warrior of Dark", 331, 0, Type::Entity),
    ff3_event(103, L"Defeat Ahriman", 333, 5),
    ff3_contact(104, L"Free the fourth Warrior of Dark", 333, 0, Type::Entity),
    ff3_contact(105, L"Face the Cloud of Darkness", 335, 4, Type::Entity)
};
}

Stage ff3_stage(const Ff3Receipts& scenario)
{
    if (scenario[105] == 1) return {{}, {}, true};
    if (scenario[0] != 1) return {};
    for (const auto& step : kFf3Journey) {
        // Later saves whose first airship has been spent never return to the
        // desert for a boarding receipt that can no longer be obtained.
        if (step.receipt == 194 && scenario[20] == 1) continue;
        if (!scenario[step.receipt]) return {};
        if (scenario[step.receipt] == 1) continue;
        if (step.receipt == 33 && scenario[32] == 1)
            return {interact(Id::Ff3EyeRecovery, L"Take the Nepto Eye", 173, 5, Type::Entity)};
        return {step.target};
    }
    return {};
}
