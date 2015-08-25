
#include "main.h"

#include <boost/filesystem.hpp>

void ResultsMode::init(ml::ApplicationData& _app, AppState &_state)
{
    app = &_app;
    state = &_state;

    selectedObjectIndex = -1;
}

void ResultsMode::newScene(const string &filename)
{
    resultScene.load(filename, state->database);

    selectedObjectIndex = -1;

    //
    // Create accelerators
    //
    objectAccelerators.clear();
    objectAccelerators.resize(resultScene.objects.size());

    for (UINT objectIndex = 0; objectIndex < resultScene.objects.size(); objectIndex++)
    {
        const ObjectInstance &object = resultScene.objects[objectIndex];
        const ModelAssetData &ModelAssetData = state->assets.loadModel(app->graphics.castD3D11(), object.object.modelId);

        vector< pair<const ml::TriMeshf *, mat4f> > meshes;
        for (const auto &mesh : ModelAssetData.data)
			meshes.push_back(std::make_pair(&mesh.meshOriginal.getTriMesh(), object.modelToWorld));

        objectAccelerators[objectIndex].build(meshes);
    }
}

void ResultsMode::render()
{
    //
    // TODO: UI button
    //
    const bool showCollisionMesh = false;

    vector<vec3f> objectColors(resultScene.objects.size(), vec3f(1, 1, 1));
    state->renderer.renderObjects(state->assets, app->graphics.castD3D11(), state->camera.getCameraPerspective(), resultScene, objectColors, showCollisionMesh);

    for (const Agent &a : resultScene.agents)
    {
        vec3f color = vec3f(0.8f, 0.8f, 0.8f);
        state->renderer.renderAgent(app->graphics.castD3D11(), state->camera.getCameraPerspective(), a, color);
    }
}

void ResultsMode::sortResultsDirectory(const string &sourceDir, const string &targetDir, const function<double(const SceneScorecard&)> &valueFunction)
{
    Directory dir(sourceDir);
    ml::util::makeDirectory(targetDir);

    vector<Scene*> allScenes;

    for (const string &s : dir.filesWithSuffix(".sss"))
    {
        Scene *newScene = new Scene();
        newScene->load(sourceDir + s, state->database);
        allScenes.push_back(newScene);
    }

    auto sceneSorter = [&](const Scene *a, const Scene *b) {
        return valueFunction(a->scorecard) > valueFunction(b->scorecard);
    };

    std::sort(allScenes.begin(), allScenes.end(), sceneSorter);

    UINT sceneIndex = 0;
    for (const Scene *scene : allScenes)
    {
        scene->save(targetDir + ml::util::zeroPad(sceneIndex++, 4) + ".sss");
    }
}

void ResultsMode::sortAllResults()
{
    cout << "Sorting all results folders...";
    Directory resultsDir(synthParams().synthesisDir + "results/");
    for (const string &s : resultsDir.directories())
    {
        if (!ml::util::contains(s, "Sorted") && !ml::util::contains(s, "final-"))
        {
            //sortResultsDirectory(resultsDir.path() + s + "/", resultsDir.path() + s + "SortedObjectCount/", [](const SceneScorecard& s) { return s.objectCount; });
            sortResultsDirectory(resultsDir.path() + s + "/", resultsDir.path() + s + "SortedScanGeometry/", [](const SceneScorecard& s) { return s.scanGeometry; });
            //sortResultsDirectory(resultsDir.path() + s + "/", resultsDir.path() + s + "SortedWorstGaze/", [](const SceneScorecard& s) { return s.worstGaze; });
            //sortResultsDirectory(resultsDir.path() + s + "/", resultsDir.path() + s + "SortedWorstOffset/", [](const SceneScorecard& s) { return s.worstOffsetScore; });
            //sortResultsDirectory(resultsDir.path() + s + "/", resultsDir.path() + s + "SortedAvgOffset/", [](const SceneScorecard& s) { return s.averageOffsetScore; });
            //sortResultsDirectory(resultsDir.path() + s + "/", resultsDir.path() + s + "SortedAll/", [](const SceneScorecard& s) { return s.averageOffsetScore + s.scanGeometry + s.objectCount * 0.1; });
        }
    }
    cout << "done" << endl;
}

