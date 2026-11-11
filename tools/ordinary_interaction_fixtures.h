#pragma once

#include <fstream>
#include <iterator>
#include "json.h"

// Route tests still replay these native controls. The DLL only embeds the
// active catalogs, so gameplay cannot load or announce the archived recipes.
inline bool append_ordinary_interactions(json::Value& catalog, int game, const wchar_t* section = L"puzzles")
{
    std::ifstream file("docs/re/ordinary_puzzle_interaction_fixtures_20261106.json", std::ios::binary);
    json::Value archive;
    if (!json::parse(std::string(std::istreambuf_iterator<char>(file), {}), archive)) return false;
    const auto* group = archive.get(L"ff" + std::to_wstring(game));
    const auto* retired = group ? group->get(section) : nullptr;
    if (!retired || !catalog.get(section)) return false;
    auto& fixtures = catalog.obj.at(section).arr;
    fixtures.insert(fixtures.end(), retired->arr.begin(), retired->arr.end());
    if (const auto* types = group->get(L"guidance_types"); types && std::wstring_view(section) == L"story_guides")
        catalog.obj.at(L"guidance_types").obj.insert(types->obj.begin(), types->obj.end());
    return true;
}
