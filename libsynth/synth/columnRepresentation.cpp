
#include "libsynth.h"

void ColumnRepresentation::reset(const ColumnRepresentation &base)
{
    *this = base;
    for (auto &c : columns)
    {
        c.value.maxAnyZ = 0.0f;
        c.value.maxSupportZ = 0.0f;
    }
}

double ColumnRepresentation::score(const ColumnRepresentation &a, const ColumnRepresentation &b)
{
    const float maxSupportDist = 0.75f;
    const float maxResidueDist = 0.5f;

    auto falloffFunction = [](float maxValue, float x)
    {
        if (x > maxValue)
            return 0.0f;
        return 1.0f - x / maxValue;
    };

    double sum = 0.0;
    for (UINT y = 0; y < a.columns.getDimY(); y++)
    {
        for (UINT x = 0; x < a.columns.getDimX(); x++)
        {
            auto &aVal = a.columns(x, y);
            auto &bVal = b.columns(x, y);
            sum += falloffFunction(maxSupportDist, fabs(aVal.maxSupportZ - bVal.maxSupportZ));
            sum += falloffFunction(maxSupportDist, fabs(aVal.residueSize() - bVal.residueSize()));
        }
    }
    return sum * 0.5f / float(a.columns.getNumElements());
}

ColumnRepresentation ColumnRepresentation::constructFromScan(const Scan &s, float _columnDimension, const GeometricRepresentation &geometry)
{
    ColumnRepresentation result;
    result.columnDimension = _columnDimension;

    const float buffer = 0.25f;
    bbox3f scanBBox = s.transformedMesh.getBoundingBox();
    result.bbox = bbox2f(vec2f(scanBBox.getMinX() - buffer, scanBBox.getMinY() - buffer), vec2f(scanBBox.getMaxX() + buffer, scanBBox.getMaxY() + buffer));

    UINT xCount = ml::math::ceil(result.bbox.getExtentX() / result.columnDimension);
    UINT yCount = ml::math::ceil(result.bbox.getExtentY() / result.columnDimension);

    result.columns.allocate(xCount, yCount);

    for (UINT y = 0; y < yCount; y++)
    {
        for (UINT x = 0; x < xCount; x++)
        {
            auto &c = result.columns(x, y);
            //c.center = result.bbox.getMin() + vec2f(x * result.columnDimension, y * result.columnDimension);
            c.maxAnyZ = 0.0f;
            c.maxSupportZ = 0.0f;
        }
    }

    cout << "Creating scan accelerator" << endl;
    ml::TriMeshAcceleratorBVHf *accel = new ml::TriMeshAcceleratorBVHf();
    accel->build(s.transformedMesh);
    
    result.includeObjectRayTracing(*accel, scanBBox, mat4f::identity(), mat4f::identity(), false);

    //Timer t0;
    //result.includeObjectRayTracing(*accel, scanBBox, mat4f::identity(), mat4f::identity(), false);
    //cout << "Tracing time A: " << t0.getElapsedTime() << "s" << endl;

    //result.includeObjectRayTracing(*accel, scanBBox, mat4f::identity(), mat4f::identity(), false);

    //Timer t1;
    //result.includeObjectRayTracing(*accel, scanBBox, mat4f::identity(), mat4f::identity(), false);
    //cout << "Tracing time B: " << t1.getElapsedTime() << "s" << endl;

    for (const Plane &plane : geometry.planes)
    {
        const float minSupportSurfaceArea = 1.0f * 1.0f;
        const float supportingPlaneZOffset = 0.02f;
        if (fabs(plane.normal.z) > 0.8f)
        {
            bbox3f bbox(plane.convex_hull);
            bbox.transform(mat4f::translation(-vec3f::eZ * supportingPlaneZOffset));
            if (bbox.getExtentX() * bbox.getExtentY() > minSupportSurfaceArea)
                result.includeObjectBBoxSimplification(OBBf(bbox), true);
        }
    }

#ifndef _DEBUG
    //
    // very silly, but deleting a BVH in debug is way too costly (taking 10+ minutes)
    //
    delete accel;
#endif

    return result;
}

void ColumnRepresentation::includeObjectBBoxSimplification(const OBBf &obb, bool isSupport)
{
    const float floorThreshold = 0.25f;

    Trianglef t[2];
    SynthUtil::makeBBoxTopTriangles(obb, t[0], t[1]);

    for (UINT y = 0; y < columns.getDimY(); y++)
    {
        for (UINT x = 0; x < columns.getDimX(); x++)
        {
            SceneColumn &c = columns(x, y);
            const vec2f center = getColumnCenter(x, y);
            float iMax = std::max(findHighestInteresction(center, t[0]), findHighestInteresction(center, t[1]));

            if (iMax < floorThreshold)
                iMax = 0.0f;

            c.maxAnyZ = std::max(c.maxAnyZ, iMax);
            if (isSupport)
                c.maxSupportZ = std::max(c.maxSupportZ, iMax);
        }
    }
}

