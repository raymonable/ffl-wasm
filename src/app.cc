//
// Created by Raymond on 2/10/2025.
//

#include <app.hh>

GlobalInitializerData globalInitializerData;

// TODO: remove these below
const unsigned char cJasmineStoreData[96] = {
    0x03, 0x00, 0x00, 0x40, 0xA0, 0x41, 0x38, 0xC4, 0xA0, 0x84, 0x00, 0x00, 0xDB, 0xB8, 0x87, 0x31, 0xBE, 0x60, 0x2B, 0x2A, 0x2A, 0x42, 0x00, 0x00, 0x59, 0x2D, 0x4A, 0x00, 0x61, 0x00, 0x73, 0x00, 0x6D, 0x00, 0x69, 0x00, 0x6E, 0x00, 0x65, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x37, 0x12, 0x10, 0x7B, 0x01, 0x21, 0x6E, 0x43, 0x1C, 0x0D, 0x64, 0xC7, 0x18, 0x00, 0x08, 0x1E, 0x82, 0x0D, 0x00, 0x30, 0x41, 0xB3, 0x5B, 0x82, 0x6D, 0x00, 0x00, 0x6F, 0x00, 0x73, 0x00, 0x69, 0x00, 0x67, 0x00, 0x6F, 0x00, 0x6E, 0x00, 0x61, 0x00, 0x6C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x3A
};
FFLCharModel gCharModel;
FFLShaderCallback gShaderCallback;

#include <iostream>

void shaderDrawCallback(void* pObj, const FFLDrawParam* drawParam) {
    if (drawParam->primitiveParam.pIndexBuffer != nullptr) {
        std::cout << drawParam->modulateParam.type << std::endl;

        uint32_t indexCount = drawParam->primitiveParam.indexCount;
        const uint16_t* indices = static_cast<const uint16_t*>(drawParam->primitiveParam.pIndexBuffer);
        uint16_t maxIndex = 0;
        for (uint32_t i = 0; i < indexCount; ++i)
            if (indices[i] > maxIndex)
                maxIndex = indices[i];
        uint32_t vertexCount = maxIndex + 1;

        // for now let's just parse the vertex buffer
        auto buffer = drawParam->attributeBufferParam.attributeBuffers[FFL_ATTRIBUTE_BUFFER_TYPE_POSITION];
        uint32_t stride = buffer.stride;
        auto* ptr = static_cast<uint8_t*>(buffer.ptr);

        if (ptr != nullptr) {

            for (uint32_t i = 0; i < vertexCount; ++i) {
                void* elementPtr = ptr + (i * stride);
                std::cout << "stride" << " " << std::to_string(stride) << std::endl;
                auto* position = static_cast<float*>(elementPtr);
                std::string b = "v ";
                for (int j = 0; 3 > j; j++)
                    b += std::to_string(position[j]) + " ";
                EM_ASM({
                    if (!window["b"])
                        window["b"] = [];
                    window["b"].push(Module.UTF8ToString($0));
                }, b.c_str());
            };
        }
    }
}

void* initBuffer(int size) {
    if (size < 1)
        return nullptr;
    globalInitializerData.size = size;
    globalInitializerData.buffer = malloc(size);
    return globalInitializerData.buffer;
};
bool init() {
    if (globalInitializerData.buffer == nullptr)
        return false;
    FFLResourceDesc resourceDesc;

    // We're only going to support High for now
    resourceDesc.size[FFL_RESOURCE_TYPE_HIGH] = static_cast<size_t>(globalInitializerData.size);
    resourceDesc.pData[FFL_RESOURCE_TYPE_HIGH] = globalInitializerData.buffer;

    if (FFLInitRes(FFL_FONT_REGION_JP_US_EU, &resourceDesc) != FFL_RESULT_OK)
        // TODO: better error logging?
        return false;

    if (!FFLIsAvailable())
        return false;

    gShaderCallback.pDrawFunc = shaderDrawCallback;
    FFLSetShaderCallback(&gShaderCallback);

    FFLInitResGPUStep();

    return true;
};

bool test() {
    FFLCharModelSource modelSource{};
    modelSource.index = 0;
    modelSource.pBuffer = cJasmineStoreData;
    modelSource.dataSource = FFL_DATA_SOURCE_STORE_DATA;

    const u32 expressionFlag = 1 << FFL_EXPRESSION_NORMAL;

    FFLCharModelDesc modelDesc{};
    modelDesc.resolution = static_cast<FFLResolution>(512);
    modelDesc.expressionFlag = expressionFlag;
    modelDesc.modelFlag = FFL_MODEL_FLAG_NORMAL;
    modelDesc.resourceType = FFL_RESOURCE_TYPE_HIGH;

    if (FFLInitCharModelCPUStep(&gCharModel, &modelSource, &modelDesc) != FFL_RESULT_OK)
        return false;

    auto* charModel = (FFLiCharModel*)&gCharModel;

    FFLDrawOpa(&gCharModel);
    FFLDrawXlu(&gCharModel);

    return true;
};