
#include "main.h"

const float locationBoxSize = 0.01f;

void FieldMode::init(ml::ApplicationData& _app, AppState &_state)
{
    app = &_app;
    state = &_state;

    selectedCandidateIndex = -1;
    selectedLocationIndex = -1;
    selectedRotationIndex = -1;
    selectedAgentIndex = 0;

    field = VizzerFieldDistributionBase;

    gradient = ml::ColorGradient(ml::LodePNG::load("../../shaders/BlackToGreen.png"));

    rayVisualizationDirty = false;
}

//void FieldMode::updateScanObjectVisualization()
//{
//    state->locatedBBoxes.clear();
//    
//    if (candidateValid())
//    {
//        const CandidateObject &candidate = state->synth.candidateObjects[selectedCandidateIndex];
//        for (const auto &scanObject : candidate.scanLocations->locations)
//        {
//            mat4f cubeToWorld = scanObject.oobb.getOBBToWorld() * mat4f::scale(0.5f) * mat4f::translation(vec3f(1.0f, 1.0f, 1.0f));
//            state->locatedBBoxes.push_back(ml::D3D11TriMesh(app->graphics, ml::Shapesf::wireframeBox(cubeToWorld, vec4f(1.0f, 1.0f, 1.0f, 1.0f), 0.002f)));
//        }
//    }
//}

void FieldMode::updateColumns()
{
    Synthesizer &synth = state->synth;
    const Scene *sceneOverride = NULL;

    if (synthParams().columnRepresentationScene.size() > 0)
    {
        cout << "overriding column representation from " << synthParams().columnRepresentationScene << endl;
        sceneOverride = &state->database.scenes.getScene(synthParams().columnRepresentationScene);
    }

    const Scene &scene = sceneOverride ? *sceneOverride : synth.scene;

    columns.reset(state->sceneTemplate.geometry.columns);

    for (UINT objectIndex = 1; objectIndex < scene.objects.size(); objectIndex++)
    {
        const ObjectInstance &o = scene.objects[objectIndex];
        bool isSupport = state->database.categories.getCategory(o.categoryName()).isObjectSupport;

        const auto &accelerator = state->assets.loadModel(app->graphics.castD3D11(), o.object.modelId).getAcceleratorOriginal();

        //columns.includeObjectBBoxSimplification(o.worldBBox, isSupport);
        columns.includeObjectRayTracing(accelerator, bbox3f(o.worldBBox), o.modelToWorld, o.modelToWorld.getInverse(), isSupport);
    }

    if (!candidateValid())
        return;

    const CandidateObject &candidate = synth.candidateObjects[selectedCandidateIndex];

    if (selectedLocationIndex != -1 && selectedRotationIndex != -1)
    {
        const PlacementLocation &l = candidate.locations[selectedLocationIndex];
        const ObjectScoreEntry &r = l.rotations[selectedRotationIndex];
        const ObjectScoreAgentEntry &agentData = r.agents[selectedAgentIndex];
        const Model &model = state->database.models.getModel(candidate.object.modelId);

        const mat4f modelToWorld = synth.makeModelToWorld(candidate.object, l, l.rotations[selectedRotationIndex].rotation);
        bool isSupport = state->database.categories.getCategory(model.categoryName).isObjectSupport;

        const ModelAssetData &asset = state->assets.loadModel(app->graphics.castD3D11(), candidate.object.modelId);

        //columns.includeObjectBBoxSimplification(modelToWorld * OBBf(model.bbox), isSupport);
        //columns.includeObjectRayTracing(asset.getAcceleratorOriginal(), bbox3f(modelToWorld * OBBf(model.bbox)), modelToWorld, modelToWorld.getInverse(), isSupport);
        columns.includeObjectMaxZMap(asset.getMaxZMap(), modelToWorld, modelToWorld.getInverse(), asset.getModelToVoxel(), isSupport);
    }

    if (state->showScanColumns)
    {
        columns = state->sceneTemplate.geometry.columns;
    }

    updateColumnVisualization();
}

