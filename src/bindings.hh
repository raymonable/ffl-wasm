//
// Created by Raymond on 2/12/2025.
//

#pragma once

#include <libraries.hh>

class Buffer {
public:
	Buffer(int s) : size(s) {
		this->ptr = malloc(size);
		memset(this->ptr, 0, size);
	};
    ~Buffer() {
      	if (this->ptr != nullptr)
      		free(this->ptr);
    };
    template <typename T>
    T get() {
		return static_cast<T>(this->ptr);
    };
	int size = 0;
private:
	void* ptr = nullptr;
};

FFLCharModel* getMii();
bool initializeGL();

extern "C" {
	EMSCRIPTEN_KEEPALIVE void* init(int size);
	EMSCRIPTEN_KEEPALIVE bool mii();

	EMSCRIPTEN_KEEPALIVE void* generateTextures(int expression);
}