
struct TextureData
{
    string name;
    ml::D3D11Texture2D texture;
};

struct ModelMeshData
{
    ModelMeshData()
    {
        texture = NULL;
    }
    //
    // mesh
    //
    ml::D3D11TriMesh meshOriginal;
	ml::D3D11TriMesh meshSimplified;

    //
    // material
    //
    ml::Materialf material;

    //
    // texture
    //
    const TextureData *texture;
};

struct ModelAssetData
{
    ~ModelAssetData()
    {
        
    }

    void makeAllCaches() const
    {
        const auto &a = getAcceleratorOriginal();
        const auto &b = getAcceleratorSimplified();
        const auto &c = getAcceleratorBBoxOnly();
        const auto &d = getModelToVoxel();
        const auto &e = getVoxelization();
        const auto &f = getMaxZMap();
    }

    vector < pair < const ml::TriMeshf*, mat4f> > getMeshOriginalList(const mat4f &transform) const
    {
        vector < pair < const ml::TriMeshf*, mat4f> > result;
        for (const ModelMeshData &m : data)
			result.push_back(std::make_pair(&m.meshOriginal.getTriMesh(), transform));
        return result;
    }
	vector < pair < const ml::TriMeshf*, mat4f> > getMeshSimplifiedList(const mat4f &transform) const
	{
		vector < pair < const ml::TriMeshf*, mat4f> > result;
		for (const ModelMeshData &m : data)
			result.push_back(std::make_pair(&m.meshSimplified.getTriMesh(), transform));
		return result;
	}
	const ml::TriMeshAcceleratorBVHf& getAcceleratorOriginal() const
    {
		if (acceleratorOriginalCache.triangleCount() == 0)
			acceleratorOriginalCache.build(getMeshOriginalList(mat4f::identity()));
		return acceleratorOriginalCache;
    }
	const ml::TriMeshAcceleratorBVHf& getAcceleratorSimplified() const
	{
		if (acceleratorSimplifiedCache.triangleCount() == 0)
			acceleratorSimplifiedCache.build(getMeshSimplifiedList(mat4f::identity()));
		return acceleratorSimplifiedCache;
	}
    const ml::TriMeshAcceleratorBVHf& getAcceleratorBBoxOnly() const
    {
        if (acceleratorBBoxOnlyCache.triangleCount() == 0)
        {
            bbox3f bbox;
            for (const auto &m : getMeshOriginalList(mat4f::identity()))
                bbox.include(m.first->getBoundingBox());
            acceleratorBBoxOnlyCache.build(ml::Shapesf::box(bbox), true);
        }
        return acceleratorBBoxOnlyCache;
    }
    const ml::BinaryGrid3& getVoxelization() const
    {
        if (voxelizationCache.getDimX() == 0)
        {
            bbox3f modelBBox;
            for (const auto &d : data)
                modelBBox.include(d.meshOriginal.getTriMesh().getBoundingBox());
            modelBBox.scale(1.1f);

            UINT maxVoxelDimension = 64;

            if (synthParams().haoSynthesisMode)
                maxVoxelDimension = 10;

            float voxelSize = modelBBox.getMaxExtent() / maxVoxelDimension;
            modelToVoxelCache = mat4f::scale(1.0f / voxelSize) * mat4f::translation(-modelBBox.getMin());

            /*cout << "modelToVoxel:" << endl << modelToVoxel << endl;
            cout << "voxel size: " << voxelSize << endl;
            cout << "bbox min: " << modelBBox.getMin() << endl;*/
            
            vec3ui dimensions = ml::math::max(vec3ui(modelBBox.getExtent() / voxelSize), 1U);
            
            voxelizationCache.allocate(dimensions);
            
            for (const auto &d : data)
                d.meshOriginal.getTriMesh().voxelize(voxelizationCache, modelToVoxelCache, false);
        }
        return voxelizationCache;
    }
    /*const ml::DistanceField3f& getDistanceField() const
    {
        if (distanceFieldCache.getDimX() == 0)
        {
            //ml::Timer t;
            //cout << "computing voxelization...";

            const auto &voxelization = getVoxelization();

            //cout << t.getElapsedTimeMS();
            //cout << " ms done " << std::endl;
            //cout << "computing distance field...";
            //t.start();
            distanceFieldCache.generateFromBinaryGrid(voxelization, 10);

            //std::cout << t.getElapsedTimeMS();
            //cout << " ms done" << endl;
            //std::cout << voxelization.getDimensions() << std::endl;
            //std::cout << std::endl << std::endl;
        }
        return distanceFieldCache;
    }*/
    const mat4f& getModelToVoxel() const
    {
        getVoxelization();
        return modelToVoxelCache;
    }

	const ml::Grid2f& getMaxZMap() const {
		if (maxZMap.getDimX() == 0) {
			const ml::BinaryGrid3& voxelization = getVoxelization();	//this is just a hack to get the dimension
			maxZMap.allocate(voxelization.getDimX(), voxelization.getDimY());
			
			const ml::TriMeshAcceleratorBVHf& accel = getAcceleratorOriginal();
			const mat4f voxelToModel = modelToVoxelCache.getInverse();

			for (size_t j = 0; j < maxZMap.getDimY(); j++) {
				for (size_t i = 0; i < maxZMap.getDimX(); i++) {
					
					vec3f p((float)i, (float)j, (float)voxelization.getDimZ() + 5);
					p = voxelToModel * p;

					ml::TriMeshAcceleratorBVHf::Intersection intersect = accel.intersect(ml::Rayf(p, -vec3f::eZ));
					if (intersect.isValid()) {
						maxZMap(i, j) = (modelToVoxelCache * intersect.getSurfacePosition()).z;
					}
					else {
						maxZMap(i, j) = 0.0f;
					}
				}
			}

		}
		return maxZMap;
	}

    vector<ModelMeshData> data;

private:
    mutable ml::TriMeshAcceleratorBVHf acceleratorOriginalCache;
	mutable ml::TriMeshAcceleratorBVHf acceleratorSimplifiedCache;
    mutable ml::TriMeshAcceleratorBVHf acceleratorBBoxOnlyCache;

    mutable ml::BinaryGrid3 voxelizationCache;
    //mutable ml::DistanceField3f distanceFieldCache;
    mutable ml::mat4f modelToVoxelCache;
	mutable ml::Grid2f maxZMap;
};

class AssetManager
{
public:
    AssetManager()
    {
        database = nullptr;
    }
    void init(const Database &_database)
    {
        database = &_database;
    }

    void loadAllModels(ml::GraphicsDevice &g, const set<string> &categories);
    void clearCategory(const string &category);
    void clear();

    const ModelAssetData& loadModel(ml::GraphicsDevice &g, const string &modelId);
    const TextureData& loadTexture(ml::GraphicsDevice &g, const string &modelId, const string &textureId);

    void swapTexture(ml::GraphicsDevice &g, const string &textureId, const string &filename);

private:
    map<string, ModelAssetData> models;
    map<string, TextureData> textures;

    const Database *database;
};
