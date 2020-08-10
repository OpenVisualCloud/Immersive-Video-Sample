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

package com.example.omafdashaccesslibrary;

import android.util.Log;
import com.sun.jna.Pointer;
import com.sun.jna.Memory;
import com.sun.jna.ptr.IntByReference;
import com.sun.jna.ptr.LongByReference;

import java.nio.IntBuffer;
import java.nio.LongBuffer;

public class OmafAccess {
    private final String TAG = "OMAF_DASH_ACCESS";
    private Pointer mHandle;
    private JnaOmafAccess.DASHSTREAMINGCLIENT mContext;

    public OmafAccess(){
        mContext = new JnaOmafAccess.DASHSTREAMINGCLIENT();
        mHandle = null;
    }

    public OmafAccess(String url, String cache, int source_type, boolean enable_extractor, JnaOmafAccess._omafDashParams.ByValue omaf_params){
        this();
        SetClientCtx(url, cache, source_type, enable_extractor, omaf_params);
    }

    private void SetClientCtx(String url, String cache, int source_type, boolean enable_extractor, JnaOmafAccess._omafDashParams.ByValue omaf_params){
        mContext.media_url = url;
        mContext.cache_path = cache;
        mContext.source_type = source_type;
        mContext.enable_extractor = enable_extractor;
        mContext.omaf_params = omaf_params;
    }

    public int Initialize( ){
        mHandle = JnaOmafAccess.INSTANCE.OmafAccess_Init(this.mContext);

        if(mHandle == null){
            Log.e(TAG, "Failed to initialize dash access !!!");
            return -1;
        }

        return 0;
    }

    public int OpenMedia(boolean enablePredictor, String pluginName, String libPath){
        if(mHandle == null){
            Log.e(TAG, "Omaf Access Handle is NULL; cannot continue !!!");
            return -1;
        }

        return JnaOmafAccess.INSTANCE.OmafAccess_OpenMedia(this.mHandle, this.mContext, enablePredictor, pluginName, libPath);
    }

    public int SeekMedia( long time){
        if(mHandle == null){
            Log.e(TAG, "Omaf Access Handle is NULL; cannot continue !!!");
            return -1;
        }

        return JnaOmafAccess.INSTANCE.OmafAccess_SeekMedia( this.mHandle, time );
    }

    public int CloseMedia(){
        if(mHandle == null){
            Log.e(TAG, "Omaf Access Handle is NULL; cannot continue !!!");
            return -1;
        }

        return JnaOmafAccess.INSTANCE.OmafAccess_CloseMedia(this.mHandle);
    }

    public int GetMediaInfo( JnaOmafAccess.DASHMEDIAINFO info){
        if(mHandle == null){
            Log.e(TAG, "Omaf Access Handle is NULL; cannot continue !!!");
            return -1;
        }
        return JnaOmafAccess.INSTANCE.OmafAccess_GetMediaInfo(this.mHandle, info);
    }

    public int GetPacket(int stream_id, JnaOmafAccess.DASHPACKET[] packet, IntByReference size, LongByReference pts, byte needParams, byte clearBuf){
        if(mHandle == null){
            Log.e(TAG, "Omaf Access Handle is NULL; cannot continue !!!");
            return -1;
        }
        return JnaOmafAccess.INSTANCE.OmafAccess_GetPacket(this.mHandle, stream_id, packet, size, pts, needParams, clearBuf);
    }

    public int SetupHeadSetInfo( JnaOmafAccess.HEADSETINFO clientInfo){
        if(mHandle == null){
            Log.e(TAG, "Omaf Access Handle is NULL; cannot continue !!!");
            return -1;
        }
        return JnaOmafAccess.INSTANCE.OmafAccess_SetupHeadSetInfo(this.mHandle, clientInfo);
    }

    public int ChangeViewport( JnaOmafAccess.HEADPOSE pose){
        if(mHandle == null){
            Log.e(TAG, "Omaf Access Handle is NULL; cannot continue !!!");
            return -1;
        }
        return JnaOmafAccess.INSTANCE.OmafAccess_ChangeViewport(this.mHandle, pose);
    }

    public int Statistic( JnaOmafAccess.DASHSTATISTICINFO info){
        if(mHandle == null){
            Log.e(TAG, "Omaf Access Handle is NULL; cannot continue !!!");
            return -1;
        }
        return JnaOmafAccess.INSTANCE.OmafAccess_Statistic(this.mHandle, info);
    }

    public int Close(){
        if(mHandle == null){
            Log.e(TAG, "Omaf Access Handle is NULL; cannot continue !!!");
            return -1;
        }
        return JnaOmafAccess.INSTANCE.OmafAccess_Close(this.mHandle);
    }

}
