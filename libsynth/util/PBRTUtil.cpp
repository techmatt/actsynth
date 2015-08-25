
#include "libsynth.h"

#include <boost/filesystem.hpp>

namespace PBRT
{
    string PBRTFilename()
    {
        return synthParams().PBRTDir + "scene.pbrt";
    }

    void CopyPBRTFiles()
    {
        string path_light_maps_source("V:\\data\\synthesis\\pbrt\\lightMaps");
        string path_light_maps_target(synthParams().PBRTDir + "/lightMaps");
        if (!boost::filesystem::exists(path_light_maps_target))
            boost::filesystem::create_directory(path_light_maps_target);
        for (boost::filesystem::directory_iterator file(path_light_maps_source); file != boost::filesystem::directory_iterator(); ++file)
        {
            boost::filesystem::path current(file->path());
            if (boost::filesystem::is_regular_file(current))
                boost::filesystem::copy_file(current, path_light_maps_target / current.filename(), boost::filesystem::copy_option::overwrite_if_exists);
        }
    }

    void PBRTPreamble(const Cameraf &camera, UINT resolution, bool externalCamera, const string &externalLookAt)
    {
        ofstream file(PBRTFilename());

        //
        //http://www-users.cselabs.umn.edu/classes/Spring-2012/csci5108/fileformat.pdf
        //

        file << "Film \"image\"" << endl;
        file << "  \"integer xresolution\" [" << resolution << "] \"integer yresolution\" [" << resolution << "]" << endl;
        
        file << "  \"string filename\" [\"./scene.exr\"]" << endl;

        file << "Scale -1 1 1" << endl;

        if (externalCamera)
          file << "Include \"./camera.pbrt\"" << endl;
        else if (externalLookAt.size() > 0)
            file << externalLookAt << endl;
        else
          file << "LookAt " << camera.getEye() << ' ' << (camera.getEye() + camera.getLook()) << ' ' << camera.getUp() << endl;

        file << "Camera \"perspective\" \"float fov\" [60]" << endl;

        file << "SurfaceIntegrator \"whitted\"" << endl;

        file << " Sampler \"lowdiscrepancy\" \"integer pixelsamples\" [" << synthParams().PBRTSamples << "]" << endl;

        file << "WorldBegin" << endl;

        //
        // nsamples for an infinite light source does nothing. Light sampling is deterministic given the u-v sample values provided by the Sampler.
        //

        //file << "LightSource \"infinite\" \"string mapname\" [\"lightMaps/pisa_latlong.exr\"]" << endl;
        //file << "  \"integer nsamples\" [1]" << endl;

        file << "AttributeBegin" << endl;
        file << "Rotate 110 0 0 1" << endl;
        file << "LightSource \"infinite\" \"string mapname\" [\"lightMaps/skylight-surreal.exr\"]" << endl;
        file << "  \"integer nsamples\" [1]" << endl;
        file << "AttributeEnd" << endl;

        file << "AttributeBegin" << endl;
        file << "Rotate 110 0 0 1" << endl;
        file << "LightSource \"infinite\" \"string mapname\" [\"lightMaps/skylight-surreal.exr\"]" << endl;
        file << "  \"integer nsamples\" [1]" << endl;
        file << "AttributeEnd" << endl;

        file << "AttributeBegin" << endl;
        file << "Rotate 180 1 0 0" << endl;
        file << "LightSource \"infinite\" \"string mapname\" [\"lightMaps/skylight-blue.exr\"]" << endl;
        file << "  \"integer nsamples\" [1]" << endl;
        file << "AttributeEnd" << endl;

        file << "AttributeBegin" << endl;
        file << "Rotate 190 0 0 1" << endl;
        file << "LightSource \"infinite\" \"string mapname\" [\"lightMaps/skylight-pollution.exr\"]" << endl;
        file << "  \"integer nsamples\" [1]" << endl;
        file << "AttributeEnd" << endl;
    }

    void PBRTFinalize()
    {
        ofstream file;
        file.open(PBRTFilename(), std::ofstream::out | std::ofstream::app);
        file << "WorldEnd" << endl;

        cout << "PBRT export completed, rendering..." << endl;

        boost::filesystem::path cwd_backup = boost::filesystem::current_path();
        boost::filesystem::current_path(synthParams().PBRTDir);

        ml::util::runSystemCommand(synthParams().PBRTDir + "pbrt \"" + synthParams().PBRTDir + "scene.pbrt\" --ncores 7");
        ml::util::runSystemCommand(synthParams().PBRTDir + "exrtotiff \"" + synthParams().PBRTDir + "scene.exr\" \"" + synthParams().PBRTDir + "scene.tiff\"");
        ml::util::runSystemCommand(synthParams().PBRTDir + "scene.tiff");

        boost::filesystem::current_path(cwd_backup);
    }
}