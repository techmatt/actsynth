
#include "libsynth.h"

void Scan::load(const string &meshFilename, const string &alignmentFilename, const string &agentFilename)
{
    pathName = ml::util::removeExtensions(meshFilename);
    ml::MeshDataf meshData = ml::MeshIOf::loadFromFile(meshFilename);

#ifndef _DEBUG
    meshData.removeIsolatedVertices();
#endif

    baseMesh = ml::TriMeshf(meshData);
    
    baseMesh.computeNormals();

    rotation = 0.0f;
    translation = vec3f::origin;

    if (ml::util::fileExists(alignmentFilename))
    {
        ifstream file(alignmentFilename);
        file >> translation.x >> translation.y >> translation.z;
        file >> rotation;
    }

    translation.z = 0.075f;

    if (ml::util::fileExists(agentFilename))
    {
        loadAgentAnnotations(agentFilename);
    }

    transform = mat4f::translation(translation) * mat4f::rotationZ(rotation);

    transformedMesh = baseMesh;
    transformedMesh.transform(transform);

#ifdef _DEBUG
    float segmentationThresholds[segmentationCount] = { 0.1f };
#else
    float segmentationThresholds[segmentationCount] = { 0.005f, 0.02f, 0.05f };
#endif

    const bool skipSegmentation = true;
    if (!skipSegmentation)
    {
        for (UINT segmentationIndex = 0; segmentationIndex < segmentationCount; segmentationIndex++)
            //for (UINT segmentationIndex = 0; segmentationIndex < 1; segmentationIndex++)
        {
            SegmentorFelzenswalb segmentor(segmentationThresholds[segmentationIndex], 100, 0.0f);
            segmentations[segmentationIndex].segments = segmentor.makeFilteredSegments(transformedMesh);
        }
    }

    const bool skipVoxelization = true;
    if (!skipVoxelization)
    {
        const float voxelSize = 0.05f;
        auto voxelandTransform = transformedMesh.voxelize(voxelSize);
        voxelization = std::move(voxelandTransform.first);
        scanToVoxel = voxelandTransform.second;
        const float truncation = 1.0f / voxelSize;
        distanceField.generateFromBinaryGrid(voxelization, truncation);
    }
}

void Scan::saveAlignment(const string &filename) const
{
    ofstream file(filename);
    file << translation.x << " " << translation.y << " " << translation.z << endl;
    file << rotation << endl;
}

const ScanSegmentation& Scan::getSegmentationAtResolution(float maxDimension) const
{
#ifdef _DEBUG
    return segmentations[0];
#endif

    if (maxDimension < 0.4f)
        return segmentations[0];
    return segmentations[2];
}
