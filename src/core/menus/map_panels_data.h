#pragma once
#include <string>
#include <vector>

namespace ffpr::maps {
struct Marker { int icon=1; float x=0,y=0; };
struct Counter { std::wstring label,current,total; };
// Only copied rendered data crosses into the worker. Positions use the basic
// map's native world coordinates (positive y north),16 units per field tile.
struct Snapshot {
    bool basic=false,autodetail=false,announce=true;
    std::wstring locale,name;
    float x=0,y=0,wrap_width=0,wrap_height=0;
    std::vector<Marker> markers;
    std::vector<int> services;
    std::vector<Counter> counters;
    double requested=0,native_ms=0;unsigned slices=0;
};
struct Reading { std::wstring name;std::vector<std::wstring> details;bool buffer=false; };
Reading build(const Snapshot& snapshot);
}
