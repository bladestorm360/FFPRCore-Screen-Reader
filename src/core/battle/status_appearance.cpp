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
#include <cmath>
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
ref::Method mFloatAt{kMaterial, "GetFloatImpl", 1}, mSharedMaterial{kEffectRenderer, "get_sharedMaterial", 0};
ref::Class kBlink{"Last.Battle", "BlinkConditionFunction"};
ref::Field fBlinkBodies{kBlink, "copied"};
ref::Class kDoom{"Last.Battle", "DoomConditionFunction", game::kFF4 | game::kFF5 | game::kFF6};
ref::Field fDoomView{kDoom, "doomCountEntityView"};
ref::Class kDoomEx{"Last.Battle", "DoomExConditionFunction", game::kFF4 | game::kFF5 | game::kFF6},
           kDoomEquipment{"Last.Battle", "DoomEquipmentConditionFunction", game::kFF5 | game::kFF6},
           kDoomZombie{"Last.Battle", "DoomZonbieConditionFunction", game::kFF6}; // native spelling
ref::Field fDoomExView{kDoomEx, "doomCountEntityView"}, fDoomEquipmentView{kDoomEquipment, "doomCountEntityView"},
           fDoomZombieView{kDoomZombie, "doomCountEntityView"};
ref::Class kDoomGlyph{"UnityEngine", "SpriteRenderer"};
ref::Method mDoomGlyph{kDoomGlyph, "get_sprite", 0};
ref::Class kFreeze{"Last.Battle", "FreezeConditionFunction", game::kFF6};
ref::Class kTranslucent{"Last.Battle", "TranslucentConditionFunction", game::kFF4};
ref::Class kVanish{"Last.Battle", "VanishConditionFunction", game::kFF6};
ref::Field fVanishMaterial{kVanish, "material"};
ref::Class kFloat{"Last.Battle", "LevitetoConditionFunction", game::kFF4 | game::kFF5 | game::kFF6};
ref::Class kFloatSprite{"Last.Battle", "BattleSpriteEntity", game::kFF4 | game::kFF5 | game::kFF6};
ref::Field fMoveOffsets{kFloatSprite, "movePos"};
ref::Method mMoveOffset{kFloatSprite, "GetMovePos", 1};

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
        case game::Id::FF2:
            add(art, {L"_m", L"_t"});
            groups.emplace_back(L"bc_ff2_p010");
            break;
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
        case game::Id::FF6:
            groups = {L"bc_ff6_p016", L"bc_ff6_p001_tr", L"bc_ff6_p015", L"bc_ff6_p021_c01",
                      L"bc_ff6_p025", L"bc_ff6_p011", L"bc_ff6_p017_c01", L"bc_ff6_p023"};
            break;
        default: break;
    }
    return groups;
}

int property_id(const wchar_t* property)
{
    return ref::call_static<int>(mPropertyId, ref::new_str(property));
}

std::wstring skin_description(void* function, void* renderer)
{
    if (!ref::rd<bool>(function, fSkinApplied) || ref::rd<void*>(function, fSkinRenderer) != renderer ||
        !displayed::renderer_visible(renderer)) return {};
    auto* material = ref::rd<void*>(function, fSkinMaterial);
    if (!unity::alive(material) || !mPropertyId.ptr() || !mColorAt.ptr()) return {};
    // The status already owns this material. Read its active replacement
    // color directly, because a higher priority condition can recolor it.
    const auto color = ref::call<SkinColor>(mColorAt, material, property_id(L"_OverrideSkinColor1"));
    if (color.a <= 0) return {};
    if (color.r >= 0.94f && color.g <= 0.32f && color.b <= 0.05f) return T(L"Status appearance skin red");
    if (game::is(game::Id::FF6)) {
        if (color.g >= 0.49f && color.g + 0.004f >= color.r && color.g > color.b + 0.10f)
            return T(L"Status appearance skin olive");
        if (color.r >= 0.58f && color.r > color.g + 0.12f && color.b >= color.r - 0.02f)
            return T(L"Status appearance skin violet");
    }
    return {};
}

bool blink_is_drawn(void* function, int& work)
{
    auto* copies = ref::rd<void*>(function, fBlinkBodies);
    const bool many = game::is(game::Id::FF4) || game::is(game::Id::FF5);
    const int count = many ? unity::list_count(copies) : copies ? 1 : 0;
    if (count < 1 || count > 8) return false;
    for (int index = 0; index < count && work > 0; ++index) {
        --work;
        auto* copy = many ? unity::list_at(copies, index) : copies;
        if (displayed::renderer_visible(ref::rd<void*>(copy, fBodyMeshRenderer))) return true;
    }
    return false;
}