void FieldMode::updateColumnVisualization()
{
    vector<ml::TriMeshf> meshes;
    vec3f padding = vec3f(columns.columnDimension * 0.5f * 0.75f, columns.columnDimension * 0.5f * 0.75f, 0.0f);

    for (UINT y = 0; y < columns.columns.getDimY(); y++)
    {
        for (UINT x = 0; x < columns.columns.getDimX(); x++)
        {
            const SceneColumn &c = columns.columns(x, y);
            const vec2f center = columns.getColumnCenter(x, y);

            vec3f start = vec3f(center, 0.0f);
            vec3f support = vec3f(center, c.maxSupportZ);
            vec3f top = vec3f(center, c.maxAnyZ);

            if (c.maxSupportZ > 0.0f)
            {
                bbox3f bbox;
                bbox.include(start - padding);
                bbox.include(start + padding);
                bbox.include(support);
                meshes.push_back(ml::Shapesf::box(bbox, ml::vec4f(0.2f, 0.2f, 0.8f, 1.0f)));
            }

            if (c.maxAnyZ > c.maxSupportZ)
            {
                bbox3f bbox;
                bbox.include(support - padding * 0.75f);
                bbox.include(support + padding * 0.75f);
                bbox.include(top);
                meshes.push_back(ml::Shapesf::box(bbox, ml::vec4f(0.3f, 0.7f, 0.3f, 1.0f)));
            }
        }
    }

    columnVisualization.load(app->graphics, ml::meshutil::createUnifiedMesh(meshes));
    ml::MeshIOf::saveToFile("C:/code/columnVisualization.ply", columnVisualization.getMeshData());
}

void FieldMode::updateRayVisualization()
{
    Synthesizer &synth = state->synth;
    const Scene &scene = synth.scene;

    vector<ml::TriMeshf> meshes;

    if (selectedCandidateIndex != -1 && selectedLocationIndex != -1 && selectedRotationIndex != -1)
    {
        const CandidateObject &c = state->synth.candidateObjects[selectedCandidateIndex];
        const PlacementLocation &l = c.locations[selectedLocationIndex];
        const ObjectScoreEntry &r = l.rotations[selectedRotationIndex];
        const ObjectScoreAgentEntry &agentData = r.agents[selectedAgentIndex];

        const mat4f modelToWorld = synth.makeModelToWorld(c.object, l, l.rotations[selectedRotationIndex].rotation);
        for (UINT ownerIndex : c.agents)
        {
            const InteractionMap &iMap = state->database.interactionMaps.maps[c.object.modelId];
            const Agent &agent = scene.agents[ownerIndex];
            const ObjectScoreAgentEntry &agentData = r.agents[selectedAgentIndex];

            for (const auto &interactionPair : agentData.allInteractions())
            {
                vec3f interactionColor = vec3f(InteractionMapEntry::interactionColor(interactionPair.second));
                for (const auto &ray : interactionPair.first->rays)
                {
                    vec3f color = interactionColor;

                    if (!ray.visible)
                        color *= 0.4f;

                    meshes.push_back(ml::Shapesf::sphere(0.01f, ray.centroidPos, 5, 5, ml::vec4f(color, 1.0f)));
                    meshes.push_back(ml::Shapesf::cylinder(ray.ray.origin(), ray.centroidPos, 0.003f, 5, 7, ml::vec4f(color, 1.0f)));
                }
            }
        }
    }

    rayVisualization.load(app->graphics, ml::meshutil::createUnifiedMesh(meshes));
    rayVisualizationDirty = false;
}

