
//
// libsynth.h
//
// Standard header for libsynth project
//

#ifdef _DEBUG
#define MLIB_ERROR_CHECK
#define MLIB_BOUNDS_CHECK
//#define _IDL0
#endif // _DEBUG

#include <mLibCore.h>

using ml::vec3ui;
using ml::vec2f;
using ml::vec3f;
using ml::vec4f;
using ml::mat4f;
using ml::bbox2f;
using ml::bbox3f;
using OBBf = ml::OrientedBoundingBox3f;
using ml::Rayf;
using ml::TriMeshf;
using ml::Trianglef;
using ml::Polygonf;
using ml::Directory;
using ml::RGBColor;
using ml::Cameraf;
using ml::TriMeshRayAcceleratorf;
using ml::TriMeshAcceleratorBVHf;
using ml::Timer;
using ml::ParameterFile;
using ml::Grid2;
using ml::Grid3;
using ml::SparseMatrixf;
using Bitmap = ml::ColorImageR8G8B8A8;
using ml::ColorImageR32;
using std::vector;
using std::set;
using std::map;
using std::string;
using std::pair;
using std::cout;
using std::endl;
using std::ofstream;
using std::ifstream;
using std::to_string;
using std::function;
using std::make_pair;

#include "./util/util.h"
#include "./util/PBRTUtil.h"
#include "./util/POVRayUtil.h"
#include "./util/synthParams.h"
#include "./util/HTMLExporter.h"

#include "./components/modelInstance.h"

#include "./util/objectGroupDistribution.h"
#include "./util/agentGroupDistribution.h"
#include "./util/categoryDistribution.h"
#include "./util/distribution.h"

#include "./segmentation/meshSegment.h"
#include "./segmentation/segmentor.h"
#include "./segmentation/salientSegmentor.h"

#include "./components/model.h"
#include "./components/category.h"
#include "./components/agent.h"
#include "./components/scene.h"
#include "./components/interactionMap.h"
#include "./components/activity.h"
#include "./components/activityCluster.h"
#include "./components/scan.h"

#include "./databases/categoryDatabase.h"
#include "./databases/modelDatabase.h"
#include "./databases/sceneDatabase.h"
#include "./databases/activityDatabase.h"
#include "./databases/interactionMapDatabase.h"
#include "./databases/scanDatabase.h"
#include "./databases/database.h"

#include "./segmentation/scanLocator.h"

#include "./synth/synthUtil.h"
#include "./synth/columnRepresentation.h"
#include "./synth/sceneTemplate.h"
#include "./synth/synthesizerScan.h"