void* doom_view(void* function)
{
    if (kDoom.is(function)) return ref::rd<void*>(function, fDoomView);
    if (kDoomEx.is(function)) return ref::rd<void*>(function, fDoomExView);
    if (kDoomEquipment.is(function)) return ref::rd<void*>(function, fDoomEquipmentView);
    if (kDoomZombie.is(function)) return ref::rd<void*>(function, fDoomZombieView);
    return nullptr;
}

std::wstring doom_description(void* view, int& work)
{
    if (!displayed::container_visible(view)) return {};
    auto* transform = unity::transform(view);
    constexpr int kDigits = 3;
    const int count = unity::child_count(transform);
    if (count < 1 || count > kDigits || count > work) return {};
    struct Digit { float x; wchar_t value; };
    std::array<Digit, kDigits> digits{};
    size_t shown = 0;
    constexpr std::wstring_view prefix = L"Battle_Common_DamageNumber_";
    for (int index = 0; index < count; ++index) {
        --work;
        auto* child = unity::child(transform, index);
        auto* glyph = unity::get_component(child, kDoomGlyph);
        if (!displayed::renderer_visible(glyph)) continue;
        const auto name = unity::name(ref::call<void*>(mDoomGlyph, glyph));
        const float x = unity::local_position(child).x;
        if (!std::isfinite(x) || name.size() != prefix.size() + 1 || !name.starts_with(prefix) ||
            name.back() < L'0' || name.back() > L'9') return {};
        digits[shown++] = {x, name.back()};
    }
    if (!shown) return {};
    std::sort(digits.begin(), digits.begin() + shown, [](Digit a, Digit b) { return a.x < b.x; });
    std::wstring value;
    for (size_t index = 0; index < shown; ++index) {
        if (index && digits[index - 1].x == digits[index].x) return {};
        value.push_back(digits[index].value);
    }
    return TF(L"Status appearance doom {0}", {value});
}

bool freeze_is_drawn(void* renderer)
{
    if (!displayed::renderer_visible(renderer) || !mPropertyId.ptr() || !mFloatAt.ptr() || !mColorAt.ptr()) return false;
    auto* material = ref::call<void*>(mSharedMaterial, renderer);
    if (!unity::alive(material) || ref::call<float>(mFloatAt, material, property_id(L"_ColorWeight")) <= 0.0f)
        return false;
    const auto color = ref::call<SkinColor>(mColorAt, material, property_id(L"_Color1"));
    return color.a > 0.99f && std::abs(color.r - 40.0f / 255) < 0.005f &&
           std::abs(color.g - 40.0f / 255) < 0.005f && std::abs(color.b - 200.0f / 255) < 0.005f;
}

const wchar_t* transparency_key(void* function, void* renderer, bool vanish)
{
    if (!displayed::renderer_visible(renderer) ||
        !mPropertyId.ptr() || !mFloatAt.ptr() || !mColorAt.ptr()) return nullptr;
    auto* material = ref::call<void*>(mSharedMaterial, renderer);
    if (!unity::alive(material)) return nullptr;
    if (vanish) {
        if (ref::rd<void*>(function, fVanishMaterial) != material) return nullptr;
        // Player and Trance shaders cut out their fills. The alternate
        // branch blends a fully clear tint at half weight.
        for (const auto* name : {L"_HollowOut", L"_TranceHollowOut"})
            if (ref::call<float>(mFloatAt, material, property_id(name)) == 1.0f)
                return L"Status appearance hollow body";
    }
    const float weight = ref::call<float>(mFloatAt, material, property_id(L"_TintColorWeight"));
    if (weight != (vanish ? 0.5f : 1.0f)) return nullptr;
    const auto tint = ref::call<SkinColor>(mColorAt, material, property_id(L"_TintColor"));
    return tint.a == (vanish ? 0.0f : 0.5f) ? L"Status appearance translucent body" : nullptr;
}

bool float_is_drawn(void* entity, void* renderer)
{
    if (!displayed::renderer_visible(renderer) || !ref::rd<void*>(entity, fMoveOffsets)) return false;
    // Native renderer updates write the Float displacement into the dedicated
    // offset slot. The getter only reads that initialized dictionary entry.
    constexpr int levitate_offset = 0;
    const auto offset = ref::call<unity::Vec3>(mMoveOffset, entity, levitate_offset);
    return std::isfinite(offset.y) && offset.y > 0.0f;
}

