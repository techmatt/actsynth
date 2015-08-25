
#include "libsynth.h"

vector<UINT> Scene::agentsFromObjectIndex(UINT objectIndex) const
{
    vector<UINT> result;
    for (int agentIndex = 0; agentIndex < agents.size(); agentIndex++)
    {
        const Agent &a = agents[agentIndex];
        if (a.supportObjectIndex == objectIndex || ml::util::contains(a.boundObjectIndices, objectIndex))
        {
            result.push_back(agentIndex);
        }
    }
    return result;
}

void Scene::computeWallSegments()
{
    const ml::vec2f v0(bbox.getMinX(), bbox.getMinY());
    const ml::vec2f v1(bbox.getMaxX(), bbox.getMinY());
    const ml::vec2f v2(bbox.getMaxX(), bbox.getMaxY());
    const ml::vec2f v3(bbox.getMinX(), bbox.getMaxY());

    wallSegments.clear();
    wallSegments.push_back(ml::LineSegment2f(v0, v1));
    wallSegments.push_back(ml::LineSegment2f(v1, v2));
    wallSegments.push_back(ml::LineSegment2f(v2, v3));
    wallSegments.push_back(ml::LineSegment2f(v3, v0));
}

float Scene::distanceToWall(const OBBf &bbox) const
{
    auto bboxSegments3 = bbox.getEdges();

    vector<ml::LineSegment2f> bboxSegments;
    for (const auto &s : bboxSegments3)
        bboxSegments.push_back(ml::LineSegment2f(ml::vec2f(s.p0().x, s.p0().y),
                                                 ml::vec2f(s.p1().x, s.p1().y)));

    return (float)ml::distSq(wallSegments, bboxSegments);
}

void Scene::output(const string &filename) const
{
    HTMLExporter exporter(filename, "Scene: " + id);

    ObjectInstance::output(exporter, "All objects", objects);

    for (const Agent &a : agents)
        a.output(exporter);
}

vector<UINT> Scene::objectsWithCategory(const string &categoryName, int agentBinding) const
{
    vector<UINT> result;
    for (UINT objectIndex = 0; objectIndex < objects.size(); objectIndex++)
    {
        if (objects[objectIndex].object.model->categoryName == categoryName)
        {
            if (categoryName == "Room" || agentBinding == -1 || ml::util::contains(agents[agentBinding].boundObjectIndices, objectIndex))
                result.push_back(objectIndex);
        }
    }
    return result;
}

void Scene::updateAgentFaceDirections()
{
    for (Agent &a : agents)
    {
        for (int objectIndex : a.boundObjectIndices)
        {
            if (objectIndex >= objects.size())
            {
                cout << "invalid agent binding; agent annotations likely out of date with database changes" << endl;
            }
            auto &o = objects[objectIndex];

            mat4f worldToModel = o.modelToWorld.getInverse();

            vec3f modelCenter = o.object.model->bbox.getCenter();
            vec3f headPos = worldToModel * a.headPos;

            //o.object.agentFace = ( vec2f(headPos) - vec2f(modelCenter) ).getNormalized();
            o.object.agentFace = (worldToModel.getInverse().getTranspose() * vec4f(a.gazeDir, 0.0f)).getVec2().getNormalized();
        }
    }
}
