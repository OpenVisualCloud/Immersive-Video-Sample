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
package com.example.myapplication;

import androidx.test.ext.junit.runners.AndroidJUnit4;

import com.example.omafdashaccesslibrary.JnaOmafAccess;
import com.example.omafdashaccesslibrary.OmafAccess;
import com.sun.jna.ptr.IntByReference;
import com.sun.jna.ptr.LongByReference;

import org.junit.Test;
import org.junit.runner.RunWith;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

import static org.junit.Assert.assertEquals;

/**
 * Instrumented test, which will execute on an Android device.
 *
 * @see <a href="http://d.android.com/tools/testing">Testing documentation</a>
 */
@RunWith(AndroidJUnit4.class)
public class UnitTestOmafDashAccess {
    //set up initial parameters
    private final String url_static = "http://192.168.43.166:8080/testOMAFstatic/Test.mpd";
    private final String url_static_https = "https://192.168.43.166:443/testOMAFstatic/Test.mpd";
    private final String cache_path = "sdcard/Android/data/tmp/";



    @Test
    public void sampleOmafDashAccessAPI() {
        //1. new OmafAccess object
        OmafAccess omafAccess = new OmafAccess(url_static_https, cache_path);
        //2. initialize
        int ret = 0;
        ret = omafAccess.Initialize();
        assertEquals(ret, 0);
        //3. set headinfo
        JnaOmafAccess.HEADPOSE.ByReference pose = new JnaOmafAccess.HEADPOSE.ByReference();
        JnaOmafAccess.HEADSETINFO clientInfo = new JnaOmafAccess.HEADSETINFO(0, 2, pose, 80, 80, 960, 960);
        ret = omafAccess.SetupHeadSetInfo(clientInfo);
        assertEquals(ret, 0);
        //4. open media
        ret = omafAccess.OpenMedia(false); // -1
        assertEquals(ret, 0);
        //5. get media information
        JnaOmafAccess.DASHMEDIAINFO info = new JnaOmafAccess.DASHMEDIAINFO();
        ret = omafAccess.GetMediaInfo(info);
        assertEquals(ret, 0);
        try {
            Thread.sleep(4000);
            //6. test hevc file output
            int stream_id = 0;
            File file = new File(cache_path + "frame100.h265");
            if (file.exists()) {
                file.delete();
            }
            file.createNewFile();
            FileOutputStream outStream = null;
            outStream = new FileOutputStream(file);
            //7. get packets
            byte needHeader = 1;
            for (int i = 0; i < 100; i++) {
                JnaOmafAccess.DASHPACKET.ByReference dashPackets = new JnaOmafAccess.DASHPACKET.ByReference();
                IntByReference size = new IntByReference();
                LongByReference pts = new LongByReference();
                byte clearbuf = 0;
                ret = omafAccess.GetPacket(stream_id, dashPackets, size, pts, needHeader, clearbuf);
                assertEquals(ret, 0);
                ByteBuffer byteBuffer = dashPackets.buf.getByteBuffer(0, dashPackets.size);
                byte[] bytes = new byte[byteBuffer.remaining()];
                byteBuffer.get(bytes, 0, bytes.length);
                if (dashPackets.buf != null && dashPackets.size != 0 && size.getValue() != 0) {
                    outStream.write(bytes);
                    if (needHeader == 1) {
                        needHeader = 0;
                    }
                }
            }
            outStream.close();
            omafAccess.CloseMedia();
            omafAccess.Close();
        }catch (IOException e) {
            e.printStackTrace();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

}
