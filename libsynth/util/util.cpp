
#include "libsynth.h"

//
// TODO: there is a *serious* problem where FreeImage causes a runtime error with boost if the freeimage header is included first.
//
#include <mLibBoost.h>

#include "mLibOpenMesh.h"
#include <mLibFreeImage.h>

string util::cacheDir()
{
    return "../../cache/";
}

void util::decompressedGZipFile(const string &filenameIn, const string &filenameOut)
{
    std::ifstream fileIn(filenameIn, std::ios_base::in | std::ios_base::binary);
    try {
        boost::iostreams::filtering_istream in;
        in.push(boost::iostreams::gzip_decompressor());
        in.push(fileIn);

        std::ofstream fileOut(filenameOut);
        for (string str; std::getline(in, str);)
        {
            fileOut << str << '\n';
        }
    }
    catch (const boost::iostreams::gzip_error& e) {
        cout << e.what() << endl;
    }
}

string util::copyToLocalCache(const string &path)
{
    const string dir = cacheDir();
    const string filename = ml::util::fileNameFromPath(path);
    const string targetFile = dir + filename;

    ml::util::makeDirectory(dir);

    if (!ml::util::fileExists(targetFile))
    {
        cout << "Copying " << path << " to local cache" << endl;
        ml::util::copyFile(path, targetFile);
    }

    return targetFile;
}

string util::copyToLocalCache(const string &path, const string &cacheSubDirectory)
{
    const string dir = cacheDir() + cacheSubDirectory + "/";
    const string filename = ml::util::fileNameFromPath(path);
    const string targetFile = dir + filename;

    ml::util::makeDirectory(dir);

    if (!ml::util::fileExists(targetFile))
    {
        cout << "Copying " << path << " to local cache" << endl;
        ml::util::copyFile(path, targetFile);
    }

    return targetFile;
}

string util::cacheModel(const string &modelId)
{
    if (ml::util::startsWith(modelId, "wss."))
        return cacheModelWSS(modelId);

    if (ml::util::startsWith(modelId, "hao."))
        return cacheModelHao(modelId);

    cout << "unknown model type: " << modelId << endl;
    return "";
}

string util::cacheModelHao(const string &modelId)
{
    const string strippedModelId = ml::util::split(modelId, ".").back();
    const string category = ml::util::split(modelId, ".")[1];

    const string localMeshPath = cacheDir() + "models/" + strippedModelId + ".mesh";
    ml::util::makeDirectory(cacheDir() + "models/");

    if (ml::util::fileExists(localMeshPath))
    {
        return localMeshPath;
    }

    const string modelDir = R"(V:\data\models\repositories\hao-shapenet\)" + category + "\\" + strippedModelId + "\\";
    const string remoteObjPath = modelDir + "model_aligned.obj";
    const string remoteMtlPath = modelDir + "model.mtl";

    if (!ml::util::fileExists(remoteObjPath) ||
        !ml::util::fileExists(remoteMtlPath))
    {
        cout << "Model data not found for " << modelId << endl;
        return "";
    }

    const string localObjPath = cacheDir() + "temp.obj";
    const string localMtlPath = cacheDir() + "temp.mtl";

    ml::util::copyFile(remoteObjPath, localObjPath);
    ml::util::copyFile(remoteMtlPath, localMtlPath);

    const ml::MeshDataf geometry = ml::MeshIO<float>::loadFromFile(localObjPath);
    const auto triMeshPairs = geometry.splitByMaterial(localMtlPath);

    vector<ml::TriMeshf> triMeshes;
    vector<ml::Materialf> materials;

    for (const auto &p : triMeshPairs)
    {
        triMeshes.push_back(std::move(p.first));
        materials.push_back(std::move(p.second));
    }

    auto simplifiedTriMeshes = triMeshes;

    std::ofstream meshFile(localMeshPath, std::ios::binary | std::ios::out);
    boost::archive::binary_oarchive archive(meshFile);
    archive << triMeshes << simplifiedTriMeshes << materials;
    meshFile.close();

    return localMeshPath;
}

