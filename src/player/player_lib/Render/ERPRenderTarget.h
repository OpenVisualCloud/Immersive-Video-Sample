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
//! \file     ERPRenderTarget.h
//! \brief    another implementation of RenderTarget to support multi render sources.
//!
#ifndef _ERPRENDERTARGET_H_
#define _ERPRENDERTARGET_H_

#include "../Common/Common.h"
#include "../Common/RegionData.h"
#include "RenderBackend.h"
#include "../MediaSource/RenderSourceFactory.h"
#include <map>
#include "RenderTarget.h"

VCD_NS_BEGIN

class ERPRenderTarget : public RenderTarget
{
public:
    ERPRenderTarget();
    ~ERPRenderTarget();

    //! \brief Initialize RenderTarget data according to mediaSource Info
    //!
    //! \param  [in] struct MediaSourceInfo *
    //!         Media Source Info
    //!         [in] uint32_t
    //!         render to texture fbo handle.
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    virtual RenderStatus Initialize(RenderSourceFactory* rsFactory);

    //! \brief Create a render target
    //!
    //! \param  [in] RenderBackend*
    //!         RenderBackend interface
    //!         [in] struct SourceWH*
    //!         target width and height
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    virtual RenderStatus CreateRenderTarget();

    //! \brief Update the render target
    //!
    //! \param  [in] RenderBackend*
    //!         RenderBackend interface
    //!         [in] struct RegionInfo*
    //!         region information including source width and height and rwpk.
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    virtual RenderStatus Update( float yaw, float pitch, float hFOV, float vFOV, uint64_t pts );

    virtual RenderStatus UpdateDisplayTex();


private:
    //! \brief transfer RegionInfo to a pair vectors describing the relationship between index and packedRegion information.
    //!
    //! \param  [in] std::vector<std::vector<std::pair<uint32_t, std::vector<uint32_t>>>> the high and low region relationship between index and offset/W/H in the packedSource
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    RenderStatus TransferRegionInfo(std::map<int32_t, std::vector<TileInformation>>& org_region);

    //! \brief get the RenderSource from a frame
    //!
    //! \param  [in] [out] std::vector<std::vector<std::pair<uint32_t, std::vector<uint32_t>>>>&
    //!         regionInfo including packed w/h/l/r and proj w/h/l/r and tile information.
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    RenderStatus GetRenderMultiSource( std::map<int32_t, std::vector<TileInformation>> &regionInfoTransfer);

    //! \brief get the needed tile Ids within the region
    //!
    //! \param  [in] SphereRegion*
    //!         SphereRegion* of the region
    //!         [in] SourceInfo*
    //!         Source Information including width heigth tile number
    //!
    //! \return std::vector<uint32_t>
    //!         return needed tile Ids within the region
    //!
    std::vector<uint32_t> GetRegionTileId(struct SphereRegion *sphereRegion, struct SourceInfo *sourceInfo);

    //! \brief transfer the tile Id to SphereRegion.
    //!
    //! \param  [in] uint32_t tileId
    //!         input tileId
    //!         [in] SourceInfo*
    //!         Source Information including width heigth tile number
    //!         [out] SphereRegion*
    //!         transfered sphereRegion according to the tile Id
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    RenderStatus TransferTileIdToRegion(uint32_t tileId, struct SourceInfo *sourceInfo, SphereRegion *sphereRegion);

    RenderStatus GetTilesInViewport(float yaw, float pitch, float hFOV, float vFOV, uint32_t row, uint32_t col, std::vector<uint32_t>& TilesInViewport);

    int32_t findQuality(RegionData *regionInfo, RectangularRegionWisePacking rectRWP, int32_t& source_idx);

    RenderStatus CalcQualityRanking();

    bool findTileID(std::vector<TileInformation> vecTile, uint32_t tile_id);

};

VCD_NS_END
#endif /* _ERPRENDERTARGET_H_ */