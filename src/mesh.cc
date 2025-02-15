//
// Created by Raymond on 2/14/2025.
//

#include <bindings.hh>

FFLModulateType meshType;
std::map<FFLAttributeBufferType, Buffer*> meshBufferCache;
Buffer* indexBuffer = nullptr;
RGB rgb;
MeshData* meshData = nullptr;

void shaderDrawCallback(void* pObj, const FFLDrawParam* drawParam) {
    // Probably not the most efficient way to do this, but I want to avoid using a shit ton of memory since we have
    //      only 64 megabytes of heap
    if (meshType != drawParam->modulateParam.type) return;

    rgb = {
        .r = drawParam->modulateParam.pColorR->r,
        .g = drawParam->modulateParam.pColorR->g,
        .b = drawParam->modulateParam.pColorR->b,
    };

    for (int type = 0; type < FFL_ATTRIBUTE_BUFFER_TYPE_MAX; type++) {
        const FFLAttributeBuffer* buffer = &drawParam->attributeBufferParam.attributeBuffers[type];
        void* ptr = buffer->ptr;

        const auto stride = static_cast<int>(buffer->stride);
        const auto size = static_cast<int>(buffer->size);
        printf("Stride: %s\nSize: %s\nBuffer Type:%s\n", std::to_string(stride).c_str(), std::to_string(size).c_str(), std::to_string(type).c_str());
        if (ptr != nullptr) {
            auto typeCast = static_cast<FFLAttributeBufferType>(type);
            if (meshBufferCache.find(typeCast) != meshBufferCache.end())
                delete meshBufferCache[typeCast]; // Remove existing buffer
            meshBufferCache[typeCast] = new Buffer(size); // ... and create a new one! :3
            memcpy(
                meshBufferCache[typeCast]->get<void*>(),
                ptr, size
            );

            if (type == FFL_ATTRIBUTE_BUFFER_TYPE_POSITION) {
                for (size_t index = 0; index < 25; index++) {
                    printf("Data!! %s: %s\n", std::to_string(index).c_str(), std::to_string(*reinterpret_cast<float*>(reinterpret_cast<unsigned char*>(ptr) + (stride * index))).c_str());
                }
            }
        }
    };

    if (indexBuffer != nullptr)
        delete indexBuffer;
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

    // Honestly, why are these separate?
    FFLDrawOpa(miiCharacterModel);
    FFLDrawXlu(miiCharacterModel);

    meshData = new MeshData{
        .vertexCount = static_cast<uint32_t>(meshBufferCache[FFL_ATTRIBUTE_BUFFER_TYPE_POSITION]->size / 4 / sizeof(GLfloat)),
        .indexCount = static_cast<uint32_t>(indexBuffer->size / sizeof(GLuint)),

        .rgb = rgb,
        .indexData = indexBuffer->get<void*>(),

        .positionData = meshBufferCache[FFL_ATTRIBUTE_BUFFER_TYPE_POSITION]->get<void*>(),
        .uvData = meshBufferCache[FFL_ATTRIBUTE_BUFFER_TYPE_TEXCOORD]->get<void*>(),
        .normalData = meshBufferCache[FFL_ATTRIBUTE_BUFFER_TYPE_NORMAL]->get<void*>(),
        .tangentData = meshBufferCache[FFL_ATTRIBUTE_BUFFER_TYPE_TANGENT]->get<void*>(),
    };
    return meshData;
};

