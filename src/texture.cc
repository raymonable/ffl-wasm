//
// Created by Raymond on 2/13/2025.
//

#include <bindings.hh>

FFLTextureCallback fflTextureCallback;
FFLShaderCallback fflShaderCallback;

GLuint meshBuffers[FFL_ATTRIBUTE_BUFFER_TYPE_MAX];

void setupBuffer(FFLAttributeBufferType type, const FFLAttributeBuffer* bufferPtr) {
    void* ptr = bufferPtr->ptr;

    auto size = static_cast<int>(bufferPtr->size);
    auto stride = static_cast<int>(bufferPtr->stride);

    GLuint buffer = meshBuffers[type];
    if (!buffer)
        glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, size, ptr, GL_STATIC_DRAW);

    switch (type) {
        case FFL_ATTRIBUTE_BUFFER_TYPE_POSITION:
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
            glEnableVertexAttribArray(0);
            break;
        case FFL_ATTRIBUTE_BUFFER_TYPE_TEXCOORD:
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
            glEnableVertexAttribArray(1);
            break;
        // We're never going to draw with non-facial meshes, so we can disregard every other data type.
        default: break;
    }
};

void updateColors(const FFLDrawParam* drawParam) {
    auto program = getProgram();

    glUniform3f(glGetUniformLocation(program, "addonR"), 1, 1, 1);
    glUniform3f(glGetUniformLocation(program, "addonG"), 1, 1, 1);
    glUniform3f(glGetUniformLocation(program, "addonB"), 1, 1, 1);

    glUniform1i(4, drawParam->modulateParam.mode);

    switch (drawParam->modulateParam.mode) {
        case FFL_MODULATE_MODE_RGB_LAYERED:
            glUniform3fv(glGetUniformLocation(program, "addonG"), 1, &drawParam->modulateParam.pColorG->r);
            glUniform3fv(glGetUniformLocation(program, "addonB"), 1, &drawParam->modulateParam.pColorB->r);
        case FFL_MODULATE_MODE_LUMINANCE_ALPHA:
        case FFL_MODULATE_MODE_ALPHA:
            glUniform3fv(glGetUniformLocation(program, "addonR"), 1, &drawParam->modulateParam.pColorR->r);
        break;
        default: break;
    };
}

void shaderCallback(void* pObj, const FFLDrawParam* drawParam) {
    auto program = getProgram();
    glUseProgram(program);

    updateColors(drawParam);

#ifdef RIO_DEBUG
    printf("Mode: %s. Type: %s.\n", std::to_string(drawParam->modulateParam.mode).c_str(), std::to_string(drawParam->modulateParam.type).c_str());
#endif

    const auto textureHandle = reinterpret_cast<GLuint>(drawParam->modulateParam.pTexture2D);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureHandle);

    const auto attribBuffers = drawParam->attributeBufferParam.attributeBuffers;
    if (attribBuffers[FFL_ATTRIBUTE_BUFFER_TYPE_POSITION].ptr != nullptr)
        setupBuffer(FFL_ATTRIBUTE_BUFFER_TYPE_POSITION, &attribBuffers[FFL_ATTRIBUTE_BUFFER_TYPE_POSITION]);
    if (attribBuffers[FFL_ATTRIBUTE_BUFFER_TYPE_TEXCOORD].ptr != nullptr)
        setupBuffer(FFL_ATTRIBUTE_BUFFER_TYPE_TEXCOORD, &attribBuffers[FFL_ATTRIBUTE_BUFFER_TYPE_TEXCOORD]);

    GLuint indexBuffer;
    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, drawParam->primitiveParam.indexCount * sizeof(unsigned short), drawParam->primitiveParam.pIndexBuffer, GL_STATIC_DRAW);

    glDrawElements(drawParam->primitiveParam.primitiveType, drawParam->primitiveParam.indexCount, GL_UNSIGNED_SHORT, 0);

    glDeleteBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void textureCallback(void* v, const FFLTextureInfo* pTextureInfo, FFLTexture* pTexture) {
    GLuint handle;
    glGenTextures(1, &handle);
    glBindTexture(GL_TEXTURE_2D, handle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLint format = 0;
    switch (pTextureInfo->format) {
        case FFL_TEXTURE_FORMAT_R8_UNORM:
            format = GL_LUMINANCE;
            break;
        case FFL_TEXTURE_FORMAT_R8_G8_UNORM:
            format = GL_LUMINANCE_ALPHA;
            break;
        case FFL_TEXTURE_FORMAT_R8_G8_B8_A8_UNORM:
            format = GL_RGBA;
            break;
        default:
            glBindTexture(GL_TEXTURE_2D, 0);
            glDeleteTextures(1, &handle);
            return;
    };

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        format,
        pTextureInfo->width,
        pTextureInfo->height,
        0,
        format,
        GL_UNSIGNED_BYTE,
        pTextureInfo->imagePtr
    );

    glBindTexture(GL_TEXTURE_2D, 0);
    *(void**)pTexture = (void*)handle;
};

