//
// Created by Raymond on 2/12/2025.
//

#include <bindings.hh>

Buffer* gFFLDataBuffer = nullptr;

Buffer* gMiiDataBuffer = nullptr;
FFLCharModel* gMiiCharModel;
FFLCharModel* getMii() {
    return gMiiCharModel;
};

// (global export)
void* init(int size) {
    if (gFFLDataBuffer != nullptr) {
        FFLResourceDesc resourceDescription;

        // Currently only supporting HIGH
        resourceDescription.size[FFL_RESOURCE_TYPE_HIGH] = gFFLDataBuffer->size;
        resourceDescription.pData[FFL_RESOURCE_TYPE_HIGH] = gFFLDataBuffer->get<void*>();

        if (FFLInitRes(FFL_FONT_REGION_JP_US_EU, &resourceDescription) != FFL_RESULT_OK)
            return nullptr;
        if (!FFLIsAvailable() || !initializeGL())
            return nullptr;

        initializeDrawCallbacks();
        FFLInitResGPUStep();

        gMiiDataBuffer = new Buffer(96);
        return gMiiDataBuffer->get<void*>();
    } else {
        gFFLDataBuffer = new Buffer(size);
        return gFFLDataBuffer->get<void*>();
    }
};

// (global export) call this after updating the mii buffer
bool mii() {
    if (!gMiiDataBuffer)
        return false;
    if (!gMiiCharModel)
        gMiiCharModel = new FFLCharModel();

    FFLCharModelSource modelSource{};
    modelSource.index = 0;
    modelSource.pBuffer = gMiiDataBuffer->get<unsigned char*>();
    modelSource.dataSource = FFL_DATA_SOURCE_STORE_DATA;

    FFLCharModelDesc modelDescription{};
    modelDescription.resolution = FFL_RESOLUTION_TEX_512;
    modelDescription.expressionFlag = 1 << FFL_EXPRESSION_NORMAL; // TODO: replace this with an "all" flag, dunno how that works :sob:
    modelDescription.modelFlag = FFL_MODEL_FLAG_NORMAL;
    modelDescription.resourceType = FFL_RESOURCE_TYPE_HIGH;

    if (FFLInitCharModelCPUStep(gMiiCharModel, &modelSource, &modelDescription) != FFL_RESULT_OK)
        return false;

    return true;
};