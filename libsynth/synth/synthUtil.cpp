
#include "libsynth.h"

string stageName(SynthesizerStage stage)
{
    switch (stage)
    {
    case SynthesizerStagePlaceAgents: return "place agents";
    case SynthesizerStageChooseCandidateObjects: return "select next objects";
    case SynthesizerStageEvaluatePlacementA: return "evaluate placement A";
    case SynthesizerStageEvaluatePlacementB: return "evaluate placement B";
    case SynthesizerStagePlaceBestObject: return "place best object";
    default: return "unknown stage";
    }
}

Polygonf SynthUtil::makeOBBTopPolygon(const OBBf &obb)
{
    vec3f v[4];

    v[0] = obb.getAnchor();
    v[1] = obb.getAnchor() + obb.getAxis(0);
    v[2] = obb.getAnchor() + obb.getAxis(0) + obb.getAxis(1);
    v[3] = obb.getAnchor() + obb.getAxis(1);

    Polygonf poly;
    poly.points.resize(4);
    for (int i = 0; i < 4; i++)
        poly.points[i] = vec2f(v[i].x, v[i].y);

    return poly;

    /*std::vector< point3d<FloatType> > result(8);

    vec2f axis0 = obb.getAxis();

    result[0] = (m_Anchor + m_AxesScaled[0] * (FloatType)0.0 + m_AxesScaled[1] * (FloatType)0.0 + m_AxesScaled[2] * (FloatType)0.0);
    result[1] = (m_Anchor + m_AxesScaled[0] * (FloatType)1.0 + m_AxesScaled[1] * (FloatType)0.0 + m_AxesScaled[2] * (FloatType)0.0);
    result[2] = (m_Anchor + m_AxesScaled[0] * (FloatType)1.0 + m_AxesScaled[1] * (FloatType)1.0 + m_AxesScaled[2] * (FloatType)0.0);
    result[3] = (m_Anchor + m_AxesScaled[0] * (FloatType)0.0 + m_AxesScaled[1] * (FloatType)1.0 + m_AxesScaled[2] * (FloatType)0.0);

    return result;*/
}

void SynthUtil::makeBBoxTopTriangles(const OBBf &obb, Trianglef &t0, Trianglef &t1)
{
    vec3f vMin = obb.getAnchor();
    vec3f vMax = obb.getAnchor();
    for (const vec3f &v : obb.getVertices())
    {
        vMin = ml::math::min(vMin, v);
        vMax = ml::math::max(vMax, v);
    }

    vec3f v0(vMin.x, vMin.y, vMax.z);
    vec3f v1(vMin.x, vMax.y, vMax.z);
    vec3f v2(vMax.x, vMax.y, vMax.z);
    vec3f v3(vMax.x, vMin.y, vMax.z);

    t0 = Trianglef(v0, v1, v2);
    t1 = Trianglef(v0, v2, v3);
}

double InteractionRays::score(bool visibilityOptional) const
{
    double score = 0.0;
    double maxScore = (double)rays.size();
    for (const auto &r : rays)
    {
        if (r.visible || visibilityOptional)
            score += r.score;
    }
    if (maxScore == 0.0)
        return 0.0;
    return score / maxScore;
}

double InteractionRays::score(bool visibilityOptional, const ml::TriMeshRayAcceleratorf &occluder, const mat4f &modelToWorld) const
{
    double score = 0.0;
    double maxScore = (double)rays.size();
    for (const auto &r : rays)
    {
        if (visibilityOptional)
        {
            score += r.score;
        }
        else if (r.visible)
        {
            //
            // check to see if the ray is occluded by occluder
            //
            bool visible = true;
            Rayf modelRay = modelToWorld.getInverse() * r.ray;
            auto intersection = occluder.intersect(modelRay);
            if (intersection.valid())
            {
                double centroidDist = ml::dist(r.ray.origin(), r.centroidPos);
                double intersectionDist = ml::dist(r.ray.origin(), modelToWorld.transformAffine(intersection.getSurfacePosition()));
                if (intersectionDist < centroidDist)
                    visible = false;
            }
            if (visible)
                score += r.score;
        }
    }
    if (maxScore == 0.0)
        return 0.0;
    return score / maxScore;
}

double SynthScore::overhangScore(double overhang)
{
    if (overhang <= 0.001)
        return 1.0;
    return (1.0 - overhang) * 0.1;
}

double SynthScore::wallDistScore(const Category &c, float wallDist)
{
    if (wallDist < 0.01f)
        return 0.0;

    // TODO: consider WallScore
    return 1.0;
    double sum = wallDistAcceptable(c, wallDist, 0.1f) +
        wallDistAcceptable(c, wallDist, 0.2f) +
        wallDistAcceptable(c, wallDist, 0.5f);
    return sum / 3.0;
}