void FieldMode::renderCandidateObject()
{
    const Synthesizer &synth = state->synth;

    if (selectedCandidateIndex >= synth.candidateObjects.size() ||
        selectedLocationIndex >= synth.candidateObjects[selectedCandidateIndex].locations.size() ||
        selectedRotationIndex >= synth.candidateObjects[selectedCandidateIndex].locations[selectedLocationIndex].rotations.size())
    {
        selectedCandidateIndex = -1;
        selectedLocationIndex = -1;
        selectedRotationIndex = -1;
        return;
    }

    const CandidateObject &candidate = synth.candidateObjects[selectedCandidateIndex];

    if (selectedAgentIndex >= candidate.agents.size())
        selectedAgentIndex = 0;

    for (UINT locationIndex = 0; locationIndex < candidate.locations.size(); locationIndex++)
    {
        const PlacementLocation &l = candidate.locations[locationIndex];

        double fieldValue = -1.0;

        if (l.rotations.size() == 0)
        {
            cout << "No rotations found at location" << endl;
            return;
        }

        if (field == VizzerFieldCombinedScore)
            fieldValue = ml::util::maxValue(l.rotations, [&](const ObjectScoreEntry &e) { return e.collisionScore + e.wallDistScore + e.agents[selectedAgentIndex].baseOffsetScore; });
        if (field == VizzerFieldWallDistance)
            fieldValue = ml::util::maxValue(l.rotations, [&](const ObjectScoreEntry &e) { return e.wallDistScore; });
        if (field == VizzerFieldScanSupport)
            fieldValue = ml::util::maxValue(l.rotations, [&](const ObjectScoreEntry &e) { return e.scanPlacementScore; });
        if (field == VizzerFieldDistributionBase)
            fieldValue = ml::util::maxValue(l.rotations, [&](const ObjectScoreEntry &e) { return e.agents[selectedAgentIndex].baseOffsetScore; });
        if (field == VizzerFieldVisibilityGaze)
            fieldValue = ml::util::maxValue(l.rotations, [&](const ObjectScoreEntry &e) { return e.agents[selectedAgentIndex].gaze.score(false); });
        if (field == VizzerFieldVisibilityFintertip)
            fieldValue = ml::util::maxValue(l.rotations, [&](const ObjectScoreEntry &e) { return e.agents[selectedAgentIndex].fingertip.score(false); });
        if (field == VizzerFieldVisibilityBackSupport)
            fieldValue = ml::util::maxValue(l.rotations, [&](const ObjectScoreEntry &e) { return e.agents[selectedAgentIndex].backSupport.score(true); });

        vec3f color = vec3f(gradient.value(fieldValue));

        state->renderer.renderBox(app->graphics.castD3D11(), state->camera.getCameraPerspective(), l.sample.pos, locationBoxSize, color);
    }

    if (selectedLocationIndex != -1 && selectedRotationIndex != -1)
    {
        const PlacementLocation &l = candidate.locations[selectedLocationIndex];
        const ObjectScoreEntry &r = l.rotations[selectedRotationIndex];
        const ObjectScoreAgentEntry &agentData = r.agents[selectedAgentIndex];

        const mat4f modelToWorld = synth.makeModelToWorld(candidate.object, l, l.rotations[selectedRotationIndex].rotation);
        state->renderer.renderModel(state->assets, app->graphics.castD3D11(), state->camera.getCameraPerspective(), candidate.object.modelId, modelToWorld, vec3f(1.0f, 1.0f, 1.0f), state->showCollisionMesh);

        const Category &category = state->database.categories.getCategory(candidate.object.model->categoryName);

        if (category.isHandOrientation)
        {
            const vec3f p0 = modelToWorld * candidate.object.model->bbox.getCenter();

            vec3f p1 = modelToWorld * (candidate.object.model->bbox.getCenter() + vec3f(candidate.object.agentFace.x, candidate.object.agentFace.y, 0.0f));
            p1 = (p1 - p0).getNormalized() * 0.35f + p0;

            state->renderer.renderCylinder(app->graphics.castD3D11(), state->camera.getCameraPerspective(), p0, p1, vec3f(0.9f, 0.9f, 0.1f));
        }
    }
}

