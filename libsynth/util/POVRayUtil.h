
namespace POVRay
{
  string POVRayBaseDir();
  string POVRayMainFile();
  void POVRayPreamble(const Cameraf &camera, UINT width, UINT height=0);
  void POVRayFinalize();
}
