
#include "libsynth.h"

bool ActivityDatabase::hasActivityCategoryDistribution(const string &activity, const string &category) const
{
    if (activities.count(activity) == 0)
        return false;
    if (activities.at(activity).categoryDistributions.count(category) == 0)
        return false;
    return true;
}

const ActivityCategoryDistributions& ActivityDatabase::getActivityCategoryDistribution(const string &activity, const string &category) const
{
    if (!hasActivityCategoryDistribution(activity, category))
    {
        cout << "activity-category not found: " << activity << "-" << category << endl;
    }
    return activities.at(activity).categoryDistributions.at(category);
}

void ActivityDatabase::loadFromScenes(const SceneDatabase &scenes, const CategoryDatabase &categories, const InteractionMapDatabase &interactionMaps)
{
    for (const Agent *a : scenes.agents())
    {
        //if (a->isActivityCluster())
        //{
        //    activityClusters[a->activity].id = a->activity;
        //}
        //else
        //{
        activities[a->activity].id = a->activity;
        //}
    }

    for (auto &activity : activities)
    {
        loadActivity(scenes, categories, interactionMaps, activity.second);
    }
}

CategoryDistribution ActivityDatabase::makeSupportDistribution(const SceneDatabase &scenes, const string &activityName)
{
    vector<string> supportInstances;
    
    for (const Agent *agent : scenes.agents(activityName))
        supportInstances.push_back(agent->supportObject().categoryName());

    return CategoryDistribution(supportInstances);
}

ObjectGroupDistribution* ActivityDatabase::makeObjectGroupDistribution(const SceneDatabase &scenes, const CategoryDatabase &categories, const string &activityName)
{
    vector< vector<DisembodiedObject> > objectGroupInstances;
    
    for (const Agent *agent : scenes.agents(activityName))
    {
        vector<DisembodiedObject> objectGroup;
        for (const ObjectInstance *objectInstance : agent->boundObjects())
            objectGroup.push_back(objectInstance->object);
        objectGroupInstances.push_back(std::move(objectGroup));
    }

    return new ObjectGroupDistributionIndependent(objectGroupInstances, categories);
}

void ActivityDatabase::makeActivityCategoryDistributions(const SceneDatabase &scenes, const CategoryDatabase &categories, const InteractionMapDatabase &interactionMaps, const string &activityName, ActivityCategoryDistributions &dist)
{
    for (const string &distributionType : ActivityCategoryDistributions::distributionList())
    {
        vector<vec3f> samples;
        vector<string> sampleDescriptions;
        for (const Agent *agent : scenes.agents(activityName))
        {
            auto objects = agent->boundObjects();
            objects.push_back(&agent->supportObject());
            for (auto instance : objects)
                if (instance->categoryName() == dist.category)
                {
                    string description = agent->scene->id + "-" + instance->categoryName() + "-" + instance->object.modelId;
                    if(interactionMaps.hasInteractionMapEntry(instance->object.modelId, distributionType))
                    {
                        const auto &centroids = interactionMaps.getInteractionMapEntry(instance->object.modelId, distributionType).centroids;
                        for (const InteractionMapEntry::Centroid &centroid : centroids)
                        {
                            vec3f worldPos = instance->modelToWorld * centroid.pos;
                            vec3f agentCoord = agent->worldToAgentBasis(worldPos);
                            samples.push_back(agentCoord);
                            sampleDescriptions.push_back(description);
                        }
                    }

                    if (distributionType == "center")
                    {
                        vec3f worldPos = instance->worldBBox.getCenter();
                        vec3f agentCoord = agent->worldToAgentBasis(worldPos);
                        samples.push_back(agentCoord);
                        sampleDescriptions.push_back(description);
                    }
                
                    if (distributionType == "base")
                    {
                        vec3f worldPos = instance->contactPosition;
                        vec3f agentCoord = agent->worldToAgentBasis(worldPos);
                        samples.push_back(agentCoord);
                        sampleDescriptions.push_back(description);
                    }
                }
        }

        if (samples.size() > 0)
            dist.getDistribution(distributionType) = new Distribution3DMemorizer(samples, sampleDescriptions);
    }
}

void ActivityDatabase::loadActivity(const SceneDatabase &scenes, const CategoryDatabase &categories, const InteractionMapDatabase &interactionMaps, Activity &activity)
{
    activity.supportDistribution = makeSupportDistribution(scenes, activity.id);
    activity.objectGroupDistribution = makeObjectGroupDistribution(scenes, categories, activity.id);

    for (const Agent *agent : scenes.agents(activity.id))
    {
        std::set<string> categoryNames;

        categoryNames.insert(agent->supportObject().categoryName());
        for (const ObjectInstance *objectInstance : agent->boundObjects())
            categoryNames.insert(objectInstance->categoryName());

        for (const string &category : categoryNames)
            if (activity.categoryDistributions.count(category) == 0)
                activity.categoryDistributions[category] = ActivityCategoryDistributions(category);
    }

    for (auto &dist : activity.categoryDistributions)
        makeActivityCategoryDistributions(scenes, categories, interactionMaps, activity.id, dist.second);

    //
    // TODO: activity blockers should be voxelized from a set of skeletons and then loaded from a file. For now I just use a generic blocker for all activities.
    //
    if (activity.id == "use desktop PC")
    {
        vec3f center(0.5f, 0.0f, 0.2f);
        vec3f variance(0.15f, 0.15f, 0.15f);
        bbox3f defaultBlocker = bbox3f(center - variance, center + variance);
        activity.agentFreeSpaceBlockers.push_back(defaultBlocker);
    }
}

void ActivityDatabase::output(const string &baseDir) const
{
    ml::util::makeDirectory(baseDir + "activities/");
    for (const auto &p : activities)
    {
        p.second.output(baseDir + "activities/" + p.first + ".html");
    }

    /*for (const auto &p : activityClusters)
    {
        p.second.output(baseDir + "activities/" + p.first + ".html");
    }*/
}
