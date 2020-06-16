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

import static com.example.omafdashaccesslibrary.JnaOmafAccess.SourceType.MultiResSource;
import static org.junit.Assert.assertEquals;

/**
 * Instrumented test, which will execute on an Android device.
 *
 * @see <a href="http://d.android.com/tools/testing">Testing documentation</a>
 */
@RunWith(AndroidJUnit4.class)
public class UnitTestOmafDashAccess {
    //set up initial parameters
    private final String url_static = "http://192.168.43.166:8080/4k_small_tiles_30M/Test.mpd";
    private final String url_static_https = "https://192.168.43.166:443/4k_small_tiles_30M/Test.mpd";
    private final String cache_path = "sdcard/Android/data/tmp/";



    @Test
    public void sampleOmafDashAccessAPI() {
        //1. new OmafAccess object
        int source_type = MultiResSource;
        boolean enable_extractor = false;
        OmafAccess omafAccess = new OmafAccess(url_static_https, cache_path, source_type, enable_extractor);
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
        ret = omafAccess.OpenMedia(false, "", ""); // -1
        assertEquals(ret, 0);
        //5. get media information
        JnaOmafAccess.DASHMEDIAINFO info = new JnaOmafAccess.DASHMEDIAINFO();
        ret = omafAccess.GetMediaInfo(info);
        assertEquals(ret, 0);
        try {
            //6. test hevc file output
            int stream_id = 0;
            File file1 = new File(cache_path + "frame300high.h265");
            if (file1.exists()) {
                file1.delete();
            }
            file1.createNewFile();
            FileOutputStream outStream1 = null;
            outStream1 = new FileOutputStream(file1);

            File file2 = new File(cache_path + "frame300low.h265");
            if (file2.exists()) {
                file2.delete();
            }
            file2.createNewFile();
            FileOutputStream outStream2 = null;
            outStream2 = new FileOutputStream(file2);
            //7. get packets
            byte needHeader = 1;
            int frame_count = 0;
            while (frame_count < 300) {
                JnaOmafAccess.DASHPACKET[] dashPackets = new JnaOmafAccess.DASHPACKET[16];
                IntByReference size = new IntByReference();
                LongByReference pts = new LongByReference();
                byte clearbuf = 0;
                ret = omafAccess.GetPacket(stream_id, dashPackets, size, pts, needHeader, clearbuf);
                if (ret == 0 && dashPackets[0].buf != null && dashPackets[0].size != 0 && size.getValue() != 0) {
                    ByteBuffer byteBuffer1 = dashPackets[0].buf.getByteBuffer(0, dashPackets[0].size);
                    byte[] bytes1 = new byte[byteBuffer1.remaining()];
                    byteBuffer1.get(bytes1, 0, bytes1.length);
                    outStream1.write(bytes1);
                    frame_count++;
                }
                if (ret == 0 && dashPackets[1].buf != null && dashPackets[1].size != 0 && size.getValue() != 0) {
                    ByteBuffer byteBuffer2 = dashPackets[1].buf.getByteBuffer(0, dashPackets[1].size);
                    byte[] bytes2 = new byte[byteBuffer2.remaining()];
                    byteBuffer2.get(bytes2, 0, bytes2.length);
                    outStream2.write(bytes2);
                }
            }
            outStream1.close();
            outStream2.close();
            omafAccess.CloseMedia();
            omafAccess.Close();
        }catch (IOException e) {
            e.printStackTrace();
        }
    }

}
