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

//!
//! \file:   DefaultSegmentation.h
//! \brief:  Default segmentation class definition
//! \detail: Define the operation and needed data for default segmentation.
//!
//! Created on April 30, 2019, 6:04 AM
//!

#ifndef _DEFAULTSEGMENTATION_H_
#define _DEFAULTSEGMENTATION_H_

#include <mutex>
#include "Segmentation.h"
#include "DashSegmenter.h"

VCD_NS_BEGIN

//!
//! \class DefaultSegmentation
//! \brief Define the operation and needed data for default segmentation
//!

class DefaultSegmentation : public Segmentation
{
public:
    //!
    //! \brief  Constructor
    //!
    DefaultSegmentation()
    {
        m_segNum = 0;
        m_audioSegNum = 0;
        m_audioPrevSegNum = 0;
        m_audioSegCtxsConsted = false;
        m_framesNum = 0;
        m_videoSegInfo = NULL;
        m_projType  = VCD::OMAF::ProjectionFormat::PF_ERP;
        m_isEOS = false;
        m_nowKeyFrame = false;
        m_prevSegNum = 0;
        m_isFramesReady = false;
        m_aveETPerSegThread = 0;
        m_lastETPerSegThread = 0;
        m_threadNumForET = 0;
        m_videosNum = 0;
        m_videosBitrate = NULL;
        m_prevSegedFrmNum = 0;
        m_currSegedFrmNum = 0;
        m_currProcessedFrmNum = 0;
        m_isMpdGenInit = false;
    };

    //!
    //! \brief  Copy Constructor
    //!
    //! \param  [in] streams
    //!         media streams map set up in OmafPackage
    //! \param  [in] extractorTrackMan
    //!         pointer to the extractor track manager
    //!         created in OmafPackage
    //! \param  [in] initInfo
    //!         initial information input by library interface
    //!         which includes segmentation information
    //!
    DefaultSegmentation(std::map<uint8_t, MediaStream*> *streams, ExtractorTrackManager *extractorTrackMan, InitialInfo *initInfo) : Segmentation(streams, extractorTrackMan, initInfo)
    {
        m_segNum = 0;
        m_audioSegNum = 0;
        m_audioPrevSegNum = 0;
        m_audioSegCtxsConsted = false;
        m_framesNum = 0;
        m_videoSegInfo = NULL;
        m_projType  = VCD::OMAF::ProjectionFormat::PF_ERP;
        m_isEOS = false;
        m_nowKeyFrame = false;
        m_prevSegNum = 0;
        m_isFramesReady = false;
        m_aveETPerSegThread = 0;
        m_lastETPerSegThread = 0;
        m_threadNumForET = 0;
        m_videosNum = 0;
        m_videosBitrate = NULL;
        m_prevSegedFrmNum = 0;
        m_currSegedFrmNum = 0;
        m_currProcessedFrmNum = 0;
        m_isMpdGenInit = false;
    };

    DefaultSegmentation(const DefaultSegmentation& src)
    {
        m_segNum = src.m_segNum;
        m_audioSegNum = src.m_audioSegNum;
        m_audioPrevSegNum = src.m_audioPrevSegNum;
        m_audioSegCtxsConsted = src.m_audioSegCtxsConsted;
        m_framesNum = src.m_framesNum;
        m_videoSegInfo = std::move(src.m_videoSegInfo);
        m_projType  = src.m_projType;
        m_isEOS = src.m_isEOS;
        m_nowKeyFrame = src.m_nowKeyFrame;
        m_prevSegNum = src.m_prevSegNum;
        m_isFramesReady = src.m_isFramesReady;
        m_aveETPerSegThread = src.m_aveETPerSegThread;
        m_lastETPerSegThread = src.m_lastETPerSegThread;
        m_threadNumForET = src.m_threadNumForET;
        m_videosNum = src.m_videosNum;
        m_videosBitrate = std::move(src.m_videosBitrate);
        m_prevSegedFrmNum = src.m_prevSegedFrmNum;
        m_currSegedFrmNum = src.m_currSegedFrmNum;
        m_currProcessedFrmNum = src.m_currProcessedFrmNum;
        m_isMpdGenInit = src.m_isMpdGenInit;
    };

