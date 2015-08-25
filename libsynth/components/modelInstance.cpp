
#include "libsynth.h"

mat4f DisembodiedObject::makeBaseTransformCentered() const
{
    mat4f centerFaceScale = mat4f::scale(scale) * mat4f::face(modelNormal, vec3f::eZ) * mat4f::translation(-model->bbox.getCenter());
    return centerFaceScale;
}

mat4f DisembodiedObject::makeBaseTransform(float elevation) const
{
    //vec3f base(model->bbox.getCenter().x, model->bbox.getCenter().y, model->bbox.getMinZ());
    mat4f centerFaceScale = mat4f::scale(scale) * mat4f::face(modelNormal, vec3f::eZ) * mat4f::translation(-model->bbox.getCenter());
	ml::bbox3f bbox((centerFaceScale * OBBf(model->bbox)).getVertices());
    return mat4f::translation(vec3f::eZ * (bbox.getExtentZ() * 0.5f + elevation)) * centerFaceScale;
}

OBBf DisembodiedObject::computeWorldBBox() const
{
	return makeBaseTransform() * OBBf(model->bbox);
}

void DisembodiedObject::output(HTMLExporter &e, const string &name, const vector< vector<DisembodiedObject> > &samples)
{
    e.appendHeader1(name);
    for (auto &s : samples)
    {
        map<string, int> categoryCounts;
        for (auto &o : s)
        {
            const string &categoryName = o.model->categoryName;
            if (categoryCounts.count(categoryName) == 0)
                categoryCounts[categoryName] = 0;
            categoryCounts[categoryName]++;
        }

        e.file << "<p>";
        for (auto &p : categoryCounts)
        {
            e.file << p.first << "-" << p.second << " ";
        }
        e.file << "</p>" << endl;
    }
    e.appendLineBreak();
}

string ObjectInstance::parentCategoryName() const
{
    if (parentObjectInstanceIndex != -1)
        return scene->objects[parentObjectInstanceIndex].categoryName();
    else
        return "none";
}

string ObjectInstance::categoryName() const
{
    return object.model->categoryName;
}

void ObjectInstance::output(HTMLExporter &e, const string &name) const
{
    vector<ObjectInstance> instance;
    instance.push_back(*this);
    output(e, name, instance);
}

void ObjectInstance::output(HTMLExporter &e, const string &name, const vector<ObjectInstance> &instances)
{
    vector<const ObjectInstance*> instancesPtr;
    for (const auto &i : instances)
        instancesPtr.push_back(&i);
    output(e, name, instancesPtr);
}

void ObjectInstance::output(HTMLExporter &e, const string &name, const vector<const ObjectInstance*> &instances)
{
    ml::Grid2<string> grid(3, instances.size());

    int y = 0;
    for (const ObjectInstance *inst : instances)
    {
        grid(0, y) = inst->object.model->imageHTML();

        if (inst->object.model->categoryName == "uncategorized")
            grid(1, y) = "<small>" + inst->object.modelId + "</small>";
        else
            grid(1, y) = "<b>" + inst->object.model->categoryName + "</b>";
        grid(2, y) = inst->parentCategoryName();
        y++;
    }
    e.appendTableColumnWrap(name, grid, 10);
}
