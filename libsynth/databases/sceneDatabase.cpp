#include "libsynth.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

vector<const Agent*> SceneDatabase::agents() const
{
    vector<const Agent*> result;
    for (const auto &s : scenes)
        for (const auto &a : s.second.agents)
            result.push_back(&a);
    return result;
}

vector<const Agent*> SceneDatabase::agents(const string &activityFilter) const
{
    vector<const Agent*> result;
    for (const auto &s : scenes)
        for (const auto &a : s.second.agents)
            if (a.activity == activityFilter)
                result.push_back(&a);
    return result;
}

void SceneDatabase::output(const string &baseDir) const
{
    ml::util::makeDirectory(baseDir + "scenes/");
    for (const auto &p : scenes)
    {
        p.second.output(baseDir + "scenes/" + p.first + ".html");
    }
}

void SceneDatabase::load(const string &scenesCSVFile, const string &sceneListFile, const string &synthesisDataDirectory, int maxScenes)
{
	using boost::property_tree::ptree;  using boost::property_tree::read_json;
	
    cout << "Loading scenes from " << scenesCSVFile << "..." << endl;

    std::set<string> sceneList;
    for (const string &line : ml::util::getFileLines(sceneListFile, 3))
        sceneList.insert(ml::util::split(line, "\t")[0]);

    auto allScenes = ml::util::getFileLines(scenesCSVFile, 50);
	
    //std::ofstream schemaFile("sceneSchema.txt");
    //util::writePTreeSchema(scenesData, schemaFile);
    
    //
	// Iterate over scene nodes
    //
    for (const string &line : allScenes)
    {
        auto parts = ml::util::splitOnFirst(line, ",\"");
        const string sceneId = parts.first;
        if (sceneList.count(sceneId) == 0 ||
            (maxScenes > 0 && scenes.size() >= maxScenes))
            continue;
		
        Scene &scene = scenes[sceneId];
        scene.id = sceneId;

		float sceneScaleFactor = 0.0254f;
        
        //
		// Parse stringified scene data into property tree
        //
		ptree parsedData;
        parts.second.pop_back();
        read_json(std::istringstream(parts.second), parsedData);
        for (const auto& modelPair : parsedData.get_child("objects"))
        {
			const ptree& modelTree = modelPair.second;

            //
            // Example modelTree structure:
            //
            //{"format":"ssj","objects":[{"index":0,"modelID":"room01","parentIndex":-1,"renderStateArr":[true,false,false,false],"cu":[1,0,0],"cv":[0,1,0],"cw":[0,0,1],"parentMeshI":-1,"parentTriI":-1,"parentUV":[0,0],"cubeFace":0,"scale":1,"rotation":0,"transform":[1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1]},{"index":1,"modelID":"c339231d8110cbdfe3f7a74e12a274ef","parentIndex":0,"renderStateArr":[true,false,false,true],"cu":[1,0,0],"cv":[0,1,0],"cw":[0,0,1],"parentMeshI":0,"parentTriI":21,"parentUV":[0.4529224634170532,0.2506796419620514],"cubeFace":0,"scale":0.0013063172204521396,"rotation":0,"transform":[0.0013063171645626426,0,0,0,0,0.0013063171645626426,0,0,0,0,0.0013063171645626426,0,18.62924575805664,62.475196838378906,3.0926268100738525,1]},{"index":2,"modelID":"3d3b4b8874f2aaddc397356311cbeea4","parentIndex":0,"renderStateArr":[true,false,false,true],"cu":[1,0,0],"cv":[0,1,0],"cw":[0,0,1],"parentMeshI":0,"parentTriI":21,"parentUV":[0.1570987105369568,0.7129069566726685],"cubeFace":0,"scale":1.6556449971777984,"rotation":1.5707963267948966,"transform":[1.0137902426392756e-16,1.6556450128555298,0,0,-1.6556450128555298,1.0137902426392756e-16,0,0,0,0,1.6556450128555298,0,52.8715934753418,14.002052307128906,3.0926268100738525,1]},{"index":3,"modelID":"19198a5f2d98f65ae3f7a74e12a274ef","parentIndex":2,"renderStateArr":[true,false,false,true],"cu":[-3.3306693385732656e-16,1,-4.311977389697666e-16],"cv":[-1,1.6653346692866328e-16,-9.806275130908406e-16],"cw":[5.911412763339016e-16,-1.5428096329306242e-16,1],"parentMeshI":2,"parentTriI":107,"parentUV":[0.5619503259658813,0.3918013870716095],"cubeFace":0,"scale":2.3890855674264966,"rotation":-1.1845213053989068,"transform":[-0.9000653028488159,2.213054895401001,1.782076429376585e-15,0,-2.213054895401001,-0.9000653028488159,-1.8368930135910944e-15,0,3.6859041040420825e-16,1.4122870438355384e-15,2.389085531234741,0,75.24629211425781,7.660619735717773,31.269908905029297,1]}],"cameras":[{"name":"current","eye":[233.65310668945312,-150.615234375,81.56144714355469],"lookAt":[151.10118103027344,109.3305435180664,44.025691986083984],"up":[-0.020738713443279266,0.06530353426933289,0.49784162640571594]}]}

			// Get index and modelID
            const int modelIndex = std::stoi(modelTree.get<string>("index"));
			
            while (scene.objects.size() <= modelIndex)
                scene.objects.push_back( ObjectInstance(&scene) );

            ObjectInstance &objectInstance = scene.objects[modelIndex];
            
            objectInstance.object.modelId = modelTree.get<string>("modelID");

            if (!ml::util::startsWith(objectInstance.object.modelId, "wss.") ||
                !ml::util::startsWith(objectInstance.object.modelId, "archive3d."))
            {
                objectInstance.object.modelId = "wss." + objectInstance.object.modelId;
            }

            objectInstance.parentObjectInstanceIndex = std::stoi(modelTree.get<string>("parentIndex"));

            //
            // this coordiante frame is not defined for all SceneStudio files
            //
            //objectInstance.cU      = vec3f( &(util::readPTreeVectorFloat(modelTree.get_child("cu"))[0]) ).getNormalized();
            //objectInstance.cV      = vec3f( &(util::readPTreeVectorFloat(modelTree.get_child("cv"))[0]) ).getNormalized();
            //objectInstance.cNormal = vec3f( &(util::readPTreeVectorFloat(modelTree.get_child("cw"))[0]) ).getNormalized();
            
            //
            // parentMeshIndex and parentTriIndex are unreliable
            //
            objectInstance.parentUV = ml::vec2f(&(util::readPTreeVectorFloat(modelTree.get_child("parentUV"))[0]));

            //objectInstance.object.cubeFace = std::stoi(modelTree.get<string>("cubeFace"));
            objectInstance.object.scale = sceneScaleFactor * std::stof(modelTree.get<string>("scale"));
            
			// Get transform
            objectInstance.modelToWorld = mat4f::scale(sceneScaleFactor) * mat4f(&(util::readPTreeVectorFloat(modelTree.get_child("transform"))[0])).getTranspose();
            
            //
            // note that at this point, bounding boxes and contactPosition are not initialized because these require the mesh data,
            // parentMeshIndex+parentTriIndex are unreliable.
            // distanceToWall and coordianteFrameRotation require bounding box information
            // they will all get computed in loadObjectData().
            //
            objectInstance.parentMeshIndex = -1;
            objectInstance.parentTriIndex = -1;
			objectInstance.worldBBox = OBBf();
            objectInstance.contactPosition = vec3f::origin;
            objectInstance.contactNormal = vec3f::origin;
            objectInstance.object.distanceToWall = -1.0f;
            objectInstance.object.coordianteFrameRotation = -666.0f;
            objectInstance.object.modelDown = vec3f::origin;
            objectInstance.object.modelNormal = vec3f::origin;
		}
	}
    cout << scenes.size() << " scenes loaded" << endl;
}

