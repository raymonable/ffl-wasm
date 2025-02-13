//
// Created by Raymond on 2/12/2025.
//

#include <bindings.hh>

#include <iostream>
void faceShaderDrawCallback(void* pObj, const FFLDrawParam* drawParam) {
    // TODO: move shader compilation to init

    glEnable(GL_BLEND);

    std::cout << "mode " << drawParam->modulateParam.mode << " type " << drawParam->modulateParam.type << std::endl;

    GLuint textureHandle = (GLuint)drawParam->modulateParam.pTexture2D;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureHandle);

    if (drawParam->modulateParam.type < FFL_MODULATE_TYPE_SHAPE_MAX)
    {
        // Only apply texture wrap to shapes (glass, faceline)
        // since those are NPOT textures
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    } else {
        // Otherwise do not use repeat wrap
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    for (int type = 0; type < FFL_ATTRIBUTE_BUFFER_TYPE_MAX; type++) {
        const FFLAttributeBuffer* buffer = &drawParam->attributeBufferParam.attributeBuffers[type];
        void* ptr = buffer->ptr;

        auto size = static_cast<int>(buffer->size);
        auto stride = static_cast<int>(buffer->stride);

        // TODO: major cleanup on aisle this function please
        GLuint vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, size, ptr, GL_STATIC_DRAW);

        if (ptr != nullptr)
            std::cout << type << std::endl;
            switch (type) {
                case FFL_ATTRIBUTE_BUFFER_TYPE_POSITION: {
                    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
                    glEnableVertexAttribArray(0);
                    break;
                }
                case FFL_ATTRIBUTE_BUFFER_TYPE_TEXCOORD: {
                    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
                    glEnableVertexAttribArray(1);
                    break;
                }
                // NOTE: color is needed for eyes and such
                default: break;
                // We shouldn't be drawing actual Mii heads with this function, so we can completely disregard the other two
            }
    };

    GLuint indexBufferHandle;
    glGenBuffers(1, &indexBufferHandle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferHandle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, drawParam->primitiveParam.indexCount * sizeof(unsigned short), drawParam->primitiveParam.pIndexBuffer, GL_STATIC_DRAW);

    glDrawElements(drawParam->primitiveParam.primitiveType, drawParam->primitiveParam.indexCount, GL_UNSIGNED_SHORT, 0);
}
FFLShaderCallback gFaceShaderCallback;
FFLTextureCallback gFaceTextureCallback{};

