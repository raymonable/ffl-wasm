//
// Created by Raymond on 2/12/2025.
//

#include <bindings.hh>

Buffer* gFFLDataBuffer = nullptr;

Buffer* gMiiDataBuffer = nullptr;
FFLCharModel gMiiCharModel;
FFLCharModel getMii() {
    return gMiiCharModel;
};

void* init(int size) {
    if (gFFLDataBuffer != nullptr) {
        FFLResourceDesc resourceDescription;

        // Currently only supporting HIGH
        resourceDescription.size[FFL_RESOURCE_TYPE_HIGH] = gFFLDataBuffer->size;
        resourceDescription.pData[FFL_RESOURCE_TYPE_HIGH] = gFFLDataBuffer->get<void*>();

        if (FFLInitRes(FFL_FONT_REGION_JP_US_EU, &resourceDescription) != FFL_RESULT_OK)
            return nullptr;

        if (!FFLIsAvailable())
            return nullptr;

        FFLInitResGPUStep();

        delete gFFLDataBuffer;
        gMiiDataBuffer = new Buffer(96);
        return gMiiDataBuffer->get<void*>();
    } else {
        gFFLDataBuffer = new Buffer(size);
        return gFFLDataBuffer->get<void*>();
    }
};

bool mii() {
    if (!gMiiDataBuffer)
        return false;

    FFLCharModelSource modelSource{};
    modelSource.index = 0;
    modelSource.pBuffer = gMiiDataBuffer->get<char*>();
    modelSource.dataSource = FFL_DATA_SOURCE_STORE_DATA;

    // TODO: expression and resolution should be dynamic
    FFLCharModelDesc modelDescription{};
    modelDescription.resolution = static_cast<FFLResolution>(512);
    modelDescription.expressionFlag = FFL_EXPRESSION_ANGER;
    modelDescription.modelFlag = FFL_MODEL_FLAG_NORMAL;
    modelDescription.resourceType = FFL_RESOURCE_TYPE_HIGH;

    if (FFLInitCharModelCPUStep(&gMiiCharModel, &modelSource, &modelDescription) != FFL_RESULT_OK)
        return false;

    // TODO: register custom shader callback so we can copy the facial meshes to Three.js
    // TODO: render requested expressions

    return true;
};