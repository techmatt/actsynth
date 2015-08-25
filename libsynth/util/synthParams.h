
struct SynthParams
{
    SynthParams()
    {
        memset(this, 0, sizeof(SynthParams));
    }
    void load(const ml::ParameterFile &params)
    {
        params.readParameter("synthesisDir", synthesisDir);
        params.readParameter("scanName", scanName);
        params.readParameter("interactionMapDir", interactionMapDir);
        params.readParameter("PBRTDir", PBRTDir);
        params.readParameter("FinalResultsDir", FinalResultsDir);
        params.readParameter("synthRenderingOutputDir", synthRenderingOutputDir);
        //params.readParameter("sceneList", sceneList);

        params.readParameter("columnDimension", columnDimension);

        params.readParameter("keepTopXCategories", keepTopXCategories);
        params.readParameter("samplesPerCategory", samplesPerCategory);
        params.readParameter("maxSampleCount", maxSampleCount);

        params.readParameter("PBRTSamples", PBRTSamples);
        params.readParameter("PBRTResolution", PBRTResolution);

        params.readParameter("rotationsPerLocation", rotationsPerLocation);

        params.readParameter("agentHeightSitting", agentHeightSitting);
        params.readParameter("agentHeightStanding", agentHeightStanding);

        params.readParameter("forceConsistentCategory", forceConsistentCategory);

        params.readParameter("columnRepresentationScene", columnRepresentationScene);

        params.readParameter("architectureScale", architectureScale);

        params.readParameter("agentSupportFirst", agentSupportFirst);

        params.readParameter("minTotalPixelCount", minTotalPixelCount);
        params.readParameter("maxOcclusionPercentage", maxOcclusionPercentage);
        params.readParameter("minScreenCoverage", minScreenCoverage);
        params.readParameter("maxScreenCoverage", maxScreenCoverage);
        params.readParameter("renderFieldOfView", renderFieldOfView);

        params.readParameter("maxCategoryInstanceCount", maxCategoryInstanceCount);

        params.readParameter("haoSynthesisMode", haoSynthesisMode);
        params.readParameter("skipTextureLoading", skipTextureLoading);
        params.readParameter("fisherSynthesisMode", fisherSynthesisMode);
    }

    string synthesisDir;
    string scanName;
    string interactionMapDir;
    string PBRTDir;
    string FinalResultsDir;
    string synthRenderingOutputDir;

    string scanAgentSuffix;

    float columnDimension;

    UINT keepTopXCategories;
    UINT samplesPerCategory;
    UINT maxSampleCount;

    UINT PBRTSamples;

    UINT PBRTResolution;

    UINT rotationsPerLocation;

    bool forceConsistentCategory;

    float agentHeightSitting;
    float agentHeightStanding;

    string columnRepresentationScene;

    float architectureScale;

    bool agentSupportFirst;

    UINT minTotalPixelCount;
    float maxOcclusionPercentage;
    float minScreenCoverage;
    float maxScreenCoverage;
    float renderFieldOfView;

    UINT maxCategoryInstanceCount;
    bool haoSynthesisMode;
    bool skipTextureLoading;
    bool fisherSynthesisMode;
};

extern SynthParams g_synthParams;

inline const SynthParams& synthParams()
{
    return g_synthParams;
}

inline SynthParams& mutableSynthParams()
{
  return g_synthParams;
}

inline void initSynthParams(const ml::ParameterFile &params)
{
    g_synthParams.load(params);
}
