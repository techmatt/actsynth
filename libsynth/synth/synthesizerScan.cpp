
#include "libsynth.h"

#include <mLibD3D11.h>
#include "../synthesisCommon/assetManager.h"

void Synthesizer::start(const Database &_database, ml::D3D11GraphicsDevice &_graphics, AssetManager &_assets, const SceneTemplate &_sceneTemplate, const string &architectureModelId, float architectureScale)
{
    database = &_database;
    graphics = &_graphics;
    assets = &_assets;
    sceneTemplate = &_sceneTemplate;
    scene.id = "synthesis-" + architectureModelId + to_string(ml::util::randomUniform());
    earlyTermination = false;
    verbose = true;

    //locator.init(_database, _graphics, _assets, _targetScan);

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
	architecture.worldBBox = architecture.modelToWorld * OBBf(architecture.object.model->bbox);

    architecture.contactPosition = vec3f::origin;
    architecture.contactNormal = vec3f::origin;

    scene.agents.clear();

    scene.bbox = ml::bbox3f(architecture.worldBBox);

    scene.computeWallSegments();

    candidateObjects.clear();

    stage = SynthesizerStagePlaceAgents;
}

void Synthesizer::outputState(const string &filename) const
{
    HTMLExporter exporter(filename, "Synthesis scene state: " + scene.id);

    ObjectInstance::output(exporter, "All objects", scene.objects);

    for (const Agent &a : scene.agents)
        a.output(exporter);

    /*if (active.score < 0.0)
    {
        exporter.appendBoldText("No active object", "");
    }
    else
    {
        exporter.appendHeader1("Active object: " + active.object.model->categoryName);
        exporter.appendBoldText("Agents: ", ml::convert::toString(active.agents));
        exporter.appendBoldText("Parent: ", scene.objects[active.parentObjectIndex].object.model->categoryName + "-" + to_string(active.parentObjectIndex));
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

    exporter.appendHeader1("Placement locations: " + locations.size());*/

}

void Synthesizer::synthesize()
{
    const float maxSynthTime = 1000.0f;

    Timer t;

    verbose = false;
    for (UINT stepIndex = 0; stepIndex < 2000; stepIndex++)
    {
        step();

        if (t.getElapsedTime() >= maxSynthTime)
        {
            earlyTermination = true;
            return;
        }
    }
}

void Synthesizer::step()
{
    if (earlyTermination)
    {
        if (verbose) cout << "done" << endl;
        return;
    }

    if (verbose) cout << "Executing step: " << stageName(stage) << endl;
    
    if (stage == SynthesizerStagePlaceAgents)
    {
        placeAgents();
        addAgentActivityBlockers();
        sampleAgentObjects();
        if (agents.size() == 0) cout << "No agents!" << endl;
        stage = SynthesizerStageChooseCandidateObjects;
        return;
    }

    if (stage == SynthesizerStageChooseCandidateObjects)
    {
        chooseCandidateObjects();
        if (candidateObjects.size() > 0)
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
        stage = SynthesizerStagePlaceBestObject;
        return;
    }

    if (stage == SynthesizerStagePlaceBestObject)
    {
        placeBestObject();
        stage = SynthesizerStageChooseCandidateObjects;
        return;
    }
}

