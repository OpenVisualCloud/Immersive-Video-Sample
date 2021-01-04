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
//! \file:   TileSelectionPlugins_API.h
//! \brief:  the class for general tile selection API.
//! \detail: it's class API to select tiles for all kinds of projections.
//!
//! Created on Dec 9, 2020, 3:18 PM
//!

#ifndef _TILESELECTION_PLUGINS_API_H_
#define _TILESELECTION_PLUGINS_API_H_

#include "360SCVPAPI.h"

class TileSelection
{
public:
    //!
    //! \brief  construct
    //!
    TileSelection() {};
    //!
    //! \brief  de-construct
    //!
    virtual ~TileSelection() {};
    //! \brief Initialze the pan zoom tile selection module
    //!
    //! \return int32_t Error code
    //!
    virtual int32_t Initialize(param_360SCVP *pParam) = 0;
    //! \brief Set viewport instant information
    //!
    //! \return int32_t Error code
    virtual int32_t SetViewportInfo(HeadPose* pose) = 0;
    //! \brief Tile selection process
    //!
    //! \param  [in]
    //!
    //! \return int32_t tile number in total
    //!
    virtual int32_t GetTilesInViewport(TileDef* pOutTile) = 0;
    //! \brief UnInitialze the pan zoom tile selection module
    //!
    //! \return int32_t Error code
    //!
    virtual int32_t UnInit() = 0;
};

typedef TileSelection* CreateTileSelection();
typedef void DestroyTileSelection(TileSelection*);

#endif /* _TILESELECTION_PLUGINS_API_H_ */
