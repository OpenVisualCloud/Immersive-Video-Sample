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
//! \brief    Defines class for JNIObjects.
//!

#ifdef _ANDROID_OS_

#ifndef _JNIOBJECTS_H_
#define _JNIOBJECTS_H_

#include <jni.h>
#include <vector>
#include <GLES3/gl3.h>
#include "../../../utils/ns_def.h"

VCD_NS_BEGIN

extern JavaVM* jvm_global;

typedef enum {
    SURFACE_CREATE_OK = 0,
    SURFACE_CREATE_ERROR = -1,
    SURFACE_ERROR = -2,
} Surface_ERROR;

typedef struct JNIPARAMS
{
    JavaVM *javaVM;
    jobject activity;
} JNIParams;

// JNIContext

class JNIContext
{
public:
    JNIContext();
    ~JNIContext();

    static JavaVM* GetJVM();

    static jobject GetActivity();

    static JNIEnv* GetJNIEnv();

    static void Detach_env();

private:
    static JavaVM* mJavaVM;
    static jobject mActivity;
    static JNIEnv* mJEnv;
};

// SurfaceTexture

class SurfaceTexture
{
public:
    SurfaceTexture();
    ~SurfaceTexture();

    Surface_ERROR CreateSurfaceTexture();
    Surface_ERROR UpdateR2T();
    std::vector<float> GetTransformMatrix();
    uint64_t GetPts();

    GLuint GetTexture();
    jobject	GetJavaObject();

private:

    JNIEnv* mJEnv;
    jobject	mJavaObject;

    GLuint mTexture;
    uint64_t mPts;
    std::vector<float> mTransformMatrix;

    jmethodID mUpdateTexImageMethodId;
    jmethodID mGetTimestampMethodId;
    jmethodID mGetTransformMatrixId;

};

//Surface

class Surface
{
public:

    Surface();
    ~Surface();
    Surface_ERROR CreateSurface();
    SurfaceTexture *GetSurfaceTexture();
    jobject GetJobject();

private:

    JNIEnv *mEnv;
    jobject mJobject;
    SurfaceTexture *mSurfaceTexture;
};

VCD_NS_END

#endif
#endif // _JNIOBJECTS_H_