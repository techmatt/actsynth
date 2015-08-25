
//
// -1 = unacceptable, at least one axis is too large
// 0 = unacceptable, too small
// 1 = acceptable
//
typedef std::function<int(const OBBf&)> ScanSegmentFilter;

struct ScanLocalizedObject
{
    ScanLocalizedObject()
    {
        score = 0.0f;
    }
	ScanLocalizedObject(const OBBf &_oobb, float _score)
    {
        oobb = _oobb;
        score = _score;
    }
    OBBf oobb;
    float score;
};

struct ScanLocatorResults
{
    string modelId;
    DisembodiedObject *object;
    float maxScore;
    vector<ScanLocalizedObject> locations;
};

namespace ml
{
    class D3D11GraphicsDevice;
}
class AssetManager;

class ScanLocator
{
public:
    void init(const Database &_database, ml::D3D11GraphicsDevice &_graphics, AssetManager &_assets, const Scan &_scan);

    static string nameObject(const Scan &scan, const DisembodiedObject &object)
    {
        return ml::util::replace(scan.name + "-" + object.modelId + "-" + to_string(object.scale), '.', '_');
    }

    const ScanLocatorResults& locateModel(const DisembodiedObject &object);

	float scoreObject(const DisembodiedObject &object, const OBBf &targetBox) const;
    float scoreObject(const DisembodiedObject &object, const mat4f &modelToScan) const;

	static mat4f makeModelToOOBBTransform(const DisembodiedObject &object, const OBBf &targetBox, float ZRotationOffset);

    static ScanSegmentFilter makeFilter(const vec3f &targetDimensions, float absoluteThreshold, float relativeScoreThreshold);
    static vector<ScanLocalizedObject> locate(const ScanSegmentation &segmentation, bool floor, const vec3f &targetDimensions);
    static vector<ScanLocalizedObject> locate(const ScanSegmentation &segmentation, bool floor, ScanSegmentFilter filter, float score);
    static ScanLocalizedObject locate(const ScanSegmentation &segmentation, bool floor, ScanSegmentFilter filter, UINT seedSegmentIndex);

private:
    void cacheModel(const DisembodiedObject &object);
	static float oobbOverlap(const OBBf &a, const OBBf &b);
	static mat4f alignBBoxes(const OBBf &startBox, const OBBf &endBox, float ZRotationOffset);

    const Scan *scan;
    const Database *database;
    ml::D3D11GraphicsDevice *graphics;
    AssetManager *assets;

    map<string, ScanLocatorResults> cache;
};
