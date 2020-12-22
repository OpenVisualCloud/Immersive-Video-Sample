/*
 * Copyright (c) 2020, Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.

 *
 */
//!
//! \file     JNIObjects.h
//! \brief    Defines class for JNIObject.
//!

#ifdef _ANDROID_OS_

#include "JNIObjects.h"
#include "../Render/RenderBackend.h"
#include <GLES2/gl2ext.h>

VCD_NS_BEGIN

// JNIContext

JavaVM* JNIContext::mJavaVM = nullptr;
jobject JNIContext::mActivity = nullptr;
JNIEnv* JNIContext::mJEnv = nullptr;

JavaVM* jvm_global;

JNIContext::JNIContext()
{
    mJavaVM = nullptr;
    mActivity = nullptr;
    mJEnv = nullptr;
}

JNIContext::~JNIContext()
{
    mJavaVM = nullptr;
    mActivity = nullptr;
    mJEnv = nullptr;
}

JavaVM* JNIContext::GetJVM()
{
    mJavaVM = jvm_global;
    return mJavaVM;
}

jobject JNIContext::GetActivity()
{
    return mActivity;
}

JNIEnv* JNIContext::GetJNIEnv()
{
    GetJVM();
    if (mJavaVM == nullptr)
    {
        ANDROID_LOGD("Java vm is null ptr!");
    }
    int status = mJavaVM->GetEnv((void **)&mJEnv, JNI_VERSION_1_6);
    if (status < 0) {
        status = mJavaVM->AttachCurrentThread(&mJEnv, NULL);
        if (status < 0)
        {
            ANDROID_LOGD("get env failed!");
            return nullptr;
        }
    }
    return mJEnv;
}

void JNIContext::Detach_env()
{
    mJavaVM->DetachCurrentThread();
}

// SurfaceTexture
SurfaceTexture::SurfaceTexture()
{
    mJEnv = nullptr;
    mJavaObject = nullptr;
    mTexture = 0;
    mPts = 0;
    mTransformMatrix.clear();
    mUpdateTexImageMethodId = nullptr;
    mGetTimestampMethodId = nullptr;
    mGetTransformMatrixId = nullptr;
}

SurfaceTexture::~SurfaceTexture()
{
    if (mJavaObject != nullptr)
    {
        mJEnv->DeleteGlobalRef(mJavaObject);
        mJavaObject = nullptr;
    }
    if (mTexture != 0)
    {
        RenderBackend *renderBackend = RENDERBACKEND::GetInstance();
        renderBackend->DeleteTextures(1, &mTexture);
        mTexture = 0;
    }
    mTransformMatrix.clear();
    mJEnv = nullptr;
    mUpdateTexImageMethodId = nullptr;
    mGetTimestampMethodId = nullptr;
    mGetTransformMatrixId = nullptr;
}

Surface_ERROR SurfaceTexture::CreateSurfaceTexture()
{
    // 1. Get jni env
    mJEnv = JNIContext::GetJNIEnv();
    // 2. Find class
    jclass surfaceTexture_class = mJEnv->FindClass("android/graphics/SurfaceTexture");
    if (surfaceTexture_class == nullptr) return SURFACE_CREATE_ERROR;
    // 3. Create bind texture
    RenderBackend *renderBackend = RENDERBACKEND::GetInstance();
    renderBackend->GenTextures(1, &mTexture);
    renderBackend->BindTexture(GL_TEXTURE_EXTERNAL_OES, mTexture);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // 4. Create java object of SurfaceTexture
    const jmethodID surfaceTexture_constructor = mJEnv->GetMethodID(surfaceTexture_class, "<init>", "(I)V");
    if (surfaceTexture_constructor == nullptr) return SURFACE_CREATE_ERROR;
    jobject surfaceTexture_object = mJEnv->NewObject(surfaceTexture_class, surfaceTexture_constructor, mTexture);
    if (surfaceTexture_object == nullptr) return SURFACE_CREATE_ERROR;
    // 5. Create a global ref for SUrfaceTexture
    mJavaObject = mJEnv->NewGlobalRef(surfaceTexture_object);
    if (mJavaObject == nullptr) return SURFACE_CREATE_ERROR;
    // 6. Get updateTexImage method id
    const jmethodID updateTexImageMethodId = mJEnv->GetMethodID(surfaceTexture_class, "updateTexImage", "()V");
    if (updateTexImageMethodId == nullptr)
    {
        LOG(ERROR) << "Failed to get method id of updateTexImage!" << std::endl;
        return SURFACE_CREATE_ERROR;
    }
    mUpdateTexImageMethodId = updateTexImageMethodId;
    // 7. Get getTimestamp method id
    const jmethodID getTimestampMethodId = mJEnv->GetMethodID(surfaceTexture_class, "getTimestamp", "()J");
    if (getTimestampMethodId == nullptr)
    {
        LOG(ERROR) << "Failed to get method id of getTimestamp!" << std::endl;
        return SURFACE_CREATE_ERROR;
    }
    mGetTimestampMethodId = getTimestampMethodId;
    // 8. Get getTransformMatrix method id
    const jmethodID getTransformMatrixMethodId = mJEnv->GetMethodID(surfaceTexture_class, "getTransformMatrix", "([F)V");
    if (getTransformMatrixMethodId == nullptr)
    {
        LOG(ERROR) << "Failed to get method id of getTransformMatrix!" << std::endl;
        return SURFACE_CREATE_ERROR;
    }
    mGetTransformMatrixId = getTransformMatrixMethodId;
    // 9. Delete local refs
    if (surfaceTexture_object) mJEnv->DeleteLocalRef(surfaceTexture_object);
    if (surfaceTexture_class) mJEnv->DeleteLocalRef(surfaceTexture_class);
    // if (attached == 1) JNIContext::Detach_env();
    return SURFACE_CREATE_OK;
}

