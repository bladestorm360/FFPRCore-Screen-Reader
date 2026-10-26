#pragma once
namespace ffpr::battle::special_feedback {
// Called only inside a rendered damage producer. The innermost native result
// type wins, except for the verified HP-typed secondary MP absorption result.
int damage_type(int native_type,bool rendered_recovery=false);
// BasicFunction.ViewEntity/AfterViewDamage can call CreateDamageView directly,
// bypassing the per-unit view wrappers. Bind the resource while that exact
// native function owner is still available at the typed damage producer.
int owned_damage_type(void* function_owner,int native_type,bool rendered_recovery);
}