int SynthScore::wallDistAcceptable(const Category &c, float wallDist, float threshold)
{
    for (const auto &instance : c.instances)
    {
        if (fabs(instance.distanceToWall - wallDist) < threshold)
            return 1;
    }
    return 0;
}

double SynthScore::distributionScore(const Distribution3D *distribution, const vec3f &offset, float scaleFactor)
{
    if (distribution == NULL)
        return 0.0;
    double sum = distributionPointAcceptable(distribution, offset, 0.1f * scaleFactor) +
        distributionPointAcceptable(distribution, offset, 0.2f * scaleFactor) +
        distributionPointAcceptable(distribution, offset, 0.3f * scaleFactor) +
        distributionPointAcceptable(distribution, offset, 0.4f * scaleFactor);
    return sum / 4.0;
}

int SynthScore::distributionPointAcceptable(const Distribution3D *distribution, const vec3f &offset, float threshold)
{
    const float ZThresholdScale = 3.0f;
    for (const vec3f& sample : distribution->samples())
    {
        vec3f diff = offset - sample;

        bool acceptable = fabs(diff[0]) <= threshold &&
            fabs(diff[1]) <= threshold &&
            fabs(diff[2]) <= threshold * ZThresholdScale;
        if (acceptable)
            return 1;
    }
    return 0;
}

double SynthScore::handOrientationScore(const DisembodiedObject &object, const mat4f &modelToWorld, const Agent &agent)
{
    const vec3f diff = modelToWorld * (object.model->bbox.getCenter() + vec3f(object.agentFace.x, object.agentFace.y, 0.0f)) - modelToWorld * object.model->bbox.getCenter();

    //const OBBf worldBBox = modelToWorld * OBBf(object.model->bbox);
    //vec3f agentFaceDir = (worldBBox.getCenter() - agent.handPos());
    //agentFaceDir.z = 0.0f;
    //float dot = diff.getNormalized() | agentFaceDir.getNormalized();

    float dot = diff.getNormalized() | agent.gazeDir;

    if (dot < 0.0f)
        return 0.0f;

    return dot;
}

double SynthScore::scanObjectScore(const ScanLocalizedObject &scanObject, const OBBf &oobb, float distThreshold)
{
    //
    // this will not work for objects that have been rotated so that Z is not up
    //
    
    const vec3f axisA0 = scanObject.oobb.getAxis(0).getNormalized();
    const vec3f axisA1 = scanObject.oobb.getAxis(1).getNormalized();
    const vec3f axisB0 = oobb.getAxis(0).getNormalized();
    const vec3f axisB1 = oobb.getAxis(1).getNormalized();

    //
    // for  now, ignore the correspondence between the axes
    //
    auto f = [](const vec3f &a, const vec3f &b) {
        float x = a | b;
        return x * x;
    };

    const float cosineScale = ml::math::max(f(axisA0, axisB0), f(axisA0, axisB1), f(axisA1, axisB0), f(axisA1, axisB1));
    
    //
    // TODO: consider adding Z-scale factor
    //
    const float dist = vec3f::dist(oobb.getCenter(), scanObject.oobb.getCenter());
    if (dist >= distThreshold)
        return 0.0;
    
    return (1.0 - dist / distThreshold) * cosineScale;
}

double SynthScore::scanLocatorResultsScore(const ScanLocatorResults &results, const DisembodiedObject &object, const mat4f &modelToWorld)
{
    double sum = scanLocatorResultsScore(results, object, modelToWorld, 0.1f) +
                 scanLocatorResultsScore(results, object, modelToWorld, 0.2f) +
                 scanLocatorResultsScore(results, object, modelToWorld, 0.4f);
    return sum / 3.0f;
}

double SynthScore::scanLocatorResultsScore(const ScanLocatorResults &results, const DisembodiedObject &object, const mat4f &modelToWorld, float distThreshold)
{
	const OBBf oobb = modelToWorld * OBBf(object.model->bbox);

    double maxScore = 0.0;
    for (const ScanLocalizedObject &scanObject : results.locations)
    {
        double curScore = scanObjectScore(scanObject, oobb, distThreshold);
        maxScore = std::max(maxScore, curScore);
    }
    return maxScore;
}

vector<AgentCategoryData> SynthesizerAgent::makeSortedCategories() const
{
    vector<AgentCategoryData> result;
    for (auto &e : categories)
    {
        if (e.second.activityInsertionScore > 0.0f)
            result.push_back(e.second);
    }

    std::random_shuffle(result.begin(), result.end());
    std::stable_sort(result.begin(), result.end(), [](const AgentCategoryData &a, const AgentCategoryData &b)
    {
        if (a.activityInsertionScore == b.activityInsertionScore)
        {
            return a.category->averageBBoxDiagonalLength > b.category->averageBBoxDiagonalLength;
        }
        return a.activityInsertionScore > b.activityInsertionScore;
    });

    return result;
}