Surface_ERROR SurfaceTexture::UpdateR2T()
{
    if (mJEnv == nullptr || mJavaObject == nullptr)
    {
        LOG(ERROR) << "Error in initializing java env or SurfaceTexture java object!" << std::endl;
        return SURFACE_CREATE_ERROR;
    }
    mJEnv->CallVoidMethod(mJavaObject, mUpdateTexImageMethodId);
    mPts = mJEnv->CallLongMethod(mJavaObject, mGetTimestampMethodId);
    jfloatArray jfarray = mJEnv->NewFloatArray(16);
    mJEnv->CallVoidMethod(mJavaObject, mGetTransformMatrixId, jfarray);
    jfloat* arrayData = mJEnv->GetFloatArrayElements(jfarray, nullptr);
    if (arrayData == nullptr) return SURFACE_CREATE_ERROR;
    jsize array_length = mJEnv->GetArrayLength(jfarray);
    for (uint32_t i = 0; i < array_length; i++)
    {
        mTransformMatrix[i] = arrayData[i];
    }
    mJEnv->ReleaseFloatArrayElements(jfarray, arrayData, JNI_COMMIT);
    mJEnv->DeleteLocalRef(jfarray);
}

std::vector<float> SurfaceTexture::GetTransformMatrix()
{
    return mTransformMatrix;
}

uint64_t SurfaceTexture::GetPts()
{
    return mPts;
}

GLuint SurfaceTexture::GetTexture()
{
    return mTexture;
}
jobject	SurfaceTexture::GetJavaObject()
{
    return mJavaObject;
}

// Surface

Surface::Surface()
{
    mEnv = nullptr;
    mJobject = nullptr;
    mSurfaceTexture = nullptr;
}

Surface::~Surface()
{
    if (mJobject != nullptr)
    {
        mEnv->DeleteGlobalRef(mJobject);
        mJobject = nullptr;
    }
}

Surface_ERROR Surface::CreateSurface()
{
    mSurfaceTexture = new SurfaceTexture();
    mSurfaceTexture->CreateSurfaceTexture();
    if (mSurfaceTexture == nullptr) return SURFACE_CREATE_ERROR;
    // 1. get jni env
    mEnv = JNIContext::GetJNIEnv();
    // 2. find class
    jclass surface_class = mEnv->FindClass("android/view/Surface");
    if (surface_class == nullptr) return SURFACE_CREATE_ERROR;
    // 3. create Surface from a SurfaceTexture
    const jmethodID surface_constructor = mEnv->GetMethodID(surface_class, "<init>", "(Landroid/graphics/SurfaceTexture;)V");//<init> - construct id
    if (surface_class == nullptr)
    {
        LOG(INFO) << "Surface construct not found!" << std::endl;
        return SURFACE_CREATE_ERROR;
    }
    jobject surfaceTexture_object = mSurfaceTexture->GetJavaObject();
    if (surfaceTexture_object == nullptr) return SURFACE_CREATE_ERROR;
    jobject surface_object = mEnv->NewObject(surface_class, surface_constructor, surfaceTexture_object);
    if (surface_object == nullptr) return SURFACE_CREATE_ERROR;
    // 4. Create a global ref of Surface
    mJobject = mEnv->NewGlobalRef(surface_object);
    if (mJobject == nullptr) return SURFACE_CREATE_ERROR;
    // 5. Delete local refs
    if (surface_object) mEnv->DeleteLocalRef(surface_object);
    if (surface_class) mEnv->DeleteLocalRef(surface_class);
    // if (attached == 1) JNIContext::Detach_env();
    return SURFACE_CREATE_OK;
}

SurfaceTexture* Surface::GetSurfaceTexture()
{
    return mSurfaceTexture;
}

jobject Surface::GetJobject()
{
    return mJobject;
}

VCD_NS_END
#endif // _ANDROID_OS_