void ResultsMode::keyDown(UINT key)
{
    if (key == KEY_ESCAPE) selectedObjectIndex = -1;
    if (key == KEY_G) sortAllResults();

    if (selectedObjectIndex != -1 && selectedObjectIndex < resultScene.objects.size())
    {
        auto &o = resultScene.objects[selectedObjectIndex];
        float angle = 0.0f;
        if (key == KEY_LEFT)
        {
            angle = 1.0f;
        }
        if (key == KEY_RIGHT)
        {
            angle = -1.0f;
        }
        o.modelToWorld = mat4f::translation(o.worldBBox.getCenter()) * mat4f::rotationZ(angle) * mat4f::translation(-o.worldBBox.getCenter()) * o.modelToWorld;
        if (key == KEY_DELETE)
        {
            o.modelToWorld = mat4f::scale(0.0f);
        }
        if (key == KEY_S)
        {
            resultScene.save(synthParams().synthesisDir + "s.sss");
        }
    }
}

void ResultsMode::mouseDown(ml::MouseButtonType button)
{
    bool shift = (GetAsyncKeyState(VK_SHIFT) != 0);
    Scene &scene = state->database.scenes.getScene(state->curScene);

    float screenX = (float)app->input.mouse.pos.x / (app->window.getWidth() - 1.0f);
    float screenY = (float)app->input.mouse.pos.y / (app->window.getHeight() - 1.0f);
    ml::Rayf ray = state->camera.getScreenRay(screenX, screenY);

    if (shift && button == ml::MouseButtonLeft)
    {
        selectedObjectIndex = -1;

        ml::TriMeshRayAcceleratorf::Intersection intersection;
        UINT objectIndex;
        if (ml::TriMeshRayAcceleratorf::getFirstIntersection(ray, ml::util::toVecConstPtr(objectAccelerators), intersection, objectIndex))
        {
            selectedObjectIndex = objectIndex;
            cout << selectedObjectIndex << endl;
        }
    }
}

string ResultsMode::helpText() const
{
    string text;
    text += "Shift + left click: select object|";
    text += "G: sort all results";
    return text;
}

vector< pair<string, RGBColor> > ResultsMode::statusText() const
{
    vector< pair<string, RGBColor> > result;

    result.push_back(std::make_pair(string("Object count: ") + to_string(resultScene.scorecard.objectCount), RGBColor::White));
    result.push_back(std::make_pair(string("Scan geometry: ") + to_string(resultScene.scorecard.scanGeometry), RGBColor::White));
    result.push_back(std::make_pair(string("Worst gaze: ") + to_string(resultScene.scorecard.worstGaze), RGBColor::White));
    result.push_back(std::make_pair(string("Worst offset: ") + to_string(resultScene.scorecard.worstOffsetScore), RGBColor::White));
    result.push_back(std::make_pair(string("Average offset: ") + to_string(resultScene.scorecard.averageOffsetScore), RGBColor::White));

    return result;
}

void ResultsMode::renderResultsFolder(const string &dir) const
{
    cout << "rendering result folder: " << dir << endl;
    Directory directory(dir);
    for (const string &file : directory.filesWithSuffix(".sss"))
    {
        renderSceneD3D(directory.path() + file, directory.path() + ml::util::removeExtensions(file) + ".png");
    }
}

void ResultsMode::renderSceneD3D(const string &sceneFilename, const string &outputFilename) const
{
    cout << "rendering " << sceneFilename << endl;
    Scene scene;
    scene.load(sceneFilename, state->database);
    
    ml::Cameraf camera = state->camera;
    camera.updateAspectRatio(1.0f);

    ml::D3D11RenderTarget renderTarget;
    renderTarget.load(app->graphics, 1024, 1024);
    renderTarget.clear(vec4f(0.0f, 0.0f, 0.0f, 1.0f));
    renderTarget.bind();

    vector<vec3f> objectColors(scene.objects.size(), vec3f(1, 1, 1));
    state->renderer.renderObjects(state->assets, app->graphics.castD3D11(), camera.getCameraPerspective(), scene, objectColors, false);

	Bitmap bmp;
    renderTarget.captureColorBuffer(bmp);
    ml::LodePNG::save(bmp, outputFilename);

    app->graphics.castD3D11().bindRenderTarget();

}

