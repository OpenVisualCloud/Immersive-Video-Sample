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
//! \file:   ExtractorTrackManager.h
//! \brief:  Extractor track manager class definition
//! \detail: Define the extractor track manager to handle all extractor tracks.
//!
//! Created on April 30, 2019, 6:04 AM
//!

#ifndef _EXTRACTORTRACKMANAGER_H_
#define _EXTRACTORTRACKMANAGER_H_

#include "VROmafPacking_data.h"
#include "VROmafPacking_def.h"
#include "MediaStream.h"
//#include "VideoStream.h"
#include "ExtractorTrack.h"
#include "ExtractorTrackGenerator.h"

VCD_NS_BEGIN

//!
//! \class ExtractorTrackManager
//! \brief Define the extractor track manager to handle all extractor tracks
//!

class ExtractorTrackManager
{
public:
    //!
    //! \brief  Constructor
    //!
    ExtractorTrackManager();

    //!
    //! \brief  Copy Constructor
    //!
    //! \param  [in] initInfo
    //!         initial information input by library interface
    //!
    ExtractorTrackManager(InitialInfo *initInfo);

    ExtractorTrackManager(const ExtractorTrackManager& src);

    ExtractorTrackManager& operator=(ExtractorTrackManager&& other);

    //!
    //! \brief  Destructor
    //!
    ~ExtractorTrackManager();

    //!
    //! \brief  Initialize the extractor track manager
    //!
    //! \param  [in] mediaStreams
    //!         pointer to media streams map set up in OmafPackage
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t Initialize(std::map<uint8_t, MediaStream*> *mediaStreams);

    //!
    //! \brief  Get all extractor tracks in the manager
    //!
    //! \return std::map<uint8_t, ExtractorTrack*>*
    //!         the pointer to the extractor tracks map
    //!
    std::map<uint16_t, ExtractorTrack*>* GetAllExtractorTracks()
    {
        return &m_extractorTracks;
    }
private:
    //!
    //! \brief  Add each extractor track into the map
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    int32_t AddExtractorTracks();
private:
    std::map<uint8_t, MediaStream*>    *m_streams;            //!< media streams map set up in OmafPackage
    std::map<uint16_t, ExtractorTrack*> m_extractorTracks;     //!< extractor tracks map
    ExtractorTrackGenerator            *m_extractorTrackGen;  //!< extractor track generator to generate all extractor tracks
    InitialInfo                        *m_initInfo;           //!< the initial information input by library interface
};

VCD_NS_END;
#endif /* _EXTRACTORTRACKMANAGER_H_ */