void FieldMode::render()
{
    const Scene &scene = state->synth.scene;

    if (state->showScene)
    {
        vector<vec3f> objectColors(scene.objects.size(), vec3f(1, 1, 1));
        state->renderer.renderObjects(state->assets, app->graphics.castD3D11(), state->camera.getCameraPerspective(), scene, objectColors, state->showCollisionMesh);
    }

    const bool haoObjectTest = false;
    if (haoObjectTest)
    {
        static int tableModelIndex = 0;

        if (GetAsyncKeyState(VK_LEFT)) tableModelIndex = tableModelIndex - 1;
        if (GetAsyncKeyState(VK_RIGHT)) tableModelIndex = tableModelIndex + 1;

        const Category &c = state->database.categories.getCategory("Table");
        const auto &o = c.instances[ ml::math::mod(tableModelIndex, c.instances.size())];
        const mat4f modelToWorld = mat4f::translation(vec3f(2.0f, 2.0f, 0.2f)) * o.makeBaseTransform();

        state->renderer.renderModel(state->assets, app->graphics.castD3D11(), state->camera.getCameraPerspective(), o.model->id, modelToWorld, vec3f(1.0f, 1.0f, 1.0f), false);
    }

    renderCandidateObject();

    if (rayVisualizationDirty)
    {
        updateRayVisualization();
    }
    state->renderer.renderMesh(app->graphics.castD3D11(), state->camera.getCameraPerspective(), rayVisualization, vec3f(1.0f, 1.0f, 1.0f));
    state->renderer.renderMesh(app->graphics.castD3D11(), state->camera.getCameraPerspective(), columnVisualization, vec3f(1.0f, 1.0f, 1.0f));
}

void FieldMode::keyDown(UINT key)
{
    const Synthesizer &synth = state->synth;

    if (key == KEY_C) updateColumns();
    if (key == KEY_V) columnVisualization = ml::D3D11TriMesh(app->graphics, TriMeshf());

    int rotationDelta = 0, agentDelta = 0, fieldDelta = 0, candidateDelta = 0;
    if (key == KEY_UP) candidateDelta = -1;
    if (key == KEY_DOWN) candidateDelta = 1;
    if (key == KEY_LEFT) rotationDelta = -1;
    if (key == KEY_RIGHT) rotationDelta = 1;
    if (key == KEY_N) fieldDelta = -1;
    if (key == KEY_M) fieldDelta = 1;
    if (key == KEY_K) agentDelta = -1;
    if (key == KEY_L) agentDelta = 1;

    if (key == KEY_B)
    {
        const int synthCount = 10000;
        const int minNumObjects = synthParams().scanName == "dining2" ? 30 : 15;

        const string baseVideoDir = synthParams().synthesisDir + "video" + synthParams().scanName + "/";

        ml::util::makeDirectory(baseVideoDir);

        ml::Cameraf camera = state->camera;
        camera.updateAspectRatio(1.0f);

        ml::D3D11RenderTarget renderTarget;
        renderTarget.load(app->graphics, 1024, 1024);
        renderTarget.bind();
        

        for (int synthIndex = 0; synthIndex < synthCount; synthIndex++)
        {
            cout << "rendering synth " << synthIndex << endl;

            const string outDir = baseVideoDir + ml::util::zeroPad(synthIndex, 3) + "/";
            ml::util::makeDirectory(outDir);

            ofstream cameraFile(outDir + "camera.txt");
            cameraFile << "LookAt " << camera.getEye() << ' ' << (camera.getEye() + camera.getLook()) << ' ' << camera.getUp() << endl;

            mutableSynthParams().forceConsistentCategory = (rand() % 2 == 0);

            Synthesizer synth;
            synth.start(state->database, app->graphics.castD3D11(), state->assets, state->sceneTemplate, "wss.room01", 0.0254f * synthParams().architectureScale);

            int prevObjectCount = -1, count = 0;

            for (UINT stepIndex = 0; stepIndex < 1000; stepIndex++)
            {
                if (synth.scene.objects.size() != prevObjectCount || stepIndex == 1)
                {
                    prevObjectCount = (int)synth.scene.objects.size();

                    renderTarget.clear(vec4f(1.0f, 1.0f, 1.0f, 1.0f));

                    vector<vec3f> objectColors(synth.scene.objects.size(), vec3f(1, 1, 1));
                    state->renderer.renderObjects(state->assets, app->graphics.castD3D11(), state->camera.getCameraPerspective(), synth.scene, objectColors, state->showCollisionMesh);

                    string countName;
                    if (stepIndex == 1)
                    {
                        state->renderer.renderMesh(app->graphics.castD3D11(), state->camera.getCameraPerspective(), state->targetScanMesh);
                        countName = "_scan";
                    }
                    else
                    {
                        countName = ml::util::zeroPad(count++, 4);
                    }

                    Bitmap bmp;
                    renderTarget.captureColorBuffer(bmp);
                    ml::LodePNG::save(bmp, outDir + countName + ".png");
                }

                synth.step();
                
                cout << "step " << stepIndex << " done" << endl;
            }

            synth.scene.save(outDir + "final.sss");

            if (synth.scene.objects.size() < minNumObjects) {
                ml::util::deleteDirectory(outDir);
                synthIndex--;
                cout << "not enough objects -> discarded" << endl;
            }
            
        }

        app->graphics.castD3D11().bindRenderTarget();
    }

    if (fieldDelta != 0)
        field = (VizzerField)ml::math::mod(field + fieldDelta, VizzerFieldCount);

    if (synth.candidateObjects.size() == 0)
        return;

    if (candidateDelta != 0)
    {
        selectedCandidateIndex = ml::math::mod(selectedCandidateIndex + candidateDelta, synth.candidateObjects.size());
        //updateScanObjectVisualization();
        viewMax(selectedCandidateIndex);
    }

    if (selectedCandidateIndex == -1)
        return;
    const CandidateObject &candidate = synth.candidateObjects[selectedCandidateIndex];

    if (rotationDelta != 0 && selectedLocationIndex != -1)
    {
        const PlacementLocation &l = candidate.locations[selectedLocationIndex];
        selectedRotationIndex = ml::math::mod(selectedRotationIndex + rotationDelta, l.rotations.size());
        rayVisualizationDirty = true;
    }
    if (agentDelta != 0 && candidate.agents.size() > 0)
    {
        selectedAgentIndex = ml::math::mod(selectedAgentIndex + agentDelta, candidate.agents.size());
    }
}

