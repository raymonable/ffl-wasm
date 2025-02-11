//
// Created by Raymond on 2/10/2025.
//
#pragma once

#include <libraries.hh>

struct GlobalInitializerData {
    int size = 0;
    void* buffer = nullptr;
};

extern "C" {
    EMSCRIPTEN_KEEPALIVE void* initBuffer(int size);
    EMSCRIPTEN_KEEPALIVE bool init();

    EMSCRIPTEN_KEEPALIVE bool test();
}