void SceneDatabase::updateModelReferences(ModelDatabase &models)
{
    for (auto &s : scenes)
    {
        for (auto &object : s.second.objects)
        {
            auto model = models.models.find(object.object.modelId);
            if (model == models.models.end())
            {
                cout << "model not found: " << object.object.modelId << " in scene " << s.first << endl;
                object.object.model = &models.models.find("not-found")->second;
            }
            else
            {
                object.object.model = &model->second;
            }
        }
    }
}

void SceneDatabase::loadObjectData()
{
    struct SceneObjectData
    {
        vec3f contactPosition;
        vec3f contactNormal;
		OBBf worldBBox;
        bbox3f modelBBox;

        vec3f modelNormal;
        vec3f modelDown;

        //these are needed because value stored in scenes is incorrect
        int parentMeshIndex;
        int parentTriIndex;
    };

    const string sceneObjectDataPath = "C:/Code/SceneSynth/cache/sceneObjectData.tsv";

    map<string, SceneObjectData> idToObjectMap;

    if (ml::util::fileExists(sceneObjectDataPath))
    {
        for (const string &line : ml::util::getFileLines(sceneObjectDataPath, 3))
        {
            auto parts = ml::util::split(line, '\t');
            if (parts.size() == 2 && parts[1].size() / 2 == sizeof(SceneObjectData))
            {
                SceneObjectData &data = idToObjectMap[parts[0]];
                ml::util::decodeBytes(parts[1], (unsigned char *)&data);
            }
        }
    }

    bool newModelFound = false;

    auto getFaceCenters = [](const ml::bbox3f &b, const mat4f &m) {
        vector<vec3f> result;
        vec3f center = b.getCenter();
        vec3f min = b.getMin();
        vec3f max = b.getMax();
        result.push_back(vec3f(min.x, center.y, center.z));
        result.push_back(vec3f(max.x, center.y, center.z));
        result.push_back(vec3f(center.x, min.y, center.z));
        result.push_back(vec3f(center.x, max.y, center.z));
        result.push_back(vec3f(center.x, center.y, min.z));
        result.push_back(vec3f(center.x, center.y, max.z));
        for (auto &c : result)
            c = m * c;
        return result;
    };

    auto getBestCenterDist = [](const vector<vec3f> &centers, const vec3f &v) {
        return ml::util::minValue(centers, [&](const vec3f &c) { return vec3f::dist(v, c); });
    };

    for (auto &s : scenes)
    {
        int objectIndex = 0;
        for (auto &object : s.second.objects)
        {
            string id = s.first + ":" + to_string(objectIndex);
            if (idToObjectMap.count(id) == 0)
            {
                newModelFound = true;
                SceneObjectData &data = idToObjectMap[id];

				//
				// TODO: hookup simlified meshes
				//
                vector<ml::TriMeshf> meshes;
				vector<ml::TriMeshf> simplifiedMeshes;
                vector<ml::Materialf> materials;
                util::loadCachedModel(object.object.modelId, meshes, simplifiedMeshes, materials);

                vector<vec3f> points;
                for (const auto &mesh : meshes)
                    for (const auto &v : mesh.getVertices())
                        points.push_back(v.position);
                
                if (points.size() == 0)
                {
                    cout << "No points found for " << object.object.modelId << " in " << s.first << endl;
                    object.parentObjectInstanceIndex = -1;
                }

                data.modelBBox = ml::bbox3f(points);
				data.worldBBox = object.modelToWorld * OBBf(data.modelBBox);

                if (object.parentObjectInstanceIndex >= 0)
                {
                    const auto &parentObject = s.second.objects[object.parentObjectInstanceIndex];
					vector<ml::TriMeshf> parentMeshes, parentMeshesSimplified;
                    vector<ml::Materialf> materials;
					util::loadCachedModel(parentObject.object.modelId, parentMeshes, parentMeshesSimplified, materials);

                    auto centers = getFaceCenters(data.modelBBox, object.modelToWorld);
                    float bestFitness = std::numeric_limits<float>::max();
                    for (UINT parentMeshIndex = 0; parentMeshIndex < parentMeshes.size(); parentMeshIndex++)
                    {
                        const ml::TriMeshf &parentMesh = parentMeshes[parentMeshIndex];

                        for (UINT parentTriIndex = 0; parentTriIndex < parentMesh.getIndices().size(); parentTriIndex++)
                        {
                            ml::vec3ui parentTriIndices = parentMesh.getIndices()[parentTriIndex];
                            const auto v0 = parentObject.modelToWorld * parentMesh.getVertices()[parentTriIndices.x].position;
                            const auto v1 = parentObject.modelToWorld * parentMesh.getVertices()[parentTriIndices.y].position;
                            const auto v2 = parentObject.modelToWorld * parentMesh.getVertices()[parentTriIndices.z].position;

                            vec3f v = v0 + (v1 - v0) * object.parentUV.x + (v2 - v0) * object.parentUV.y;
                            float fitness = getBestCenterDist(centers, v);
                            if (fitness < bestFitness)
                            {
                                bestFitness = fitness;
                                data.parentMeshIndex = parentMeshIndex;
                                data.parentTriIndex = parentTriIndex;
                            }
                        }
                    }
                    
                    const ml::TriMeshf &parentMesh = parentMeshes[data.parentMeshIndex];
                    ml::vec3ui parentTriIndices = parentMesh.getIndices()[data.parentTriIndex];
                    const auto v0 = parentObject.modelToWorld * parentMesh.getVertices()[parentTriIndices.x].position;
                    const auto v1 = parentObject.modelToWorld * parentMesh.getVertices()[parentTriIndices.y].position;
                    const auto v2 = parentObject.modelToWorld * parentMesh.getVertices()[parentTriIndices.z].position;

                    data.contactPosition = v0 + (v1 - v0) * object.parentUV.x + (v2 - v0) * object.parentUV.y;
                    data.contactNormal = ml::math::triangleNormal(v0, v1, v2);

                    auto quantizeVector = [](vec3f v) {
                        v = v.getNormalized();
                        for (int iter = 0; iter < 3; iter++)
                        {
                            for (float &x : v.array)
                            {
                                if (fabs(x) < 0.25)
                                    x = 0.0;
                            }
                            v = v.getNormalized();
                        }
                        return v;
                    };

                    data.modelNormal = quantizeVector(object.modelToWorld.getInverse() * (data.contactPosition + data.contactNormal) - data.modelBBox.getCenter());
                    data.modelDown = quantizeVector(object.modelToWorld.getInverse() * (data.contactPosition - vec3f::eZ) - data.modelBBox.getCenter());
                    //cout << "normal: " << data.modelNormal << endl;
                    //cout << "down: " << data.modelDown << endl;

                    //cout << bestFitness << endl;
                    //cout << data.contactNormal << endl;
                }
                else
                {
                    data.parentMeshIndex = -1;
                    data.parentTriIndex = -1;
                    data.contactNormal = vec3f::eZ;
                    data.contactPosition = vec3f::origin;
                    data.modelNormal = vec3f::origin;
                    data.modelDown = vec3f::origin;
                }
            }
            SceneObjectData &data = idToObjectMap[id];
            object.object.modelDown = data.modelDown;
            object.object.modelNormal = data.modelNormal;
            object.object.model->bbox = data.modelBBox;
            object.worldBBox = data.worldBBox;
            object.contactPosition = data.contactPosition;
            object.contactNormal = data.contactNormal;
            object.parentMeshIndex = data.parentMeshIndex;
            object.parentTriIndex = data.parentTriIndex;

            if (object.contactNormal.z > 0.5f)
                object.object.contactType = ContactType::ContactUpright;
            else
                object.object.contactType = ContactType::ContactWall;

            objectIndex++;
        }

        s.second.bbox = ml::bbox3f(s.second.objects[0].worldBBox);
        s.second.computeWallSegments();
    }

    //
    // update distanceToWall and coordianteFrameRotation
    //
    for (auto &s : scenes)
    {
        for (auto &object : s.second.objects)
        {
            object.object.coordianteFrameRotation = util::frameZRotation(object.object.model->bbox, object.modelToWorld);
            object.object.distanceToWall = s.second.distanceToWall(object.worldBBox);
        }
    }

    if (newModelFound)
    {
        cout << "New model found, saving scene object data" << endl;
        vector<string> lines;
        for (const auto &p : idToObjectMap)
            lines.push_back(p.first + "\t" + ml::util::encodeBytes((unsigned char *)&p.second, sizeof(SceneObjectData)));
        ml::util::saveLinesToFile(lines, sceneObjectDataPath);
    }
    
}

void SceneDatabase::loadAgentAnnotations(const string &path)
{
    ml::Directory dir(path);
    for (const string &filename : dir.files())
    {
        if (ml::util::endsWith(filename, ".arv"))
        {
            const string sceneId = ml::util::remove(filename, ".arv");

            if (scenes.count(sceneId) == 0)
            {
                cout << "Scene not found: " << sceneId << endl;
            }
            else
            {
                Scene &entry = scenes[sceneId];
                entry.loadAgentAnnotations(dir.path() + filename);
                entry.updateAgentFaceDirections();
            }
        }
    }
}
