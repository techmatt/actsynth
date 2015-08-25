
#include "libsynth.h"

void Database::loadAll()
{
    string synthesisDir = synthParams().synthesisDir;
    string scanDir = synthesisDir + "scans/";
    string scenesCSVFile = synthesisDir + "allScenes.csv";
    //string sceneListFile = synthesisDir + "sceneList" + synthParams().sceneList + ".txt";
    string sceneListFile = synthesisDir + "sceneListAll.txt";
    string modelsFile = synthesisDir + "modelsSimple.tsv";
    string categoryOverrideFile = synthesisDir + "categoryOverride.tsv";
    string modelBlacklistFile = synthesisDir + "modelBlacklist.txt";
    string modelScaleFile = "V:/data/models/data-tables/modelScales.csv";
    string interactionMapPath = synthesisDir + synthParams().interactionMapDir;
    string agentAnnotationPath = synthesisDir + "agentAnnotations/";

    if (!ml::util::fileExists(scenesCSVFile))
    {
        cout << "Required file not found, map V drive: " << scenesCSVFile << endl;
        return;
    }

#ifdef _DEBUG
    sceneListFile = synthesisDir + "sceneListDebug.txt";
#endif

    scenes.load(scenesCSVFile, sceneListFile, synthesisDir, -1);
    
    models.loadSimple(modelsFile);
    models.overrideCategories(categoryOverrideFile);
    models.loadBlacklist(modelBlacklistFile);

    vector<string> haoCategories = { "Table", "Chair" };

    if (synthParams().haoSynthesisMode)
    {
        for (const string &s : haoCategories)
        {
            models.loadHaoCategory(s);
            models.computeHaoBBoxes(s);
        }
    }

    interactionMaps.load(models, interactionMapPath);
    
    scenes.updateModelReferences(models);
    scenes.loadObjectData();
    scenes.loadAgentAnnotations(agentAnnotationPath);

    scans.load(scanDir);

    categories.loadFromScenes(scenes);
    //categories.rescaleFromModelScaleFile(modelScaleFile);

    if (synthParams().haoSynthesisMode)
    {
        for (const string &s : haoCategories)
        {
            categories.makeHaoInstances(models, s);
        }
    }

    const string categoryListDir = synthesisDir + "categoryLists/";
    categories.loadCategoryPropertyList(categoryListDir + "categoryListHandOrientation.txt", [](Category &c) -> bool& { return c.isHandOrientation; } );
    categories.loadCategoryPropertyList(categoryListDir + "categoryListShared.txt", [](Category &c) -> bool& { return c.isShared; });

    // removes object support for scenes without planes...
    categories.loadCategoryPropertyList(categoryListDir + "categoryListObjectSupport.txt", [](Category &c) -> bool& { return c.isObjectSupport; });

    categories.loadCategoryPropertyList(categoryListDir + "categoryListAgentSupport.txt", [](Category &c) -> bool& { return c.isAgentSupport; });
    categories.loadCategoryPropertyList(categoryListDir + "categoryListTopSupport.txt", [](Category &c) -> bool& { return c.isTopSupport; });
    categories.loadCategoryPropertyList(categoryListDir + "categoryListBBoxCollision.txt", [](Category &c) -> bool& { return c.isBBoxCollision; });
    categories.loadCategoryPropertyList(categoryListDir + "categoryListLeftRight.txt", [](Category &c) -> bool& { return c.isLeftRight; });
    categories.loadCategoryPropertyList(categoryListDir + "categoryListNoSimplify.txt", [](Category &c) -> bool& { return c.isNoSimplify; });
    categories.loadCategoryPropertyList(categoryListDir + "categoryListDiversity.txt", [](Category &c) -> bool& { return c.isDiversity; });
    categories.loadCategoryPropertyList(categoryListDir + "categoryListNonEssential.txt", [](Category &c) -> bool& { return c.isNonEssential; });

    activities.loadFromScenes(scenes, categories, interactionMaps);
}

void Database::output(const string &baseDir) const
{
    ml::util::makeDirectory(baseDir);
    models.output(baseDir);
    scenes.output(baseDir);
    activities.output(baseDir);
    categories.output(baseDir);
    interactionMaps.output(models, baseDir);

    for (const auto &s : scenes.scenes)
    {
        const string sceneSaveDir = baseDir + "scenes/";
        ml::util::makeDirectory(sceneSaveDir);
        s.second.save(sceneSaveDir + s.first + ".sss");
    }
}