void Synthesizer::placeAgents()
{
    scene.agents = sceneTemplate->agentDistribution.generateAgentSamples();
    for (Agent &a : scene.agents)
        a.scene = &scene;
    
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

void Synthesizer::addAgentActivityBlockers()
{
    for (const Agent &a : scene.agents)
    {
        const mat4f agentFloorToWorld = mat4f::translation(a.headPos.x, a.headPos.y, 0.0f) * mat4f::face(vec3f::eX, a.gazeDir);
        const Activity &activity = database->activities.getActivity(a.activity);
        for (const bbox3f &bbox : activity.agentFreeSpaceBlockers)
        {
            ActivityBlocker blocker;
            blocker.obb = agentFloorToWorld * OBBf(bbox);

            blocker.accel = new ml::TriMeshAcceleratorBVHf();
            blocker.accel->build(ml::Shapesf::box(blocker.obb), true);

            activityBlockers.push_back(blocker);
        }
    }
}

void Synthesizer::sampleAgentObjects()
{
    for (SynthesizerAgent &a : agents)
    {
        auto &activity = database->activities.getActivity(a.agent->activity);
        auto &distribution = *activity.objectGroupDistribution;

        auto sample = distribution.sample();

        for (const string &categoryName : distribution.categoryList())
        {
            AgentCategoryData &data = a.categories[categoryName];

            data.category = &database->categories.getCategory(categoryName);
            data.sceneCount = 0;
            
            data.sampledCount = 0;
            for (const DisembodiedObject &o : sample)
            {
                if (o.model->categoryName == categoryName)
                    data.sampledCount++;
            }

            //
            // matt-bedroom exceptions
            //
            if (ml::util::contains(sceneTemplate->debugScan->name, "matt-bedroom"))
            {
                if (activity.id == "bed" && categoryName == "WallArt")
                    data.sampledCount = 0;

                if (categoryName == "FloorLamp")
                    data.sampledCount = 1;

                if (categoryName == "Laptop")
                    data.sampledCount = 1;
            }

            //
            // Gates381 exceptions
            //
            if (ml::util::contains(synthParams().scanName, "381"))
            {
                if (activity.id == "living room" && categoryName != "Couch" && categoryName != "SideTable")
                    data.sampledCount = 0;
                if (categoryName == "FloorLamp")
                    data.sampledCount = 0;
                if (synthParams().fisherSynthesisMode && (categoryName == "Monitor" || categoryName == "Keyboard" || categoryName == "Mouse"))
                {
                    data.sampledCount = ml::math::clamp((int)data.sampledCount + (int)(rand() % 3) - 1, 0, 3);
                }
            }

            if (data.category->isObjectSupport)
                data.sampledCount = std::max(data.sampledCount, 1U);
        }

        if (verbose)
        {
            cout << activity.id << " agent: ";
            for (const auto &data : a.categories)
            {
                string countString = "";
                if (data.second.sampledCount > 1)
                    countString = "-" + std::to_string(data.second.sampledCount);

                if (data.second.sampledCount > 0)
                    cout << data.first << countString << " ";
            }
            cout << endl;
        }
    }
}

void Synthesizer::chooseCandidateObjects()
{
    updateAgentCategories();

    candidateObjects.clear();

    //
    // choose a random agent
    //
    UINT chosenAgentIndex = rand() % agents.size();

    if (synthParams().agentSupportFirst)
    {
        for (UINT i = 0; i < agents.size(); i++)
        {
            if (agents[i].agent->supportObjectIndex == (UINT)-1 && agents[i].agent->activity != "bed")
            {
                chosenAgentIndex = i;
            }
        }
    }

    const auto &agent = agents[chosenAgentIndex];

    vector<AgentCategoryData> sortedCategories = agent.makeSortedCategories();

    vector<AgentCategoryData> filteredCategories;
    for (const AgentCategoryData &categoryData : sortedCategories)
    {
        const Category &category = *categoryData.category;

        UINT parentsInScene = 0, parentsExpected = 0;
        for (const auto &parentCategory : category.parentCategoryFrequencies)
        {
            if (parentCategory.first == "Room")
            {
                parentsExpected++;
                parentsInScene++;
            }
            else if (agent.categories.count(parentCategory.first) != 0)
            {
                const AgentCategoryData &parentCategoryData = (*agent.categories.find(parentCategory.first)).second;
                if (parentCategoryData.sampledCount > 0)
                    parentsExpected++;
                if (scene.objectsWithCategory(parentCategory.first, chosenAgentIndex).size() > 0)
                    parentsInScene++;
            }
        }
        
        if (categoryData.activityInsertionScore == 0.0)
            cout << "invalid category?" << endl;

        //
        // An object should only be added if all its children have been added
        //
        if (parentsInScene >= parentsExpected)
            filteredCategories.push_back(categoryData);
    }

    //
    // filteredCategories are already sorted
    //

    if (filteredCategories.size() > synthParams().keepTopXCategories)
        filteredCategories.resize(synthParams().keepTopXCategories);

    //
    // Choose randomly chosen model samples from each category
    //
    for (const AgentCategoryData &categoryData : filteredCategories)
    {
        const Category &category = *categoryData.category;
            
        vector<const DisembodiedObject*> samples = category.sampleInstances(synthParams().samplesPerCategory);

        //
        // Some categories take *so* long to collide, we're better off just picking one at random to save time
        //
        if (samples.size() > 1 && category.name == "Shoes")
            samples.resize(1);

        if (synthParams().forceConsistentCategory && !category.isDiversity)
        {
            auto objects = scene.objectsWithCategory(category.name, -1);
            if (objects.size() > 0)
            {
                samples.clear();
                samples.push_back(&scene.objects[objects[0]].object);
            }
        }

        for (const DisembodiedObject *object : samples)
        {
            CandidateObject candidate;
            
            if (category.isShared)
            {
                //
                // shared objects are shared by all agents in the scene
                //
                for (UINT agentIndex = 0; agentIndex < agents.size(); agentIndex++)
                    candidate.agents.push_back(agentIndex);
            }
            else
            {
                candidate.agents.push_back(chosenAgentIndex);
            }
            candidate.object = *object;

            vector<UINT> parentObjectIndices;
            for (const auto &parentCategory : category.parentCategoryFrequencies)
                ml::util::push_back(parentObjectIndices, scene.objectsWithCategory(parentCategory.first, chosenAgentIndex));

            //
            // For now, there is a special case for mouse-on-mousepad. Should really be learned from bayes net distribution.
            //
            if (categoryData.category->name == "ComputerMouse")
            {
                auto mousepadIndices = scene.objectsWithCategory("MousePad", chosenAgentIndex);
                if (mousepadIndices.size() > 0)
                    parentObjectIndices = mousepadIndices;
            }

            if (parentObjectIndices.size() == 0)
            {
                cout << "Object with no valid parent: " << category.name << endl;
                earlyTermination = true;
            }
            else
            {
                candidate.parentObjectIndex = ml::util::randomElement(parentObjectIndices);

                candidate.isAgentSupport = categoryData.category->isAgentSupport;

                candidateObjects.push_back(candidate);
            }

            //
            // for desks, tables, etc. add in smaller or larger versions as well.
            //
            if (category.name == "Bed")
            {
                candidate.object.scale *= 0.9f;
                candidateObjects.push_back(candidate);
            }
            /*if (database->categories.getCategory(candidate.object.model->categoryName).supportCategory)
            {
                candidate.object.scale *= 1.15f;
                candidateObjects.push_back(candidate);

                candidate.object.scale *= 1.15f;
                candidateObjects.push_back(candidate);
            }*/

            //candidate.scanLocations = &locator.locateModel(candidate.object);
        }
    }

    if (candidateObjects.size() == 0)
    {
        if (verbose) cout << "No valid objects found for agent, trying again" << endl;
        return;
    }

    if (verbose) cout << "Candidate objects: " << candidateObjects.size() << endl;
    for (const CandidateObject &o : candidateObjects)
    {
        if (o.object.model->categoryName == "Room") cout << "Trying to insert room..." << endl;
        if (verbose) cout << o.object.model->categoryName << " -> " << o.object.modelId << endl;
    }
}

double Synthesizer::collisionScore(const DisembodiedObject &baseObject, const mat4f &modelToWorld) const
{
    const auto *modelAccelerator = &assets->loadModel(*graphics, baseObject.modelId).getAcceleratorSimplified();
    const Category &category = database->categories.getCategory(baseObject.model->categoryName);
    if (category.isBBoxCollision)
        modelAccelerator = &assets->loadModel(*graphics, baseObject.modelId).getAcceleratorBBoxOnly();
    
    const mat4f worldToModel = modelToWorld.getInverse();

    //
    // some categories should be checked for collision with the architecture; generally, this list should be kept as small as possible because this
    // is *very* expensive
    //
    UINT objectStartIndex = 1;
    if (category.name == "FloorLamp" || category.name == "MagazineBin" || category.name == "ChestOfDrawers" || category.name == "Bed" || category.name == "SideTable")
        objectStartIndex = 0;

    //
    // check collisions against objects in scene
    //
    for (UINT objectIndex = objectStartIndex; objectIndex < scene.objects.size(); objectIndex++)
    {
        auto &sceneObject = scene.objects[objectIndex];
        const auto *sceneObjectAccelerator = &assets->loadModel(*graphics, sceneObject.object.modelId).getAcceleratorSimplified();
        if (database->categories.getCategory(sceneObject.object.model->categoryName).isBBoxCollision)
            sceneObjectAccelerator = &assets->loadModel(*graphics, sceneObject.object.modelId).getAcceleratorBBoxOnly();

        const mat4f sceneObjectToModel = worldToModel * sceneObject.modelToWorld;

        if (modelAccelerator->collision(*sceneObjectAccelerator, sceneObjectToModel)) {
            return 0.0;
        }
    }

    //
    // check collision against activity blockers
    //
    if (!category.isAgentSupport)
    {
        for (const ActivityBlocker &blocker : activityBlockers)
        {
            if (modelAccelerator->collision(*blocker.accel, worldToModel)) {
                return 0.0;
            }
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

void Synthesizer::computeLeftRight(const DisembodiedObject &object, const vec3f &contactPos, ObjectScoreEntry &result)
{
    const Category &category = database->categories.getCategory(object.model->categoryName);
    
    if (category.isLeftRight)
    {
        const Agent &agent = scene.agents[result.agents[0].agentIndex];
        UINT existingCount = 0;
        for (const ObjectInstance *o : agent.boundObjects())
        {
            if (o->categoryName() == category.name)
                existingCount++;
        }

        if (existingCount >= 2)
            cout << "Too many instances for left-right category: " << category.name << "," << existingCount << endl;

        bool rightTarget = (existingCount % 2 == 0);

        bool side = (agent.rightDir | (contactPos - agent.headPos)) > 0.0f;

        result.leftRightScore = 0.0f;
        if (side == rightTarget)
            result.leftRightScore = 1.0f;
        
        if (result.leftRightScore == 0.0f)
            result.acceptable = 0.0f;
    }
}

void Synthesizer::computeBaseOffset(const DisembodiedObject &object, const mat4f &modelToWorld, const vec3f &contactPos, ObjectScoreEntry &result)
{
    const Category &category = database->categories.getCategory(object.model->categoryName);
    
    //
    // A location is only acceptable if the base offset term is non-zero for at least one agent
    //
    result.acceptable = 0.0f;

    for (size_t agentIndex = 0; agentIndex < result.agents.size(); agentIndex++)
    {
        ObjectScoreAgentEntry &agentEntry = result.agents[agentIndex];
        const Agent &agent = scene.agents[agentEntry.agentIndex];
        agentEntry.baseOffsetScore = 0.0f;

        if (database->activities.hasActivityCategoryDistribution(agent.activity, category.name))
        {
            const ActivityCategoryDistributions &distributions = database->activities.getActivityCategoryDistribution(agent.activity, category.name);

            agentEntry.baseOffset = agent.worldToAgentBasis(contactPos);

            float offsetScaleFactor = 1.0f;

            agentEntry.baseOffsetScore = SynthScore::distributionScore(distributions.base, agentEntry.baseOffset, offsetScaleFactor);

            if (agentEntry.baseOffsetScore > 0.0f)
                result.acceptable = 1.0f;

            if ((category.isHandOrientation && !category.isAgentSupport) || ml::util::startsWith(object.modelId, "hao.Chair."))
            {
                if (object.agentFace.lengthSq() == 0.0)
                {
                    //cout << "Null agent face -- " << category.name << endl;
                    agentEntry.handOrientationScore = 0.0f;
                }
                else
                {
                    agentEntry.handOrientationScore = SynthScore::handOrientationScore(object, modelToWorld, agent);
                }
            }
        }
    }
}

void Synthesizer::computeOverhang(const DisembodiedObject &object, const ObjectInstance &parent, const mat4f &modelToWorld, ObjectScoreEntry &result)
{
    result.overhang = 0.0f;
    result.overhangScore = 1.0f;

    //
    // cannot overhang the room
    //
    if (parent.parentObjectInstanceIndex == -1)
        return;

    bbox3f modelBBox = object.model->bbox;
    modelBBox.scale(1.2f);
    const OBBf obb = modelToWorld * OBBf(modelBBox);

    Polygonf startPoly = SynthUtil::makeOBBTopPolygon(obb);
    float startArea = startPoly.convexArea();
    if (startArea <= 0.001f)
        return;

    Polygonf parentPoly = SynthUtil::makeOBBTopPolygon(parent.worldBBox);

    Polygonf clippedPoly = Polygonf::clip(startPoly, parentPoly);

    result.overhang = 1.0f - clippedPoly.convexArea() / startPoly.convexArea();
    result.overhangScore = SynthScore::overhangScore(result.overhang);

    if (object.model->categoryName == "Laptop")
        result.overhangScore = (result.overhang < 0.3f) ? 1.0f : 0.0f;
    
    if (result.overhangScore != 1.0f && result.overhang >= 0.01f)
        result.acceptable = 0.0f;
}

void Synthesizer::computeInteractionRays(const DisembodiedObject &object, const mat4f &modelToWorld, ObjectScoreEntry &result)
{
    for (size_t agentIndex = 0; agentIndex < result.agents.size(); agentIndex++)
    {
        ObjectScoreAgentEntry &agentEntry = result.agents[agentIndex];
        const Agent &agent = scene.agents[agentEntry.agentIndex];

        makeInteractionMapRays(object, modelToWorld, agent, "gaze", agentEntry.gaze);
        makeInteractionMapRays(object, modelToWorld, agent, "fingertip", agentEntry.fingertip);
        makeInteractionMapRays(object, modelToWorld, agent, "backSupport", agentEntry.backSupport);
    }
}

void Synthesizer::computeCollision(const DisembodiedObject &object, const mat4f &modelToWorld, ObjectScoreEntry &result)
{
    result.collisionBBoxScore = collisionBBoxScore(object, modelToWorld);
    result.collisionScore = collisionScore(object, modelToWorld);

    if (result.collisionScore == 0.0f)
        result.acceptable = 0.0f;
}

void Synthesizer::computeScanPlacement(const DisembodiedObject &object, const mat4f &modelToWorld, ObjectScoreEntry &result)
{
    const Category &category = database->categories.getCategory(object.model->categoryName);
    const ModelAssetData &asset = assets->loadModel(*graphics, object.modelId);
    
    memcpy(&columnsTemp.columns(0, 0), &baseColumns.columns(0, 0), sizeof(SceneColumn)* baseColumns.columns.getNumElements());

    //columnsTemp.includeObjectRayTracing(accelerator, bbox3f(worldBBox), modelToWorld, modelToWorld.getInverse(), category.supportCategory);
    columnsTemp.includeObjectMaxZMap(asset.getMaxZMap(), modelToWorld, modelToWorld.getInverse(), asset.getModelToVoxel(), category.isObjectSupport);
    //columnsTemp.includeObjectBBoxSimplification(worldBBox, category.supportCategory);

    result.scanPlacementScore = ColumnRepresentation::score(sceneTemplate->geometry.columns, columnsTemp);
}

void Synthesizer::computeAllTerms(const DisembodiedObject &object, const ObjectInstance &parent, const mat4f &modelToWorld, const vec3f &contactPos, ObjectScoreEntry &result)
{
    computeBaseOffset(object, modelToWorld, contactPos, result);
    
    //
    // we don't compute left-right here, we just assume the speakers have already been placed correctly.
    //
    //computeLeftRight(object, contactPos, result);

    computeOverhang(object, parent, modelToWorld, result);
    computeInteractionRays(object, modelToWorld, result);
    computeScanPlacement(object, modelToWorld, result);
    computeCollision(object, modelToWorld, result);
}

void Synthesizer::updateObjectScores()
{
    objectScores.clear();
    objectScores.resize(scene.objects.size());
    for (UINT objectIndex = 0; objectIndex < scene.objects.size(); objectIndex++)
    {
        ObjectScoreEntry &entry = objectScores[objectIndex];
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
            if (agentIndices.size() == 0)
            {
                cout << "Terminating scene -- object not bound to any agents found: " << instance.categoryName() << endl;
                earlyTermination = true;
            }
            computeAllTerms(instance.object, scene.objects[instance.parentObjectInstanceIndex], instance.modelToWorld, instance.contactPosition, entry);
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
        const ObjectScoreEntry &entry = objectScores[objectIndex];
        result += entry.objectOnlyScoreTerms();

        for (const ObjectScoreAgentEntry &agent : entry.agents)
        {
            result += agent.baseOffsetScore * SynthScore::baseOffsetScale();
            result += agent.handOrientationScore * SynthScore::handOrientationScale();

            result += SynthScore::gazeClamp(agent.gaze.score(false)) * SynthScore::gazeScale();
            result += agent.fingertip.score(false) * SynthScore::fingertipScale();
            result += agent.backSupport.score(true) * SynthScore::backSupportScale();
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
        const ObjectScoreEntry &entry = objectScores[objectIndex];
        result += entry.objectOnlyScoreTerms();

        for (const ObjectScoreAgentEntry &agent : entry.agents)
        {
            result += agent.baseOffsetScore * SynthScore::baseOffsetScale();
            if (!synthParams().fisherSynthesisMode)
            {
                result += agent.handOrientationScore * SynthScore::handOrientationScale();
                result += SynthScore::gazeClamp(agent.gaze.score(false, accelerator, modelToWorld)) * SynthScore::gazeScale();
                result += agent.fingertip.score(false, accelerator, modelToWorld) * SynthScore::fingertipScale();
                result += agent.backSupport.score(true, accelerator, modelToWorld) * SynthScore::backSupportScale();
            }
        }
    }

    result += activeScore.objectOnlyScoreTerms();
    for (const ObjectScoreAgentEntry &agent : activeScore.agents)
    {
        result += agent.baseOffsetScore * SynthScore::baseOffsetScale();

        if (!synthParams().fisherSynthesisMode)
        {
            result += agent.handOrientationScore * SynthScore::handOrientationScale();
            result += SynthScore::gazeClamp(agent.gaze.score(false)) * SynthScore::gazeScale();
            result += agent.fingertip.score(false) * SynthScore::fingertipScale();
            result += agent.backSupport.score(true) * SynthScore::backSupportScale();
        }
    }

    return result;
}

void Synthesizer::evaluatePlacementA()
{
    Timer t;
    if(verbose) cout << "Evaluating fast fields...";
    UINT locationCount = 0, acceptableCount = 0;
    for (CandidateObject &o : candidateObjects)
    {
        for (PlacementLocation &l : o.locations)
        {
            for (ObjectScoreEntry &r : l.rotations)
            {
                mat4f modelToWorld = makeModelToWorld(o.object, l, r.rotation);
                computeBaseOffset(o.object, modelToWorld, l.sample.pos, r);
                computeOverhang(o.object, scene.objects[o.parentObjectIndex], modelToWorld, r);
                computeLeftRight(o.object, l.sample.pos, r);
                locationCount++;
                if (r.acceptable > 0.0f)
                {
                    computeInteractionRays(o.object, modelToWorld, r);
                    acceptableCount++;
                }
            }
        }
    }
    if (verbose)
    {
        cout << t.getElapsedTime() << "s" << endl;
        cout << acceptableCount << " / " << locationCount << " acceptable locations" << endl;
    }
}

void Synthesizer::evaluatePlacementB()
{
    updateBaseColumns();

    Timer tCollision;
    if (verbose) cout << "Evaluating collisions...";
    UINT collisionCount = 0, acceptableCount = 0;
    for (CandidateObject &o : candidateObjects)
    {
        for (PlacementLocation &l : o.locations)
        {
            for (ObjectScoreEntry &r : l.rotations)
            {
                if (r.acceptable)
                {
                    mat4f modelToWorld = makeModelToWorld(o.object, l, r.rotation);
                    computeCollision(o.object, modelToWorld, r);
                    collisionCount++;
                    if (r.acceptable)
                        acceptableCount++;
                }
            }
        }
    }

    if (verbose)
    {
        cout << tCollision.getElapsedTime() << "s" << endl;
        cout << acceptableCount << " / " << collisionCount << " collision free locations" << endl;
    }

    Timer tScanPlacement;
    if(verbose) cout << "Evaluating scan placement...";
    for (CandidateObject &o : candidateObjects)
    {
        for (PlacementLocation &l : o.locations)
        {
            for (ObjectScoreEntry &r : l.rotations)
            {
                if (r.acceptable)
                {
                    mat4f modelToWorld = makeModelToWorld(o.object, l, r.rotation);
                    computeScanPlacement(o.object, modelToWorld, r);
                }
            }
        }
    }
    if (verbose) cout << tScanPlacement.getElapsedTime() << "s" << endl;
}

void Synthesizer::updateBaseColumns()
{
    baseColumns.reset(sceneTemplate->geometry.columns);
    for (UINT objectIndex = 1; objectIndex < scene.objects.size(); objectIndex++)
    {
        const ObjectInstance &o = scene.objects[objectIndex];
        const auto &accelerator = assets->loadModel(*graphics, o.object.modelId).getAcceleratorOriginal();

        bool isSupport = database->categories.getCategory(o.categoryName()).isObjectSupport;

        baseColumns.includeObjectRayTracing(accelerator, bbox3f(o.worldBBox), o.modelToWorld, o.modelToWorld.getInverse(), isSupport);
    }
    columnsTemp = baseColumns;
}

void Synthesizer::chooseObjectAndLocation(int candidateObjectFilter, int &chosenCandidateObjectIndex, int &chosenLocationIndex, int &chosenRotationIndex, bool silentMode) const
{
    struct AcceptableLocation
    {
        UINT candidateObjectIndex;
        UINT locationIndex;
        UINT rotationIndex;
        double score;
    };

    chosenCandidateObjectIndex = -1;
    chosenLocationIndex = -1;
    chosenRotationIndex = -1;
    
    vector<AcceptableLocation> acceptableLocations;

    for (UINT candidateObjectIndex = 0; candidateObjectIndex < candidateObjects.size(); candidateObjectIndex++)
    {
        const CandidateObject &candidate = candidateObjects[candidateObjectIndex];

        for (UINT locationIndex = 0; locationIndex < candidate.locations.size(); locationIndex++)
        {
            const PlacementLocation &l = candidate.locations[locationIndex];

            for (UINT rotationIndex = 0; rotationIndex < l.rotations.size(); rotationIndex++)
            {
                const ObjectScoreEntry &entry = l.rotations[rotationIndex];
                mat4f modelToWorld = makeModelToWorld(candidate.object, l, entry.rotation);
                double combinedScore = scoreArrangement(entry, candidate.object.modelId, modelToWorld);

                if (combinedScore <= 0.0)
                    cout << "unexpected non-positive score" << endl;

                if (combinedScore > 0.0 && entry.acceptable &&
                    (candidateObjectFilter == -1 || candidateObjectFilter == candidateObjectIndex))
                {
                    AcceptableLocation a;
                    a.candidateObjectIndex = candidateObjectIndex;
                    a.locationIndex = locationIndex;
                    a.rotationIndex = rotationIndex;
                    a.score = combinedScore;
                    acceptableLocations.push_back(a);
                }
            }
        }
    }

    if (acceptableLocations.size() == 0)
    {
        if (verbose && !silentMode) cout << "No acceptable locations" << endl;
        return;
    }

    std::random_shuffle(acceptableLocations.begin(), acceptableLocations.end());
    std::stable_sort(acceptableLocations.begin(), acceptableLocations.end(), [](const AcceptableLocation &a, const AcceptableLocation &b) { return a.score < b.score; });

    const AcceptableLocation &result = acceptableLocations.back();
    chosenCandidateObjectIndex = result.candidateObjectIndex;
    chosenLocationIndex = result.locationIndex;
    chosenRotationIndex = result.rotationIndex;

    const ObjectScoreEntry &r = candidateObjects[chosenCandidateObjectIndex].locations[result.locationIndex].rotations[result.rotationIndex];
}

void Synthesizer::placeBestObject()
{
    ObjectInstance instance(&scene);

    int candidateIndex, locationIndex, rotationIndex;
    chooseObjectAndLocation(-1, candidateIndex, locationIndex, rotationIndex, false);

    if (candidateIndex == -1 || locationIndex == -1 || rotationIndex == -1)
    {
        if (candidateObjects.size() == 0)
            cout << "no candidate objects?" << endl;

        const Category &category = database->categories.getCategory(candidateObjects[0].object.model->categoryName);

        if (!synthParams().haoSynthesisMode)
            cout << "Unplaceable object: " << category.name << endl;

        const bool noLocations = candidateObjects[0].locations.size() == 0 || candidateObjects[0].locations[0].rotations.size() == 0;
        if (noLocations)
            cout << "no locations?" << endl;

        bool categoryEssential = !category.isNonEssential;

        if (synthParams().haoSynthesisMode)
        {
            categoryEssential = (category.name == "Chair" || category.name == "Table");
        }

        if (!categoryEssential && !noLocations)
        {

            candidateIndex = 0;
            locationIndex = 0;
            rotationIndex = 0;

            candidateObjects[0].object.scale = 0.0f;
        }
        else
        {
            cout << "essential category: " << category.name << "; terminating scene" << endl;
            earlyTermination = true;
            return;
        }
    }

    CandidateObject &candidate = candidateObjects[candidateIndex];
    instance.object = candidate.object;

    const PlacementLocation& location = candidate.locations[locationIndex];
    
    const Category& category = database->categories.getCategory(candidate.object.model->categoryName);

    instance.modelToWorld = makeModelToWorld(candidate.object, location, location.rotations[rotationIndex].rotation);

    instance.parentObjectInstanceIndex = candidate.parentObjectIndex;
    instance.parentMeshIndex = location.sample.meshIndex;
    instance.parentTriIndex = location.sample.triangleIndex;
    instance.parentUV = location.sample.uv;

    instance.contactPosition = location.sample.pos;
    instance.contactNormal = location.sample.normal;

	instance.worldBBox = instance.modelToWorld * OBBf(candidate.object.model->bbox);

    scene.objects.push_back(instance);

    bool usefulCheck = false;

    for (UINT candidateAgentIndex = 0; candidateAgentIndex < candidate.agents.size(); candidateAgentIndex++)
    {
        const UINT sceneAgentIndex = candidate.agents[candidateAgentIndex];
        const Agent &sceneAgent = scene.agents[sceneAgentIndex];

        bool usefulToAgent = true;
        if (category.isShared)
        {
            //
            // Deals with the multi-dining-table case. Only add tables that are good enough for each agent.
            //
            if (location.rotations[rotationIndex].agents[candidateAgentIndex].baseOffsetScore == 0.0)
            {
                usefulToAgent = false;
            }
        }
        if (category.name == "Table" && scene.objectsWithCategory("Table", sceneAgentIndex).size() > 0)
        {
            usefulToAgent = false;
        }
        
        if (usefulToAgent)
        {
            usefulCheck = true;
            if (candidate.isAgentSupport)
                scene.agents[sceneAgentIndex].supportObjectIndex = ((UINT)scene.objects.size() - 1);
            else
            {
                scene.agents[sceneAgentIndex].boundObjectIndices.push_back((UINT)scene.objects.size() - 1);
                if (verbose && candidate.object.model->categoryName == "Table")
                    cout << "Binding table to agent " << sceneAgentIndex << endl;
            }
        }
    }

    if (!usefulCheck)
    {
        cout << "Useless object, terminating scene: " << category.name << endl;
        earlyTermination = true;
    }

    candidateObjects.clear();

    updateObjectScores();
    updateScorecard();
}

void Synthesizer::updateScorecard()
{
    //
    // since geometry is a global term, all objects should have the same geometry score
    //
    scene.scorecard.scanGeometry = objectScores[1].scanPlacementScore;
    scene.scorecard.objectCount = (double)scene.objects.size();
    
    UINT offsetCount = 0;
    scene.scorecard.averageOffsetScore = 0.0f;
    scene.scorecard.worstOffsetScore = 2.0f;
    scene.scorecard.worstGaze = 2.0f;
    bool anomolyDetected = false;
    for (UINT objectIndex = 1; objectIndex < scene.objects.size(); objectIndex++)
    {
        const auto &score = objectScores[objectIndex];
        if (score.collisionScore == 0.0f)
        {
            //
            // our collision function doesn't exclude the current object, so every object will alwyas seem to collide with the scene.
            //
        }
        if (score.agents.size() == 1)
        {
            scene.scorecard.worstOffsetScore = std::min(scene.scorecard.worstOffsetScore, score.agents[0].baseOffsetScore);
            scene.scorecard.averageOffsetScore += score.agents[0].baseOffsetScore;
            scene.scorecard.worstGaze = std::min(scene.scorecard.worstGaze, score.agents[0].gaze.score(false));
            offsetCount++;
        }
    }
    scene.scorecard.averageOffsetScore /= offsetCount;

    if (anomolyDetected)
    {
        scene.scorecard.averageOffsetScore = -1.0f;
    }
}

mat4f Synthesizer::makeModelToWorld(const DisembodiedObject &object, const PlacementLocation &location, float rotation) const
{
    if (object.contactType == ContactUpright)
    {
        float elevation = 0.003f;

        //
        // Plates need to be a bit higher to ensure that collisions occur...
        //
        if (object.model->categoryName == "Plate")
            elevation = 0.005f;

        return mat4f::translation(location.sample.pos) * mat4f::rotationZ(rotation) * object.makeBaseTransform(elevation);
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
            float curRotationDist = distSq(downDir, -vec3f::eZ);
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
    for (CandidateObject &o : candidateObjects)
    {
        makeLocations(o);
    }
}

void Synthesizer::makeLocations(CandidateObject &candidate)
{
    const ObjectInstance &parentObject = scene.objects[candidate.parentObjectIndex];
    const Category &parentCategory = database->categories.getCategory(parentObject.object.model->categoryName);

    vector< pair<const ml::TriMeshf*, mat4f> > objectMeshes;
    auto &renderData = assets->loadModel(*graphics, parentObject.object.modelId);
    for (auto &m : renderData.data)
    {
        objectMeshes.push_back(std::make_pair(&m.meshOriginal.getTriMesh(), parentObject.modelToWorld));
    }

    vector<ml::TriMeshSampler<float>::Sample> samples;
    
    //
    // the ground plane should have more samples
    //
    UINT scaleFactor = 1;
    if (candidate.parentObjectIndex == 0)
        scaleFactor = 4;

    if (candidate.object.contactType == ContactUpright)
        samples = ml::TriMeshSampler<float>::sample(objectMeshes, 1000.0f, synthParams().maxSampleCount * scaleFactor, vec3f::eZ);
    if (candidate.object.contactType == ContactWall)
        samples = ml::TriMeshSampler<float>::sample(objectMeshes, 1000.0f, synthParams().maxSampleCount * scaleFactor, [](const vec3f& v) { return fabs(v.z) < 0.01f; });

    if (parentCategory.isTopSupport && synthParams().scanName != "dining-round")
    {
        //
        // only sample from the top-most surface
        //
        float maxZ = 0.0f;
        for (const auto &sample : samples)
            maxZ = std::max(sample.pos.z, maxZ);

        const float threshold = 0.01f;
        vector<ml::TriMeshSampler<float>::Sample> newSamples;
        for (const auto &sample : samples)
        {
            if (maxZ - sample.pos.z < threshold)
                newSamples.push_back(sample);
        }
        samples = newSamples;
    }

    candidate.locations.clear();
    candidate.locations.resize(samples.size());

    for (UINT sampleIndex = 0; sampleIndex < samples.size(); sampleIndex++)
    {
        const auto &sample = samples[sampleIndex];
        auto &location = candidate.locations[sampleIndex];
        location.sample = sample;

        UINT rotationCount = synthParams().rotationsPerLocation;
        if (candidate.object.contactType == ContactWall)
            rotationCount = 1;

        for (UINT rotationIndex = 0; rotationIndex < rotationCount; rotationIndex++)
        {
            float rotation;

            if (synthParams().haoSynthesisMode)
            {
                rotation = ml::util::randomUniform(0.0f, 360.0f);
            }
            else
            {
                const Category &category = database->categories.getCategory(candidate.object.model->categoryName);
                const auto &instance = category.sampleInstance();

                rotation = fmod(ml::util::randomInteger(-1, 1) * instance.coordianteFrameRotation + ml::util::randomInteger(-4, 3) * 90.0f, 360.0f);

                if (candidate.object.model->categoryName == "Computer")
                {
                    //rotation = ml::util::randomUniform(0.0f, 360.0f);
                }
            }

            if (location.closestRotation(rotation) > 1.0f)
            {
                location.rotations.push_back(ObjectScoreEntry(rotation, candidate.agents));
            }
        }

        std::sort(location.rotations.begin(), location.rotations.end());
    }

    if(verbose) cout << "Sampled " << samples.size() << " candidate locations" << endl;
}

void Synthesizer::updateAgentCategories()
{
    int agentIndex = 0;
    for (SynthesizerAgent &agent : agents)
    {
        auto &activity = database->activities.getActivity(agent.agent->activity);
        auto &distribution = *activity.objectGroupDistribution;
        for (const string &categoryName : distribution.categoryList())
        {
            AgentCategoryData &data = agent.categories[categoryName];

            data.category = &database->categories.getCategory(categoryName);
            data.sceneCount = (UINT)scene.objectsWithCategory(categoryName, agentIndex).size();

            double sceneInsertionScore = distribution.probabilityCategoryCountAtLeast(categoryName, data.sceneCount + 1);
            double agentInsertionScore = 1.0;
            if (data.sceneCount >= data.sampledCount)
                agentInsertionScore = 0.0;

            if (sceneInsertionScore != 0.0 && (categoryName == "MousePad" || categoryName == "Laptop"))
                sceneInsertionScore = 0.99;

            data.activityInsertionScore = sceneInsertionScore * agentInsertionScore;
        }
        
        //
        // TODO: fix this if activities have multiple possible supporting categories
        //
        const string supportingCategory = activity.supportDistribution.categories.begin()->first;

        if (supportingCategory != "Room")
        {
            AgentCategoryData &data = agent.categories[supportingCategory];

            data.category = &database->categories.getCategory(supportingCategory);
            data.sceneCount = 0;

            if (agent.agent->supportObjectIndex == (UINT)-1)
                data.activityInsertionScore = 2.0;
            else
                data.activityInsertionScore = 0.0;
        }
        
        agentIndex++;
    }
}

void Synthesizer::makeInteractionMapRays(const DisembodiedObject &object, const mat4f &modelToWorld, const Agent &agent, const string &interactionMapName, InteractionRays &result) const
{
    const string &categoryName = object.model->categoryName;

    result.rays.clear();

    if (!database->interactionMaps.hasInteractionMapEntry(object.modelId, interactionMapName))
    {
        return;
    }
    const InteractionMapEntry &entry = database->interactionMaps.getInteractionMapEntry(object.modelId, interactionMapName);

    const auto &centroids = entry.centroids;
    if (centroids.size() == 0 || (centroids.size() > 10 && synthParams().scanName == "dining-large-nowalld"))
    {
        return;
    }

    vector < pair<const ml::TriMeshAcceleratorBVHf*, mat4f> > allAccelerators;
    
    for (UINT objectIndex = 0; objectIndex < scene.objects.size(); objectIndex++)
    {
        //
        // skip the base architecture, since collisions with that are very unlikely to occlude things
        //
        if (object.contactType == ContactWall || objectIndex != 0)
        {
            const auto &object = scene.objects[objectIndex];
            const auto &accelerator = assets->loadModel(*graphics, object.object.modelId).getAcceleratorOriginal();
            allAccelerators.push_back(std::make_pair(&accelerator, object.modelToWorld.getInverse()));
        }
    }
    
	const auto &selfAccelerator = assets->loadModel(*graphics, object.modelId).getAcceleratorOriginal();
    allAccelerators.push_back(std::make_pair(&selfAccelerator, modelToWorld.getInverse()));

    result.rays.resize(centroids.size());

    bool visibilityOptional = (interactionMapName == "backSupport" || interactionMapName == "hips");

    vec3f rayOrigin = agent.headPos;

    if (interactionMapName == "fingertip")
    {
        rayOrigin = agent.handPos();
    }

    if (interactionMapName == "backSupport")
    {
        rayOrigin = agent.spinePos();
    }

    UINT centroidIndex = 0;
    for (const auto &centroid : centroids)
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
                    // for monitors, gaze is better when directly viewing the object head-on
                    //if (categoryName == "Monitor" || categoryName == "Laptop")
                    score = cosineVisiblityScore;
                }

                if (interactionMapName == "fingertip")
                {
                    score -= 0.01f * centroidDist;
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