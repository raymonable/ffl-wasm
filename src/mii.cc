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
auto fflResolution = FFL_RESOLUTION_TEX_512;

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

        if (size != 0) {
            // Am I stupid? Does a function to cast the integer to an enum already exist? I really hope so, honestly
            switch (size) {
                case 128:
                    fflResolution = FFL_RESOLUTION_TEX_128; break;
                case 192:
                    fflResolution = FFL_RESOLUTION_TEX_192; break;
                case 256:
                    fflResolution = FFL_RESOLUTION_TEX_256; break;
                case 512:
                    fflResolution = FFL_RESOLUTION_TEX_512; break;
                case 768:
                    fflResolution = FFL_RESOLUTION_TEX_768; break;
                case 1024:
                    fflResolution = FFL_RESOLUTION_TEX_1024; break;
                default: break;
            }
        } else
            fflResolution = FFL_RESOLUTION_TEX_512;;

        gMiiDataBuffer = new Buffer(96);
        return gMiiDataBuffer->get<void*>();
    } else {
        gFFLDataBuffer = new Buffer(size);
        return gFFLDataBuffer->get<void*>();
    }
};

// (global export) call this after updating the mii buffer
bool mii(bool allExpressions) {
    if (!gMiiDataBuffer)
        return false;
    if (!gMiiCharModel)
        gMiiCharModel = new FFLCharModel();

    FFLCharModelSource modelSource{};
    modelSource.index = 0;
    modelSource.pBuffer = gMiiDataBuffer->get<unsigned char*>();
    modelSource.dataSource = FFL_DATA_SOURCE_STORE_DATA;

    FFLCharModelDesc modelDescription{};
    modelDescription.resolution = fflResolution;
    modelDescription.modelFlag = FFL_MODEL_FLAG_NORMAL | FFL_MODEL_FLAG_NEW_EXPRESSIONS;
    modelDescription.resourceType = FFL_RESOURCE_TYPE_HIGH;

    if (allExpressions) {
        for (uint32_t index = 0; index < FFL_EXPRESSION_MAX; index++)
            FFLSetExpressionFlagIndex(&modelDescription.allExpressionFlag, index, true);
    } else
        modelDescription.expressionFlag =
              1 << FFL_EXPRESSION_NORMAL
            | 1 << FFL_EXPRESSION_BLINK
            | 1 << FFL_EXPRESSION_SMILE
            | 1 << FFL_EXPRESSION_HAPPY;

    if (FFLInitCharModelCPUStep(gMiiCharModel, &modelSource, &modelDescription) != FFL_RESULT_OK)
        return false;

    return true;
};