void textureDeleteCallback() {

}

// (global)
void initializeDrawCallbacks() {
    fflTextureCallback.useOriginalTileMode = false;
    fflTextureCallback.pCreateFunc = textureCallback;
    // TODO: delete callback
    FFLSetTextureCallback(&fflTextureCallback);

    fflShaderCallback.pDrawFunc = shaderCallback;
    FFLSetShaderCallback(&fflShaderCallback);
};

struct RenderTexture {
    GLuint textureHandle{};
    GLuint framebufferHandle{};
    int width = 0;
    int height = 0;
};
std::map<std::string, RenderTexture> renderTextures{};

RenderTextureExport* renderTextureExport = nullptr;
// TODO: switch this to use the custom Buffer class? tbd
void* currentTextureExportData = nullptr;

// (global export)
RenderTextureExport* exportTexture(const char* target) {
    if (currentTextureExportData != nullptr)
        free(currentTextureExportData);
    free(renderTextureExport);
    if (renderTextures.find(target) != renderTextures.end()) {
        auto renderTexture = renderTextures[target];
        glBindFramebuffer(GL_FRAMEBUFFER, renderTexture.framebufferHandle);
        currentTextureExportData = malloc(renderTexture.width * renderTexture.height * 4);
        glReadPixels(0, 0, renderTexture.width, renderTexture.height, GL_RGBA, GL_UNSIGNED_BYTE, currentTextureExportData);
        renderTextureExport = new RenderTextureExport{
            .width = static_cast<uint32_t>(renderTexture.width),
            .height = static_cast<uint32_t>(renderTexture.height),
            .data = currentTextureExportData
        };
        return renderTextureExport;
    }
    return nullptr;
};

void bindRenderTexture(const std::string& target) {
    glBindFramebuffer(GL_FRAMEBUFFER, renderTextures[target].framebufferHandle);
    glViewport(0, 0, renderTextures[target].width, renderTextures[target].height);
};
void bindRenderTexture(const std::string& target, int width, int height) {
    if (!(renderTextures.find(target) != renderTextures.end())) {
        glGenFramebuffers(1, &renderTextures[target].framebufferHandle);
        glGenTextures(1, &renderTextures[target].textureHandle);

        glBindFramebuffer(GL_FRAMEBUFFER, renderTextures[target].framebufferHandle);
        glBindTexture(GL_TEXTURE_2D, renderTextures[target].textureHandle);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA,
            width,
            height,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            nullptr
        );
        renderTextures[target].width = width;
        renderTextures[target].height = height;

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTextures[target].textureHandle, 0);
    };
    bindRenderTexture(target);
}
int getRenderTextureHandle(const char* target) {
    if (renderTextures.find(target) != renderTextures.end())
        return static_cast<int>(renderTextures[target].textureHandle);
    return 0;
};

