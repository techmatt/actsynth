
namespace PBRT
{
    string PBRTFilename();
    void CopyPBRTFiles();
    void PBRTPreamble(const Cameraf &camera, UINT resolution, bool externalCamera=false, const string &externalLookAt = "");
    void PBRTFinalize();
}
