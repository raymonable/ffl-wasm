//
// Created by Raymond on 2/14/2025.
//

#include <bindings.hh>

FFLModulateType meshType;
MeshData* meshData = nullptr;

void shaderDrawCallback(void* pObj, const FFLDrawParam* drawParam) {
    // Probably not the most efficient way to do this, but I want to avoid using a shit ton of memory since we have
    //      only 64 megabytes of heap
    if (meshType != drawParam->modulateParam.type) return;

    meshData = new MeshData{
        .rgb = {
            .r = drawParam->modulateParam.pColorR->r,
            .g = drawParam->modulateParam.pColorR->g,
            .b = drawParam->modulateParam.pColorR->b,
        }
    };

    for (int type = 0; type < FFL_ATTRIBUTE_BUFFER_TYPE_MAX; type++) {
        const FFLAttributeBuffer* buffer = &drawParam->attributeBufferParam.attributeBuffers[type];
        void* ptr = buffer->ptr;

        const auto stride = static_cast<int>(buffer->stride);
        const auto size = static_cast<int>(buffer->size);

#ifdef RIO_DEBUG
        printf("Stride: %s\nSize: %s\nBuffer Type:%s\n", std::to_string(stride).c_str(), std::to_string(size).c_str(), std::to_string(type).c_str());
#endif

        auto typeCast = static_cast<FFLAttributeBufferType>(type);
        unsigned char** bufferPtr = nullptr;
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
        if (bufferPtr != nullptr)
            if (ptr != nullptr) {
                *bufferPtr = reinterpret_cast<unsigned char*>(malloc(size));
                memcpy(*bufferPtr, ptr, size);
            } else
                *bufferPtr = nullptr;
    }

    meshData->vertexCount = drawParam->attributeBufferParam.attributeBuffers[FFL_ATTRIBUTE_BUFFER_TYPE_POSITION].size / sizeof(float) / 4;
    meshData->indexCount = drawParam->primitiveParam.indexCount / sizeof(unsigned short);

    meshData->indexData = reinterpret_cast<unsigned char*>(malloc(meshData->indexCount * sizeof(unsigned short) * 2));
    memcpy(meshData->indexData, drawParam->primitiveParam.pIndexBuffer, meshData->indexCount * sizeof(unsigned short) * 2);
};
FFLShaderCallback shaderCallback;

MeshData* getMesh(const char* object) {
    if (meshData != nullptr) {
        free(meshData->indexData);
        free(meshData->normalData);
        free(meshData->tangentData);
        free(meshData->texCoordData);
        free(meshData->positionData);

        free(meshData);
    };

    shaderCallback.pDrawFunc = shaderDrawCallback;
    FFLSetShaderCallback(&shaderCallback);

    const auto miiCharacterModel = getMii();
    const std::string objectName = object;

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
    
    // Honestly, why are these separate?
    FFLDrawOpa(miiCharacterModel);
    FFLDrawXlu(miiCharacterModel);

    return meshData;
};

