//
// Created by Raymond on 2/12/2025.
//

#pragma once

#include <libraries.hh>

class Buffer {
public:
	Buffer(size_t s) : size(s) {
		this->ptr = malloc(size);
	};
    ~Buffer() {
      	if (this->ptr != nullptr)
      		free(this->ptr);
    };
    template <typename T>
    T get() {
		return static_cast<T>(this->ptr);
    };
	size_t size;
private:
	void* ptr = nullptr;
};

FFLCharModel getMii();

extern "C" {
	EMSCRIPTEN_KEEPALIVE void* init(int size);
	EMSCRIPTEN_KEEPALIVE bool mii();

	EMSCRIPTEN_KEEPALIVE void* renderFaceTextures(FFLExpression);
}