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
//! \file:   DashMPDWriterPluginAPI.h
//! \brief:  Dash MPD writer plugin interfaces
//!
//! Created on Dec. 1, 2021, 6:04 AM
//!

#ifndef _DASHMPDWRITERPLUGINAPI_H_
#define _DASHMPDWRITERPLUGINAPI_H_

#include "MediaData.h"
#include "MediaStream.h"
#include "VROmafPacking_def.h"
#include "OmafStructure.h"


class MPDWriterBase
{
public:
    //!
    //! \brief  Constructor
    //!
    MPDWriterBase() {};

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
    MPDWriterBase(
        std::map<MediaStream*, VCD::MP4::MPDAdaptationSetCtx*> *streamsASCtxs,
        std::map<uint32_t, VCD::MP4::MPDAdaptationSetCtx*> *extractorASCtxs,
        SegmentationInfo *segInfo,
        VCD::OMAF::ProjectionFormat projType,
        Rational frameRate,
        uint8_t  videoNum,
        bool cmafEnabled) {};


    //!
    //! \brief  Destructor
    //!
    virtual ~MPDWriterBase() {};

    //!
    //! \brief  Initialize the MPD writer, including
    //!         collecting region wise packing information,
    //!         and content coverage information for all
    //!         extractor tracks.
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t Initialize() = 0;

    //!
    //! \brief  Write the MPD file according to segmentation information
    //!
    //! \param  [in] totalFramesNum
    //!         total number of frames written into segments
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t WriteMpd(uint64_t totalFramesNum) = 0;

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
    virtual int32_t UpdateMpd(uint64_t segNumber, uint64_t framesNumber) = 0;

protected:
};

typedef MPDWriterBase* CreateMPDWriter(
        std::map<MediaStream*, VCD::MP4::MPDAdaptationSetCtx*> *streamsASCtxs,
        std::map<uint32_t, VCD::MP4::MPDAdaptationSetCtx*> *extractorASCtxs,
        SegmentationInfo *segInfo,
        VCD::OMAF::ProjectionFormat projType,
        Rational frameRate,
        uint8_t  videoNum,
        bool cmafEnabled);

typedef void DestroyMPDWriter(MPDWriterBase*);

#endif /* _DASHMPDWRITERPLUGINAPI_H_ */
