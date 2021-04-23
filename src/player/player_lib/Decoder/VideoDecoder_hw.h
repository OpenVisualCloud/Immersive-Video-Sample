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
//! \file     VideoDecoder_hw.h
//! \brief    Defines class for VideoDecoder_hw derived from MediaDecoder.
//!

#ifdef _ANDROID_OS_

#ifndef _VIDEODEOCODER_HW_H__
#define _VIDEODEOCODER_HW_H__

#include "MediaDecoder.h"
#include "../Common/JNIObjects.h"
#include "../../../utils/Threadable.h"
#include <list>

#define MAX_FRAME_SIZE 60

VCD_NS_BEGIN

typedef struct PacketInfo{
     DashPacket *pkt;
     bool      bCodecChange;
     bool      bEOS;
     uint64_t  pts;
     uint32_t  video_id;
     bool      bCatchup;
}PacketInfo;

typedef struct DecodedFrame{
     OutputSurface      *output_surface;
     RegionWisePacking  *rwpk;
     uint64_t           pts;
     bool               bFmtChange;
     int32_t            numQuality;
     SourceResolution   *qtyResolution;
     uint32_t           video_id;
     bool               bEOS;
     bool               bCatchup;
}DecodedFrame;

typedef struct FrameData{
     uint64_t           pts;
     RegionWisePacking* rwpk;
     int32_t            numQuality;
     SourceResolution*  qtyResolution;
     bool               bCodecChange;
     uint32_t           video_id;
     bool               bCatchup;
}FrameData;

class DecoderContext
{
public:
     DecoderContext()
     {
         height         = 0;
         width          = 0;
         numQuality     = 0;
         tileRowNum     = 0;
         tileColNum     = 0;
         listPacket.clear();
         listFrameData.clear();
         bPacketEOS     = false;
         mMediaCodec    = nullptr;
         mMediaFormat   = nullptr;
         mOutputSurface = nullptr;
     };
     ~DecoderContext(){
          while(get_size_of_packet()>0){
               PacketInfo* pktInfo = listPacket.front();
               listPacket.pop_front();
               SAFE_DELETE(pktInfo->pkt);
               SAFE_DELETE(pktInfo);
          }

          while(get_size_of_framedata()>0){
               FrameData* data = listFrameData.front();
               listFrameData.pop_front();
               SAFE_DELETE_ARRAY(data->rwpk->rectRegionPacking);
               SAFE_DELETE_ARRAY(data->qtyResolution);
               SAFE_DELETE(data->rwpk);
               SAFE_DELETE(data);
          }
     };

     void push_packet(PacketInfo* pktInfo)
     {
          ScopeLock lock(PacketLock);
          listPacket.push_back(pktInfo);
     };

     void push_framedata(FrameData* data)
     {
          ScopeLock lock(DataLock);
          listFrameData.push_back(data);
     };

     void push_frame(DecodedFrame* frame)
     {
          ScopeLock lock(FrameLock);
          listFrame.push_back(frame);
     }

     PacketInfo* pop_packet()
     {
          ScopeLock lock(PacketLock);
          PacketInfo* pkt = listPacket.front();
          if (!listPacket.empty())
               listPacket.pop_front();
          return pkt;
     };

     FrameData* pop_framedata()
     {
          ScopeLock lock(DataLock);
          FrameData* data = listFrameData.front();
          if (!listFrameData.empty())
               listFrameData.pop_front();
          return data;
     };

     DecodedFrame* pop_frame()
     {
          ScopeLock lock(FrameLock);
          DecodedFrame* frame = listFrame.front();
          if (!listFrame.empty())
               listFrame.pop_front();
          return frame;
     };

     DecodedFrame* get_front_of_frame()
     {
          ScopeLock lock(FrameLock);
          DecodedFrame* frame = listFrame.front();
          return frame;
     };

     FrameData* get_front_of_framedata()
     {
          ScopeLock lock(DataLock);
          FrameData* data = listFrameData.front();
          return data;
     };

     PacketInfo* get_front_of_packet()
     {
          ScopeLock lock(PacketLock);
          PacketInfo* packet = listPacket.front();
          return packet;
     };

