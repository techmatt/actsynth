
#include "libsynth.h"

#include <mLibBoost.h>

template<class Archive>
inline void serialize(Archive& ar, Agent& a, const unsigned int version) {
    ar & a.headPos & a.gazeDir & a.rightDir & a.supportPos & a.activity & a.supportObjectIndex & a.boundObjectIndices;
}

template<class Archive>
inline void serialize(Archive& ar, InteractionMapEntry& i, const unsigned int version) {
    ar & i.type & i.centroids & i.faceValues;
}

template<class Archive>
inline void serialize(Archive& ar, InteractionMapEntry::Centroid& c, const unsigned int version) {
    ar & c.pos & c.meshIndex & c.triangleIndex & c.uv;
}

template<class Archive>
inline void serialize(Archive& ar, ObjectInstance& o, const unsigned int version) {
    ar & o.object & o.modelToWorld &
        o.parentObjectInstanceIndex & o.parentMeshIndex & o.parentTriIndex & o.parentUV &
        o.contactPosition & o.contactNormal & o.worldBBox;
}

template<class Archive>
inline void serialize(Archive& ar, DisembodiedObject& o, const unsigned int version) {
    ar & o.modelId & o.contactType & o.modelNormal & o.modelDown &
        o.scale & o.distanceToWall & o.coordianteFrameRotation & o.agentFace;
}

template<class Archive>
inline void serialize(Archive& ar, SceneScorecard& s, const unsigned int version) {
    ar & s.scanGeometry & s.objectCount & s.worstGaze & s.worstOffsetScore & s.averageOffsetScore;
}

void Scan::saveAgentAnnotations(const string &filename) const
{
    std::ofstream file(filename, std::ios::binary | std::ios::out);
    boost::archive::binary_oarchive archive(file);

    archive << agents;
}

void Scan::loadAgentAnnotations(const string &filename)
{
    std::ifstream file(filename, std::ios::binary | std::ios::in);
    boost::archive::binary_iarchive archive(file);

    archive >> agents;

    for (Agent &a : agents)
        a.scene = NULL;
}

void Scene::saveAgentAnnotations(const string &filename) const
{
    std::ofstream file(filename, std::ios::binary | std::ios::out);
    boost::archive::binary_oarchive archive(file);

    archive << agents;
}

void Scene::loadAgentAnnotations(const string &filename)
{
    std::ifstream file(filename, std::ios::binary | std::ios::in);
    boost::archive::binary_iarchive archive(file);

    archive >> agents;

    for (Agent &a : agents)
        a.scene = this;
}

void Scene::save(const string &filename) const
{
    std::ofstream file(filename, std::ios::binary | std::ios::out);
    boost::archive::binary_oarchive archive(file);

    archive << id << objects << agents << scorecard;
}

void Scene::load(const string &filename, const Database &database)
{
    if (!ml::util::fileExists(filename))
        cout << "File not found: " << filename << endl;

    sourceFilename = filename;

    std::ifstream file(filename, std::ios::binary | std::ios::in);
    boost::archive::binary_iarchive archive(file);

    archive >> id >> objects >> agents >> scorecard;

    for (ObjectInstance &o : objects)
    {
        o.scene = this;
        o.object.model = const_cast<Model*>(&database.models.getModel(o.object.modelId));
    }

    for (Agent &a : agents)
        a.scene = this;
}

void InteractionMap::save(const string &filename) const
{
    std::ofstream file(filename, std::ios::binary | std::ios::out);
    boost::archive::binary_oarchive archive(file);

    archive << entries;
}

void InteractionMap::load(const string &filename)
{
    std::ifstream file(filename, std::ios::binary | std::ios::in);
    boost::archive::binary_iarchive archive(file);

    archive >> entries;

    //
    // validate entry types match
    //
    for (auto &e : entries)
    {
        e.second.type = e.first;
    }
}