    DefaultSegmentation& operator=(DefaultSegmentation&& other)
    {
        m_segNum = other.m_segNum;
        m_audioSegNum = other.m_audioSegNum;
        m_audioPrevSegNum = other.m_audioPrevSegNum;
        m_audioSegCtxsConsted = other.m_audioSegCtxsConsted;
        m_framesNum = other.m_framesNum;
        m_videoSegInfo = NULL;
        m_projType  = other.m_projType;
        m_isEOS = other.m_isEOS;
        m_nowKeyFrame = other.m_nowKeyFrame;
        m_prevSegNum = other.m_prevSegNum;
        m_isFramesReady = other.m_isFramesReady;
        m_aveETPerSegThread = other.m_aveETPerSegThread;
        m_lastETPerSegThread = other.m_lastETPerSegThread;
        m_threadNumForET = other.m_threadNumForET;
        m_videosNum = other.m_videosNum;
        m_videosBitrate = NULL;
        m_prevSegedFrmNum = other.m_prevSegedFrmNum;
        m_currSegedFrmNum = other.m_currSegedFrmNum;
        m_currProcessedFrmNum = other.m_currProcessedFrmNum;
        m_isMpdGenInit = other.m_isMpdGenInit;
        return *this;
    };

    //!
    //! \brief  Destructor
    //!
    virtual ~DefaultSegmentation();

    //!
    //! \brief  Execute the segmentation process for
    //!         all video streams
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t VideoSegmentation();

    //!
    //! \brief  End the segmentation process for
    //!         all video streams
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t VideoEndSegmentation();

    //!
    //! \brief  Execute the segmentation process for
    //!         all audio streams
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t AudioSegmentation();

    //!
    //! \brief  End the segmentation process for
    //!         all audio streams
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t AudioEndSegmentation();

private:
    //!
    //! \brief  Write povd box for segments,
    //!         including projection type,
    //!         region wise packing information,
    //!         and content coverage information
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    //virtual int32_t VideoWritePovdBox();

    //!
    //! \brief  Construct track segmentation context
    //!         for all video tile tracks
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t ConstructTileTrackSegCtx();

    //!
    //! \brief  Construct track segmentation context
    //!         for all extractor tracks
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t ConstructExtractorTrackSegCtx();

    //!
    //! \brief  Construct track segmentation context
    //!         for all audio tracks
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t ConstructAudioTrackSegCtx();

    //!
    //! \brief  Write segments for specified video stream
    //!
    //! \param  [in] stream
    //!         pointer to specified video stream
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t WriteSegmentForEachVideo(MediaStream *stream, bool isKeyFrame, bool isEOS);

    //!
    //! \brief  Write segment for specified extractor track
    //!
    //! \param  [in] extractorTrack
    //!         pointer to specified extractor track
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t WriteSegmentForEachExtractorTrack(
        ExtractorTrack *extractorTrack,
        bool isKeyFrame,
        bool isEOS);

    //!
    //! \brief  Write segments for specified audio stream
    //!
    //! \param  [in] stream
    //!         pointer to specified audio stream
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t WriteSegmentForEachAudio(MediaStream *stream, FrameBSInfo *frameData, bool isKeyFrame, bool isEOS);

    //!
    //! \brief  End segmentation process for specified video stream
    //!
    //! \param  [in] stream
    //!         pointer to specified video stream
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t EndEachVideo(MediaStream *stream);

    //!
    //! \brief  End segmentation process for specified audio stream
    //!
    //! \param  [in] stream
    //!         pointer to specified audio stream
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t EndEachAudio(MediaStream *stream);

    //!
    //! \brief  Start segmentation thread for specified extractor track
    //!
    //! \param  [in] extractorTrack
    //!         pointer to the specified extractor track
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t StartExtractorTrackSegmentation(ExtractorTrack *extractorTrack);

    //!
    //! \brief  Start last segmentation thread for rest extractor tracks
    //!
    //! \param  [in] extractorTrack
    //!         pointer to the first extractor track in rest extractor
    //!         tracks which are not created segmentation thread
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t StartLastExtractorTrackSegmentation(ExtractorTrack *extractorTrack);

    //!
    //! \brief  extractor track segmentation thread function
    //!
    //! \param  [in] pThis
    //!         this DefaultSegmentation
    //!
    //! \return void*
    //!         return NULL
    //!
    static void* ExtractorTrackSegThread(void *pThis);

