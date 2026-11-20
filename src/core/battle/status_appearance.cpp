#include "status_appearance.h"
#include "battle_util.h"
#include "../displayed_ui.h"
#include "../game.h"
#include "../modtext.h"
#include "../refs.h"
#include "../text.h"
#include "../unity.h"
#include <algorithm>
#include <array>
#include <vector>

namespace ffpr::battle {
namespace {
ref::Class kSprite{"Last.Battle", "BattleSpriteEntity"};
ref::Field fStatusController{kSprite, "ConditionEffectEntityController"}, fStatusBody{kSprite, "characterSR"};
ref::Class kEffectController{"Last.Battle", "ConditionEffectEntityController"};
ref::Field fStatusEffects{kEffectController, "conditionEffectList"};
ref::Class kEffect{"Last.Battle", "ConditionEffectEntity"};
ref::Field fStatusAsset{kEffect, "effectId"};
ref::Class kEffectBase{"Last.Battle", "EffectEntityBase"};
ref::Field fParticles{kEffectBase, "particleSystems"};
ref::Class kParticle{"UnityEngine", "ParticleSystem"};
ref::Method mPlaying{kParticle, "get_isPlaying", 0}, mParticleCount{kParticle, "get_particleCount", 0};
ref::Class kEffectRenderer{"UnityEngine", "Renderer"};
ref::Class kInspector{"Last.Battle.Effect", "EffectReactionInspector"};
ref::Field fReaction{kInspector, "currentReaction"}, fReactionStarted{kInspector, "isStart"},
           fReactionEnded{kInspector, "isEnd"};
ref::Class kReaction{"Last.Battle.Effect", "EffectReaction"};
ref::Field fReactionTargets{kReaction, "entityMeshRenderers"};
ref::Class kPaths{"System.Asset", "AssetBundlePathMatch"};
ref::Field fPaths{kPaths, "originalData"};
ref::Class kResources{"Last.Management", "ResourceManager"};
ref::Field fCompleted{kResources, "completeAssetDic"};
ref::Class kSkin{"Last.Battle", "ChangeSkinColorConditionFunctionBase"};
ref::Field fSkinApplied{kSkin, "changedSkinColor"}, fSkinRenderer{kSkin, "effectRender"}, fSkinMaterial{kSkin, "effectMaterial"};
ref::Class kBodyMesh{"Last.Battle", "EntityMeshRenderer"};
ref::Field fBodyMeshRenderer{kBodyMesh, "meshRenderer"};
ref::Class kShader{"UnityEngine", "Shader"}, kMaterial{"UnityEngine", "Material"};
ref::Method mPropertyId{kShader, "PropertyToID", 1}, mColorAt{kMaterial, "GetColorImpl", 1};

constexpr int kStatusLimit = 64;
struct SkinColor { float r, g, b, a; };

void* string_entry(void* dictionary, void* key)
{
    if (!dictionary || !key) return nullptr;
    auto* method = il2cpp::find_method(il2cpp::object_class(dictionary), "TryGetValue", 2);
    auto* address = method ? il2cpp::method_pointer(method) : nullptr;
    if (!address) return nullptr;
    using Lookup = bool (*)(void*, void*, void**, const il2cpp::Method*);
    void* found = nullptr;
    return reinterpret_cast<Lookup>(address)(dictionary, key, &found, method) ? found : nullptr;
}

std::vector<std::wstring> transformation_groups(std::wstring_view art)
{
    std::vector<std::wstring> groups;
    auto add = [&](std::wstring_view base, std::initializer_list<std::wstring_view> suffixes) {
        for (auto suffix : suffixes) groups.emplace_back(std::wstring(base) + std::wstring(suffix));
    };
    switch (game::current()) {
        case game::Id::FF2: add(art, {L"_m", L"_t"}); break;
        case game::Id::FF3:
            // Each party slot has its own shape and palette, regardless of job.
            for (auto slot : {L"bc_ff3_p022_c00", L"bc_ff3_p022_c01", L"bc_ff3_p022_c02", L"bc_ff3_p022_c03"})
                add(slot, {L"_m", L"_t"});
            break;
        case game::Id::FF4: add(art, {L"_m", L"_p", L"_t"}); break;
        case game::Id::FF5:
            add(art.substr(0, art.find(L"_j")), {L"_m", L"_t"});
            add(art, {L"_o", L"_z"});
            break;
        case game::Id::FF6: groups = {L"bc_ff6_p016", L"bc_ff6_p001_tr", L"bc_ff6_p015"}; break;
        default: break;
    }
    return groups;
}

std::wstring visible_skin(void* info, void* body, int& work)
{
    auto* functions = info_functions(info);
    auto* renderer = ref::rd<void*>(body, fBodyMeshRenderer);
    const int count = unity::list_count(functions);
    if (count < 1 || count > kStatusLimit || !displayed::renderer_visible(renderer)) return {};
    for (int index = 0; index < count && work > 0; ++index) {
        --work;
        auto* function = unity::list_at(functions, index);
        if (!kSkin.is(function) || !ref::rd<bool>(function, fSkinApplied) ||
            ref::rd<void*>(function, fSkinRenderer) != renderer) continue;
        auto* material = ref::rd<void*>(function, fSkinMaterial);
        if (!unity::alive(material) || !mPropertyId.ptr() || !mColorAt.ptr()) continue;
        // The status already owns this material. Read its active override
        // directly, because a higher priority condition can recolor it.
        const auto id = ref::call_static<int>(mPropertyId, ref::new_str(L"_OverrideSkinColor1"));
        const auto color = ref::call<SkinColor>(mColorAt, material, id);
        if (color.a <= 0) continue;
        if (color.r >= 0.94f && color.g <= 0.32f && color.b <= 0.05f) return T(L"Status appearance skin red");
        if (game::is(game::Id::FF6)) {
            if (color.g >= 0.49f && color.g + 0.004f >= color.r && color.g > color.b + 0.10f)
                return T(L"Status appearance skin olive");
            if (color.r >= 0.58f && color.r > color.g + 0.12f && color.b >= color.r - 0.02f)
                return T(L"Status appearance skin violet");
        }
    }
    return {};
}

bool rendered_reaction(void* effect, void* body, int& work)
{
    if (work <= 0) return false;
    --work;
    auto* inspector = unity::get_component(effect, kInspector);
    if (!inspector || !ref::rd<bool>(inspector, fReactionStarted) || ref::rd<bool>(inspector, fReactionEnded)) return false;
    auto* reaction = ref::rd<void*>(inspector, fReaction);
    auto* targets = ref::rd<void*>(reaction, fReactionTargets);
    const int count = unity::array_length(targets);
    if (count < 1 || count > 9) return false;
    for (int index = 0; index < count && work > 0; ++index) {
        --work;
        if (unity::array_at(targets, index) == body) return true;
    }
    return false;
}

bool status_is_drawn(void* effect, void* body, int id, int& work)
{
    if (work <= 0 || !displayed::container_visible(effect)) return false;
    --work;
    if (displayed::renderer_visible(unity::get_component(effect, kEffectRenderer))) return true;
    // FFVI's verified Silence prefab draws the little bubble on a child.
    if (game::is(game::Id::FF6) && id == 68 && work > 0) {
        --work;
        if (displayed::renderer_visible(unity::get_component_in_children(effect, kEffectRenderer, false))) return true;
    }
    auto* particles = ref::rd<void*>(effect, fParticles);
    const int count = unity::array_length(particles);
    if (count >= 0 && count <= kStatusLimit) {
        for (int index = 0; index < count && work > 0; ++index) {
            --work;
            auto* particle = unity::array_at(particles, index);
            if (unity::alive(particle) && ref::call<bool>(mPlaying, particle) && ref::call<int>(mParticleCount, particle) > 0 &&
                displayed::renderer_visible(unity::get_component(particle, kEffectRenderer))) return true;
        }
    }
    return rendered_reaction(effect, body, work);
}
}

std::wstring transformed_body_art(void* sprite, std::wstring_view art, std::wstring_view pose)
{
    if (!sprite || pose.empty() || pose.size() > 32 || game::is(game::Id::FF1)) return {};
    // Use the initialized address and completed-asset dictionaries already
    // used by LoadPlayerMotions. A pose name is shared by many different
    // costumes. Only an exact match to the rendered Sprite is evidence of a
    // transformation. No asset request or singleton getter runs here.
    auto* paths = ref::rd<void*>(il2cpp::read_static_ptr(kPaths.get(), "instance"), fPaths);
    auto* manager_class = kResources.get();
    auto* manager = manager_class ? il2cpp::read_static_ptr(il2cpp::class_parent(manager_class), "instance") : nullptr;
    auto* assets = ref::rd<void*>(manager, fCompleted);
    if (!paths || !assets) return {};

    auto* pose_key = ref::new_str(std::wstring(pose));
    for (const auto& group : transformation_groups(art)) {
        auto* group_table = string_entry(paths, ref::new_str(group));
        auto* address = string_entry(group_table, pose_key);
        if (string_entry(assets, address) == sprite) return group;
    }
    return {};
}

std::wstring visible_status_appearance(void* entity, void* info)
{
    auto* body = ref::rd<void*>(entity, fStatusBody);
    if (!body) return {};
    int work = kStatusLimit;
    std::vector<std::wstring> details;
    auto skin = visible_skin(info, body, work);
    if (!skin.empty()) details.push_back(std::move(skin));
    auto* controller = ref::rd<void*>(entity, fStatusController);
    auto* effects = ref::rd<void*>(controller, fStatusEffects);
    const int count = unity::list_count(effects);
    if (count <= 0 || count > kStatusLimit) return text::join(details, L" ");
    std::array<int, kStatusLimit> described{};
    int described_count = 0;
    for (int index = 0; index < count && work > 0; ++index) {
        auto* effect = unity::list_at(effects, index);
        if (!kEffect.is(effect)) continue;
        const int id = ref::rd<int>(effect, fStatusAsset);
        if (id <= 0 || std::find(described.begin(), described.begin() + described_count, id) !=
                described.begin() + described_count) continue;
        const auto key = L"Status appearance effect " + std::to_wstring(id);
        if (!modtext::has(key) || !status_is_drawn(effect, body, id, work)) continue;
        described[described_count++] = id;
        details.push_back(T(key));
    }
    return text::join(details, L" ");
}
}
