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
//! \file     MediaSource.h
//! \brief    Defines class for MediaSource.
//!
#ifndef _MEDIASOURCE_H_
#define _MEDIASOURCE_H_

#include "../Common/Common.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>
#include "RenderSourceFactory.h"

#define MAX_SURFACE_NUM 10

VCD_NS_BEGIN

//! \brief video information
class VideoInfo{
public:
    VideoInfo()
    {
        codec_type       = VideoCodec_NONE;
        streamID         = -1;
        bit_rate         = 0;
        height           = 0;
        width            = 0;
        sourceHighTileRow= 0;
        sourceHighTileCol= 0;
        framerate_num    = 1;
        framerate_den    = 1;
        mProjFormat      = 0;
        mFpt             = 0;
        mime_type        = "";
        codec            = "";
        mPixFmt          = PixelFormat::PIX_FMT_YUV420P;
        source_number    = 0;
        source_resolution= nullptr;
    };
    ~VideoInfo()=default;

public:
    Codec_Type            codec_type;
    int32_t               streamID;
    int32_t               bit_rate;
    int32_t               height;
    int32_t               width;
    int32_t               sourceHighTileRow;
    int32_t               sourceHighTileCol;
    int32_t               framerate_num;
    int32_t               framerate_den;
    int32_t               mProjFormat;
    int32_t               mFpt;
    int32_t               mPixFmt;
    std::string           mime_type;
    std::string           codec;
    int32_t               source_number;
    SourceResolution     *source_resolution;
};

//! \brief audio information
class AudioInfo{
public:
    AudioInfo()
    {
        streamID         = -1;
        codec_type       = AudioCodec_NONE;
        bit_rate         = 0;
        channels         = 0;
        sample_rate      = 0;
        channel_bytes    = 1;
        mime_type        = "";
        codec            = "";
    };
    ~AudioInfo()=default;

public:
    int32_t               streamID;
    int32_t               bit_rate;
    int32_t               channels;
    int32_t               sample_rate;
    int32_t               channel_bytes;
    Codec_Type            codec_type;
    std::string           mime_type;
    std::string           codec;
};

//! \brief Media information for playable stream
class MediaInfo{
public:
    MediaInfo(){
        mDuration = 0;
        mStreamingType = 1;
        mVideoInfo.clear();
        mAudioInfo.clear();
        mActiveAudioID = 0;
        mActiveVideoID = 0;

    };
    ~MediaInfo()=default;
public:
    void AddVideoInfo(int32_t id, VideoInfo vi){
        mVideoInfo[id] = vi;
    };
    void AddAudioInfo(int32_t id, AudioInfo ai){
        mAudioInfo[id] = ai;
    };
    uint32_t GetVideoInfo(int32_t id, VideoInfo& vi){
        if(mVideoInfo.find(id)!=mVideoInfo.end()){
            vi = mVideoInfo[id];
            return 0;
        }
        return -1;
    };
    int32_t GetAudioInfo(int32_t id, AudioInfo& ai){
        if(mAudioInfo.find(id)!=mAudioInfo.end()){
            ai = mAudioInfo[id];
            return 0;
        }
        return -1;
    };

    void GetActiveVideoInfo(VideoInfo& vi){
        GetVideoInfo(mActiveVideoID, vi);
    };

    void GetActiveAudioInfo(AudioInfo& ai){
        GetAudioInfo(mActiveVideoID, ai);
    };

    void SetActiveVideo(int32_t id){
        if(mVideoInfo.find(id)!=mVideoInfo.end()){
            mActiveVideoID = id;
        }
    };
    void SetActiveAudio(int32_t id){
        if(mAudioInfo.find(id)!=mAudioInfo.end()){
            mActiveAudioID = id;
        }
    };
public:
    std::map<int32_t, VideoInfo>     mVideoInfo;
    std::map<int32_t, AudioInfo>     mAudioInfo;
    uint64_t      mDuration;
    int32_t       mStreamingType;
    int32_t       mActiveVideoID;
    int32_t       mActiveAudioID;
};

//! \brief MediaSource: abstract class for player source
class MediaSource
{
public:
    MediaSource(){
        m_mediaSourceInfo.width           = 0;
        m_mediaSourceInfo.height          = 0;
        m_mediaSourceInfo.projFormat      = VCD::OMAF::PF_UNKNOWN;
        m_mediaSourceInfo.pixFormat       = PixelFormat::INVALID;
        m_mediaSourceInfo.hasAudio        = false;
        m_mediaSourceInfo.audioChannel    = 0;
        m_mediaSourceInfo.stride          = 0;
        m_mediaSourceInfo.numberOfStreams = 0;
        m_mediaSourceInfo.frameRate       = 0;
        m_mediaSourceInfo.duration        = 0;
        m_mediaSourceInfo.frameNum        = 0;
        m_mediaSourceInfo.currentFrameNum = 0;
        m_mediaSourceInfo.sourceWH        = NULL;
        m_mediaSourceInfo.sourceNumber    = 0;
        isAllValid                        = false;
        m_sourceType                      = MediaSourceType::SOURCE_NONE;
        m_rsFactory                       = NULL;

    };

