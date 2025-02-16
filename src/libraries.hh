//
// Created by Raymond on 2/10/2025.
//

#ifndef LIBRARIES_HH
#define LIBRARIES_HH

#include <string>
#include <cstring>
#include <map>

#include <malloc.h>

#include <emscripten.h>
#include <emscripten/heap.h>

#define GL_GLEXT_PROTOTYPES
#include <GLES3/gl3.h>

#include <nn/ffl.h>
#include <nn/ffl/FFLTextureCallback.h>
#include <nn/ffl/FFLiFacelineTexture.h>
#include <nn/ffl/FFLiRawMask.h>

#include <rio.h>

#endif //LIBRARIES_HH