    //!
    //! \brief  extractor track segmentation thread function for last extractor
    //!         tracks
    //!
    //! \param  [in] pThis
    //!         this DefaultSegmentation
    //!
    //! \return void*
    //!         return NULL
    //!
    static void* LastExtractorTrackSegThread(void *pThis);

    //!
    //! \brief  Generate extractor track segments
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t ExtractorTrackSegmentation();

    //!
    //! \brief  Generate extractor track segments for last
    //!         extractor tracks
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t LastExtractorTrackSegmentation();

    //!
    //! \brief  Set frames ready status for extractor track
    //!
    //! \param  [in] isFramesReady
    //!         whether frames ready for extractor track
    //!
    //! \return void
    //!
    void SetFramesReadyStatus(bool isFramesReady)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_isFramesReady = isFramesReady;
    };

    //!
    //! \brief  Check whether there are only audio streams
    //!         in input streams
    //!
    //! \return bool
    //!         true if there are only audio streams, else false
    //!
    bool OnlyAudio();

    //!
    //! \brief  Check whether there is audio stream in input
    //!         streams
    //!
    //! \return bool
    //!         true if there is audio stream, else false
    //!
    bool HasAudio();

private:
    std::map<MediaStream*, TrackSegmentCtx*>       m_streamSegCtx;       //!< map of media stream and its track segmentation context
    std::map<ExtractorTrack*, TrackSegmentCtx*>    m_extractorSegCtx;    //!< map of extractor track and its track segmentation context
    std::map<VCD::MP4::TrackId, TrackConfig>       m_allTileTracks;      //!< map of track and its track configuration
    std::map<MediaStream*, bool>                   m_framesIsKey;        //!< map of media stream and its current frame status (IDR or not)
    std::map<MediaStream*, bool>                   m_streamsIsEOS;       //!< map of media stream and its current EOS status
    VCD::OMAF::ProjectionFormat                    m_projType;           //!< picture projection type
    VideoSegmentInfo                               *m_videoSegInfo;      //!< pointer to the video segment information
    std::map<uint8_t, std::map<uint32_t, VCD::MP4::TrackId>> m_tilesTrackIdxs;     //!< map of tile and its track index
    std::map<VCD::MP4::TrackId, TrackSegmentCtx*>            m_trackSegCtx;        //!< map of tile track and its track segmentation context
    uint64_t                                       m_segNum;             //!< current written segments number
    std::mutex                                     m_audioMutex;
    uint64_t                                       m_audioSegNum;
    uint64_t                                       m_audioPrevSegNum;
    bool                                           m_audioSegCtxsConsted;
    uint64_t                                       m_framesNum;          //!< current written frames number
    std::map<pthread_t, ExtractorTrack*>           m_extractorThreadIds; //!< map of thread ID for extractor track segmentation and corresponding extractor track
    bool                                           m_isEOS;              //!< whether EOS has been gotten for all media streams
    bool                                           m_nowKeyFrame;        //!< whether current frames are key frames for each corresponding media stream
    uint64_t                                       m_prevSegNum;         //!< previously written segments number
    std::mutex                                     m_mutex;              //!< thread mutex for main segmentation thread
    bool                                           m_isFramesReady;      //!< whether frames are ready for extractor track
    uint16_t                                       m_aveETPerSegThread;  //!< average extractor tracks number in segmentation thread
    uint16_t                                       m_lastETPerSegThread; //!< extractor tracks number in last segmentation thread
    uint16_t                                       m_threadNumForET;     //!< threads number for extractor track segmentation
    uint32_t                                       m_videosNum;          //!< video streams number
    uint64_t                                       *m_videosBitrate;     //!< video stream bitrate array
    uint64_t                                       m_prevSegedFrmNum;    //!< previous number of frames which have been segmented for their tile tracks
    uint64_t                                       m_currSegedFrmNum;    //!< newest number of frames which have been segmented for their tile tracks
    uint64_t                                       m_currProcessedFrmNum;//!< newest number of frames which have been segmented for both tiles tracks and extractor tracks
    bool                                           m_isMpdGenInit;       //!< flag for whether MPD generator has been initialized
};

VCD_NS_END;
#endif /* _DEFAULTSEGMENTATION_H_ */