void ResultsMode::renderPBRT(const string& path)
{
    /*vector< pair<string, string> > directories;
    
    //directories.push_back(make_pair(synthParams().FinalResultsDir + "final-matt-bedroom/", "LookAt 4.57941 1.3959 2.99138 3.86231 1.85715 2.46889 -0.439437 0.282654 0.852633"));
    //directories.push_back(make_pair(synthParams().FinalResultsDir + "final-michael-living/", "LookAt 5.75793 5.30054 2.82148 4.99858 5.11121 2.19897 -0.604011 -0.150598 0.782597"));
    //directories.push_back(make_pair(synthParams().FinalResultsDir + "final-gates381/", "LookAt 5.97678 4.2156 3.96178 5.34566 4.31839 3.19294 -0.758837 0.123584 0.639435"));
    //directories.push_back(make_pair(synthParams().FinalResultsDir + "final-angie-new/", "LookAt 4.4934 0.16482 4.54291 3.98062 0.647197 3.83273 -0.517276 0.486605 0.704013"));

    //string baseVideoDir = R"(V:\data\synthesis\videoFinal\dining2\)";
    string baseVideoDir = R"(V:\data\synthesis\videoFinal\gates386\)";
    for (const string &videoDir : Directory::enumerateDirectories(baseVideoDir))
    {
        //directories.push_back(make_pair(baseVideoDir + videoDir + "/", "LookAt 5.18822 0.580186 2.31713 4.64819 1.11552 1.66768 -0.461228 0.457221 0.760405"));
        directories.push_back(make_pair(baseVideoDir + videoDir + "/", "LookAt 4.78234 3.15413 2.43974 4.32709 3.82906 1.85903 -0.324725 0.481424 0.814115"));
    }

    PBRT::CopyPBRTFiles();

    int imgCount = 0;
    ml::ColorImageRGB averageImg;

    for (auto &directory : directories)
    {
        for (string filename : Directory::enumerateFiles(directory.first, ".sss"))
        {
            const string fullPath = directory.first + filename;
            const string resultFile = ml::util::replace(fullPath, ".sss", ".tiff");

            if (ml::util::fileExists(resultFile))
            {
                cout << "skipping " << resultFile << endl;

                ml::ColorImageRGB image;
                
                ml::FreeImageWrapper::loadImage(resultFile, image);
                if (averageImg.getWidth() == 0)
                    averageImg = image;
                else
                {
                    for (auto &p : image)
                    {
                        averageImg(p.x, p.y) += p.value;
                    }
                }

                ml::FreeImageWrapper::saveImage(synthParams().synthesisDir + ml::util::zeroPad(imgCount, 4) + ".png", image);

                imgCount++;
            }
            else
            {
                cout << "rendering " << resultFile << endl;
                Scene scene;
                scene.load(fullPath, state->database);

                PBRT::PBRTPreamble(state->camera, synthParams().PBRTResolution, false, directory.second);

                vector<vec3f> objectColors(resultScene.objects.size(), vec3f(1, 1, 1));
                state->renderer.renderPBRTObjects(state->assets, app->graphics, PBRT::PBRTFilename(), state->camera, scene, objectColors);

                PBRT::PBRTFinalize();

                ml::util::copyFile(synthParams().PBRTDir + "scene.tiff", resultFile);
            }
        }
    }

    for (auto &p : averageImg)
        p.value /= (float)imgCount;

    ml::FreeImageWrapper::saveImage(synthParams().synthesisDir + "average.png", averageImg);*/

    PBRT::CopyPBRTFiles();
    PBRT::PBRTPreamble(state->camera, synthParams().PBRTResolution);

    vector<vec3f> objectColors(resultScene.objects.size(), vec3f(1, 1, 1));
    state->renderer.renderPBRTObjects(state->assets, app->graphics, PBRT::PBRTFilename(), state->camera, resultScene, objectColors);

    PBRT::PBRTFinalize();

    ml::util::copyFile(synthParams().PBRTDir + "scene.tiff", ml::util::replace(resultScene.sourceFilename, ".sss", ".tiff"));

    // renderPOVRay here
    //POVRay::POVRayPreamble(state->camera, 800);
    //state->renderer.renderPOVRayObjects(state->assets, app->graphics, POVRay::POVRayMainFile(), state->camera, resultScene, objectColors);
    //POVRay::POVRayFinalize();
}
