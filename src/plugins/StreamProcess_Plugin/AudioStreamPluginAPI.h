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
 */

//!
//! \file:  AudioStreamPluginAPI.h
//! \brief: Audio stream process plugin interfaces
//!
//! Created on November 6, 2020, 6:04 AM
//!


#ifndef _AUDIOSTREAMPLUGINAPI_H_
#define _AUDIOSTREAMPLUGINAPI_H_

#include "MediaStream.h"
#include "VROmafPacking_def.h"
#include "OmafStructure.h"

#include <list>
#include <mutex>

#define SAMPLES_NUM_IN_FRAME    1024

//!
//! \class AudioStream
//! \brief Define the interface class for audio stream process plugin
//!

class AudioStream : public MediaStream
{
public:
    //!
    //! \brief  Constructor
    //!
    AudioStream() {};

    //!
    //! \brief  Destructor
    //!
    virtual ~AudioStream() {};

    //!
    //! \brief  Initialize the audio stream
    //!
    //! \param  [in] streamIdx
    //!         the index of the audio in all streams
    //! \param  [in] bs
    //!         pointer to the BSBuffer information of
    //!         the audio stream, including sample rate
    //!         and bitrate, channel number and so on.
    //! \param  [in] initInfo
    //!         pointer to the initial information input
    //!         by the library interface
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t Initialize(uint8_t streamIdx, BSBuffer *bs, InitialInfo *initInfo) = 0;

    //!
    //! \brief  Get the sample rate of the audio stream
    //!
    //! \return uint16_t
    //!         the sample rate of the audio stream
    //!
    virtual uint32_t GetSampleRate() = 0;

    //!
    //! \brief  Get the channel number of the audio stream
    //!
    //! \return uint8_t
    //!         the channel number of the audio stream
    //!
    virtual uint8_t GetChannelNum() = 0;

    //!
    //! \brief  Get the bit rate of the audio stream
    //!
    //! \return uint16_t
    //!         the bit rate of the audio stream, in the unit of kbps
    //!
    virtual uint16_t  GetBitRate() = 0;

    //!
    //! \brief  Add frame information for a new frame into
    //!         frame information list of the audio
    //!
    //! \param  [in] frameInfo
    //!         pointer to the frame information of the new frame
    //!
    //! \return int32_t
    //!         ERROR_NONE if success, else failed reason
    //!
    virtual int32_t AddFrameInfo(FrameBSInfo *frameInfo) = 0;

    //!
    //! \brief  Fetch the front frame information in frame
    //!         information list as current frame information
    //!
    //! \return void
    //!
    virtual void SetCurrFrameInfo() = 0;

    //!
    //! \brief  Get the current frame information
    //!
    //! \return FrameBSInfo*
    //!         the pointer to the current frame information
    //!
    virtual FrameBSInfo* GetCurrFrameInfo() = 0;

    //!
    //! \brief  Destroy current frame information
    //!
    //! \return void
    //!
    virtual void DestroyCurrFrameInfo() = 0;

    //!
    //! \brief  Destroy all frame information belong to current
    //!         segment
    //!
    //! \return void
    //!
    virtual void DestroyCurrSegmentFrames() = 0;

    //!
    //! \brief  Set the EOS status for the audio stream
    //!
    //! \param  [in] isEOS
    //!         the status will be set to EOS of the audio stream
    //!
    //! \return void
    //!
    virtual void SetEOS(bool isEOS) = 0;

    //!
    //! \brief  Get the EOS status of the audio stream
    //!
    //! \return bool
    //!         the EOS status of the audio stream
    //!
    virtual bool GetEOS() = 0;

    //!
    //! \brief  Add current frame to frames list for current
    //!         segment
    //!
    //! \return void
    //!
    virtual void AddFrameToSegment() = 0;

    //!
    //! \brief  Get current buffered frames number in
    //!         frame list which have not been written
    //!         into segments
    //!
    //! \return uint32_t
    //!         current buffered frames number
    //!
    virtual uint32_t GetBufferedFrameNum() = 0;

    virtual std::vector<uint8_t> GetPackedSpecCfg() = 0;

    virtual uint8_t  GetHeaderDataSize() = 0;

protected:
};

typedef AudioStream* CreateAudioStream();
typedef void DestroyAudioStream(AudioStream*);

#endif /* _AUDIOSTREAMPLUGINAPI_H_ */
