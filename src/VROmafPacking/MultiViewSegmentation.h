/*
 * Copyright (c) 2021, Intel Corporation
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
//! \file:   MultiViewSegmentation.h
//! \brief:  Multi view segmentation class definition
//! \detail: Define the operation and needed data for multi view segmentation.
//!
//! Created on Nov. 24, 2021, 6:04 AM
//!

#ifndef _MULTIVIEWSEGMENTATION_H_
#define _MULTIVIEWSEGMENTATION_H_

#include <mutex>
#include "Segmentation.h"
#include "DashSegmenter.h"

VCD_NS_BEGIN

//!
//! \class MultiViewSegmentation
//! \brief Define the operation and needed data for multi view segmentation
//!

class MultiViewSegmentation : public Segmentation
{
public:
    //!
    //! \brief  Constructor
    //!
    MultiViewSegmentation()
    {
        m_segNum = 0;
        m_audioSegNum = 0;
        m_audioPrevSegNum = 0;
        m_audioSegCtxsConsted = false;
        m_framesNum = 0;
        m_videoSegInfo = NULL;
        m_projType  = VCD::OMAF::ProjectionFormat::PF_PLANAR;
        m_isEOS = false;
        m_nowKeyFrame = false;
        m_prevSegNum = 0;
        //m_isFramesReady = false;
        //m_aveETPerSegThread = 0;
        //m_lastETPerSegThread = 0;
        //m_threadNumForET = 0;
        m_videosNum = 0;
        //m_videosBitrate = NULL;
        //m_prevSegedFrmNum = 0;
        //m_currSegedFrmNum = 0;
        //m_currProcessedFrmNum = 0;
        m_mpdWriter = NULL;
        m_isMpdGenInit = false;
        //m_segWriterPluginHdl = NULL;
    };

    //!
    //! \brief  Assignment Constructor
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
    MultiViewSegmentation(std::map<uint8_t, MediaStream*> *streams, ExtractorTrackManager *extractorTrackMan, InitialInfo *initInfo, PackingSourceMode sourceMode) : Segmentation(streams, extractorTrackMan, initInfo, sourceMode)
    {
        m_segNum = 0;
        m_audioSegNum = 0;
        m_audioPrevSegNum = 0;
        m_audioSegCtxsConsted = false;
        m_framesNum = 0;
        m_videoSegInfo = NULL;
        m_projType  = VCD::OMAF::ProjectionFormat::PF_PLANAR;
        m_isEOS = false;
        m_nowKeyFrame = false;
        m_prevSegNum = 0;
        //m_isFramesReady = false;
        //m_aveETPerSegThread = 0;
        //m_lastETPerSegThread = 0;
        //m_threadNumForET = 0;
        m_videosNum = 0;
        //m_videosBitrate = NULL;
        //m_prevSegedFrmNum = 0;
        //m_currSegedFrmNum = 0;
        //m_currProcessedFrmNum = 0;
        m_mpdWriter = NULL;
        m_isMpdGenInit = false;

        //CreateSegWriterPluginHdl();
    };

    MultiViewSegmentation(const MultiViewSegmentation& src)
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
        //m_isFramesReady = src.m_isFramesReady;
        //m_aveETPerSegThread = src.m_aveETPerSegThread;
        //m_lastETPerSegThread = src.m_lastETPerSegThread;
        //m_threadNumForET = src.m_threadNumForET;
        m_videosNum = src.m_videosNum;
        //m_videosBitrate = std::move(src.m_videosBitrate);
        //m_prevSegedFrmNum = src.m_prevSegedFrmNum;
        //m_currSegedFrmNum = src.m_currSegedFrmNum;
        //m_currProcessedFrmNum = src.m_currProcessedFrmNum;
        m_mpdWriter = std::move(src.m_mpdWriter);
        m_isMpdGenInit = src.m_isMpdGenInit;
        //m_segWriterPluginHdl = src.m_segWriterPluginHdl;
    };

    MultiViewSegmentation& operator=(MultiViewSegmentation&& other)
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
        //m_isFramesReady = other.m_isFramesReady;
        //m_aveETPerSegThread = other.m_aveETPerSegThread;
        //m_lastETPerSegThread = other.m_lastETPerSegThread;
        //m_threadNumForET = other.m_threadNumForET;
        m_videosNum = other.m_videosNum;
        //m_videosBitrate = NULL;
        //m_prevSegedFrmNum = other.m_prevSegedFrmNum;
        //m_currSegedFrmNum = other.m_currSegedFrmNum;
        //m_currProcessedFrmNum = other.m_currProcessedFrmNum;
        m_mpdWriter = NULL;
        m_isMpdGenInit = other.m_isMpdGenInit;
        //m_segWriterPluginHdl = other.m_segWriterPluginHdl;
        return *this;
    };

    //!
    //! \brief  Destructor
    //!
    virtual ~MultiViewSegmentation();

    //!
    //! \brief  Initialize the basic process
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t Initialize()
    {
        int32_t ret = Segmentation::Initialize();
        return ret;
    };

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

    //int32_t CreateDashSegmentWriter(TrackSegmentCtx *trackSegCtx);

    //!
    //! \brief  Construct track segmentation context
    //!         for all video tracks
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t ConstructVideoTrackSegCtx();

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
    int32_t WriteSegmentForEachVideo(MediaStream *stream, FrameBSInfo *frameData, bool isKeyFrame, bool isEOS);

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

    //!
    //! \brief  Create segment writer plugin handle
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    //int32_t CreateSegWriterPluginHdl()
    //{
    //    if (!m_segWriterPluginPath || !m_segWriterPluginName)
    //    {
    //        OMAF_LOG(LOG_ERROR, "Segment generation plugin is not assigned ! \n");
    //        return OMAF_ERROR_NULL_PTR;
    //    }
    //
    //    if (m_isCMAFEnabled)
    //    {
    //        if (0 == strcmp(m_segWriterPluginName, "SegmentWriter"))
    //        {
    //            OMAF_LOG(LOG_ERROR, "Plugin SegmentWriter can't generate CMAF segments !\n");
    //            return OMAF_ERROR_BAD_PARAM;
    //        }
    //    }
    //
    //    uint32_t pathLen = strlen(m_segWriterPluginPath);
    //
    //    char pluginLibName[1024];
    //    memset_s(pluginLibName, 1024, 0);
    //    if (m_segWriterPluginPath[pathLen - 1] == '/')
    //    {
    //        snprintf(pluginLibName, 1024, "%slib%s.so", m_segWriterPluginPath, m_segWriterPluginName);
    //    }
    //    else
    //    {
    //        snprintf(pluginLibName, 1024, "%s/lib%s.so", m_segWriterPluginPath, m_segWriterPluginName);
    //    }
    //
    //    OMAF_LOG(LOG_INFO, "Segment generation plugin is %s\n", pluginLibName);
    //
    //    m_segWriterPluginHdl = dlopen(pluginLibName, RTLD_LAZY);
    //    const char *dlsymErr1 = dlerror();
    //    if (!(m_segWriterPluginHdl))
    //    {
    //        OMAF_LOG(LOG_ERROR, "Failed to open segment writer plugin %s !\n", pluginLibName);
    //        if (dlsymErr1)
    //        {
    //            OMAF_LOG(LOG_ERROR, "Get error msg %s \n", dlsymErr1);
    //        }
    //        return OMAF_ERROR_DLOPEN;
    //    }

    //    return ERROR_NONE;
    //};
    int32_t CreateDashMPDWriter();

private:
    std::map<MediaStream*, TrackSegmentCtx*>       m_streamSegCtx;       //!< map of media stream and its track segmentation context
    std::map<MediaStream*, VCD::MP4::MPDAdaptationSetCtx*> m_streamASCtx; //!< map of media stream and its MPD adaptation set context
    std::map<MediaStream*, bool>                   m_framesIsKey;        //!< map of media stream and its current frame status (IDR or not)
    std::map<MediaStream*, bool>                   m_streamsIsEOS;       //!< map of media stream and its current EOS status
    VCD::OMAF::ProjectionFormat                    m_projType;           //!< picture projection type
    VideoSegmentInfo                               *m_videoSegInfo;      //!< pointer to the video segment information
    uint64_t                                       m_segNum;             //!< current written segments number
    std::mutex                                     m_audioMutex;
    uint64_t                                       m_audioSegNum;
    uint64_t                                       m_audioPrevSegNum;
    bool                                           m_audioSegCtxsConsted;
    uint64_t                                       m_framesNum;          //!< current written frames number
    bool                                           m_isEOS;              //!< whether EOS has been gotten for all media streams
    bool                                           m_nowKeyFrame;        //!< whether current frames are key frames for each corresponding media stream
    uint64_t                                       m_prevSegNum;         //!< previously written segments number
    //std::mutex                                     m_mutex;              //!< thread mutex for main segmentation thread
    uint32_t                                       m_videosNum;          //!< video streams number
    //uint64_t                                       *m_videosBitrate;     //!< video stream bitrate array
    //uint64_t                                       m_prevSegedFrmNum;    //!< previous number of frames which have been segmented for their tile tracks
    //uint64_t                                       m_currSegedFrmNum;    //!< newest number of frames which have been segmented for their tile tracks
    MPDWriterBase*                                 m_mpdWriter;            //!< MPD file writer created based on plugin
    bool                                           m_isMpdGenInit;       //!< flag for whether MPD generator has been initialized
    //void                                           *m_segWriterPluginHdl;
    std::map<VCD::MP4::TrackId, TrackSegmentCtx*>  m_trackSegCtx;        //!< map of track index and track segmentation context
};

VCD_NS_END;
#endif /* _MULTIVIEWSEGMENTATION_H_ */
