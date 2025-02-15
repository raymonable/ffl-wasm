//
// Created by Raymond on 2/14/2025.
//

#include <bindings.hh>

FFLModulateType meshType;
std::map<FFLAttributeBufferType, Buffer*> meshBufferCache;
Buffer* indexBuffer = nullptr;
MeshData* meshData = nullptr;

void shaderDrawCallback(void* pObj, const FFLDrawParam* drawParam) {
    // Probably not the most efficient way to do this, but I want to avoid using a shit ton of memory since we have
    //      only 64 megabytes of heap
    if (meshType != drawParam->modulateParam.type) return;

    for (int type = 0; type < FFL_ATTRIBUTE_BUFFER_TYPE_MAX; type++) {
        const FFLAttributeBuffer* buffer = &drawParam->attributeBufferParam.attributeBuffers[type];
        const void* ptr = buffer->ptr;

        const auto size = static_cast<int>(buffer->size);
        if (ptr != nullptr) {
            auto typeCast = static_cast<FFLAttributeBufferType>(type);
            if (meshBufferCache.find(typeCast) != meshBufferCache.end())
                delete meshBufferCache[typeCast]; // Remove existing buffer
            meshBufferCache[typeCast] = new Buffer(size); // ... and create a new one! :3
            memcpy(
                meshBufferCache[typeCast]->get<void*>(),
                ptr, size
            );
        }
    };

    indexBuffer = new Buffer(static_cast<int>(drawParam->primitiveParam.indexCount));
    memcpy(
        indexBuffer->get<void*>(),
        drawParam->primitiveParam.pIndexBuffer, drawParam->primitiveParam.indexCount * sizeof(uint32_t)
    );
};
FFLShaderCallback shaderCallback;

MeshData* getMesh(const char* object) {
    if (meshData != nullptr)
        free(meshData);

    shaderCallback.pDrawFunc = shaderDrawCallback;
    FFLSetShaderCallback(&shaderCallback);

    const auto miiCharacterModel = getMii();

    switch (std::string(object)) {
        case "FACELINE":
            meshType = FFL_MODULATE_TYPE_SHAPE_FACELINE; break;
        case "BEARD":
            meshType = FFL_MODULATE_TYPE_SHAPE_BEARD; break;
        case "NOSE":
            meshType = FFL_MODULATE_TYPE_SHAPE_NOSE; break;
        case "FOREHEAD":
            meshType = FFL_MODULATE_TYPE_SHAPE_FOREHEAD; break;
        case "HAIR":
            meshType = FFL_MODULATE_TYPE_SHAPE_HAIR; break;
        case "CAP":
            meshType = FFL_MODULATE_TYPE_SHAPE_CAP; break;
        case "MASK":
            meshType = FFL_MODULATE_TYPE_SHAPE_MASK; break;
        case "NOSELINE":
            meshType = FFL_MODULATE_TYPE_SHAPE_NOSELINE; break;
        case "GLASS":
            meshType = FFL_MODULATE_TYPE_SHAPE_GLASS; break;
        default: break;
    };

    // Honestly, why are these separate?
    FFLDrawOpa(miiCharacterModel);
    FFLDrawXlu(miiCharacterModel);

    meshData = new MeshData{
        .vertexCount = static_cast<uint32_t>(meshBufferCache[FFL_ATTRIBUTE_BUFFER_TYPE_POSITION]->size / 3 / sizeof(float)),
        .indexCount = static_cast<uint32_t>(indexBuffer->size / sizeof(uint32_t)),

        .indexData = indexBuffer->get<void*>(),

        .positionData = meshBufferCache[FFL_ATTRIBUTE_BUFFER_TYPE_POSITION]->get<void*>(),
        .uvData = meshBufferCache[FFL_ATTRIBUTE_BUFFER_TYPE_TEXCOORD]->get<void*>(),
        .normalData = meshBufferCache[FFL_ATTRIBUTE_BUFFER_TYPE_NORMAL]->get<void*>(),
        .tangentData = meshBufferCache[FFL_ATTRIBUTE_BUFFER_TYPE_TANGENT]->get<void*>(),
        .colorData = meshBufferCache[FFL_ATTRIBUTE_BUFFER_TYPE_COLOR]->get<void*>(),
    };
    return meshData;
};