void ColumnRepresentation::includeObjectRayTracing(const ml::TriMeshRayAcceleratorf &accel, const bbox3f &worldBBox, const mat4f &modelToWorld, const mat4f &worldToModel, bool isSupport)
{
    const float floorThreshold = 0.25f;

    vector< pair<const TriMeshRayAcceleratorf*, ml::mat4f> > transformedAccelerator;
    transformedAccelerator.push_back(std::make_pair(&accel, worldToModel));

    for (UINT y = 0; y < columns.getDimY(); y++)
    {
        for (UINT x = 0; x < columns.getDimX(); x++)
        {
            SceneColumn &c = columns(x, y);
            const vec2f center = getColumnCenter(x, y);
            float iMax = 0.0f;

            const int offsetRadius = 1;
            for (int xOffset = -offsetRadius; xOffset <= offsetRadius; xOffset++)
            {
                for (int yOffset = -offsetRadius; yOffset <= offsetRadius; yOffset++)
                {
                    vec2f offset(columnDimension * 0.5f * float(xOffset) / (offsetRadius + 0.2f), columnDimension * 0.5f * float(yOffset) / (offsetRadius + 0.2f));

                    vec3f centerTest(center + offset, worldBBox.getCenter().z);
                    if (worldBBox.intersects(centerTest))
                    {
                        iMax = std::max(iMax, findHighestInteresction(center + offset, transformedAccelerator, modelToWorld, worldToModel));
                    }
                }
            }

            if (iMax < floorThreshold)
                iMax = 0.0f;

            c.maxAnyZ = std::max(c.maxAnyZ, iMax);
            if (isSupport)
                c.maxSupportZ = std::max(c.maxSupportZ, iMax);
        }
    }
}

void ColumnRepresentation::includeObjectMaxZMap(const ml::Grid2f& maxZMap, const mat4f &modelToWorld, const mat4f &worldToModel, const mat4f& modelToVoxel, bool isSupport)
{
    const float floorThreshold = 0.25f;

	mat4f worldToModelVoxel = modelToVoxel * worldToModel;
	mat4f modelVoxelToWorld = worldToModelVoxel.getInverse();


    std::array<vec3f, 4> points;
    points[0] = vec3f(0, 0, 0);
    points[1] = vec3f(0, (float)maxZMap.getDimY(), 0);
    points[2] = vec3f((float)maxZMap.getDimX(), (float)maxZMap.getDimY(), 0);
    points[3] = vec3f((float)maxZMap.getDimX(), 0, 0);

    bbox2f minbounds;
    for (auto& p : points) {
        minbounds.include((modelVoxelToWorld * p).getVec2());
    }

    minbounds.setMin(ml::math::max(minbounds.getMin(), bbox.getMin()));
    minbounds.setMax(ml::math::min(minbounds.getMax(), bbox.getMax()));
    
    UINT xMin = (UINT)((minbounds.getMin().x - bbox.getMinX()) / columnDimension);
    UINT xMax = (UINT)((minbounds.getMax().x - bbox.getMinX()) / columnDimension + 1);

    UINT yMin = (UINT)((minbounds.getMin().y - bbox.getMinY()) / columnDimension);
    UINT yMax = (UINT)((minbounds.getMax().y - bbox.getMinY()) / columnDimension + 1);

    xMax = std::min<UINT>((UINT)columns.getDimX() - 1u, xMax);
    yMax = std::min<UINT>((UINT)columns.getDimY() - 1u, yMax);

    for (size_t j = yMin; j < yMax; j++) {
        for (size_t i = xMin; i < xMax; i++) {

            SceneColumn& c = columns(i, j);
            const vec2f center = getColumnCenter((int)i, (int)j);

            float iMax = 0.0f;

            const int offsetRadius = 1;
            for (int xOffset = -offsetRadius; xOffset <= offsetRadius; xOffset++)
            {
                for (int yOffset = -offsetRadius; yOffset <= offsetRadius; yOffset++)
                {
                    vec2f offset(columnDimension * 0.5f * float(xOffset) / (offsetRadius + 0.2f), columnDimension * 0.5f * float(yOffset) / (offsetRadius + 0.2f));

                    vec3f lookUpf = worldToModelVoxel * vec3f(center + offset, 0.0f);	//this is [0;1]^2
                    unsigned int x = ml::math::round(lookUpf.x);
                    unsigned int y = ml::math::round(lookUpf.y);
                    if (maxZMap.isValidCoordinate(x, y))
                    {
                        lookUpf.z = maxZMap(x, y);
                        vec3f reProj = modelVoxelToWorld * lookUpf;

                        iMax = std::max(iMax, reProj.z);
                    }
                }
            }

            if (iMax < floorThreshold)
                iMax = 0.0f;

            c.maxAnyZ = std::max(c.maxAnyZ, iMax);
            if (isSupport)
                c.maxSupportZ = std::max(c.maxSupportZ, iMax);
        }
	}
}

/*ColumnRepresentation ColumnRepresentation::constructFromTriangles(const ColumnRepresentation &base, const vector<Trianglef> &supportTriangles, const vector<Trianglef> &residueTriangles)
{
    //
    // TODO: this function could also be done with a ray accelerator, which would be faster depending on the number of triangles
    //

    auto findHighestInteresction = [](const vec2f &center, const vector<Trianglef> &triangles)
    {
        float maxZ = 0.0f;
        Rayf ray(vec3f(center, maxZ), -vec3f::eZ);
        for (const Trianglef &tri : triangles)
        {
            float t, u, v;
            if (ml::intersection::intersectRayTriangle(tri, ray, t, u, v))
            {
                vec3f intersect = ray.getHitPoint(t);
                maxZ = std::max(maxZ, intersect.z);
            }
        }
        return maxZ;
    };

    ColumnRepresentation result = base;
    for (SceneColumn &c : result.columns)
    {
        float supportMax = findHighestInteresction(c.center, supportTriangles);
        float residueMax = findHighestInteresction(c.center, residueTriangles);

        c.planeZ = supportMax;
        c.planeResidue = 0.0f;
        if (residueMax > supportMax)
            c.planeResidue = residueMax - supportMax;
    }
    return result;
}*/
