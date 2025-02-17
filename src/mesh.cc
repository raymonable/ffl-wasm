//
// Created by Raymond on 2/14/2025.
//

#include <bindings.hh>

FFLModulateType meshType;
MeshDataBuffers meshDatas[FFL_MODULATE_TYPE_SHAPE_MAX];

void unpackNormals(uint32_t packed, float& x, float& y, float& z)
{
    // TODO: upgrade to C++ casts
    // Extract each 10-bit component, sign-extended
    int32_t nx = ((int32_t)(packed << 22)) >> 22;
    int32_t ny = ((int32_t)(packed << 12)) >> 22;
    int32_t nz = ((int32_t)(packed << 2))  >> 22;
    // int32_t nw = (packed >> 30) & 0x03; // The 2-bit w component, not used for normals

    // Convert to float in range [-1, 1]
    x = nx / 511.0f;
    y = ny / 511.0f;
    z = nz / 511.0f;
}

EMSCRIPTEN_KEEPALIVE
void shaderDrawCallback(void* pObj, const FFLDrawParam* drawParam) {
    if (drawParam->modulateParam.type >= FFL_MODULATE_TYPE_SHAPE_MAX || drawParam->primitiveParam.indexCount < 1) return;

#ifdef RIO_DEBUG
    printf("!!!! Mesh Type:%s\n", std::to_string(drawParam->modulateParam.type).c_str());
#endif

    meshDatas[drawParam->modulateParam.type].meshData = new MeshData{
        .rgb = {
            .r = drawParam->modulateParam.pColorR->r,
            .g = drawParam->modulateParam.pColorR->g,
            .b = drawParam->modulateParam.pColorR->b,
        }
    };
    const auto meshData = meshDatas[drawParam->modulateParam.type];

    for (int type = 0; type < FFL_ATTRIBUTE_BUFFER_TYPE_MAX; type++) {
        const FFLAttributeBuffer* buffer = &drawParam->attributeBufferParam.attributeBuffers[type];
        void* ptr = buffer->ptr;

        const auto stride = static_cast<int>(buffer->stride);
        const auto size = static_cast<int>(buffer->size);

#ifdef RIO_DEBUG
        printf("Stride: %s Size: %s Buffer Type:%s\n", std::to_string(stride).c_str(), std::to_string(size).c_str(), std::to_string(type).c_str());
        printf("@ ptr: %p type: %d\n ", ptr, type);
#endif

        auto typeCast = static_cast<FFLAttributeBufferType>(type);
        /*
        float** bufferPtr = nullptr;
        switch (typeCast) {
            case FFL_ATTRIBUTE_BUFFER_TYPE_POSITION:
                bufferPtr = &meshData->positionData; break;
            case FFL_ATTRIBUTE_BUFFER_TYPE_NORMAL:
                bufferPtr = &meshData->normalData; break;
            case FFL_ATTRIBUTE_BUFFER_TYPE_TEXCOORD:
                bufferPtr = &meshData->texCoordData; break;
            case FFL_ATTRIBUTE_BUFFER_TYPE_TANGENT:
                bufferPtr = &meshData->tangentData; break;
            default: break;
        }
         **/
        if (ptr != nullptr) {
            switch (typeCast) {
                case FFL_ATTRIBUTE_BUFFER_TYPE_POSITION:
                    case FFL_ATTRIBUTE_BUFFER_TYPE_TEXCOORD: {
                        meshDatas[drawParam->modulateParam.type].attributeBuffers[typeCast] = new Buffer(size);
                        auto bufferPtr = meshDatas[drawParam->modulateParam.type].attributeBuffers[typeCast]->get<float*>();
                        memcpy(bufferPtr, ptr, size);
                        break;
                }
                case FFL_ATTRIBUTE_BUFFER_TYPE_NORMAL: {
                    meshDatas[drawParam->modulateParam.type].attributeBuffers[typeCast] = new Buffer(size * sizeof(float));
                    auto bufferPtr = meshDatas[drawParam->modulateParam.type].attributeBuffers[typeCast]->get<float*>();
                    memset(bufferPtr, '\0', size * sizeof(float));
                    // Probably could be optimized, but this will do for now
                    for (size_t i = 0; i < size; i++) {
                        size_t ptrIdx = i * sizeof(float);
                        uint32_t packedNormal = reinterpret_cast<uint32_t*>(ptr)[i];
                        unpackNormals(packedNormal,
                            reinterpret_cast<float*>(bufferPtr)[ptrIdx],
                            reinterpret_cast<float*>(bufferPtr)[ptrIdx + 1],
                            reinterpret_cast<float*>(bufferPtr)[ptrIdx + 2]
                        );
                    };
                    break;
                }
                case FFL_ATTRIBUTE_BUFFER_TYPE_TANGENT: {
                    meshDatas[drawParam->modulateParam.type].attributeBuffers[typeCast] = new Buffer(size * sizeof(float));
                    auto floatPtr = meshDatas[drawParam->modulateParam.type].attributeBuffers[typeCast]->get<float*>();
                    memset(floatPtr, '\0', size * sizeof(float));
                    for (size_t i = 0; i < size; i++) {
                        // Reimplementation of FFL-Testing's to be more memory-efficient (probably)
                        size_t ptrIdx = i * sizeof(float);
                        int8_t* tangent = reinterpret_cast<int8_t*>(ptr) + i;
                        if (tangent[3] == 0)
                            continue;
                        float magnitude = 0;
                        for (size_t j = 0; j < 4; j++) {
                            floatPtr[ptrIdx + j] = static_cast<float>(tangent[j]) / 127.0f;
                            magnitude += floatPtr[ptrIdx + j] * floatPtr[ptrIdx + j];
                        }
                        magnitude = sqrtf(magnitude);
                        if (magnitude != 0)
                            for (size_t j = 0; j < 3; j++)
                                floatPtr[ptrIdx + j] /= magnitude;
                    };
                    break;
                };
                default: break;
            }
        }
        if (meshDatas[drawParam->modulateParam.type].attributeBuffers[typeCast] != nullptr)
            meshDatas[drawParam->modulateParam.type].meshData->attributeData[typeCast] = meshDatas[drawParam->modulateParam.type].attributeBuffers[typeCast]->get<float*>();
    }

    meshDatas[drawParam->modulateParam.type].meshData->vertexCount = drawParam->attributeBufferParam.attributeBuffers[FFL_ATTRIBUTE_BUFFER_TYPE_POSITION].size / sizeof(float) / 4;
    meshDatas[drawParam->modulateParam.type].meshData->indexCount = drawParam->primitiveParam.indexCount / sizeof(unsigned short);

    meshDatas[drawParam->modulateParam.type].indexBuffer = new Buffer(meshDatas[drawParam->modulateParam.type].meshData->indexCount * sizeof(unsigned short) * 2);
    memcpy(meshDatas[drawParam->modulateParam.type].indexBuffer->get<uint16_t*>(), drawParam->primitiveParam.pIndexBuffer, meshDatas[drawParam->modulateParam.type].meshData->indexCount * sizeof(unsigned short) * 2);
    meshDatas[drawParam->modulateParam.type].meshData->indexData = meshDatas[drawParam->modulateParam.type].indexBuffer->get<uint16_t*>();
};
FFLShaderCallback shaderCallback;

