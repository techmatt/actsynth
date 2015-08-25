
#include "libsynth.h"

#include <mLibD3D11.h>
#include "../synthesisCommon/assetManager.h"

bool g_useOriginalCollisionTest = true;

double InteractionRays::score(bool visibilityOptional) const
{
    double score = 0.0;
    double maxScore = (double)rays.size();
    for (const auto &r : rays)
    {
        if (r.visible || visibilityOptional)
            score += r.score;
        maxScore += 1.0;
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

void Synthesizer::start(const Database &_database, ml::D3D11GraphicsDevice &_graphics, AssetManager &_assets, const string &architectureModelId, float architectureScale, const Scan *targetScanPtr)
{
    database = &_database;
    graphics = &_graphics;
    assets = &_assets;
    scene.id = "synthesis-" + architectureModelId + std::to_string(ml::util::randomUniform());
    done = false;

    targetScan = targetScanPtr;

    scene.objects.clear();
    scene.objects.push_back(ObjectInstance(&scene));
    ObjectInstance &architecture = scene.objects[0];

    architecture.object.modelId = architectureModelId;
    architecture.object.model = const_cast<Model *>(&database->models.getModel(architectureModelId));
    architecture.object.contactType = ContactType::ContactUpright;
    architecture.object.scale = 1.0f;
    architecture.object.coordianteFrameRotation = 0.0f;
    architecture.object.distanceToWall = 0.0f;

    architecture.parentObjectInstanceIndex = -1;
    architecture.parentMeshIndex = -1;
    architecture.parentTriIndex = -1;
    architecture.parentUV = ml::vec2f::origin;
    architecture.contactNormal = vec3f::eZ;

    architecture.modelToWorld = mat4f::scale(architectureScale);
    architecture.worldBBox = architecture.modelToWorld * ml::OOBBf(architecture.object.model->bbox);

    architecture.contactPosition = vec3f::origin;
    architecture.contactNormal = vec3f::origin;

    scene.agents.clear();

    scene.bbox = ml::bbox3f(architecture.worldBBox);

    scene.computeWallSegments();

    active.score = -1.0;

    stage = SynthesizerStagePlaceAgents;
}

void Synthesizer::outputState(const string &filename) const
{
    HTMLExporter exporter(filename, "Synthesis scene state: " + scene.id);

    ObjectInstance::output(exporter, "All objects", scene.objects);

    for (const Agent &a : scene.agents)
        a.output(exporter);

    if (active.score < 0.0)
    {
        exporter.appendBoldText("No active object", "");
    }
    else
    {
        exporter.appendHeader1("Active object: " + active.object.model->categoryName);
        exporter.appendBoldText("Agents: ", ml::convert::toString(active.agents));
        exporter.appendBoldText("Parent: ", scene.objects[active.parentObjectIndex].object.model->categoryName + "-" + std::to_string(active.parentObjectIndex));
        exporter.appendBoldText("Support: ", ml::convert::toString(active.isAgentSupport));
        exporter.appendBoldText("Score: ", ml::convert::toString(active.score));
    }

    exporter.appendHeader1("Synthesis agent state");

    for (const SynthesizerAgent &agent : agents)
    {
        exporter.appendHeader2("Agent: " + agent.agent->activity);
        if (agent.supportObjectPlaced)
        {
            exporter.appendBoldText("Support object placed", "");
        }
        else if (agent.supportObject.model != nullptr)
        {
            exporter.appendBoldText("Support object: ", agent.supportObject.model->categoryName);
        }

        ml::Grid2<string> grid(2, agent.objectsToPlace.size());

        int col = 0;
        for (const DisembodiedObject &object : agent.objectsToPlace)
        {
            grid(0, col) = object.model->imageHTML();
            grid(1, col) = "<b>" + object.model->categoryName + "</b>";
            col++;
        }
        exporter.appendTableColumnWrap("Objects to place", grid, 10);
    }

    exporter.appendHeader1("Placement locations: " + locations.size());

}

void Synthesizer::step()
{
    if (done)
    {
        cout << "done" << endl;
        return;
    }

    cout << "Executing step: " << stageName(stage) << endl;
    
    if (stage == SynthesizerStagePlaceAgents)
    {
        placeAgents();
        stage = SynthesizerStageSelectAgentObjects;
        return;
    }

    if (stage == SynthesizerStageSelectAgentObjects)
    {
        selectAgentObjects();
        stage = SynthesizerStageSelectNextObject;
        return;
    }

    if (stage == SynthesizerStageSelectNextObject)
    {
        selectNextObject();
        stage = SynthesizerStageEvaluatePlacementA;
        return;
    }
    
    if (stage == SynthesizerStageEvaluatePlacementA)
    {
        makeLocations();
        evaluatePlacementA();
        stage = SynthesizerStageEvaluatePlacementB;
        return;
    }
    
    if (stage == SynthesizerStageEvaluatePlacementB)
    {
        evaluatePlacementB();
        stage = SynthesizerStagePlaceObject;
        return;
    }

    if (stage == SynthesizerStagePlaceObject)
    {
        placeObject();
        stage = SynthesizerStageSelectNextObject;
        return;
    }
}

void Synthesizer::placeAgents()
{
    //
    // eventually, this should sample new arrangements of agents. For now, these arrangements are copied
    // from a file, and then their object sets are cleared.
    //
    if (targetScan == NULL)
    {
        scene.loadAgentAnnotations(R"(V:\data\synthesis\agentAnnotations\wssScenes.scene00031.arv)");

        vec3f shift = -vec3f::eY * 0.75f;

        for (Agent &a : scene.agents)
        {
            a.headPos += shift;
            a.supportPos += shift;
        }
    }
        
    else
    {
        scene.agents = targetScan->agents;
        for (Agent &a : scene.agents)
            a.scene = &scene;
    }
    
    for (Agent &a : scene.agents)
    {
        a.boundObjectIndices.clear();
        a.supportObjectIndex = (UINT)-1;
    }

    agents.resize(scene.agents.size());
    for (UINT agentIndex = 0; agentIndex < scene.agents.size(); agentIndex++)
    {
        agents[agentIndex].agent = &scene.agents[agentIndex];
    }
}

void Synthesizer::selectAgentObjects()
{
    for (SynthesizerAgent &a : agents)
    {
        const auto &activity = database->activities.activities.at(a.agent->activity);

        a.objectsToPlace = activity.objectGroupDistribution->sample();
        
        string supportCategoryName = activity.supportDistribution.sample();
        const Category &supportCategory = database->categories.getCategory(supportCategoryName);
        if (supportCategoryName == "room")
        {
            a.supportObjectPlaced = true;
        }
        else
        {
            a.supportObjectPlaced = false;
            a.supportObject = supportCategory.sampleInstance();
        }
    }
}

void Synthesizer::selectNextObject()
{
    vector<ActiveObject> allObjects;

    //
    // Accumulate all possible objects
    //
    for (UINT agentIndex = 0; agentIndex < agents.size(); agentIndex++)
    {
        const auto &agent = agents[agentIndex];

        if (!agent.supportObjectPlaced)
        {
            ActiveObject active;
            active.agents.push_back(agentIndex);
            active.isAgentSupport = true;
            active.object = agent.supportObject;

            //
            // For now, all support objects are assumed to be placed directly on the base architecture
            //
            active.parentObjectIndex = 0;

            allObjects.push_back(active);
        }

        UINT objectsToPlaceIndex = 0;
        for (const DisembodiedObject &o : agent.objectsToPlace)
        {
            const Category &category = *o.model->category;
            
            for (const auto &parentCategory : category.parentCategoryFrequencies)
            {
                for (const UINT parentObjectIndex : scene.objectsWithCategory(parentCategory.first, agentIndex))
                {
                    ActiveObject active;
                    active.agents.push_back(agentIndex);
                    active.isAgentSupport = false;
                    active.object = o;
                    active.objectsToPlaceIndex.push_back(objectsToPlaceIndex);
                    active.parentObjectIndex = parentObjectIndex;
                    allObjects.push_back(active);
                }
            }

            objectsToPlaceIndex++;
        }
    }

    if (allObjects.size() == 0)
    {
        cout << "No new objects can be placed" << endl;
        done = true;
        active.score = -1.0;
        return;
    }

    //
    // Score all objects
    //
    for (auto &o : allObjects)
    {
        o.score = 0.0;
        
        //
        // Favor shared objects
        //
        if (o.agents.size() > 1) o.score += 2000.0;

        //
        // Favor support furniture
        //
        if (o.isAgentSupport) o.score += 1000.0;

        //
        // Favor any object on the floor
        // This is a bad idea -- it will force all objects that can go on both the floor
        // and another object to go on the floor.
        //
        //if (o.parentObjectIndex == 0) o.score += 500.0;

        //
        // Favor a specific object property for debugging
        //
        //if (o.object.model->categoryName == "Monitor") o.score += 500.0;
        //if (o.object.contactType == ContactWall) o.score += 500.0;

        //
        // Favor larger objects
        //
        mat4f baseTransform = o.object.makeBaseTransform();
        ml::OOBBf worldOOBB = baseTransform * ml::OOBBf(o.object.model->bbox);
        ml::bbox3f worldAABB(worldOOBB.getVertices());
        double surfaceArea = worldAABB.getExtentX() * worldAABB.getExtentY() +
            worldAABB.getExtentX() * worldAABB.getExtentZ() +
            worldAABB.getExtentY() * worldAABB.getExtentZ();
        o.score += surfaceArea;
    }

    std::sort(allObjects.begin(), allObjects.end(), [](const ActiveObject &a, const ActiveObject &b) { return a.score < b.score; });

    active = allObjects.back();

    cout << "Next object: " << active.object.model->categoryName << " a=" << ml::convert::toString(active.agents) << " s=" << active.score << endl;

    if (active.isAgentSupport)
    {
        for (UINT agent : active.agents)
            agents[agent].supportObjectPlaced = true;
    }
    else
    {
        for (UINT index = 0; index < active.agents.size(); index++)
            ml::util::removeSwap(agents[active.agents[index]].objectsToPlace, active.objectsToPlaceIndex[index]);
    }
}

double Synthesizer::collisionScore(const DisembodiedObject &object, const mat4f &modelToWorld) const
{
    const auto *modelAccelerator = &assets->loadModel(*graphics, object.modelId).getAcceleratorSimplified();
	if (g_useOriginalCollisionTest)
		modelAccelerator = &assets->loadModel(*graphics, object.modelId).getAcceleratorOriginal();

    for (UINT objectIndex = 1; objectIndex < scene.objects.size(); objectIndex++)
    {
        auto &object = scene.objects[objectIndex];
		const auto *sceneObjectAccelerator = &assets->loadModel(*graphics, object.object.modelId).getAcceleratorSimplified();
		if (g_useOriginalCollisionTest)
			sceneObjectAccelerator = &assets->loadModel(*graphics, object.object.modelId).getAcceleratorOriginal();

        mat4f sceneObjectToModel = modelToWorld.getInverse() * object.modelToWorld;

        if (modelAccelerator->collision(*sceneObjectAccelerator, sceneObjectToModel)) {
            return 0.0;
        }
    }
    return 1.0;
}

double Synthesizer::collisionBBoxScore(const DisembodiedObject &object, const mat4f &modelToWorld) const
{
	const auto &modelAccelerator = assets->loadModel(*graphics, object.modelId).getAcceleratorSimplified();

    for (UINT objectIndex = 1; objectIndex < scene.objects.size(); objectIndex++)
    {
        auto &object = scene.objects[objectIndex];
		const auto &sceneObjectAccelerator = assets->loadModel(*graphics, object.object.modelId).getAcceleratorSimplified();
        mat4f sceneObjectToModel = modelToWorld.getInverse() * object.modelToWorld;

        if (modelAccelerator.collisionBBoxOnly(sceneObjectAccelerator, sceneObjectToModel)) {
            return 0.0;
        }
    }
    return 1.0;
}

void Synthesizer::makeObjectScoreEntry(const DisembodiedObject &object, const mat4f &modelToWorld, const vec3f &contactPos, ObjectScoreEntry &result)
{
    const Category &category = *object.model->category;
    const string &activityName = scene.agents[result.agents[0].agentIndex].activity;
    const ActivityCategoryDistributions &distributions = database->activities.getActivityCategoryDistribution(activityName, category.name);

    const ml::OOBBf worldBBox = modelToWorld * ml::OOBBf(object.model->bbox);

    //
    // wall distance score
    //
    result.wallDist = scene.distanceToWall(worldBBox);
    result.wallDistScore = SynthScore::wallDistScore(category, (float)result.wallDist);

    //
    // wall rotation score
    //
    result.wallRotation = 0.0f;
    result.wallRotationScore = 0.0f;

    result.collisionScore = collisionScore(object, modelToWorld);
    //result.collisionScore = 0.0f;
    result.collisionBBoxScore = collisionBBoxScore(object, modelToWorld);

    //
    // per-agent scores
    //
    for (size_t agentIndex = 0; agentIndex < result.agents.size(); agentIndex++)
    {
        ObjectScoreAgentEntry &agentEntry = result.agents[agentIndex];
        const Agent &agent = scene.agents[agentEntry.agentIndex];
        
        agentEntry.baseOffset = agent.worldToAgentBasis(contactPos);
        agentEntry.baseOffsetScore = SynthScore::distributionScore(distributions.base, agentEntry.baseOffset);

        if (category.useHandOrientation)
            agentEntry.handOrientationScore = SynthScore::handOrientationScore(active.object, modelToWorld, agent);

        makeInteractionMapRays(object, modelToWorld, agent, "gaze", agentEntry.gaze);
        makeInteractionMapRays(object, modelToWorld, agent, "fingertip", agentEntry.fingertip);
        makeInteractionMapRays(object, modelToWorld, agent, "backSupport", agentEntry.backSupport);
        //makeInteractionMapRays(object, modelToWorld, agent, "palm", agentEntry.palm);
        //makeInteractionMapRays(object, modelToWorld, agent, "hips", agentEntry.hips);
    }
}

void Synthesizer::updateScores()
{
    scores.clear();
    scores.resize(scene.objects.size());
    for (UINT objectIndex = 0; objectIndex < scene.objects.size(); objectIndex++)
    {
        ObjectScoreEntry &entry = scores[objectIndex];
        ObjectInstance &instance = scene.objects[objectIndex];

        //
        // rotation is not meaningful for placed objects
        //
        entry.rotation = -1.0f;

        auto agentIndices = scene.agentsFromObjectIndex(objectIndex);
        entry.agents.resize(agentIndices.size());
        for (UINT agentIndex = 0; agentIndex < agentIndices.size(); agentIndex++)
        {
            entry.agents[agentIndex].agentIndex = agentIndices[agentIndex];
        }

        if (objectIndex != 0)
        {
            makeObjectScoreEntry(instance.object, instance.modelToWorld, instance.contactPosition, entry);
        }
    }
}

double Synthesizer::scoreArrangement() const
{
    double result = 0.0;

    //
    // skip the base architecture
    //
    for (UINT objectIndex = 1; objectIndex < scene.objects.size(); objectIndex++)
    {
        const ObjectScoreEntry &entry = scores[objectIndex];
        result += entry.objectOnlyScoreTerms();

        for (const ObjectScoreAgentEntry &agent : entry.agents)
        {
            result += agent.baseOffsetScore * SynthScore::baseOffsetScale();
            result += agent.handOrientationScore * SynthScore::handOrientationScale();

            result += agent.gaze.score(false);
            result += agent.fingertip.score(false);
            result += agent.backSupport.score(true);
        }
    }

    return result;
}


double Synthesizer::scoreArrangement(const ObjectScoreEntry &activeScore, const string &modelId, const mat4f &modelToWorld) const
{
    const auto &accelerator = assets->loadModel(*graphics, modelId).getAcceleratorOriginal();

    double result = 0.0;

    //
    // skip the base architecture
    //
    for (UINT objectIndex = 1; objectIndex < scene.objects.size(); objectIndex++)
    {
        const ObjectScoreEntry &entry = scores[objectIndex];
        result += entry.objectOnlyScoreTerms();

        for (const ObjectScoreAgentEntry &agent : entry.agents)
        {
            result += agent.baseOffsetScore * SynthScore::baseOffsetScale();
            result += agent.handOrientationScore * SynthScore::handOrientationScale();

            result += agent.gaze.score(false, accelerator, modelToWorld);
            result += agent.fingertip.score(false, accelerator, modelToWorld);
            result += agent.backSupport.score(true, accelerator, modelToWorld);
        }
    }

    result += activeScore.objectOnlyScoreTerms();
    for (const ObjectScoreAgentEntry &agent : activeScore.agents)
    {
        result += agent.baseOffsetScore * SynthScore::baseOffsetScale();
        result += agent.handOrientationScore * SynthScore::handOrientationScale();

        result += agent.gaze.score(false);
        result += agent.fingertip.score(false);
        result += agent.backSupport.score(true);
    }

    return result;
}

void Synthesizer::evaluatePlacementA()
{
    //const ModelAssetData &modelData = assets->loadModel(*graphics, active.object.modelId);

	//ml::Timer t;

	//g_useOriginalCollisionTest = true;
 //   for (PlacementLocation &l : locations)
 //   {
 //       for (ObjectScoreEntry &r : l.rotations)
 //       {
 //           mat4f modelToWorld = makeModelToWorld(active.object, l, r.rotation);
 //           makeObjectScoreEntry(active.object, modelToWorld, l.sample.pos, r);
 //       }
 //   }
	//cout << "Use Original\t" << t.getElapsedTime() << endl;
	//t.start();
	g_useOriginalCollisionTest = false;
	for (PlacementLocation &l : locations)
	{
		for (ObjectScoreEntry &r : l.rotations)
		{
			mat4f modelToWorld = makeModelToWorld(active.object, l, r.rotation);
			makeObjectScoreEntry(active.object, modelToWorld, l.sample.pos, r);
		}
	}
	//cout << "Use Simplified\t" << t.getElapsedTime() << endl;

	
}

void Synthesizer::evaluatePlacementB()
{

}

void Synthesizer::chooseLocation(int &bestLocationIndex, int &bestRotationIndex) const
{
    struct AcceptableLocation
    {
        UINT locationIndex;
        UINT rotationIndex;
        double score;
    };

    bestLocationIndex = -1;
    bestRotationIndex = -1;
    
    vector<AcceptableLocation> acceptableLocations;

    for (UINT locationIndex = 0; locationIndex < locations.size(); locationIndex++)
    {
        const PlacementLocation &l = locations[locationIndex];

        for (UINT rotationIndex = 0; rotationIndex < l.rotations.size(); rotationIndex++)
        {
            const ObjectScoreEntry &entry = l.rotations[rotationIndex];
            mat4f modelToWorld = makeModelToWorld(active.object, l, entry.rotation);
            double combinedScore = scoreArrangement(entry, active.object.modelId, modelToWorld);

            if (combinedScore > 0.0)
            {
                AcceptableLocation a;
                a.locationIndex = locationIndex;
                a.rotationIndex = rotationIndex;
                a.score = combinedScore;
                acceptableLocations.push_back(a);
            }
        }
    }

    if (acceptableLocations.size() == 0)
    {
        cout << "No acceptable locations" << endl;
        return;
    }

    std::random_shuffle(acceptableLocations.begin(), acceptableLocations.end());
    std::stable_sort(acceptableLocations.begin(), acceptableLocations.end(), [](const AcceptableLocation &a, const AcceptableLocation &b) { return a.score < b.score; });

    const AcceptableLocation &result = acceptableLocations.back();
    bestLocationIndex = result.locationIndex;
    bestRotationIndex = result.rotationIndex;

    const ObjectScoreEntry &r = locations[result.locationIndex].rotations[result.rotationIndex];

    cout << "Location combined score: " << result.score << endl;
    /*cout << "  wallDist: " << r.wallDist << endl;
    cout << "  wallScore: " << r.wallScore << endl;
    cout << "  baseScore: " << r.agents[0].baseScore << endl;
    cout << "  gazeVisScore: " << r.agents[0].gaze.visibilityScore << endl;
    cout << "  gazeVisScore: " << r.agents[0].gaze.visibilityScore << endl;
    cout << "  gazeVisScore: " << r.agents[0].gaze.visibilityScore << endl;
    cout << "  gazeVisScore: " << r.agents[0].gaze.visibilityScore << endl;
    cout << "  gazeVisScore: " << r.agents[0].gaze.visibilityScore << endl;*/

    return;
}

void Synthesizer::placeObject()
{
    ObjectInstance instance(&scene);

    instance.object = active.object;

    int locationIndex, rotationIndex;
    chooseLocation(locationIndex, rotationIndex);
    if (locationIndex == -1 || rotationIndex == -1)
    {
        return;
    }
    const PlacementLocation& location = locations[locationIndex];
    
    instance.modelToWorld = makeModelToWorld(active.object, location, location.rotations[rotationIndex].rotation);

    instance.parentObjectInstanceIndex = active.parentObjectIndex;
    instance.parentMeshIndex = location.sample.meshIndex;
    instance.parentTriIndex = location.sample.triangleIndex;
    instance.parentUV = location.sample.uv;

    instance.contactPosition = location.sample.pos;
    instance.contactNormal = location.sample.normal;

    instance.worldBBox = instance.modelToWorld * ml::OOBBf(active.object.model->bbox);

    scene.objects.push_back(instance);

    for (UINT agentIndex : active.agents)
    {
        scene.agents[agentIndex].boundObjectIndices.push_back((UINT)scene.objects.size() - 1);
    }

    locations.clear();

    updateScores();
}

mat4f Synthesizer::makeModelToWorld(const DisembodiedObject &object, const PlacementLocation &location, float rotation) const
{
    if (object.contactType == ContactUpright)
    {
        return mat4f::translation(location.sample.pos) * mat4f::rotationZ(rotation) * object.makeBaseTransform();
    }
    else
    {
        const vec3f orientPoint = scene.bbox.getCenter();
        vec3f normal = location.sample.normal;
        if (((orientPoint - location.sample.pos) | normal) < 0.0f)
            normal = -normal;

        mat4f faceNormal = mat4f::face(vec3f::eZ, normal);

        float bestRotation = 0.0f;
        float bestRotationDist = std::numeric_limits<float>::max();
        for (UINT rotationIndex = 0; rotationIndex < 4; rotationIndex++)
        {
            float curRotation = rotationIndex * 90.0f;
            mat4f transform = mat4f::rotation(normal, curRotation) * faceNormal * object.makeBaseTransformCentered();
            vec3f downDir = transform.transformAffine(object.model->bbox.getCenter() + object.modelDown).getNormalized();
            float curRotationDist = distSqf(downDir, -vec3f::eZ);
            if (curRotationDist < bestRotationDist)
            {
                bestRotationDist = curRotationDist;
                bestRotation = curRotation;
            }
        }

        return mat4f::translation(location.sample.pos) * mat4f::rotation(normal, bestRotation) * faceNormal * object.makeBaseTransform();
    }
}

void Synthesizer::makeLocations()
{
    const ObjectInstance &parentObject = scene.objects[active.parentObjectIndex];

    vector< pair<const ml::TriMeshf*, mat4f> > objectMeshes;
    auto &renderData = assets->loadModel(*graphics, parentObject.object.modelId);
    for (auto &m : renderData.data)
    {
        objectMeshes.push_back(std::make_pair(&m.meshOriginal.getTriMesh(), parentObject.modelToWorld));
    }

    vector<ml::TriMeshSampler<float>::Sample> samples;
    const UINT maxSampleCount = 2000;

    if (active.object.contactType == ContactUpright)
        samples = ml::TriMeshSampler<float>::sample(objectMeshes, 1000.0f, maxSampleCount, vec3f::eZ);
    if (active.object.contactType == ContactWall)
        samples = ml::TriMeshSampler<float>::sample(objectMeshes, 1000.0f, maxSampleCount, [](const vec3f& v) { return fabs(v.z) < 0.01f; });

    locations.clear();
    locations.resize(samples.size());

    for (UINT sampleIndex = 0; sampleIndex < samples.size(); sampleIndex++)
    {
        const auto &sample = samples[sampleIndex];
        auto &location = locations[sampleIndex];
        location.sample = sample;

        UINT rotationCount = 10;
        if (active.object.contactType == ContactWall)
            rotationCount = 1;

        for (UINT rotationIndex = 0; rotationIndex < rotationCount; rotationIndex++)
        {
            const auto &instance = active.object.model->category->sampleInstance();
            float rotation = fmod(instance.coordianteFrameRotation + ml::util::randomInteger(0, 3) * 90.0f, 360.0f);

            if (location.closestRotation(rotation) > 2.0f)
            {
                location.rotations.push_back(ObjectScoreEntry(rotation, active.agents));
            }
        }

        std::sort(location.rotations.begin(), location.rotations.end());
    }

    cout << "Sampled " << samples.size() << " candidate locations" << endl;
}

void Synthesizer::makeInteractionMapRays(const DisembodiedObject &object, const mat4f &modelToWorld, const Agent &agent, const string &interactionMapName, InteractionRays &result) const
{
    result.rays.clear();

    /*if (object.model->categoryName == "Monitor")
    {
        int a = 5;
    }*/
    if (!database->interactionMaps.hasInteractionMapEntry(object.modelId, interactionMapName))
    {
        return;
    }
    const InteractionMapEntry &entry = database->interactionMaps.getInteractionMapEntry(object.modelId, interactionMapName);
    if (entry.centroids.size() == 0)
    {
        return;
    }

    vector < pair<const ml::TriMeshAcceleratorBVHf*, mat4f> > allAccelerators;
    
    for (UINT objectIndex = 0; objectIndex < scene.objects.size(); objectIndex++)
    {
        //
        // skip the base architecture, since collisions with that are very unlikely to occlude things
        //
        if (objectIndex != 0)
        {
            const auto &object = scene.objects[objectIndex];
            const auto &accelerator = assets->loadModel(*graphics, object.object.modelId).getAcceleratorOriginal();
            allAccelerators.push_back(std::make_pair(&accelerator, object.modelToWorld.getInverse()));
        }
    }
    
	const auto &selfAccelerator = assets->loadModel(*graphics, object.modelId).getAcceleratorOriginal();
    allAccelerators.push_back(std::make_pair(&selfAccelerator, modelToWorld.getInverse()));

    result.rays.resize(entry.centroids.size());

    bool visibilityOptional = (interactionMapName == "backSupport" || interactionMapName == "hips");

    vec3f rayOrigin = agent.headPos;

    if (interactionMapName == "fingertip")
    {
        rayOrigin = agent.handPos();
    }

    UINT centroidIndex = 0;
    for (const auto &centroid : entry.centroids)
    {
        InteractionRayEntry &rayEntry = result.rays[centroidIndex];
        rayEntry.centroidPos = modelToWorld * centroid.pos;
        rayEntry.ray = ml::Rayf(rayOrigin, rayEntry.centroidPos - rayOrigin);
        rayEntry.score = 1.0;
        rayEntry.visible = false;

        ml::TriMeshRayAcceleratorf::Intersection intersection;
        UINT intersectObjectIndex;
        if (ml::TriMeshRayAcceleratorf::getFirstIntersectionTransform(rayEntry.ray, allAccelerators, intersection, intersectObjectIndex))
        {
            //
            // NOTE: intersection is in model space.
            //

            double centroidDist = ml::dist(rayOrigin, rayEntry.centroidPos);
            double intersectDist = ml::dist(rayOrigin, modelToWorld.transformAffine(intersection.getSurfacePosition()));

            rayEntry.visible = (fabs(centroidDist - intersectDist) < 0.01);

            if (visibilityOptional || rayEntry.visible)
            {
                vec3f surfaceNormal = modelToWorld.transformNormalAffine(intersection.getSurfaceNormal()).getNormalized();
                double minNormalAngle = std::min(vec3f::angleBetween( surfaceNormal, rayEntry.ray.direction()),
                                                 vec3f::angleBetween(-surfaceNormal, rayEntry.ray.direction()));
                double cosineVisiblityScore = std::max(0.0, cos(ml::math::degreesToRadians(minNormalAngle)));

                double score = 1.0;

                if (interactionMapName == "gaze")
                {
                    // gaze is better when directly viewing the object head-on
                    score = cosineVisiblityScore;
                }

                if (interactionMapName == "backSupport")
                {
                    // back support only counts as visible when it is behind the agent
                    if ((rayEntry.ray.direction() | agent.gazeDir) > 0.0f)
                    {
                        score = 0.0;
                    }
                    else
                    {
                        // back support is best when directly behind the agent
                        score = -(rayEntry.ray.direction() | agent.gazeDir);
                    }
                }

                rayEntry.score = score;
            }    
        }
        centroidIndex++;
    }
}