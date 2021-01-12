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
 */
package com.vcd.immersive.omafplayer.MediaPlayer;
import android.content.Context;
import android.content.res.AssetManager;
import android.util.Log;
import android.view.Surface;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedReader;

import java.io.IOException;

import java.io.InputStreamReader;
import java.io.UnsupportedEncodingException;


public class NativeMediaPlayer {
    private final String TAG = "NATIVE_MEDIA_PLAYER";
    private long mHandler;
    private RenderConfig mConfig;
    private int status = 0;
    public final int MAX_DECODE_WIDTH = 4096;
    public final int MAX_DECODE_HEIGHT = 2304;
    private Context context;

    static {
        System.loadLibrary("MediaPlayer");
    }
    // input parameters
    public class RenderConfig {
        public int windowWidth;
        public int windowHeight;
        public String url;
        public int sourceType;
        public int viewportHFOV;
        public int viewportVFOV;
        public int viewportWidth;
        public int viewportHeight;
        public String cachePath;
        public boolean enableExtractor;
        public boolean enablePredictor;
        public String predictPluginName;
        public String libPath;
        public int projFormat;
        public int renderInterval;
        public int minLogLevel;
        public int maxVideoDecodeWidth;
        public int maxVideoDecodeHeight;

        public RenderConfig() {
            windowWidth = 0;
            windowHeight = 0;
            url = "";
            sourceType = 0;
            viewportHFOV = 0;
            viewportVFOV = 0;
            viewportWidth = 0;
            viewportHeight = 0;
            cachePath = "";
            enableExtractor = false;
            enablePredictor = false;
            predictPluginName = "";
            libPath = "";
            projFormat = 0;
            renderInterval = 0;
            minLogLevel = 0;
            maxVideoDecodeWidth = 0;
            maxVideoDecodeHeight = 0;
        }
    }

    public static class HeadPose {
        public float yaw;
        public float pitch;
        public long pts;

        public HeadPose() {
            yaw = 0;
            pitch = 0;
            pts = 0;
        }
    }

    public int GetCurrentStatus()
    {
        return status;
    }

    public void SetCurrentStatus(int st)
    {
        status = st;
    }
    /**
     * Original signature : <code>Handler Init()</code><br>
     * <i>native declaration : line 82</i>
     */
    public native long Init();
    /**
     * Original signature : <code>int Create(Handler, RenderConfig)</code><br>
     * <i>native declaration : line 84</i>
     */
    public native int Create(long hdl, RenderConfig config);
    /**
     * Original signature : <code>int Play(Handler)</code><br>
     * <i>native declaration : line 86</i>
     */
    public native int Play(long hdl);
    /**
     * Original signature : <code>int Pause(Handler)</code><br>
     * <i>native declaration : line 88</i>
     */
    public native int Pause(long hdl);
    /**
     * Original signature : <code>int Resume(Handler)</code><br>
     * <i>native declaration : line 90</i>
     */
    public native int Resume(long hdl);
    /**
     * Original signature : <code>int Stop(Handler)</code><br>
     * <i>native declaration : line 92</i>
     */
    public native int Stop(long hdl);
    /**
     * Original signature : <code>int Seek(Handler)</code><br>
     * <i>native declaration : line 94</i>
     */
    public native int Seek(long hdl);
    /**
     * Original signature : <code>int Start(Handler, void*)</code><br>
     * <i>native declaration : line 96</i>
     */
    public native int Start(long hdl);
    /**
     * Original signature : <code>int Close(Handler)</code><br>
     * <i>native declaration : line 98</i>
     */
    public native int Close(long hdl);
    /**
     * Original signature : <code>int GetStatus(Handler)</code><br>
     * <i>native declaration : line 100</i>
     */
    public native int GetStatus(long hdl);
    /**
     * Original signature : <code>bool IsPlaying(Handler)</code><br>
     * <i>native declaration : line 102</i>
     */
    public native boolean IsPlaying(long hdl);
    /**
     * Original signature : <code>HeadPose GetCurrentPosition(Handler)</code><br>
     * <i>native declaration : line 104</i>
     */
    public native void SetCurrentPosition(long hdl, HeadPose pose);
    /**
     * Original signature : <code>uint32_t GetWidth(Handler)</code><br>
     * <i>native declaration : line 106</i>
     */
    public native int GetWidth(long hdl);
    /**
     * Original signature : <code>uint32_t GetHeight(Handler)</code><br>
     * <i>native declaration : line 108</i>
     */
    public native int GetHeight(long hdl);
    /**
     * Original signature : <code>uint32_t GetProjectionFormat(Handler)</code><br>
     * <i>native declaration : line 108</i>
     */
    public native int GetProjectionFormat(long hdl);
    /**
     * Original signature : <code>void SetDecodeSurface(Handler, Surface, int, int)</code><br>
     * <i>native declaration : line 110</i>
     */
    public native void SetDecodeSurface(long hdl, Surface decodeSurface, int texId, int video_id);
    /**
     * Original signature : <code>void SetDisplaySurface(Handler, int)</code><br>
     * <i>native declaration : line 110</i>
     */
    public native void SetDisplaySurface(long hdl, int texId);
    /**
     * Original signature : <code>int UpdateDisplayTex(Handler, int)</code><br>
     * <i>native declaration : line 110</i>
     */
    public native int UpdateDisplayTex(long hdl, int render_count);
    /**
     * Original signature : <code>int GetTransformType(Handler)</code><br>
     * <i>native declaration : line 110</i>
     */
    public native int[] GetTransformType(long hdl);