// (global export)
void drawFaceTexture(int expression) {
    initializeDrawCallbacks();
    const auto iMiiCharacterModel = reinterpret_cast<FFLiCharModel*>(getMii());
    const int resolution = iMiiCharacterModel->charModelDesc.resolution;

    bindRenderTexture("MASK", resolution, resolution);

    FFLShaderCallback* pCallback = &fflShaderCallback;
    FFLShaderCallback** ppCallback = &pCallback;

    FFLiMaskTexturesTempObject* pObject = &iMiiCharacterModel->pTextureTempObject->maskTextures;
    FFLiInvalidatePartsTextures(&pObject->partsTextures);

    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    FFLiInvalidateRawMask(pObject->pRawMaskDrawParam[expression]);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    FFLiDrawRawMask(pObject->pRawMaskDrawParam[expression], reinterpret_cast<const FFLiShaderCallback*>(ppCallback));

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
};

// (global export)
void drawFaceline() {
    initializeDrawCallbacks();
    const auto iMiiCharacterModel = reinterpret_cast<FFLiCharModel*>(getMii());
    const int resolution = iMiiCharacterModel->charModelDesc.resolution;

    bindRenderTexture("FACELINE", resolution / 2, resolution);

    auto skinTone = FFLGetFacelineColor(iMiiCharacterModel->charInfo.parts.facelineColor);

    FFLShaderCallback* pCallback = &fflShaderCallback;
    FFLShaderCallback** ppCallback = &pCallback;

    glClearColor(skinTone.r, skinTone.g, skinTone.b, skinTone.a);
    glClear(GL_COLOR_BUFFER_BIT);

    if (iMiiCharacterModel->facelineRenderTexture.pTexture2D != nullptr) {
        glBlendEquation(GL_FUNC_ADD);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

        FFLiInvalidateTempObjectFacelineTexture(&iMiiCharacterModel->pTextureTempObject->facelineTexture);
        FFLiDrawFacelineTexture(&iMiiCharacterModel->pTextureTempObject->facelineTexture, reinterpret_cast<const FFLiShaderCallback*>(ppCallback));
    }
};

constexpr float xluRenderVertices[] = {
    1.0f, 1.0f, 1.0f, 1.0,
    1.0f, -1.0f, 1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f, 0.0f,
    -1.0f, 1.0f, 0.0f, 1.0f
};
unsigned int xluRenderIndices[] = {
    0, 1, 3,
    1, 2, 3
};

void drawXluPart(const FFLDrawParam* drawParam) {
    auto program = getProgram();

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(program);
    updateColors(drawParam);

    const auto textureHandle = reinterpret_cast<GLuint>(drawParam->modulateParam.pTexture2D);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureHandle);

    GLuint buffer{};

    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(xluRenderVertices), xluRenderVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void *>(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    GLuint indexBuffer{};
    glGenBuffers(1, &indexBuffer);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(xluRenderIndices), xluRenderIndices, GL_STATIC_DRAW);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

    glDeleteBuffers(1, &buffer);
    glDeleteBuffers(1, &indexBuffer);
};

// (global export)
void drawXlu() {
    initializeDrawCallbacks();
    const auto iMiiCharacterModel = reinterpret_cast<FFLiCharModel*>(getMii());
    const int resolution = iMiiCharacterModel->charModelDesc.resolution;

    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    // Glasses

    bindRenderTexture("GLASS", resolution / 2, resolution / 2);
    auto glassParam = FFLiGetDrawParamXluGlassFromCharModel(iMiiCharacterModel);
    if (glassParam->modulateParam.pTexture2D != nullptr)
        drawXluPart(glassParam);

    // Nose

    bindRenderTexture("NOSELINE", resolution / 2, resolution / 2);
    auto noseParam = FFLiGetDrawParamXluNoseLineFromCharModel(iMiiCharacterModel);
    // I'm concerned if there's no nose
    if (noseParam->modulateParam.pTexture2D != nullptr)
        drawXluPart(noseParam);
};