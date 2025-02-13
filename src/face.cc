//
// Created by Raymond on 2/12/2025.
//

#include <bindings.hh>

#include <iostream>
void faceShaderDrawCallback(void* pObj, const FFLDrawParam* drawParam) {
    if (drawParam->modulateParam.type == FFL_MODULATE_TYPE_SHAPE_MASK) {
        std::cout << FFL_GET_RIO_NATIVE_TEXTURE_HANDLE(drawParam->modulateParam.pTexture2D) << std::endl;
    }
}
FFLShaderCallback gFaceShaderCallback;

void* renderFaceTextures(FFLExpression expression) {
    FFLCharModel miiCharacterModel = getMii();

    gFaceShaderCallback.pDrawFunc = faceShaderDrawCallback;
    FFLSetShaderCallback(&gFaceShaderCallback);

    FFLDrawOpa(&miiCharacterModel);
    FFLDrawXlu(&miiCharacterModel);
}