    virtual ~MediaSource(){
        if (m_mediaSourceInfo.sourceWH != NULL)
        {
             delete m_mediaSourceInfo.sourceWH;
             m_mediaSourceInfo.sourceWH = NULL;
        }
    };

    //! \brief Initial in DashMediaInfo
    //!
    //!         render configuration
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    virtual RenderStatus Initialize(struct RenderConfig renderConfig, RenderSourceFactory *m_rsFactory=NULL) = 0;

    virtual RenderStatus Start() = 0;

    //! \brief Get a Video frame from the Media Source
    //!
    //! \param  [out] uint8_t **
    //!         the frame buffer
    //!         [out] struct RegionInfo *
    //!         regionInfo
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    virtual RenderStatus GetFrame(uint8_t **buffer, struct RegionInfo *regionInfo) = 0;

    //! \brief Get an Audio frame from the Media Source
    //!
    //! \param  [out] uint8_t **
    //!         the frame buffer
    //!         [out] struct RegionInfo *
    //!         regionInfo
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    virtual RenderStatus GetAudioFrame(int64_t pts, uint8_t **buffer){return RENDER_STATUS_OK;};

    //! \brief Check is file ends
    //!
    //! \return bool
    //!
    virtual bool IsEOS() = 0;
    //! \brief set yaw and pitch to change Viewport
    //!
    //! \param  [in] float
    //!         yaw angle
    //!         [in] pitch
    //!         pitch angle
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    virtual RenderStatus ChangeViewport(HeadPose *pose) = 0;

    //! \brief UpdateFrames
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, RENDER_EOS if reach EOS
    //!
    virtual RenderStatus UpdateFrames(uint64_t pts, int64_t *corr_pts) = 0;

    //! \brief get isAllValid
    //!
    //! \return bool
    //!         isAllValid
    //!
    bool getIsAllValid() {return isAllValid;};


    //! \brief SeekTo
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, RENDER_EOS if reach EOS
    //!
    virtual RenderStatus SeekTo(uint64_t pts){ return RENDER_STATUS_OK;};


    //! \brief Pause
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success
    //!
    virtual RenderStatus Pause(){ return RENDER_STATUS_OK;};

    //! \brief Play
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success
    //!
    virtual RenderStatus Play(){ return RENDER_STATUS_OK; };

    //! \brief GetMediaInfo
    //!
    //! \return MediaInfo
    //!
    virtual MediaInfo GetMediaInfo(){return mMediaInfo;};

    // only used in android player
    void SetDecodeSurface(void* surface, int tex_id, int video_id)
    {
        if (m_nativeSurface.empty())
        {
            m_nativeSurface.resize(MAX_SURFACE_NUM);
        }
        m_nativeSurface[video_id].first = tex_id;
        m_nativeSurface[video_id].second = surface;
    };
    pair<uint32_t, void*> GetDecodeSurface(int video_id) { return m_nativeSurface[video_id]; };

    //! \brief GetRenderSources
    //!
    //! \return RenderSourceFactory*
    //!
    RenderSourceFactory* GetRenderSources(){ return this->m_rsFactory; };

    virtual void SetActiveStream( int32_t video_id, int32_t audio_id ){ };

    //deprecated

    //! \brief Set Media Source Info
    //!
    //! \param  [in] void *
    //!         mediaInfo
    //!
    //! \return void *
    //!
    virtual RenderStatus SetMediaSourceInfo(void *mediaInfo){return RENDER_STATUS_OK;};
    //! \brief Get Media Source Info
    //!
    //! \return struct MediaSourceInfo
    //!
    virtual struct MediaSourceInfo GetMediaSourceInfo(){return m_mediaSourceInfo;};
    //! \brief Get SourceMetaData
    //!
    //! \return void*
    //!
    virtual void* GetSourceMetaData(){};

    void DeleteVideoBuffer(uint8_t **buffer)
    {
        uint32_t bufferNumber = 0;
        switch (mMediaInfo.mVideoInfo[0].mPixFmt){
            case PixelFormat::PIX_FMT_RGB24:
                bufferNumber = 1;
                break;
            case PixelFormat::PIX_FMT_YUV420P:
                bufferNumber = 3;
                break;
        }
        if (buffer != NULL){
            for (uint32_t i=0;i<bufferNumber;i++){
                if (buffer[i] != NULL){
                    delete buffer[i];
                    buffer[i] = NULL;
                }
            }
        }
    };

    //deprecated
    void ClearRWPK(RegionWisePacking *rwpk)
    {
        if (rwpk != NULL){
           if (rwpk->rectRegionPacking != NULL){
                delete rwpk->rectRegionPacking;
                rwpk->rectRegionPacking = NULL;
            }
            delete rwpk;
            rwpk = NULL;
        }
    };

    int32_t GetStatus() { return m_status; };

protected:
    int32_t                   m_status = STATUS_UNKNOWN;
    MediaInfo                 mMediaInfo;         //! media information
    RenderSourceFactory      *m_rsFactory;        //! Render Source list for rendering to RenderTarget
    struct MediaSourceInfo    m_mediaSourceInfo;  //! deprecated
    MediaSourceType::Enum     m_sourceType;       //! vod or live
    bool                      isAllValid;
    vector<pair<uint32_t, void*>>             m_nativeSurface;

};

VCD_NS_END
#endif /* _MEDIASOURCE_H_ */
