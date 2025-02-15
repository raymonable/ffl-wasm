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
void generateMiiMeshes();

bool initializeGL();
void initializeDrawCallbacks();

struct RenderTextureExport {
	uint32_t width;
	uint32_t height;
	void* data;
};
struct MeshData {
	uint32_t vertexCount;
	uint32_t indexCount;

	void* indexData;

	void* positionData;
	void* uvData;
	void* normalData;
	void* tangentData;
	void* colorData;
};

extern "C" {
	EMSCRIPTEN_KEEPALIVE void* init(int size);
	EMSCRIPTEN_KEEPALIVE bool mii();

	EMSCRIPTEN_KEEPALIVE void drawFaceTexture(int expression);
	EMSCRIPTEN_KEEPALIVE void drawFaceline();
	EMSCRIPTEN_KEEPALIVE void drawXlu();

	EMSCRIPTEN_KEEPALIVE MeshData* getMesh(const char* object);

	EMSCRIPTEN_KEEPALIVE int getRenderTextureHandle(const char* target);
	EMSCRIPTEN_KEEPALIVE RenderTextureExport* exportTexture(const char* target);
}