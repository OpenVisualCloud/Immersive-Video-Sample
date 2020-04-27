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
//! \file     RenderTarget.h
//! \brief    Defines base class for RenderTarget.
//!
#ifndef _RENDERTARGET_H_
#define _RENDERTARGET_H_

#include "Common.h"
#include "RenderBackend.h"

VCD_NS_BEGIN

class RenderTarget
{
public:
    RenderTarget();
    virtual ~RenderTarget();
    //! \brief Initialize RenderTarget data according to mediaSource Info
    //!
    //! \param  [in] struct MediaSourceInfo *
    //!         Media Source Info
    //!         [in] uint32_t
    //!         render to texture fbo handle.
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    RenderStatus Initialize(struct MediaSourceInfo *mediaSourceInfo, uint32_t fboR2T);
    //! \brief Create a render target
    //!
    //! \param  [in] RenderBackend*
    //!         RenderBackend interface
    //!         [in] struct SourceWH*
    //!         target width and height
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    RenderStatus CreateRenderTarget(RenderBackend *renderBackend);
    //! \brief Update the render target
    //!
    //! \param  [in] RenderBackend*
    //!         RenderBackend interface
    //!         [in] struct RegionInfo*
    //!         region information including source width and height and rwpk.
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    RenderStatus Update(RenderBackend *renderBackend, struct RegionInfo *regionInfo, float yaw, float pitch, float hFOV, float vFOV);
    //! \brief Get the Fbo R2T Handle
    //!
    //! \return uint32_t
    //!         return the Fbo R2T Handle
    //!
    uint32_t GetFboR2THandle();
    //! \brief Get the Fbo Screen Handle
    //!
    //! \return uint32_t
    //!         return the Fbo Screen Handle
    //!
    uint32_t GetFboOnScreenHandle();
    //! \brief Get the texture Of R2S
    //!
    //! \return uint32_t
    //!         return texture Of R2S
    //!
    uint32_t GetTextureOfR2S();
    //! \brief Get the target width and height
    //!
    //! \return struct SourceWH
    //!         return target width and height
    //!
    struct SourceWH GetTargetWH();
    //! \brief Set the Fbo Screen Handle
    //!
    //! \param  [in] uint32_t
    //!         handle
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    RenderStatus SetFboOnScreenHandle(uint32_t handle);
    //! \brief Set the Fbo R2T Handle
    //!
    //! \param  [in] uint32_t
    //!         handle
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    RenderStatus SetFboR2THandle(uint32_t handle);
    //! \brief Set the texture Of R2S
    //!
    //! \param  [in] uint32_t
    //!         texture
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    RenderStatus SetTextureOfR2S(uint32_t texture);
    //! \brief Set the target width and height
    //!
    //! \param  [in] struct SourceWH
    //!         targetWH
    //!
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    RenderStatus SetTargetWH(struct SourceWH targetWH);

private:
    uint32_t m_fboR2THandle;      //input
    uint32_t m_fboOnScreenHandle; //output
    uint32_t m_textureOfR2S;      //render to screen
    struct SourceWH m_targetWH;   //ScreenTexture size
    bool m_isAllHighResoInView;   //isAllHighResoInView
    float m_avgChangedTime;      //average time to change from blur to clear

    //! \brief transfer RegionInfo to a pair vectors describing the relationship between index and packedRegion information.
    //!
    //! \param  [in] RegionInfo*
    //!         RegionInfo of packedSource
    //!
    //! \return std::vector<std::vector<std::pair<uint32_t, std::vector<uint32_t>>>>
    //!         return the high and low region relationship between index and offset/W/H in the packedSource
    //!
    std::vector<std::vector<std::pair<uint32_t, std::vector<uint32_t>>>> TransferRegionInfo(struct RegionInfo *regionInfo);

    //! \brief get the RenderSource from a frame
    //!
    //! \param  [in] struct RegionInfo*
    //!         RegionWisePacking for region
    //!         [out] std::vector<uint32_t> &hasHighTileIds
    //!         high resolution tile ids
    //!         [out] std::vector<uint32_t> &hasLowTileIds
    //!         low resolution tile ids that high tiles cannot cover.
    //!         [out] std::vector<std::vector<std::pair<uint32_t, std::vector<uint32_t>>>>&
    //!         regionInfo including packed w/h/l/r and proj w/h/l/r and tile information.
    //! \return RenderStatus
    //!         RENDER_STATUS_OK if success, else fail reason
    //!
    RenderStatus GetRenderMultiSource(struct RegionInfo *regionInfo, std::vector<uint32_t> &hasHighTileIds, std::vector<uint32_t> &hasLowTileIds, std::vector<std::vector<std::pair<uint32_t, std::vector<uint32_t>>>> &regionInfoTransfer);

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
    RenderStatus TransferTileIdToRegion(uint32_t tileId, SourceInfo *sourceInfo, SphereRegion *sphereRegion);

    RenderStatus GetTilesInViewport(float yaw, float pitch, float hFOV, float vFOV, uint32_t row, uint32_t col, std::vector<uint32_t>& TilesInViewport);
};

VCD_NS_END
#endif /* _RENDERTARGET_H_ */
