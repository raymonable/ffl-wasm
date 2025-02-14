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
GLuint getProgram();

bool initializeGL();
void initializeDrawCallbacks();

struct RenderTextureExport {
	uint32_t width;
	uint32_t height;
	void* data;
};

extern "C" {
	EMSCRIPTEN_KEEPALIVE void* init(int size);
	EMSCRIPTEN_KEEPALIVE bool mii();

	EMSCRIPTEN_KEEPALIVE void drawFaceTexture(int expression);
	EMSCRIPTEN_KEEPALIVE void drawFaceline();
	EMSCRIPTEN_KEEPALIVE void drawXlu();

	EMSCRIPTEN_KEEPALIVE RenderTextureExport* exportTexture(const char* target);
}