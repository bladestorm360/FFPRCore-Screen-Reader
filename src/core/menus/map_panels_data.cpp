#include "map_panels_data.h"
#include "../modtext.h"
#include "../text.h"
#include <algorithm>
#include <cmath>
#include <cstdio>

namespace ffpr::maps {
namespace {
const wchar_t* service(int type) {
    switch(type) {
    case 1:return L"Weapon shop";case 2:return L"Armor shop";case 3:return L"Item shop";
    case 4:return L"White magic shop";case 5:return L"Black magic shop";case 6:return L"Inn";
    case 7:return L"Accessory shop";case 8:return L"Pub";case 9:return L"Chocobo";case 10:return L"Magic shop";
    default:return nullptr;
    }
}
const wchar_t* marker(int type) {
    // Last.UI.Gps.MinimapIconType, verified in each supported game.
    switch(type) {
    case 2:return L"Stairs";case 3:return L"World map exit";case 4:return L"Area transition";
    case 6:return L"Item shop";case 7:return L"Weapon shop";case 8:return L"Armor shop";
    case 9:return L"White magic shop";case 10:return L"Black magic shop";case 11:return L"Inn";
    case 12:return L"Church";case 13:return L"Pub";case 14:return L"Accessory shop";case 15:return L"Chocobo";
    default:return L"Map marker";
    }
}
float wrap(float d,float width){return width>0?d-std::round(d/width)*width:d;}
}
Reading build(const Snapshot& s) {
    modtext::LocaleScope locale(s.locale);
    Reading out;out.name=s.name.empty()?T(L"Map"):text::trim(s.name);out.buffer=s.basic;
    if(s.basic) {
        struct Line {float distance;std::wstring text;};std::vector<Line> lines;
        constexpr const wchar_t* directions[]={L"North",L"Northeast",L"East",L"Southeast",L"South",L"Southwest",L"West",L"Northwest"};
        for(const auto& row:s.markers) {
            if(row.icon==0||row.icon==5||!std::isfinite(row.x)||!std::isfinite(row.y))continue;
            const float x=wrap(row.x-s.x,s.wrap_width)/16,n=wrap(row.y-s.y,s.wrap_height)/16;
            const auto distance=std::hypot(x,n);
            if(!std::isfinite(distance))continue;
            double angle=std::atan2(x,n)*180/3.141592653589793;if(angle<0)angle+=360;
            const int direction=int(std::floor((angle+22.5)/45))%8;
            wchar_t steps[64];std::swprintf(steps,64,L"%.0f",double(distance));
            lines.push_back({distance,TF(L"{0}, about {1} steps {2}",{T(marker(row.icon)),steps,T(directions[direction])})});
        }
        std::stable_sort(lines.begin(),lines.end(),[](const auto& a,const auto& b){return a.distance<b.distance;});
        for(auto& row:lines)out.details.push_back(std::move(row.text));
        if(out.details.empty())out.details.push_back(T(L"No map markers displayed"));
    } else {
        for(int type:s.services)if(const auto* key=service(type))out.details.push_back(T(key));
        for(const auto& count:s.counters)if(!count.label.empty()&&!count.current.empty()&&!count.total.empty())
            out.details.push_back(count.label+L" "+count.current+L"/"+count.total);
        out.buffer=out.details.size()>1;
    }
    return out;
}
}
