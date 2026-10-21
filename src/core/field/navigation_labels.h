#pragma once
namespace field::filter_worker {
struct FilterInput;
// Worker-only projection of immutable bake identities onto owned list rows.
// False means cancellation; callers must discard the incomplete names/groups.
bool prepare_navigation_labels(FilterInput& input);
}
