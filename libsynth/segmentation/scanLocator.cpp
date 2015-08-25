
#include "libsynth.h"

#include <mLibD3D11.h>
#include "../synthesisCommon/assetManager.h"

const bool useFileCache = false;
const bool skipLocator = true;

void ScanLocator::init(const Database &_database, ml::D3D11GraphicsDevice &_graphics, AssetManager &_assets, const Scan &_scan)
{
    database = &_database;
    graphics = &_graphics;
    assets = &_assets;
    scan = &_scan;
}

const ScanLocatorResults& ScanLocator::locateModel(const DisembodiedObject &object)
{
    string name = nameObject(*scan, object);
    if (cache.count(name) == 0)
        cacheModel(object);
    return cache[name];
}

void ScanLocator::cacheModel(const DisembodiedObject &object)
{
	const OBBf bbox = object.computeWorldBBox();

    ScanLocatorResults &result = cache[nameObject(*scan, object)];
    
    const float maxDimension = ml::math::max(bbox.getExtent()[0], bbox.getExtent()[1], bbox.getExtent()[2]);
    cout << "locating " << object.model->categoryName << " dim=" << maxDimension << endl;
    result.locations = locate(scan->getSegmentationAtResolution(maxDimension), false, bbox.getExtent());
    
    for (ScanLocalizedObject &location : result.locations)
    {
        location.score *= scoreObject(object, location.oobb);
    }
}

mat4f ScanLocator::alignBBoxes(const OBBf &startBox, const OBBf &endBox, float ZRotationOffset)
{
	auto getDominantAxis = [](const OBBf &box)
    {
        //if (vec3f::distSq(box.getAxis(2).getNormalized(), vec3f::eZ) > 0.01f)
        //    cout << "box is not up-oriented!" << endl;
        if (box.getExtent().x > box.getExtent().y)
            return box.getAxis(0);
        else
            return box.getAxis(1);
    };

    const vec3f axisA = getDominantAxis(startBox);
    const vec3f axisB = getDominantAxis(endBox);
    //const vec3f axisB = endBox.getAxis(0);

    //const vec3f rotationVector = axisA ^ axisB;
    //return mat4f::translation(startBox.getCenter() + endBox.getCenter()) * mat4f::rotation(rotationVector, vec3f::angleBetween(axisA, axisB) + ZRotationOffset) * mat4f::translation(-startBox.getCenter());

    return mat4f::translation(startBox.getCenter() + endBox.getCenter()) * mat4f::rotationZ(ZRotationOffset) * mat4f::face(axisA, axisB) * mat4f::translation(-startBox.getCenter());
}

mat4f ScanLocator::makeModelToOOBBTransform(const DisembodiedObject &object, const OBBf &targetBox, float ZRotationOffset)
{
    mat4f baseTransform = object.makeBaseTransformCentered();
	OBBf objectBBox = baseTransform * OBBf(object.model->bbox);

    return alignBBoxes(objectBBox, targetBox, ZRotationOffset) * baseTransform;
}

// score for placing "object" at targetBox in the scan
float ScanLocator::scoreObject(const DisembodiedObject &object, const OBBf &targetBox) const
{
    mat4f fullTransformA = makeModelToOOBBTransform(object, targetBox, 0.0f);
    mat4f fullTransformB = makeModelToOOBBTransform(object, targetBox, 180.0f);
    
    return std::max(scoreObject(object, fullTransformA),
                    scoreObject(object, fullTransformB));
}

// score for placing "object", transformed by the given matrix, in the scan
float ScanLocator::scoreObject(const DisembodiedObject &object, const mat4f &modelToScan) const
{
    /*cout << "start score...";
    const auto &model = assets->loadModel(graphics->castD3D11(), object.modelId);
    
    //cout << "model.modelToVoxel" << endl << model.getModelToVoxel() << endl;
    //cout << "modelToScan" << endl << modelToScan << endl;
    //cout << "scan->scanToVoxel" << endl << scan->scanToVoxel << endl;

    mat4f gridToDF = model.getModelToVoxel() * modelToScan.getInverse() * scan->scanToVoxel.getInverse();

    const auto &distanceField = model.getDistanceField();

    //cout << "eval dist...";
    std::pair<float, size_t> distcomp = distanceField.evalDist(scan->voxelization, gridToDF, false);
    //cout << "done dist = " << distcomp.first << endl;

    float dist = distcomp.first;

    float conf = (float)distcomp.second / distanceField.getNumZeroVoxels();

    cout << "dist = " << distcomp.first << ", confidence = " << distcomp.second << endl;
    
    return 1.0f/dist;*/
    return 0.0f;
}

vector<ScanLocalizedObject> ScanLocator::locate(const ScanSegmentation &segmentation, bool floor, const vec3f &targetDimensions)
{
    vector<ScanLocalizedObject> result;

    if (!skipLocator)
    {
        ml::util::push_back(result, locate(segmentation, floor, makeFilter(targetDimensions, 0.04f, 0.8f), 1.0f));
        //ml::util::push_back(result, locate(segmentation, floor, makeFilter(targetDimensions, 0.075f, 0.75f), 0.5f));
    }
    return result;
}

