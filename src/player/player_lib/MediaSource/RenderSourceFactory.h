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
//! \file     SWRenderSourceFactory.h
//! \brief    Defines class for Managing SWRenderSource.
//!
#ifndef _RENDERSOURCEFACTORY_H_
#define _RENDERSOURCEFACTORY_H_

#include "../Common/Common.h"
#include "RenderSource.h"
#include "../Decoder/FrameHandlerFactory.h"

VCD_NS_BEGIN
class RenderSourceFactory : public FrameHandlerFactory
{
public:
    RenderSourceFactory(void *window);
    virtual ~RenderSourceFactory();

public:
     //!
     //! \brief interface to create an frame processing handler for a video_id
     //!
     //! \param  [in] video_id, ID of the relative video
     //! \return FrameHandler*
     //!         non-null ptr if success, null ptr if fail
     virtual FrameHandler* CreateHandler(uint32_t video_id, uint32_t tex_id);

     //!
     //! \brief remove an handler relative to video_id
     //!
     //! \param  [in] video_id: ID of the relative video
     //! \return RenderStatus
     //!         RENDER_STATUS_OK if success, else fail reason
     virtual RenderStatus RemoveHandler(uint32_t video_id);

     //!
     //! \brief remove all handler managed in the factory
     //!
     //! \param  [in]
     //! \return RenderStatus
     //!         RENDER_STATUS_OK if success, else fail reason
     virtual RenderStatus RemoveAll();

     //!
     //! \brief SetVideoSize
     //!
     //! \param  [in]  width
     //!         [in]  height
     void SetSourceResolution(uint32_t number, SourceResolution *resolution){
          m_sourceNumber = number;
          m_sourceResolution = resolution;
     };

     SourceResolution* GetSourceResolution()
     {
          return m_sourceResolution;
     };

     uint32_t GetSourceNumber()
     {
          return m_sourceNumber;
     };

     void SetProjectionFormat(int32_t projFormat)
     {
          m_projFormat = projFormat;
     };

     int32_t GetProjectionFormat()
     {
          return m_projFormat;
     };

     std::map<uint32_t, RenderSource*> GetRenderSources(){return mMapRenderSource;};

     uint32_t GetHighTileRow(){return m_highTileRow;};
     void SetHighTileRow(uint32_t row) {m_highTileRow = row;};

     uint32_t GetHighTileCol(){return m_highTileCol;};
     void SetHighTileCol(uint32_t col) {m_highTileCol = col;};

private:
     std::map<uint32_t, RenderSource*> mMapRenderSource;    //! map for <video_id and handler>
     SourceResolution                 *m_sourceResolution;
     uint32_t                          m_sourceNumber;
     uint32_t                          m_highTileRow;
     uint32_t                          m_highTileCol;
     int32_t                           m_projFormat;
     void                              *share_window;

};

VCD_NS_END
#endif