void add_function_appearances(void* entity, void* info, void* body, int& work, std::vector<std::wstring>& details)
{
    auto* functions = info_functions(info);
    const int count = unity::list_count(functions);
    if (count < 1 || count > kStatusLimit) return;
    auto* renderer = ref::rd<void*>(body, fBodyMeshRenderer);
    bool skin = false, blink = false, doom = false, freeze = false, transparent = false, floating = false;
    for (int index = 0; index < count && work > 0; ++index) {
        --work;
        auto* function = unity::list_at(functions, index);
        std::wstring detail;
        if (!skin && kSkin.is(function)) {
            detail = skin_description(function, renderer);
            skin = !detail.empty();
        } else if (!blink && kBlink.is(function) && blink_is_drawn(function, work)) {
            blink = true;
            detail = T(L"Status appearance blink");
        } else if (!freeze && kFreeze.is(function) && freeze_is_drawn(renderer)) {
            freeze = true;
            detail = T(L"Status appearance freeze");
        } else if (!floating && kFloat.is(function) && float_is_drawn(entity, renderer)) {
            floating = true;
            detail = T(L"Status appearance float");
        } else if (const bool vanish = !transparent && kVanish.is(function);
                   !transparent && (vanish || kTranslucent.is(function))) {
            if (const auto* key = transparency_key(function, renderer, vanish)) {
                transparent = true;
                detail = T(key);
            }
        } else if (!doom) {
            if (auto* view = doom_view(function)) detail = doom_description(view, work);
            doom = !detail.empty();
        }
        if (!detail.empty()) details.push_back(std::move(detail));
    }
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
template<class Groups>
std::wstring match_loaded_sprite(void* sprite, std::wstring_view pose, const Groups& groups)
{
    if (!sprite || pose.empty() || pose.size() > 32) return {};
    // Use the initialized address and completed-asset dictionaries already
    // used by the native loaders. Different assets share pose names. Only an
    // exact match to the rendered Sprite identifies the displayed asset.
    auto* paths = ref::rd<void*>(il2cpp::read_static_ptr(kPaths.get(), "instance"), fPaths);
    auto* manager_class = kResources.get();
    auto* manager = manager_class ? il2cpp::read_static_ptr(il2cpp::class_parent(manager_class), "instance") : nullptr;
    auto* assets = ref::rd<void*>(manager, fCompleted);
    if (!paths || !assets) return {};

    auto* pose_key = ref::new_str(std::wstring(pose));
    for (const auto& group : groups) {
        auto* group_table = string_entry(paths, ref::new_str(std::wstring(group)));
        auto* address = string_entry(group_table, pose_key);
        if (string_entry(assets, address) == sprite) return group;
    }
    return {};
}
}

std::wstring transformed_body_art(void* sprite, std::wstring_view art, std::wstring_view pose)
{
    if (game::is(game::Id::FF1)) return {};
    return match_loaded_sprite(sprite, pose, transformation_groups(art));
}

bool rendered_ff5_old_sprite(void* sprite, std::wstring_view art, std::wstring_view sprite_name,
                             std::wstring_view normalized_pose)
{
    if (!sprite || !art.starts_with(L"bc_ff5_") || normalized_pose.empty()) return false;
    const std::wstring groups[]{std::wstring(art) + L"_o"};
    // Some poses in the aged sheet use identical pixels. The exact sprite
    // address and the game-specific pixel comparison must both agree.
    if (!modtext::has(L"Status appearance changed " + groups[0] + L" " + std::wstring(normalized_pose))) return false;
    return !match_loaded_sprite(sprite, sprite_name, groups).empty();
}

std::wstring rendered_shield_art(void* sprite, std::wstring_view pose)
{
    // Installed shields can use a group from a previous title. Both ordinary
    // guarding and special guard actions share BattleShield's renderer.
    constexpr const wchar_t* groups[]{L"wp_ff1_sh00_c00", L"wp_ff1_sh01_c00", L"wp_ff2_sh00_c00",
        L"wp_ff5_sh00_c00", L"wp_ff6_sh00_c00", L"wp_ff6_sh01_c00"};
    return match_loaded_sprite(sprite, pose, groups);
}

std::wstring visible_status_appearance(void* entity, void* info)
{
    auto* body = ref::rd<void*>(entity, fStatusBody);
    if (!body) return {};
    int work = kStatusLimit;
    std::vector<std::wstring> details;
    add_function_appearances(entity, info, body, work, details);
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
