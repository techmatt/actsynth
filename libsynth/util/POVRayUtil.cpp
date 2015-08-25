
#include "libsynth.h"

#include <boost/filesystem.hpp>

namespace POVRay
{
    string POVRayBaseDir()
    {
        // let's just use the pbrt dir for now
        return synthParams().PBRTDir;
    }

    string POVRayMainFile()
    {
        return POVRayBaseDir() + "/scene.pov";
    }

    void POVRayPreamble(const Cameraf &camera, UINT width, UINT height)
    {
        if (height == 0)
            height = width;

        boost::filesystem::path path(POVRayBaseDir());
        auto it = path.end(); --it; --it;
        string output_filename = it->string(); --it;
        output_filename = it->string() + "_" + output_filename;

        ofstream file_ini(POVRayBaseDir() + "/scene.ini");
        file_ini << "Input_File_Name = \"scene.pov\"\n"
            << "Output_File_Name = " << output_filename << "\n\n"
            << "Antialias = On\nAntialias_Threshold = 0.3\nAntialias_Depth = 2\n\n"
            << "Output_Alpha = On\n\n"
            << "Width = " << width << "\nHeight = " << height << "\n\n"
            << "Initial_Frame = 1\nFinal_Frame= 1\n\n";


        ofstream file_pov(POVRayMainFile());
        file_pov << "#version 3.7;\n\n";
        file_pov << "global_settings { assumed_gamma 1.0 }\n\n";

        file_pov << "#declare ambient_value = 0.5;\n";
        file_pov << "#declare diffuse_value = 0.5;\n";
        file_pov << "#declare phong_value = 0.5;\n";
        file_pov << "background {rgbft<0.0, 0.0, 0.0, 1.0, 1.0>}\n";

        vec3f eye = camera.getEye();
        vec3f up = camera.getUp();
        vec3f center = camera.getEye() + camera.getLook();
        file_pov << "\n\n//camera\n"
            << "camera\n{\n"
            << "  perspective\n"
            << "  right -x*image_width/image_height\n"
            << "  location <" << eye[0] << "," << eye[1] << "," << eye[2] << ">\n"
            << "  sky <" << up[0] << "," << up[1] << "," << up[2] << ">\n"
            << "  look_at <" << center[0] << "," << center[1] << "," << center[2] << ">\n"
            << "  angle " << camera.getFoV() << "\n"
            << "}\n";

        file_pov << "\n\n#declare scene_radius = 3;\n";
        std::string fade_distance("2*scene_radius");
        std::string area_light_size("0.5*scene_radius");
        std::string eye_offset("0.5*scene_radius");
        file_pov << "\n\n//light\n"
            << "#declare light_value = 1.0;\n"
            << "light_source\n{\n"
            << "  <" << eye[0] << "," << eye[1] << "+" << eye_offset << "," << eye[2] << ">\n"
            << "  color rgb <light_value, light_value, light_value>\n"
            << "  fade_distance " << fade_distance << " fade_power 2\n"
            << "  area_light x*" << area_light_size << ", z*" << area_light_size << ", 8, 8 jitter orient circular adaptive 0\n"
            << ""
            << "}\n";

        file_pov.close();
    }

    void POVRayFinalize()
    {
        cout << "POVRay export completed, rendering..." << endl;

        //ml::util::runSystemCommand("pvengine.exe " + POVRayBaseDir() + "/scene.ini");
    }
}