EMSCRIPTEN_KEEPALIVE
void generateMiiMeshes() {
    auto miiCharacterModel = getMii();

    for (int idx = 0; FFL_MODULATE_TYPE_SHAPE_MAX > idx; idx++) {
        auto meshData = meshDatas[idx];
        if (meshData.meshData != nullptr) {
            delete meshData.indexBuffer;
            for (auto & attributeBuffer : meshData.attributeBuffers) {
                if (attributeBuffer != nullptr)
                    delete attributeBuffer;
            };
            delete meshData.meshData;
            meshDatas[idx] = {};
        }
    }

    shaderCallback.pDrawFunc = shaderDrawCallback;
    FFLSetShaderCallback(&shaderCallback);

    FFLDrawOpa(miiCharacterModel);
    FFLDrawXlu(miiCharacterModel);
};

MeshData* getMesh(const char* object) {
    const std::string objectName = object;

    meshType = FFL_MODULATE_TYPE_SHAPE_MAX;

    // I might be racist now. Specifically because of this. (I am joking)
    if (objectName == "FACELINE") meshType = FFL_MODULATE_TYPE_SHAPE_FACELINE;
    if (objectName == "BEARD") meshType = FFL_MODULATE_TYPE_SHAPE_BEARD;
    if (objectName == "NOSE") meshType = FFL_MODULATE_TYPE_SHAPE_NOSE;
    if (objectName == "FOREHEAD") meshType = FFL_MODULATE_TYPE_SHAPE_FOREHEAD;
    if (objectName == "HAIR") meshType = FFL_MODULATE_TYPE_SHAPE_HAIR;
    if (objectName == "CAP") meshType = FFL_MODULATE_TYPE_SHAPE_CAP;
    if (objectName == "NOSELINE") meshType = FFL_MODULATE_TYPE_SHAPE_NOSELINE;
    if (objectName == "GLASS") meshType = FFL_MODULATE_TYPE_SHAPE_GLASS;
    if (objectName == "MASK") meshType = FFL_MODULATE_TYPE_SHAPE_MASK;

    if (meshType == FFL_MODULATE_TYPE_SHAPE_MAX) return nullptr;

    return meshDatas[meshType].meshData;
};