bool FieldMode::candidateValid() const
{
    return (selectedCandidateIndex != -1 && selectedCandidateIndex < state->synth.candidateObjects.size());
}

void FieldMode::viewMax(int candidateObjectFilter)
{
    state->synth.chooseObjectAndLocation(candidateObjectFilter, selectedCandidateIndex, selectedLocationIndex, selectedRotationIndex, true);
    rayVisualizationDirty = true;
}

void FieldMode::mouseDown(ml::MouseButtonType button)
{
    const Synthesizer &synth = state->synth;

    bool shift = (GetAsyncKeyState(VK_SHIFT) != 0);

    float screenX = (float)app->input.mouse.pos.x / (app->window.getWidth() - 1.0f);
    float screenY = (float)app->input.mouse.pos.y / (app->window.getHeight() - 1.0f);
    ml::Rayf ray = state->camera.getScreenRay(screenX, screenY);

    if (shift && button == ml::MouseButtonLeft && candidateValid())
    {
        const CandidateObject &candidate = synth.candidateObjects[selectedCandidateIndex];
        selectedLocationIndex = 0;

        for (UINT locationIndex = 0; locationIndex < candidate.locations.size(); locationIndex++)
        {
            const PlacementLocation &l = candidate.locations[locationIndex];
            ml::TriMeshAcceleratorBruteForcef accelerator;
            accelerator.build(state->renderer.boxMesh().getTriMesh(), mat4f::translation(l.sample.pos) * mat4f::scale(locationBoxSize));

            ml::TriMeshRayAccelerator<float>::Intersection intersection;
            if (accelerator.intersect(ray, intersection))
            {
                selectedLocationIndex = locationIndex;
                selectedRotationIndex = 0;
                selectedAgentIndex = 0;
                rayVisualizationDirty = true;
            }
        }
    }
}

