cmake_minimum_required(VERSION 3.30)
project(ffl_wasm)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

# rio

add_compile_definitions(RIO_RELEASE FFL_USE_TEXTURE_CALLBACK)

# TODO: replace these absolute paths with non-absolute paths (probably just make a lib directory)

include_directories(src "C:/Users/Raymond/Documents/ffl/include" "C:/Users/Raymond/Documents/rio/include")
add_library( ffl STATIC IMPORTED )

add_executable(ffl_wasm
        src/app.cc)

# zlib (installed with vcpkg)

find_package(ZLIB REQUIRED)
target_link_libraries(ffl_wasm ffl ZLIB::ZLIB)

set_target_properties(ffl PROPERTIES IMPORTED_LOCATION "C:/Users/Raymond/Documents/ffl/libffl.a")
set_target_properties(ffl_wasm PROPERTIES LINK_FLAGS    "-O0 -s WASM=1 -sEXPORTED_RUNTIME_METHODS=ccall --no-entry")
