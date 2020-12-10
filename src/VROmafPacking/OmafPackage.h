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
//! \file:   OmafPackage.h
//! \brief:  OmafPackage class definition
//! \detail: OmafPackage will be called by library interface to
//!          set up the whole package process with OMAF compliance.
//!
//! Created on April 30, 2019, 6:04 AM
//!

#ifndef _OMAFPACKAGE_H_
#define _OMAFPACKAGE_H_

#include "OmafPackingCommon.h"
#include "VROmafPacking_data.h"
#include "Segmentation.h"
#include "ExtractorTrackManager.h"

#include <map>

VCD_NS_BEGIN

//!
//! \class OmafPackage
//! \brief The topmost class for OMAF Packing library
//!

class OmafPackage
{
public:
    //!
    //! \brief  Constructor
    //!
    OmafPackage();

    OmafPackage(const OmafPackage& src);

    OmafPackage& operator=(OmafPackage&& other);

    //!
    //! \brief  Destructor
    //!
    ~OmafPackage();

    //!
    //! \brief  Initialize OmafPackage
    //!
    //! \param  [in] initInfo
    //!         initial information input by library interface
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t InitOmafPackage(InitialInfo *initInfo);

    //!
    //! \brief  Set the customized logging callback function
    //!
    //! \param  [in] logFunction
    //!         the pointer to customized logging callback function
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t SetLogCallBack(LogFunction logFunction);

    //!
    //! \brief  Packet the specified media stream
    //!
    //! \param  [in] streamIdx
    //!         the index of specified stream in whole streams
    //! \param  [in] frameInfo
    //!         frame information for a new frame of specified stream
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t OmafPacketStream(uint8_t streamIdx, FrameBSInfo *frameInfo);

    //!
    //! \brief  End the packeting of all streams
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t OmafEndStreams();

private:

    //!
    //! \brief  Video Segmentation thread execution function
    //!
    //! \param  [in] pThis
    //!         this OmafPackage
    //!
    //! \return void
    //!
    static void* VideoSegmentationThread(void* pThis);

    //!
    //! \brief  Audio Segmentation thread execution function
    //!
    //! \param  [in] pThis
    //!         this OmafPackage
    //!
    //! \return void
    //!
    static void* AudioSegmentationThread(void* pThis);

    //!
    //! \brief  Add media stream into media stream map
    //!
    //! \param  [in] streamIdx
    //!         the index of stream will be added
    //! \param  [in] bs
    //!         bitstream information of the stream, including a part
    //!         of bitstream data and media type of the stream
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t AddMediaStream(uint8_t streamIdx, BSBuffer *bs);

    //!
    //! \brief  Create extractor track manager and initialize it
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t CreateExtractorTrackManager();

    //!
    //! \brief  Create segmentation for data segment
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t CreateSegmentation();

    //!
    //! \brief  Put frame information of new frame of stream into its
    //!         frame information list
    //!
    //! \param  [in] streamIdx
    //!         the index of the stream to be handled
    //! \param  [in] frameInfo
    //!         frame information of new frame of the stream
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t SetFrameInfo(uint8_t streamIdx, FrameBSInfo *frameInfo);

    //!
    //! \brief  Segment all video media streams
    //!
    //! \return void
    //!
    void SegmentAllVideoStreams();

    //!
    //! \brief  Segment all audio media streams
    //!
    //! \return void
    //!
    void SegmentAllAudioStreams();

private:
    InitialInfo                     *m_initInfo;               //!< the initial information input by library interface
    Segmentation                    *m_segmentation;           //!< the segmentation for data segment
    ExtractorTrackManager           *m_extractorTrackMan;      //!< the extractor track manager
    std::map<uint8_t, MediaStream*> m_streams;                 //!< the media streams map
    std::map<CodecId, void*>        m_streamPlugins;           //!< the map of CodecId and corresponding stream plugin handles
    bool                            m_isSegmentationStarted;   //!< whether the segmentation thread is started
    pthread_t                       m_videoThreadId;           //!< thread index of video segmentation thread
    bool                            m_hasAudio;
    pthread_t                       m_audioThreadId;           //!< thread index of audio segmentation thread
};

VCD_NS_END;
#endif /* _OMAFPACKAGE_H_ */