ScanSegmentFilter ScanLocator::makeFilter(const vec3f &targetDimensions, float absoluteThreshold, float relativeScoreThreshold)
{
    vec3f sortedDimensions = targetDimensions;
    if (sortedDimensions.x > sortedDimensions.y)
        std::swap(sortedDimensions.x, sortedDimensions.y);
	auto filter = [=](const OBBf &o)
    {
        auto scoreAxis = [=](float targetLength, float length)
        {
            const float absoluteError = fabs(length - targetLength);

            if (absoluteError < absoluteThreshold)
                return 1;

            const float relativeScore = 1.0f - absoluteError / (targetLength + absoluteThreshold);

            if (relativeScore >= relativeScoreThreshold)
                return 1;

            const float maxAcceptableLength = 2.0f * targetLength + absoluteThreshold - relativeScoreThreshold * (targetLength + absoluteThreshold);
            if (length > maxAcceptableLength)
                return -1;
            return 0;
        };
        vec3f extent = o.getExtent();
        if (extent.x > extent.y)
            std::swap(extent.x, extent.y);
        int xAxis = scoreAxis(sortedDimensions.x, extent.x);
        int yAxis = scoreAxis(sortedDimensions.y, extent.y);
        int zAxis = scoreAxis(sortedDimensions.z, extent.z);

        if (xAxis == -1 || yAxis == -1 || zAxis == -1)
            return -1;
        if (xAxis == 1 && yAxis == 1 && zAxis == 1)
            return 1;
        return 0;
    };
    return filter;
}

vector<ScanLocalizedObject> ScanLocator::locate(const ScanSegmentation &segmentation, bool floor, ScanSegmentFilter filter, float score)
{
    vector<ScanLocalizedObject> result;
    for (UINT segmentIndex = 0; segmentIndex < segmentation.segments.size(); segmentIndex++)
    {
        ScanLocalizedObject object = locate(segmentation, floor, filter, segmentIndex);
        if (object.score > 0.0f)
        {
            object.score = score;
            result.push_back(object);
        }
    }
    return result;
}

ScanLocalizedObject ScanLocator::locate(const ScanSegmentation &segmentation, bool floor, ScanSegmentFilter filter, UINT seedSegmentIndex)
{
    const MeshSegment &seedSegment = segmentation.segments[seedSegmentIndex];

    int seedScore = filter(seedSegment.oobb);
    if (seedScore == -1)
        return ScanLocalizedObject();
    if (seedScore == 1)
        return ScanLocalizedObject(seedSegment.oobb, 1.0f);

    vector<UINT> candidateSegments;
    for (UINT segmentIndex = 0; segmentIndex < segmentation.segments.size(); segmentIndex++)
    {
        const auto &segment = segmentation.segments[segmentIndex];
        int segmentScore = filter(segment.oobb);
        if (segmentScore == 0)
            candidateSegments.push_back(segmentIndex);
    }

    vector<UINT> cumulativeSegments;
    cumulativeSegments.push_back(seedSegmentIndex);
    vector<vec3f> cumulativePoints = seedSegment.points;
	OBBf cumulativeOOBB = seedSegment.oobb;
    
    while (true)
    {
        int bestMergeSegment = -1;
        float bestMergeSize = std::numeric_limits<float>::max();

        for (UINT candidateSegmentIndex : candidateSegments)
        {
            const MeshSegment &candidateSegment = segmentation.segments[candidateSegmentIndex];
            if (!ml::util::contains(cumulativeSegments, candidateSegmentIndex) && oobbOverlap(cumulativeOOBB, candidateSegment.oobb) >= 0.0f)
            {
                vector<vec3f> newPoints = cumulativePoints;
                ml::util::push_back(newPoints, candidateSegment.points);

				OBBf newOOBB = MeshSegment::computeUpOrientedOBB(newPoints);
                float oobbSize = newOOBB.getDiagonalLength();
                int score = filter(newOOBB);
                if (score == 1)
                {
                    return ScanLocalizedObject(newOOBB, 0.5f);
                }
                if (score == 0 && oobbSize < bestMergeSize)
                {
                    bestMergeSegment = candidateSegmentIndex;
                    bestMergeSize = oobbSize;
                }
            }
        }

        if (bestMergeSegment == -1)
        {
            return ScanLocalizedObject();
        }

        cumulativeSegments.push_back(bestMergeSegment);
        ml::util::push_back(cumulativePoints, segmentation.segments[bestMergeSegment].points);
        cumulativeOOBB = MeshSegment::computeUpOrientedOBB(cumulativePoints);
    }
}

float ScanLocator::oobbOverlap(const OBBf &a, const OBBf &b)
{
    //float radius = (b.getCenter() - a.getCenter()).length();
    //return (a.getExtent().length() + b.getExtent().length() - radius);
    bbox3f aBox(a), bBox(b);
    if (aBox.intersects(bBox))
        return 1.0f;
    return -1.0f;
}