string FieldMode::helpText() const
{
    string text;
    text += "Shift + left click: select location|";
    text += "Arrow keys: select rotation and object|";
    text += "N/M: select field|";
    text += "K/L: select agent|";
    text += "C/V: update or reset column representation";
    text += "B: render scenes for video";
    return text;
}

vector< pair<string, RGBColor> > FieldMode::statusText() const
{
    const auto &synth = state->synth;

    vector< pair<string, RGBColor> > result;

    result.push_back(std::make_pair("Field: " + fieldName(field), RGBColor::Blue));

    if (selectedCandidateIndex == -1 || selectedLocationIndex == -1 || selectedRotationIndex == -1 || selectedAgentIndex == -1)
        result.push_back(std::make_pair(string("No location selected"), RGBColor::White));
    else
    {
        const CandidateObject &c = synth.candidateObjects[selectedCandidateIndex];
        const PlacementLocation &l = c.locations[selectedLocationIndex];
        const ObjectScoreEntry &r = l.rotations[selectedRotationIndex];
        const ObjectScoreAgentEntry &agentData = r.agents[selectedAgentIndex];
        const Category &category = state->database.categories.getCategory(c.object.model->categoryName);

        mat4f modelToWorld = synth.makeModelToWorld(c.object, l, r.rotation);
        double combinedScore = synth.scoreArrangement(r, c.object.modelId, modelToWorld);

        result.push_back(std::make_pair("Combined score: " + to_string(combinedScore), RGBColor::White));

        result.push_back(std::make_pair("Rotation: " + to_string(r.rotation), RGBColor::White));

        //result.push_back(std::make_pair("Wall dist: " + to_string(r.wallDist), RGBColor::White));
        //result.push_back(std::make_pair("Wall dist score: " + to_string(r.wallDistScore), RGBColor::White));

        //result.push_back(std::make_pair("Wall rotation: " + to_string(r.wallRotation), RGBColor::White));
        //result.push_back(std::make_pair("Wall rotation score: " + to_string(r.wallRotationScore), RGBColor::White));

        result.push_back(std::make_pair("Overhang: " + to_string(r.overhang), RGBColor::White));
        result.push_back(std::make_pair("Overhang score: " + to_string(r.overhangScore), RGBColor::White));

        result.push_back(std::make_pair("Collision score: " + to_string(r.collisionScore), RGBColor::White));
        result.push_back(std::make_pair("Collision box score: " + to_string(r.collisionBBoxScore), RGBColor::White));

        result.push_back(std::make_pair("Base offset: " + ml::convert::toString(agentData.baseOffset), RGBColor::White));
        result.push_back(std::make_pair("Base score: " + to_string(agentData.baseOffsetScore), RGBColor::White));

        if (agentData.gaze.rays.size() > 0)
            result.push_back(std::make_pair("Gaze vis score: " + to_string(agentData.gaze.score(false)), InteractionMapEntry::interactionColor("gaze")));

        if (agentData.fingertip.rays.size() > 0)
            result.push_back(std::make_pair("Fintertip vis score: " + to_string(agentData.fingertip.score(false)), InteractionMapEntry::interactionColor("fingertip")));

        if (agentData.backSupport.rays.size() > 0)
            result.push_back(std::make_pair("Back vis score: " + to_string(agentData.backSupport.score(true)), InteractionMapEntry::interactionColor("backSupport")));

        if (category.isHandOrientation)
            result.push_back(std::make_pair("Hand orientation score: " + to_string(agentData.handOrientationScore), RGBColor(181, 230, 29)));

        result.push_back(std::make_pair("Scan object score: " + to_string(r.scanPlacementScore), RGBColor::White));

        //const float scanLocatorScore = synth.locator.scoreObject(c.object, modelToWorld);
        //const float scanLocatorScore = 0.0f;
        //result.push_back(std::make_pair("Scan location score: " + to_string(agentData.scanLocatorScore), RGBColor(255, 255, 255)));

        //result.push_back()
    }

    return result;
}
