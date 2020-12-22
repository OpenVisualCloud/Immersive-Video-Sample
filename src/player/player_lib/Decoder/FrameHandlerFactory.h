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
//! \file     FrameHandlerFactory.h
//! \brief    Defines class for FrameHandler Factory which is used to manage frame Handler.
//!           each decoder might have frame handler; decoder list is dynamic, so handler is
//!           dynamic too. need the factory to be a bridge to manage the handler and let
//!           other render source can use the handling result. There should be derived classes
//!           from FrameHandler and FrameHandlerFactory to implement the interface
//!

#ifndef _FRAMEHANDLERFACTORY_H_
#define _FRAMEHANDLERFACTORY_H_

#include "../Common/Common.h"
#include "FrameHandler.h"
#include <map>

VCD_NS_BEGIN
class FrameHandlerFactory
{
public:
     FrameHandlerFactory()=default;
     virtual ~FrameHandlerFactory()=default;
public:
     //!
     //! \brief interface to create an frame processing handler for a video_id
     //!
     //! \param  [in] video_id, ID of the relative video
     //! \return FrameHandler*
     //!         non-null ptr if success, null ptr if fail
     virtual FrameHandler* CreateHandler(uint32_t video_id, uint32_t tex_id)=0;

     //!
     //! \brief remove an handler relative to video_id
     //!
     //! \param  [in] video_id: ID of the relative video
     //! \return RenderStatus
     //!         RENDER_STATUS_OK if success, else fail reason
     virtual RenderStatus RemoveHandler(uint32_t video_id)=0;

     //!
     //! \brief remove all handler managed in the factory
     //!
     //! \param  [in]
     //! \return RenderStatus
     //!         RENDER_STATUS_OK if success, else fail reason
     virtual RenderStatus RemoveAll()=0;

};

VCD_NS_END

#endif