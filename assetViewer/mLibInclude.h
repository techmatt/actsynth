#ifndef ASSETVIEWER_MLIBINCLUDE_H_
#define ASSETVIEWER_MLIBINCLUDE_H_

//
// mLib config options
//

#ifdef _DEBUG
#define MLIB_ERROR_CHECK
#define MLIB_BOUNDS_CHECK
//#define _IDL0
#endif // _DEBUG

//
// mLib includes
//

#include <mLibCore.h>
//#include <mLibANN.h>
#include <mLibLodePNG.h>
#include <mLibD3D11.h>
#include <mLibD3D11Font.h>
#include <mLibDepthCamera.h>
#include <mLibFreeImage.h>

#endif  // ASSETVIEWER_MLIBINCLUDE_H_