string util::cacheModelWSS(const string &modelId)
{
    const string strippedModelId = ml::util::remove(modelId, "wss.");

    const string localMeshPath = cacheDir() + "models/" + strippedModelId + ".mesh";
    ml::util::makeDirectory(cacheDir() + "models/");

    if (ml::util::fileExists(localMeshPath))
    {
        return localMeshPath;
    }

    const string modelDir = R"(V:\data\models\repositories\g3dw\models\)";
    const string remoteObjGzPath = modelDir + strippedModelId + ".obj.gz";
    const string remoteMtlPath = modelDir + strippedModelId + ".mtl";

    if (!ml::util::fileExists(remoteObjGzPath) ||
        !ml::util::fileExists(remoteMtlPath))
    {
        cout << "Model data not found for " << modelId << endl;
        return "";
    }

    const string localObjPath = cacheDir() + "temp.obj";
    const string localMtlPath = cacheDir() + "temp.mtl";

    ml::util::copyFile(remoteMtlPath, localMtlPath);

    util::decompressedGZipFile(remoteObjGzPath, localObjPath);

    ml::MeshDataf geometry = ml::MeshIO<float>::loadFromFile(localObjPath);
    auto triMeshPairs = geometry.splitByMaterial(localMtlPath);

    vector<ml::TriMeshf> triMeshes;
    vector<ml::Materialf> materials;

    for (const auto &p : triMeshPairs)
    {
        triMeshes.push_back(std::move(p.first));
        materials.push_back(std::move(p.second));
    }

    //
    // here
    //
    auto simplifiedTriMeshes = triMeshes;

    for (auto& m : simplifiedTriMeshes) {
        if (m.getVertices().size() > 100) {
            auto meshData = m.getMeshData();
            //meshData.mergeCloseVertices(0.00001f, true);
            m = ml::TriMeshf(meshData);

            if (m.getVertices().size() > 1000) {
                size_t vertsBefore = m.getVertices().size();

                std::string fileIn = "testIn.ply";
                std::string fileOut = "testOut.ply";
                std::string program = "\"C:\\Program Files\\VCG\\MeshLab\\meshlabserver.exe\"";
                std::string scriptPath = "..\\..\\meshlabSimplify.mlx";

                std::remove(fileOut.c_str());

                ml::MeshIOf::saveToFile(fileIn, m.getMeshData());
                std::string command = program + " -i " + fileIn + " -o " + fileOut + " -s " + scriptPath + " 3> NUL 2> NUL 1> NUL";
                //std::string command = "dir > NUL";

                std::system(command.c_str());
                if (ml::util::fileExists(fileOut)) {
                    m = ml::TriMeshf(ml::MeshIOf::loadFromFile(fileOut));
                }
                else {
                    std::cout << "meshlab simplification failed" << std::endl;
                }

                //size_t vertsBefore = m.getVertices().size();
                //ml::OpenMeshTriMesh::decimate(m, 1000);

                size_t vertsNow = m.getVertices().size();
                if (vertsBefore != vertsNow)
                    std::cout << "mesh reduced from " << vertsBefore << " to " << vertsNow << std::endl;
            }
        }
    }

    std::ofstream meshFile(localMeshPath, std::ios::binary | std::ios::out);
    boost::archive::binary_oarchive archive(meshFile);
    archive << triMeshes << simplifiedTriMeshes << materials;
    meshFile.close();

    return localMeshPath;
}

void util::loadCachedModel(const string &modelId, vector<ml::TriMeshf> &meshes, vector<ml::TriMeshf> &simplifiedMeshes, vector<ml::Materialf> &materials)
{
    meshes.clear();
    materials.clear();

    const string localMeshPath = cacheModel(modelId);

    if (localMeshPath.size() == 0)
        return;

    std::ifstream meshFile(localMeshPath, std::ios::binary | std::ios::in);
    boost::archive::binary_iarchive archive(meshFile);

    archive >> meshes >> simplifiedMeshes >> materials;

    for (auto &m : meshes)
    {
        m.computeNormals();
        m.setColor(ml::vec4f(1.0f, 1.0f, 1.0f, 1.0f));
    }
    for (auto &m : simplifiedMeshes)
    {
        m.computeNormals();
        m.setColor(ml::vec4f(1.0f, 1.0f, 1.0f, 1.0f));
    }

    if (ml::util::startsWith(modelId, "hao."))
    {
        //
        // Switch from Y-up to Z-up
        //
        const mat4f transform = mat4f::face(vec3f::eY, vec3f::eZ);
        for (auto &m : meshes)
        {
            m.transform(transform);
            m = m.flatten();
            m.computeNormals();
        }
        for (auto &m : simplifiedMeshes)
            m.transform(transform);
    }

}