    public NativeMediaPlayer(Context text)
    {
        mHandler = 0;
        mConfig = null;
        context = text;
        ParseXmlConfig();
    }

    public int Initialize()
    {
        mHandler = Init();//native

        if (mHandler == 0)
        {
            Log.e(TAG, "Failed to init jni media player!");
            return -1;
        }
        return 0;
    }

    private String GetCfgJson()
    {
        StringBuilder stringBuilder = new StringBuilder();
        AssetManager assetManager = context.getAssets();

        try {
            BufferedReader bufferedReader = new BufferedReader(new InputStreamReader(
                    assetManager.open("cfg.json"), "utf-8"
            ));
            String jsonLines;
            while ((jsonLines = bufferedReader.readLine()) != null) {
                stringBuilder.append(jsonLines);
            }
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
        return stringBuilder.toString();
    }

    private int ParseXmlConfig()
    {
        String cfgJsonStr = GetCfgJson();
        JSONObject cfgJsonObject = null;
        try {
            cfgJsonObject = new JSONObject(cfgJsonStr);
        } catch (JSONException e) {
            e.printStackTrace();
        }
        mConfig = new RenderConfig();
        try {
            mConfig.windowHeight = cfgJsonObject.getInt("windowHeight");
            mConfig.windowWidth = cfgJsonObject.getInt("windowWidth");
            mConfig.url = cfgJsonObject.getString("url");
            mConfig.sourceType = cfgJsonObject.getInt("sourceType");
            mConfig.viewportHFOV = cfgJsonObject.getInt("viewportHFOV");
            mConfig.viewportVFOV = cfgJsonObject.getInt("viewportVFOV");
            mConfig.viewportWidth = cfgJsonObject.getInt("viewportWidth");
            mConfig.viewportHeight = cfgJsonObject.getInt("viewportHeight");

            mConfig.cachePath = cfgJsonObject.getString("cachePath");
            mConfig.enableExtractor = cfgJsonObject.getBoolean("enableExtractor");
            String predictStr = cfgJsonObject.getString("predict");
            JSONObject predictObj = new JSONObject(predictStr);
            mConfig.enablePredictor = predictObj.getBoolean("enable");
            mConfig.predictPluginName = predictObj.getString("name");
            mConfig.libPath = predictObj.getString("path");
            mConfig.maxVideoDecodeWidth = cfgJsonObject.getInt("maxVideoDecodeWidth");
            mConfig.maxVideoDecodeHeight = cfgJsonObject.getInt("maxVideoDecodeHeight");
        } catch (JSONException e) {
            e.printStackTrace();
        }
        return 0;
    }

    public int Create(String config_url)
    {
        if (mHandler == 0)
        {
            Log.e(TAG, "Native media player is invalid!");
            return -1;
        }
        return Create(mHandler, mConfig);
    }

    public int Play()
    {
        if (mHandler == 0)
        {
            Log.e(TAG, "Native media player is invalid!");
            return -1;
        }
        return Play(mHandler);
    }

    public int Pause()
    {
        if (mHandler == 0)
        {
            Log.e(TAG, "Native media player is invalid!");
            return -1;
        }
        return Pause(mHandler);
    }

    public int Resume()
    {
        if (mHandler == 0)
        {
            Log.e(TAG, "Native media player is invalid!");
            return -1;
        }
        return Resume(mHandler);
    }

    public int Stop()
    {
        if (mHandler == 0)
        {
            Log.e(TAG, "Native media player is invalid!");
            return -1;
        }
        return Stop(mHandler);
    }

    public int Seek()
    {
        if (mHandler == 0)
        {
            Log.e(TAG, "Native media player is invalid!");
            return -1;
        }
        return Seek(mHandler);
    }

    public int Start()
    {
        if (mHandler == 0)
        {
            Log.e(TAG, "Native media player is invalid!");
            return -1;
        }
        return Start(mHandler);
    }

    public int Close()
    {
        if (mHandler == 0)
        {
            Log.e(TAG, "Native media player is invalid!");
            return -1;
        }
        return Close(mHandler);
    }

    public int GetStatus()
    {
        if (mHandler == 0)
        {
            Log.e(TAG, "Native media player is invalid!");
            return -1;
        }
        return GetStatus(mHandler);
    }

    public boolean IsPlaying()
    {
        if (mHandler == 0)
        {
            Log.e(TAG, "Native media player is invalid!");
            return false;
        }
        return IsPlaying(mHandler);
    }

    public void SetCurrentPosition(HeadPose pose)
    {
        if (mHandler == 0)
        {
            Log.e(TAG, "Native media player is invalid!");
            return;
        }
        Log.i(TAG, "native player pose yaw : " + pose.yaw + " pose pitch : " + pose.pitch);
        SetCurrentPosition(mHandler, pose);
    }

    public int GetWidth()
    {
        if (mHandler == 0)
        {
            Log.e(TAG, "Native media player is invalid!");
            return -1;
        }
        return GetWidth(mHandler);
    }

    public int GetHeight()
    {
        if (mHandler == 0)
        {
            Log.e(TAG, "Native media player is invalid!");
            return -1;
        }
        return GetHeight(mHandler);
    }

    public int GetProjectionFormat()
    {
        if (mHandler == 0)
        {
            Log.e(TAG, "Native media player is invalid!");
            return -1;
        }
        return GetProjectionFormat(mHandler);
    }

    public void SetDecodeSurface(Surface surface, int texId, int video_id)
    {
        if (mHandler == 0)
        {
            Log.e(TAG, "Native media player is invalid!");
            return;
        }
        SetDecodeSurface(mHandler, surface, texId, video_id);
    }

    public void SetDisplaySurface(int texId)
    {
        if (mHandler == 0)
        {
            Log.e(TAG, "Native media player is invalid!");
            return;
        }
        SetDisplaySurface(mHandler, texId);
    }

    public int UpdateDisplayTex(int count)
    {
        if (mHandler == 0)
        {
            Log.e(TAG, "Native media player is invalid!");
            return -1;
        }
        return UpdateDisplayTex(mHandler, count);
    }

    public int[] GetTransformType()
    {
        if (mHandler == 0)
        {
            Log.e(TAG, "Native media player is invalid!");
            return null;
        }
        return GetTransformType(mHandler);
    }
}