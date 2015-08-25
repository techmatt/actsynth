
#include "libsynth.h"

vector<const ObjectInstance*> Agent::boundObjects() const
{
    vector<const ObjectInstance*> result;
    for (unsigned int boundObjectIndex : boundObjectIndices)
        result.push_back(&scene->objects[boundObjectIndex]);
    return result;
}

const ObjectInstance& Agent::supportObject() const
{
    return scene->objects[supportObjectIndex];
}

void Agent::output(HTMLExporter &e) const
{
    e.appendHeader2("Agent: " + activity);

    e.appendBoldText("Head height: ", to_string(headPos.z));

    if (supportObjectIndex != UINT(-1))
        supportObject().output(e, "support object");

    ObjectInstance::output(e, "bound objects", boundObjects());

    e.appendLineBreak();
}
