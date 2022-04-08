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
//! \file:   Segmentation.h
//! \brief:  Segmentation base class definition
//! \detail: Define the operation and needed data for segmentation, including
//!          writing segments for media streams and extractor tracks, called
//!          by OmafPackage in a separate thread.
//!
//! Created on April 30, 2019, 6:04 AM
//!

#ifndef _SEGMENTATION_H_
#define _SEGMENTATION_H_

#include "MediaStream.h"
#include "ExtractorTrackManager.h"
//#include "MpdGenerator.h"
#include "DashMPDWriterPluginAPI.h"

VCD_NS_BEGIN

//!
//! \class Segmentation
//! \brief Define the operation and needed data for segmentation
//!

class Segmentation
{
public:
    //!
    //! \brief  Constructor
    //!
    Segmentation();

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
    //!         which includs segmentation information
    //!
    Segmentation(std::map<uint8_t, MediaStream*> *streams, ExtractorTrackManager *extractorTrackMan, InitialInfo *initInfo, PackingSourceMode sourceMode);

    Segmentation(const Segmentation& src);

    Segmentation& operator=(Segmentation&& other);

    //!
    //! \brief  Destructor
    //!
    virtual ~Segmentation();

    //!
    //! \brief  Initialize the basic process
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t Initialize();

    //!
    //! \brief  Execute the segmentation process for
    //!         all video streams
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t VideoSegmentation() = 0;

    //!
    //! \brief  End the segmentation process for
    //!         all video streams
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t VideoEndSegmentation() = 0;

    //!
    //! \brief  Execute the segmentation process for
    //!         all audio streams
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t AudioSegmentation() = 0;

    //!
    //! \brief  End the segmentation process for
    //!         all audio streams
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t AudioEndSegmentation() = 0;

private:

    //!
    //! \brief  Create DASH segment writer plugin handle
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t CreateSegWriterPluginHdl();

    //!
    //! \brief  Create DASH MPD writer plugin handle
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t CreateMPDWriterPluginHdl();

protected:
    std::map<uint8_t, MediaStream*> *m_streamMap;           //!< media streams map set up in OmafPackage
    ExtractorTrackManager           *m_extractorTrackMan;   //!< pointer to the extractor track manager created in OmafPackage
    SegmentationInfo                *m_segInfo;             //!< pointer to the segmentation information
    uint64_t                        m_trackIdStarter;       //!< track index starter
    Rational                        m_frameRate;            //!< the frame rate of the video

    bool                            m_isCMAFEnabled;
    const char                      *m_segWriterPluginPath;
    const char                      *m_segWriterPluginName;
    void                            *m_segWriterPluginHdl;
    PackingSourceMode               m_sourceMode;
    const char                      *m_mpdWriterPluginPath;
    const char                      *m_mpdWriterPluginName;
    void                            *m_mpdWriterPluginHdl;
};

VCD_NS_END;
#endif /* _SEGMENTATION_H_ */