// TODO: REPLACE PLEASE
#define VAO_NOT_SUPPORTED
void TextureCallback_Create(void* v, const FFLTextureInfo* pTextureInfo, FFLTexture* pTexture)
{
    GLuint textureHandle = 0;

    // Generate a texture
    glGenTextures(1, &textureHandle);
    glBindTexture(GL_TEXTURE_2D, textureHandle);

    // Configure texture parameters (wrap and filter modes)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Determine OpenGL format based on FFLTextureInfo format
    GLenum internalFormat, format, type;
    switch (pTextureInfo->format)
    {
        case FFL_TEXTURE_FORMAT_R8_UNORM:
#ifdef VAO_NOT_SUPPORTED // GL ES 2.0 compatible types
            internalFormat = GL_LUMINANCE;
            format = GL_LUMINANCE;
#else
            internalFormat = GL_R8;
            format = GL_RED;
#endif // VAO_NOT_SUPPORTED
            type = GL_UNSIGNED_BYTE;
            break;
        case FFL_TEXTURE_FORMAT_R8_G8_UNORM:
#ifdef VAO_NOT_SUPPORTED // GL ES 2.0 compatible types
            internalFormat = GL_LUMINANCE_ALPHA;
            format = GL_LUMINANCE_ALPHA;
#else
            internalFormat = GL_RG8;
            format = GL_RG;
#endif // VAO_NOT_SUPPORTED
            type = GL_UNSIGNED_BYTE;
            break;
        case FFL_TEXTURE_FORMAT_R8_G8_B8_A8_UNORM:
            internalFormat = GL_RGBA;
            format = GL_RGBA;
            type = GL_UNSIGNED_BYTE;
            break;
        default:
            // Unsupported format
            glBindTexture(GL_TEXTURE_2D, 0);
            glDeleteTextures(1, &textureHandle);
            //free(textureHandle);
            return;
    }
    // Upload texture data
    glTexImage2D(
        GL_TEXTURE_2D,
        0, // Level 0 (base level)
        internalFormat,
        pTextureInfo->width,
        pTextureInfo->height,
        0, // Border (must be 0)
        format,
        type,
        //Hello Lady
        pTextureInfo->imagePtr
    );
/*
    // Set up mipmaps if provided
    if (pTextureInfo->numMips > 1 && pTextureInfo->mipPtr)
    {
        const unsigned char* mipData = (const unsigned char*)pTextureInfo->mipPtr;
        for (u32 mipLevel = 1; mipLevel < pTextureInfo->numMips; ++mipLevel)
        {
            u32 mipWidth = pTextureInfo->width >> mipLevel;
            u32 mipHeight = pTextureInfo->height >> mipLevel;
            if (mipWidth == 0 || mipHeight == 0)
                break;

            glTexImage2D(
                GL_TEXTURE_2D,
                mipLevel,
                internalFormat,
                mipWidth,
                mipHeight,
                0,
                format,
                type,
                mipData
            );

            // Calculate mipData offset (this assumes tightly packed mip levels)
            mipData += mipWidth * mipHeight * (type == GL_UNSIGNED_BYTE ? 1 : 4);
        }
    }
*/
    // Unbind texture and return the handle
    glBindTexture(GL_TEXTURE_2D, 0);
    *(void**)pTexture = (void*)textureHandle;
}

void* generateTextures(int expression) {
    gFaceTextureCallback.useOriginalTileMode = false;
    gFaceTextureCallback.pCreateFunc = TextureCallback_Create;
    FFLSetTextureCallback(&gFaceTextureCallback);

    gFaceShaderCallback.pDrawFunc = faceShaderDrawCallback;
    FFLSetShaderCallback(&gFaceShaderCallback);

    if (!expression) return nullptr; // TO FIX ORDER, PLEASE REMOVE ASAP

    auto miiCharacterModel = getMii();
    auto iMiiCharacterModel = reinterpret_cast<FFLiCharModel*>(miiCharacterModel);

    // TODO: handle expression

    FFLShaderCallback* pCallback = &gFaceShaderCallback;
    FFLShaderCallback** ppCallback = &pCallback;

    /*
    if (iMiiCharacterModel->facelineRenderTexture.pTexture2D != nullptr) {
        unsigned int texture;
        glViewport(0, 0, 256, 512);

        FFLiInvalidateTempObjectFacelineTexture(&iMiiCharacterModel->pTextureTempObject->facelineTexture);
        auto skinTone = FFLGetFacelineColor(iMiiCharacterModel->charInfo.parts.facelineColor);

        glClearColor(skinTone.r, skinTone.g, skinTone.b, skinTone.a);
        glClear(GL_COLOR_BUFFER_BIT);

        glBlendEquation(GL_FUNC_ADD);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

        FFLiDrawFacelineTexture(&iMiiCharacterModel->pTextureTempObject->facelineTexture, reinterpret_cast<const FFLiShaderCallback*>(ppCallback));
        // TODO: store to texture
    }*/

    // face mask

    FFLiMaskTexturesTempObject* pObject = &iMiiCharacterModel->pTextureTempObject->maskTextures;
    FFLiInvalidatePartsTextures(&pObject->partsTextures);

    glBlendFuncSeparate(GL_ONE_MINUS_DST_ALPHA, GL_DST_ALPHA, GL_SRC_ALPHA, GL_DST_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    FFLiInvalidateRawMask(pObject->pRawMaskDrawParam[0]);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    FFLiDrawRawMask(pObject->pRawMaskDrawParam[0], reinterpret_cast<const FFLiShaderCallback*>(ppCallback));

    return nullptr; // in the future, return a pointer that contains pointers for both the faceline, mask and other textures.
}