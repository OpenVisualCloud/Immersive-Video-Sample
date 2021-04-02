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
//! \file     DecoderManager.h
//! \brief    Defines class for DecoderManager which is used to manage the decoders.
//!

#ifndef _DECODERMANAGER_H_
#define _DECODERMANAGER_H_

#include "../Common/Common.h"
#include "MediaDecoder.h"
#include "FrameHandler.h"
#include "FrameHandlerFactory.h"
#include "../../../utils/Threadable.h"
#include <map>
#include <vector>

VCD_NS_BEGIN

class DecoderManager{
public:
     DecoderManager();
     ~DecoderManager();

public:
     //!
     //! \brief  Initialize decoder manager when decoding information changes
     //!
     RenderStatus Initialize(FrameHandlerFactory* factory);

     ///Video-relative operations

     //!
     //! \brief  reset the decoder when decoding information changes
     //!
     RenderStatus ResetDecoders();

     //!
     //! \brief  reset the decoder when decoding information changes
     //!
     //! \param  [in] video_id: the id of video which is complied with DashAccess definition
     //!         [in] video_codec: the codec of input stream
     //! \return RenderStatus
     //!         RENDER_STATUS_OK if success, else fail reason
     //!
     RenderStatus CreateVideoDecoder(uint32_t video_id, Codec_Type video_codec, uint64_t startPts);

     //!
     //! \brief  reset the decoder when decoding information changes
     //!
     //! \param  [in] packets: the packet lists gotten from DashAccess. it's a dynamic list.
     //!
     //! \return RenderStatus
     //!         RENDER_STATUS_OK if success, else fail reason
     //!
     RenderStatus SendVideoPackets( DashPacket* packets, uint32_t cnt );

     //!
     //! \brief  reset the decoder when decoding information changes
     //!
     //! \param  [in] struct RenderConfig
     //!         render Configuration
     //!
     //! \return RenderStatus
     //!         RENDER_STATUS_OK if success, else fail reason
     //!
     RenderStatus UpdateVideoFrame( uint32_t video_id, uint64_t pts, int64_t *corr_pts );

     //!
     //! \brief  reset the decoder when decoding information changes
     //!
     //! \param  [in] struct RenderConfig
     //!         render Configuration
     //!
     //! \return RenderStatus
     //!         RENDER_STATUS_OK if success, else fail reason
     //!
     RenderStatus UpdateVideoFrames( uint64_t pts, int64_t *corr_pts );

     void SetSurface(uint32_t video_id, uint32_t tex_id, void* surface)
     {
          m_surfaces[video_id] = surface;
          m_textures[video_id] = tex_id;
     };

     void SetDecodeInfo(DecodeInfo info) { m_decodeInfo = info; };
     ///Audio-relative operations: TBD
     RenderStatus CreateAudioDecoder(uint32_t audio_id, uint32_t audio_codec){ return RENDER_STATUS_OK; };
     RenderStatus SendAudioPackets( DashPacket* packets ){ return RENDER_STATUS_OK; };
     RenderStatus UpdateAudioFrame( ){ return RENDER_STATUS_OK; };
     bool GetEOS() // when updating frame meeting eos in all video decoders
     {
          ScopeLock lock(m_mapDecoderLock);
          bool ret = true;
          if (m_mapVideoDecoder.empty())
               return false;
          for (auto it=m_mapVideoDecoder.begin(); it!=m_mapVideoDecoder.end(); it++)
          {
               if (!it->second->GetEOS())
               {
                    ret = false;
                    break;
               }
          }
          return ret;
     };

     bool IsReady(uint64_t pts)
     {
          bool isReadyStatus = true;
          if (m_mapVideoDecoder.empty())
          {
               return false;
          }
          for (auto it = m_mapVideoDecoder.begin(); it != m_mapVideoDecoder.end(); it++)
          {
               MediaDecoder* decoder = it->second;
               if (!decoder->IsReady(pts))
               {
                    LOG(INFO)<<"decoder is not ready!"<<std::endl;
                    isReadyStatus = false;
                    break;
               }
          }
          return isReadyStatus;
     }

private:
     ///Video-relative operations
     RenderStatus CheckVideoDecoders(vector<DashPacket*> packets, std::map<uint32_t, MediaDecoder*> decoderMap, uint32_t cnt, bool isCatchup);


private:
    std::map<uint32_t, MediaDecoder*>   m_mapVideoDecoder; //! the map of video decoders
    std::map<uint32_t, MediaDecoder*>   m_mapCatchupVideoDecoder; //! the map of catch-up video decoders
    std::map<uint32_t, MediaDecoder*>   m_mapAudioDecoder; //! the map of audio decoders
    FrameHandlerFactory*                m_handlerFactory;  //! the frameHandler factory to create frameHandler for each decoder
    ThreadLock                          m_mapDecoderLock;
    ThreadLock                          m_mapCatchupDecoderLock;
    std::vector<void*>                  m_surfaces;
    std::vector<uint32_t>               m_textures;
    DecodeInfo                          m_decodeInfo;
};

VCD_NS_END

#endif
