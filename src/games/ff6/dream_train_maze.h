#pragma once
#include "story_model.h"
namespace ffpr::ff6::story {
struct MazeAdvice {
    int entity=-1, actions=0;
    field::baked_route::Cell approach;
    bool exit=false, unknown=false, cancelled=false;
};
// Read-only eight-layout control planning. Native geometry remains authoritative;
// the ordinary route consumer validates the suggested immediate approach.
MazeAdvice dream_train_maze(const Snapshot&,const json::Value& map_records,
                           const std::function<bool()>& cancelled={});
}
