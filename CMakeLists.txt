cmake_minimum_required(VERSION 3.20)
project(ffl_wasm)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

# such a pain

add_compile_definitions(RIO_DEBUG RIO_GLES RIO_NO_GL_LOADER)

include_directories(src lib/ffl/include lib/rio/include)
add_executable(ffl_wasm
        src/mii.cc
        src/face.cc
)

set(FFL_MODE "opengl-es2")
set(FFL_BUILD_STATIC ON)
set(FFL_BUILD_SHARED OFF)

set_target_properties(ffl_wasm PROPERTIES LINK_FLAGS    "-O0 -s WASM=1 -s USE_WEBGL2=0 -sFULL_ES2 -sMAX_WEBGL_VERSION=2 -s TOTAL_MEMORY=64MB -sEXPORTED_RUNTIME_METHODS=ccall,GL,UTF8ToString --no-entry")

add_subdirectory(lib/ffl)
target_link_libraries(ffl_wasm PRIVATE ffl-opengl-es2)