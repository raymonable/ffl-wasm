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
// I considered using RIO's Color4f but I figured this would be the easiest to read from HEAP
struct RGB {
	float r;
	float g;
	float b;
};
#pragma pack(1)
struct MeshData {
	uint32_t vertexCount;
	uint32_t indexCount;
	RGB rgb;

	unsigned char* positionData;
	unsigned char* texCoordData;
	unsigned char* normalData;
	unsigned char* tangentData;

	unsigned char* indexData;
};
#pragma pack()

extern "C" {
	EMSCRIPTEN_KEEPALIVE void* init(int size);
	EMSCRIPTEN_KEEPALIVE bool mii(bool);

	EMSCRIPTEN_KEEPALIVE void drawFaceTexture(int expression);
	EMSCRIPTEN_KEEPALIVE void drawFaceline();
	EMSCRIPTEN_KEEPALIVE void drawXlu();

	EMSCRIPTEN_KEEPALIVE MeshData* getMesh(const char* object);

	EMSCRIPTEN_KEEPALIVE int getRenderTextureHandle(const char* target);
	EMSCRIPTEN_KEEPALIVE RenderTextureExport* exportTexture(const char* target);
}