/*
 * Copyright (c) 2019, Intel Corporation
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
//! \file     MediaPlayer_native_lib.h
//! \brief    Define Media Player in Android.
//!

#ifdef _ANDROID_OS_

#ifndef _MEDIAPLAYER_NATIVE_LIB_H_
#define _MEDIAPLAYER_NATIVE_LIB_H_

#include <jni.h>
#include <string>
#include "MediaPlayer_Android.h"

#ifdef __cplusplus
extern "C" {
#endif
jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    VCD::VRVideo::jvm_global = vm;
    JNIEnv* env;

    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK)
    {
        ANDROID_LOGD("Get env failed!");
        return -1;
    }

    return JNI_VERSION_1_6;
}

JNIEXPORT jlong JNICALL
Java_com_vcd_immersive_omafplayer_MediaPlayer_NativeMediaPlayer_Init(JNIEnv *env, jobject thiz) {
    VCD::VRVideo::MediaPlayer_Android* pPlayer = new VCD::VRVideo::MediaPlayer_Android();
    return (jlong)pPlayer;
}

JNIEXPORT jint JNICALL
Java_com_vcd_immersive_omafplayer_MediaPlayer_NativeMediaPlayer_Create(JNIEnv *env, jobject thiz,
                                                                       jlong hdl,
                                                                       jobject config) {
    VCD::VRVideo::MediaPlayer_Android* pPlayer = (VCD::VRVideo::MediaPlayer_Android* )hdl;
    struct RenderConfig render_config;
    // 1. get config class from java
    jclass objclass = (env)->GetObjectClass(config);
    if (objclass == nullptr) return RENDER_ERROR;
    // windowWidth
    jfieldID windowWidthID = (env)->GetFieldID(objclass, "windowWidth", "I");
    jint windowWidth = (int)(env)->GetIntField(config, windowWidthID);
    render_config.windowWidth = windowWidth;
    // windowHeight
    jfieldID windowHeightID = (env)->GetFieldID(objclass, "windowHeight", "I");
    jint windowHeight = (int)(env)->GetIntField(config, windowHeightID);
    render_config.windowHeight = windowHeight;
    // url
    jfieldID urlID = (env)->GetFieldID(objclass, "url", "Ljava/lang/String;");
    jstring jstr = (jstring)(env)->GetObjectField(config, urlID);
    const char *url = (env)->GetStringUTFChars(jstr, NULL);
    render_config.url = (char *)url;
    // sourceType
    jfieldID sourceTypeID = (env)->GetFieldID(objclass, "sourceType", "I");
    jint sourceType = (int)(env)->GetIntField(config, sourceTypeID);
    render_config.sourceType = sourceType;
    // viewportHFOV
    jfieldID viewportHFOVID = (env)->GetFieldID(objclass, "viewportHFOV", "I");
    jint viewportHFOV = (int)(env)->GetIntField(config, viewportHFOVID);
    render_config.viewportHFOV = viewportHFOV;
    // viewportVFOV
    jfieldID viewportVFOVID = (env)->GetFieldID(objclass, "viewportVFOV", "I");
    jint viewportVFOV = (int)(env)->GetIntField(config, viewportVFOVID);
    render_config.viewportVFOV = viewportVFOV;
    // viewportWidth
    jfieldID viewportWidthID = (env)->GetFieldID(objclass, "viewportWidth", "I");
    jint viewportWidth = (int)(env)->GetIntField(config, viewportWidthID);
    render_config.viewportWidth = viewportWidth;
    // viewportHeight
    jfieldID viewportHeightID = (env)->GetFieldID(objclass, "viewportHeight", "I");
    jint viewportHeight = (int)(env)->GetIntField(config, viewportHeightID);
    render_config.viewportHeight = viewportHeight;
    // cachePath
    jfieldID cachePathID = (env)->GetFieldID(objclass, "cachePath", "Ljava/lang/String;");
    jstring jstr1 = (jstring)(env)->GetObjectField(config, cachePathID);
    const char *cachePath = (env)->GetStringUTFChars(jstr1, NULL);
    render_config.cachePath = (char *)cachePath;
    // enableExtractor
    jfieldID enableExtractorID = (env)->GetFieldID(objclass, "enableExtractor", "Z");
    bool enableExtractor = (bool)(env)->GetBooleanField(config, enableExtractorID);
    render_config.enableExtractor = enableExtractor;
    // enablePredictor
    jfieldID enablePredictorID = (env)->GetFieldID(objclass, "enablePredictor", "Z");
    bool enablePredictor = (bool)(env)->GetBooleanField(config, enablePredictorID);
    render_config.enablePredictor = enablePredictor;
    // predictPluginName
    jfieldID predictPluginNameID = (env)->GetFieldID(objclass, "predictPluginName", "Ljava/lang/String;");
    jstring jstr2 = (jstring)(env)->GetObjectField(config, predictPluginNameID);
    const char *predictPluginName = (env)->GetStringUTFChars(jstr2, NULL);
    render_config.predictPluginName = (char *)predictPluginName;
    // libPath
    jfieldID libPathID = (env)->GetFieldID(objclass, "libPath", "Ljava/lang/String;");
    jstring jstr3 = (jstring)(env)->GetObjectField(config, libPathID);
    const char *libPath = (env)->GetStringUTFChars(jstr3, NULL);
    render_config.libPath = (char *)libPath;
    // projFormat
    jfieldID projFormatID = (env)->GetFieldID(objclass, "projFormat", "I");
    jint projFormat = (int)(env)->GetIntField(config, projFormatID);
    render_config.projFormat = 0;
    // renderInterval
    jfieldID renderIntervalID = (env)->GetFieldID(objclass, "renderInterval", "I");
    jint renderInterval = (int)(env)->GetIntField(config, renderIntervalID);
    render_config.renderInterval = renderInterval;
    // minLogLevel
    jfieldID minLogLevelID = (env)->GetFieldID(objclass, "minLogLevel", "I");
    jint minLogLevel = (int)(env)->GetIntField(config, minLogLevelID);
    render_config.minLogLevel = minLogLevel;
    // maxVideoDecodeWidth
    jfieldID maxVideoDecodeWidthID = (env)->GetFieldID(objclass, "maxVideoDecodeWidth", "I");
    jint maxVideoDecodeWidth = (int)(env)->GetIntField(config, maxVideoDecodeWidthID);
    render_config.maxVideoDecodeWidth = maxVideoDecodeWidth;
    // maxVideoDecodeHeight
    jfieldID maxVideoDecodeHeightID = (env)->GetFieldID(objclass, "maxVideoDecodeHeight", "I");
    jint maxVideoDecodeHeight = (int)(env)->GetIntField(config, maxVideoDecodeHeightID);
    render_config.maxVideoDecodeHeight = maxVideoDecodeHeight;
    // enableCatchup
    jfieldID enableCatchupID = (env)->GetFieldID(objclass, "enableCatchup","Z");
    bool enableCatchup = (bool)(env)->GetBooleanField(config, enableCatchupID);
    render_config.enableInTimeViewportUpdate = enableCatchup;
    // responseTimesInOneSeg
    jfieldID responseTimesInOneSegID = (env)->GetFieldID(objclass, "responseTimesInOneSeg", "I");
    jint responseTimesInOneSeg = (int)(env)->GetIntField(config, responseTimesInOneSegID);
    render_config.maxResponseTimesInOneSeg = enableCatchup ? responseTimesInOneSeg : 0;
    // maxCatchupWidth
    jfieldID maxCatchupWidthID = (env)->GetFieldID(objclass, "maxCatchupWidth", "I");
    jint maxCatchupWidth = (int)(env)->GetIntField(config, maxCatchupWidthID);
    render_config.maxCatchupWidth = enableCatchup ? maxCatchupWidth : 0;
    // maxCatchupHeight
    jfieldID maxCatchupHeightID = (env)->GetFieldID(objclass, "maxCatchupHeight", "I");
    jint maxCatchupHeight = (int)(env)->GetIntField(config, maxCatchupHeightID);
    render_config.maxCatchupHeight = enableCatchup ? maxCatchupHeight : 0;
    return (jint)pPlayer->Create(render_config);
}
JNIEXPORT jint JNICALL
Java_com_vcd_immersive_omafplayer_MediaPlayer_NativeMediaPlayer_Play(JNIEnv *env, jobject thiz,
                                                                      jlong hdl) {
    VCD::VRVideo::MediaPlayer_Android* pPlayer = (VCD::VRVideo::MediaPlayer_Android* )hdl;
    return (jint)pPlayer->Play();
}
JNIEXPORT jint JNICALL
Java_com_vcd_immersive_omafplayer_MediaPlayer_NativeMediaPlayer_Pause(JNIEnv *env, jobject thiz,
                                                                      jlong hdl) {
    VCD::VRVideo::MediaPlayer_Android* pPlayer = (VCD::VRVideo::MediaPlayer_Android* )hdl;
    return (jint)pPlayer->Pause();
}
JNIEXPORT jint JNICALL
Java_com_vcd_immersive_omafplayer_MediaPlayer_NativeMediaPlayer_Resume(JNIEnv *env, jobject thiz,
                                                                       jlong hdl) {
    VCD::VRVideo::MediaPlayer_Android* pPlayer = (VCD::VRVideo::MediaPlayer_Android* )hdl;
    return (jint)pPlayer->Resume();
}
JNIEXPORT jint JNICALL
Java_com_vcd_immersive_omafplayer_MediaPlayer_NativeMediaPlayer_Stop(JNIEnv *env, jobject thiz,
                                                                     jlong hdl) {
    VCD::VRVideo::MediaPlayer_Android* pPlayer = (VCD::VRVideo::MediaPlayer_Android* )hdl;
    return (jint)pPlayer->Stop();
}
JNIEXPORT jint JNICALL
Java_com_vcd_immersive_omafplayer_MediaPlayer_NativeMediaPlayer_Seek(JNIEnv *env, jobject thiz,
                                                                     jlong hdl) {
    VCD::VRVideo::MediaPlayer_Android* pPlayer = (VCD::VRVideo::MediaPlayer_Android* )hdl;
    return (jint)pPlayer->Seek();
}
JNIEXPORT jint JNICALL
Java_com_vcd_immersive_omafplayer_MediaPlayer_NativeMediaPlayer_Start(JNIEnv *env, jobject thiz,
                                                                      jlong hdl) {
    VCD::VRVideo::MediaPlayer_Android* pPlayer = (VCD::VRVideo::MediaPlayer_Android* )hdl;
    return (jint)pPlayer->Start();
}
JNIEXPORT jint JNICALL
Java_com_vcd_immersive_omafplayer_MediaPlayer_NativeMediaPlayer_Close(JNIEnv *env, jobject thiz,
                                                                      jlong hdl) {
    VCD::VRVideo::MediaPlayer_Android* pPlayer = (VCD::VRVideo::MediaPlayer_Android* )hdl;
    return (jint)pPlayer->Close();
}
JNIEXPORT jint JNICALL
Java_com_vcd_immersive_omafplayer_MediaPlayer_NativeMediaPlayer_GetStatus(JNIEnv *env, jobject thiz,
                                                                          jlong hdl) {
    VCD::VRVideo::MediaPlayer_Android* pPlayer = (VCD::VRVideo::MediaPlayer_Android* )hdl;
    return (jint)pPlayer->GetStatus();
}
JNIEXPORT jboolean JNICALL
Java_com_vcd_immersive_omafplayer_MediaPlayer_NativeMediaPlayer_IsPlaying(JNIEnv *env, jobject thiz,
                                                                          jlong hdl) {
    VCD::VRVideo::MediaPlayer_Android* pPlayer = (VCD::VRVideo::MediaPlayer_Android* )hdl;
    return (jboolean)pPlayer->IsPlaying();
}

JNIEXPORT void JNICALL
Java_com_vcd_immersive_omafplayer_MediaPlayer_NativeMediaPlayer_SetCurrentPosition(JNIEnv *env,
                                                                                   jobject thiz,
                                                                                   jlong hdl, jobject pose) {
    VCD::VRVideo::MediaPlayer_Android* pPlayer = (VCD::VRVideo::MediaPlayer_Android* )hdl;
    jclass objclass = (env)->GetObjectClass(pose);
    if (objclass == nullptr) return;

    jfieldID yawID = (env)->GetFieldID(objclass, "yaw", "F");
    jfloat yaw = (float)(env)->GetFloatField(pose, yawID);

    jfieldID pitchID = (env)->GetFieldID(objclass, "pitch", "F");
    jfloat pitch = (float)(env)->GetFloatField(pose, pitchID);

    jfieldID ptsID = (env)->GetFieldID(objclass, "pts", "I");
    jlong pts = (float)(env)->GetIntField(pose, ptsID);

    HeadPose head_pose;
    head_pose.yaw = yaw;
    head_pose.pitch = pitch;
    head_pose.pts = pts;
    pPlayer->SetCurrentPosition(head_pose);
}

JNIEXPORT jint JNICALL
Java_com_vcd_immersive_omafplayer_MediaPlayer_NativeMediaPlayer_GetWidth(JNIEnv *env, jobject thiz,
                                                                         jlong hdl) {
    VCD::VRVideo::MediaPlayer_Android* pPlayer = (VCD::VRVideo::MediaPlayer_Android* )hdl;
    return (jint)pPlayer->GetWidth();
}
JNIEXPORT jint JNICALL
Java_com_vcd_immersive_omafplayer_MediaPlayer_NativeMediaPlayer_GetHeight(JNIEnv *env, jobject thiz,
                                                                          jlong hdl) {
    VCD::VRVideo::MediaPlayer_Android* pPlayer = (VCD::VRVideo::MediaPlayer_Android* )hdl;
    return (jint)pPlayer->GetHeight();
}

JNIEXPORT jint JNICALL
Java_com_vcd_immersive_omafplayer_MediaPlayer_NativeMediaPlayer_GetProjectionFormat(JNIEnv *env, jobject thiz,
                                                                          jlong hdl) {
    VCD::VRVideo::MediaPlayer_Android* pPlayer = (VCD::VRVideo::MediaPlayer_Android* )hdl;
    return (jint)pPlayer->GetProjectionFormat();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_vcd_immersive_omafplayer_MediaPlayer_NativeMediaPlayer_SetDecodeSurface(JNIEnv *env,
                                                                                  jobject thiz,
                                                                                  jlong hdl,
                                                                                  jobject decode_surface,
                                                                                  jint tex_id,
                                                                                  jint video_id) {
    VCD::VRVideo::MediaPlayer_Android* pPlayer = (VCD::VRVideo::MediaPlayer_Android* )hdl;
    // ANDROID_LOGD("set decode surface %p and tex id %d", (env)->NewGlobalRef(decode_surface), tex_id);
    pPlayer->SetDecodeSurface((env)->NewGlobalRef(decode_surface), tex_id, video_id);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_vcd_immersive_omafplayer_MediaPlayer_NativeMediaPlayer_SetDisplaySurface(JNIEnv *env,
                                                                                  jobject thiz,
                                                                                  jlong hdl,
                                                                                  jint tex_id) {
    VCD::VRVideo::MediaPlayer_Android* pPlayer = (VCD::VRVideo::MediaPlayer_Android* )hdl;
    // ANDROID_LOGD("set display tex id %d", tex_id);
    pPlayer->SetDisplaySurface(tex_id);
}

extern "C"
JNIEXPORT int JNICALL
Java_com_vcd_immersive_omafplayer_MediaPlayer_NativeMediaPlayer_UpdateDisplayTex(JNIEnv *env,
                                                                                 jobject thiz,
                                                                                 jlong hdl,
                                                                                 jint render_count) {
    VCD::VRVideo::MediaPlayer_Android* pPlayer = (VCD::VRVideo::MediaPlayer_Android* )hdl;
    return pPlayer->UpdateDisplayTex(render_count);
}

JNIEXPORT jintArray JNICALL
Java_com_vcd_immersive_omafplayer_MediaPlayer_NativeMediaPlayer_GetTransformType(JNIEnv *env,
                                                                                 jobject thiz,
                                                                                 jlong hdl) {
    VCD::VRVideo::MediaPlayer_Android* pPlayer = (VCD::VRVideo::MediaPlayer_Android* )hdl;
    jintArray transform_type_arr;
    int face_num = 6;
    transform_type_arr = env->NewIntArray(face_num);
    int* data = pPlayer->GetTransformType();
    env->SetIntArrayRegion(transform_type_arr, 0, face_num, data);
    SAFE_DELETE(data);
    return transform_type_arr;
}

#ifdef __cplusplus
}
#endif

#endif
#endif