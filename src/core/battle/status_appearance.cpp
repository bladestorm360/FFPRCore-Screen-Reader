#include "status_appearance.h"
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

constexpr int kStatusLimit = 64;

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

std::wstring visible_status_appearance(void* entity)
{
    auto* controller = ref::rd<void*>(entity, fStatusController);
    auto* effects = ref::rd<void*>(controller, fStatusEffects);
    const int count = unity::list_count(effects);
    if (count <= 0 || count > kStatusLimit) return {};
    auto* body = ref::rd<void*>(entity, fStatusBody);
    if (!body) return {};
    int work = kStatusLimit;
    std::vector<std::wstring> details;
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