     uint32_t get_size_of_packet()
     {
          ScopeLock lock(PacketLock);
          uint32_t size = listPacket.size();
          return size;
     };

     uint32_t get_size_of_framedata()
     {
          ScopeLock lock(DataLock);
          uint32_t size = listFrameData.size();
          return size;
     };

     uint32_t get_size_of_frame()
     {
          ScopeLock lock(FrameLock);
          uint32_t size = listFrame.size();
          return size;
     };
public:
     std::list<FrameData*>         listFrameData;
     std::list<PacketInfo*>        listPacket;
     std::list<DecodedFrame*>      listFrame;
     int32_t                       height;
     int32_t                       width;
     int32_t                       numQuality;
     SourceResolution              qtyResolution[8];
     uint32_t                      tileRowNum;
     uint32_t                      tileColNum;

     ThreadLock                    PacketLock;
     ThreadLock                    DataLock;
     ThreadLock                    FrameLock;
     bool                          bPacketEOS;

     AMediaFormat                 *mMediaFormat;
     AMediaCodec                  *mMediaCodec;
     OutputSurface                *mOutputSurface;
};


class VideoDecoder_hw : public MediaDecoder,
                     public Threadable
{
public:
     VideoDecoder_hw();
     virtual ~VideoDecoder_hw();
public:
     //!
     //! \brief initialize a video decoder based on input information
     //!
     virtual RenderStatus Initialize(int32_t id, Codec_Type codec, FrameHandler* handler, uint64_t startPts);

     //!
     //! \brief destroy a video decoder
     //!
     virtual RenderStatus Destroy();

     //!
     //! \brief  reset the decoder when decoding information changes
     //!
     virtual RenderStatus Reset(int32_t id, Codec_Type codec, uint64_t startPts);

     //!
     //! \brief  udpate frame to destination with the callback class FrameHandler
     //!
     virtual RenderStatus UpdateFrame(uint64_t pts, int64_t *corr_pts);

     //!
     //! \brief
     //!
     virtual void Run();

     //!
     //! \brief  send a coded packet to decoder
     //!
     virtual RenderStatus SendPacket(DashPacket* packet);

     //!
     //! \brief  pending a decoder
     //!
     virtual void Pending();

     virtual bool IsReady(uint64_t pts);

private:
     //!
     //! \brief  Decoder one frame
     //!
     RenderStatus DecodeFrame(DashPacket *pkt, uint32_t video_id);

     //!
     //! \brief  close decoder
     //!
     void CloseDecoder();

     //!
     //! \brief  flush frames in the decoder while before Close or reset decoder
     //!
     RenderStatus FlushDecoder(uint32_t video_id);

     //!
     //! \brief  update media information with new packet input
     //!
     //! \return bool
     //!         true if information changed, false if there is no change;
     bool MediaInfoChange(DashPacket* packet, uint64_t pts);

     //!
     //! \brief  private method to initialize the decoder.
     //!
     RenderStatus Initialize(Codec_Type codec);

     //!
     //! \brief  get frame from decoded frame list according to pts.
     //!
     RenderStatus GetFrame(uint64_t pts, DecodedFrame *&frame);

     OutputSurface* GetOutputSurface(uint64_t pts);

     RenderStatus SetRegionInfo(struct RegionInfo *regionInfo, int32_t nQuality, SourceResolution *qtyRes);

     RenderStatus CreateOutputSurface();

private:
    VideoDecoder_hw& operator=(const VideoDecoder_hw& other) { return *this; };
    VideoDecoder_hw(const VideoDecoder_hw& other) { /* do not create copies */ };

private:
     DecoderContext              *mDecCtx;
     int32_t                      mVideoId;
     FrameHandler*                mHandler;
     DashPacket                  *mPkt;
     PacketInfo                  *mPktInfo;
     RegionWisePacking           *mRwpk;
     bool                         mIsFlushed;
     uint32_t                     mWidth;
     uint32_t                     mHeight;
     long long int                mCnt;
     FILE*                        mDump_YuvFile;
     uint64_t                     mNextInputPts;
};

VCD_NS_END

#endif
#endif //_ANDROID_OS_