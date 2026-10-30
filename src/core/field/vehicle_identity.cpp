// Exact resident-model identity before the asynchronous entity scan reaches its
// vehicle stage. Native CreateProperty/SetMapObject stores the transportation ID
// as PropertyEntity.ObjectId, but ordinary actors can reuse that number. Require
// the native model's exact MapObject reference and current stay map as proof.
#include "field_internal.h"
#include "../refs.h"
#include "../unity.h"

namespace field {
namespace {
ref::Class kModel{"Last.Map", "TransportationInfo"};
ref::Class kProperty{"Last.Map", "PropertyEntity"};
ref::Field fObjectId{kProperty, "<ObjectId>k__BackingField"};
ref::Field fModelObject{kModel, "<MapObject>k__BackingField"};
ref::Field fEnabled{kModel, "<Enable>k__BackingField"};
ref::Field fStayMap{kModel, "<StayMapId>k__BackingField"};
ref::Field fId{kModel, "id"};
ref::Field fType{kModel, "type"};
ref::Field fMessage{kModel, "messageId"};
}
bool vehicle_identity_from_model(void* object, void* models, int map,
                                int& type, int& id, std::wstring& message) {
    if (!object || !models || map <= 0) return false;
    void* property = entity_property(object);
    const int candidate = property ? ref::rd<int32_t>(property, fObjectId) : 0;
    if (candidate <= 0) return false;
    // All current six-game masters have at most19 transportation rows. Bound a
    // malformed/unsupported live dictionary and use one keyed TryGetValue;
    // never walk the scene or scan every vehicle for each native marker.
    const int count = unity::dict_count(models);
    if (count < 0 || count > 64) return false;
    void* model = unity::dict_reference(models, candidate);
    if (!model || !kModel.is(model) || ref::rd<void*>(model, fModelObject) != object ||
        !ref::rd<bool>(model, fEnabled) || ref::rd<int32_t>(model, fStayMap) != map ||
        ref::rd<int32_t>(model, fId) != candidate) return false;
    type = ref::rd<int32_t>(model, fType);
    id = candidate;
    message = ref::rd_str(model, fMessage);
    return true;
}
}