Bitmap util::loadCachedTexture(const string &textureId)
{
    const string localTexturePath = cacheDir() + "textures/" + textureId;
    ml::util::makeDirectory(cacheDir() + "textures/");

	Bitmap result;

    if (!ml::util::fileExists(localTexturePath))
    {
        if (ml::util::startsWith(textureId, "hao-"))
        {
            //
            // Hao-shapenet texture
            //

            const auto parts = ml::util::split(textureId, '-');
            const string category = parts[1];
            const string modelId = parts[2];
            const string textureFilename = parts[3];

            string remoteTexturePath = R"(V:\data\models\repositories\hao-shapenet\)" + category + "\\" + modelId + "\\images\\" + textureFilename;

            if (!ml::util::fileExists(remoteTexturePath))
                remoteTexturePath = R"(V:\data\models\repositories\hao-shapenet\)" + category + "\\" + modelId + "\\models\\untitled\\" + textureFilename;

            if (!ml::util::fileExists(remoteTexturePath))
            {
                cout << "Texture data not found for " << textureId << endl;
                return result;
            }

            if (ml::util::endsWith(textureFilename, ".gif"))
            {
                cout << "Skipping GIF file: " << textureId << endl;
                return result;
            }

            cout << "Copying " << textureId << " to local cache" << endl;
            ml::util::copyFile(remoteTexturePath, localTexturePath);
        }
        else
        {
            //
            // g3dw texture
            //

            const string textureDir = R"(V:\data\models\repositories\g3dw\texture\)";
            const string overrideTexturePath = synthParams().synthesisDir + "textureOverride/" + textureId;

            string remoteTexturePath = textureDir + textureId;


            if (ml::util::fileExists(overrideTexturePath))
                remoteTexturePath = overrideTexturePath;

            if (!ml::util::fileExists(remoteTexturePath))
            {
                cout << "Texture data not found for " << textureId << endl;
                return result;
            }

            cout << "Copying " << textureId << " to local cache" << endl;
            ml::util::copyFile(remoteTexturePath, localTexturePath);
        }
    }

    ml::BaseImage<ml::vec4uc> image;
    ml::FreeImageWrapper::loadImage(localTexturePath, image);

    result.allocate(image.getWidth(), image.getHeight());
    for (UINT y = 0; y < image.getHeight(); y++)
        for (UINT x = 0; x < image.getWidth(); x++)
        {
            ml::vec4uc c = image(x, y);

            auto &o = result(x, y);
            o.r = c.x;
            o.g = c.y;
            o.b = c.z;
        }

    return result;
}

float util::frameZRotation(const ml::bbox3f &modelBBox, const mat4f &transform)
{
    vec3f worldCenter = transform * modelBBox.getCenter();
    vec3f worldEdge = transform * (modelBBox.getCenter() + vec3f::eX) - worldCenter;
    return vec3f::angleBetween(worldEdge, vec3f::eX);
}

TriMeshf util::subdivideRoom(const TriMeshf &roomMesh, float edgeLengthThreshold)
{
    return roomMesh.flatLoopSubdivision(20, edgeLengthThreshold);

    //ml::MeshDataf meshData = roomMesh.getMeshData();
    
    //while (true) {
    //    const float maxEdgeLength = meshData.subdivideFacesLoop(edgeLengthThreshold);
    //    if (maxEdgeLength <= edgeLengthThreshold) { break; }
    //}
    
    //return TriMeshf(meshData);
}

void util::processResultsFolder()
{
    Directory dir(R"(V:\data\synthesis\judgementStudyResults)");

    struct Counter
    {
        Counter()
        {
            sum = 0.0;
            count = 0;
        }
        double averageRating() const
        {
            return sum / count;
        }
        double sum;
        int count;
    };

    struct Entry
    {
        Counter& getCounter(const string &s)
        {
            if (s == "us") return us;
            if (s == "prev") return prev;
            if (s == "human") return human;
            cout << "unknown counter: " << s << endl;
            return human;
        }
        Counter us;
        Counter prev;
        Counter human;
    };

    map<string, Entry> entries;

    double totalSamples = 0, goodSamples = 0;

    for (const string &file : dir.filesWithSuffix(".csv"))
    {
        if (file == "results.csv")
            continue;
        for (const string &line : ml::util::getFileLines(dir.path() + file))
        {
            //scan,condition,image,score
            //angie-new,prev,0022.tiff,3
            auto parts = ml::util::split(line, ',');
            if (parts.size() == 4)
            {
                if (parts[0] == "scan")
                    continue;
                int score = ml::convert::toInt(parts[3]);
                if (score == -1)
                {
                    cout << "Unevaluated scene: " << line << endl;
                }
                else
                {
                    Entry &e = entries[parts[0]];
                    Counter &c = e.getCounter(parts[1]);

                    c.count++;
                    c.sum += score;

                    if (parts[1] == "us")
                    {
                        totalSamples++;
                        if (score == 4 || score == 5)
                            goodSamples++;
                    }
                }
            }
        }
    }

    ofstream resultFile(dir.path() + "results.csv");

    resultFile << "Scan,Human-made,Activity-based Synthesis,Generic Synthesis" << endl;
    for (const auto &e : entries)
    {
        resultFile << e.first << "," << e.second.human.averageRating() << "," << e.second.us.averageRating() << "," << e.second.prev.averageRating() << endl;
    }

    resultFile << endl << goodSamples / totalSamples * 100 << "% of samples marked as good" << endl;
}
