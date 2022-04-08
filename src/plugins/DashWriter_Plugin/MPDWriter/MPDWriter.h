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
//! \file:   MPDWriter.h
//! \brief:  Dash MPD writer plugin definition for omnidirectional media
//!
//! Created on Dec. 1, 2021, 6:04 AM
//!

#ifndef _MPDWRITERPLUGIN_H_
#define _MPDWRITERPLUGIN_H_

#include "../DashMPDWriterPluginAPI.h"
#include "tinyxml2.h"
#include "../../../utils/safe_mem.h"
//extern "C"
//{
//#include "safestringlib/safe_mem_lib.h"
//}

using namespace std;
using namespace tinyxml2;

class MPDWriter : public MPDWriterBase
{
public:
    //!
    //! \brief  Constructor
    //!
    MPDWriter();

    //!
    //! \brief  Copy Constructor
    //!
    //! \param  [in] streamsASCtxs
    //!         pointer to map of media stream and its
    //!         MPD adaptation set context
    //! \param  [in] extractorASCtxs
    //!         pointer to map of extractor track index
    //!         and its MPD adaptation set context
    //! \param  [in] segInfo
    //!         pointer to the segmentation information
    //! \param  [in] projType
    //!         projection type
    //! \param  [in] frameRate
    //!         video stream frame rate
    //! \param  [in] videoNum
    //!         video streams number
    //! \param  [in] cmafEnabled
    //!         flag for whether CMAF compliance is enabled
    //!
    MPDWriter(
        std::map<MediaStream*, VCD::MP4::MPDAdaptationSetCtx*> *streamsASCtxs,
        std::map<uint32_t, VCD::MP4::MPDAdaptationSetCtx*> *extractorASCtxs,
        SegmentationInfo *segInfo,
        VCD::OMAF::ProjectionFormat projType,
        Rational frameRate,
        uint8_t  videoNum,
        bool cmafEnabled);


    MPDWriter(const MPDWriter& src);

    MPDWriter& operator=(MPDWriter&& other);

    //!
    //! \brief  Destructor
    //!
    virtual ~MPDWriter();

    //!
    //! \brief  Initialize the MPD writer, including
    //!         collecting region wise packing information,
    //!         and content coverage information for all
    //!         extractor tracks.
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t Initialize();

    //!
    //! \brief  Write the MPD file according to segmentation information
    //!
    //! \param  [in] totalFramesNum
    //!         total number of frames written into segments
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t WriteMpd(uint64_t totalFramesNum);

    //!
    //! \brief  Update the MPD file according to segmentation information
    //!
    //! \param  [in] segNumber
    //!         total number of segments have been written
    //! \param  [in] framesNumber
    //!         total number of frames have been written into segments
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t UpdateMpd(uint64_t segNumber, uint64_t framesNumber);

private:

    //!
    //! \brief  Write AdaptationSet for tile track in mpd file
    //!
    //! \param  [in] periodEle
    //!         pointer to period element has been create for
    //!         mpd file using tinyxml2
    //! \param  [in] pTrackASCtx
    //!         pointer to track segmentation context for tile track
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t WriteTileTrackAS(XMLElement *periodEle, VCD::MP4::MPDAdaptationSetCtx *pTrackASCtx);

    //!
    //! \brief  Write AdaptationSet for audio track in mpd file
    //!
    //! \param  [in] periodEle
    //!         pointer to period element has been create for
    //!         mpd file using tinyxml2
    //! \param  [in] pTrackASCtx
    //!         pointer to track segmentation context for audio track
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t WriteAudioTrackAS(XMLElement *periodEle, VCD::MP4::MPDAdaptationSetCtx *pTrackASCtx);

    //!
    //! \brief  Write AdaptationSet for extractor track in mpd file
    //!
    //! \param  [in] periodEle
    //!         pointer to period element has been create for
    //!         mpd file using tinyxml2
    //! \param  [in] pTrackASCtx
    //!         pointer to track segmentation context for extractor track
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t WriteExtractorTrackAS(XMLElement *periodEle, VCD::MP4::MPDAdaptationSetCtx *pTrackASCtx);

private:
    std::map<MediaStream*, VCD::MP4::MPDAdaptationSetCtx*>    *m_streamASCtx;        //!< map of media stream and its track MPD adaptation set context
    std::map<uint32_t, VCD::MP4::MPDAdaptationSetCtx*>        *m_extractorASCtx;     //!< map of extractor track index and its track MPD adaptation set context
    SegmentationInfo                                *m_segInfo;            //!< pointer to the segmentation information
    uint32_t                                        m_miniUpdatePeriod;    //!< minimum update period of mpd file, in the unit of second
    VCD::OMAF::ProjectionFormat                     m_projType;            //!< projection type of the video frame
    char                                            m_availableStartTime[1024]; //!< available start time for mpd file for live streaming
    char                                            *m_publishTime;        //!< publish time for mpd file
    char                                            *m_presentationDur;    //!< presentation duration of dash segments

    char                                            m_mpdFileName[1024];   //!< file name of MPD file
    Rational                                        m_frameRate;           //!< video stream frame rate
    uint16_t                                        m_timeScale;           //!< timescale of video stream
    XMLDocument                                     *m_xmlDoc;             //!< XML doc element for writting mpd file created using tinyxml2
    uint8_t                                         m_vsNum;               //!< video streams number
    bool                                            m_cmafEnabled;         //!< flag for whether CMAF compliance is enabled
    uint64_t                                        m_currSegNum;          //!< current segment number
};

extern "C" MPDWriterBase* Create(
        std::map<MediaStream*, VCD::MP4::MPDAdaptationSetCtx*> *streamsASCtxs,
        std::map<uint32_t, VCD::MP4::MPDAdaptationSetCtx*> *extractorASCtxs,
        SegmentationInfo *segInfo,
        VCD::OMAF::ProjectionFormat projType,
        Rational frameRate,
        uint8_t  videoNum,
        bool cmafEnabled);

extern "C" void Destroy(MPDWriterBase* mpdWriter);

#endif /* _MPDWRITERPLUGIN_H_ */
