
namespace util
{
    //
    // Cache functions
    //
    string cacheDir();
    string copyToLocalCache(const string &path);
    string copyToLocalCache(const string &path, const string &cacheSubDirectory);

    string cacheModel(const string &modelId);
    string cacheModelWSS(const string &modelId);
    string cacheModelHao(const string &modelId);

	void loadCachedModel(const string &modelId, vector<ml::TriMeshf> &meshes, vector<ml::TriMeshf> &simplifiedMeshes, vector<ml::Materialf> &materials);
    Bitmap loadCachedTexture(const string &textureId);
    //Bitmap loadCachedTextureHao(const string &modelId, const string &textureId);

    //
    // GZip functions
    //
    void decompressedGZipFile(const string &filenameIn, const string &filenameOut);
    //stringstream getDecompressedGZipStream(const string &filename);

    //
    // Matrix functions
    //
    float frameZRotation(const ml::bbox3f &modelBBox, const mat4f &transform);

    //
    // Boost property tree functions
    //
    template <class treeType>
    void writePTreeSchema(const treeType &tree, std::ostream &os)
    {
        std::set<string> emittedLines;
        writePTreeSchema(tree, os, "root", emittedLines);
    }

    template <class treeType>
    void writePTreeSchema(const treeType &tree, std::ostream &os, const string &path, std::set<string> &emittedLines)
    {
        for (const auto& nodePair : tree)
        {
            string fullPath = path + ":" + nodePair.first;
            if (emittedLines.count(fullPath) == 0)
            {
                emittedLines.insert(fullPath);
                os << fullPath << ":" << nodePair.second.get_value<string>() << endl;
            }
            if (nodePair.first == "data")
            {
                treeType scene;
                boost::property_tree::read_json(std::istringstream(nodePair.second.get_value<string>()), scene);
                writePTreeSchema(scene, os, fullPath, emittedLines);
            }
            writePTreeSchema(nodePair.second, os, fullPath, emittedLines);
        }
    }

    template <class treeType>
    vector<float> readPTreeVectorFloat(const treeType &tree)
    {
        vector<float> result;
        for (const auto &node : tree)
            result.push_back(std::stof(node.second.get<string>("")));
        return result;
    }

    TriMeshf subdivideRoom(const TriMeshf &roomMesh, float edgeThreshold);

    void processResultsFolder();
    void makeSupplementalResults();
}
