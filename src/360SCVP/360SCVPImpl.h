/*
 * Copyright (c) 2018, Intel Corporation
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
#ifndef _360SCVP_IMPL_H_
#define _360SCVP_IMPL_H_
#include "360SCVPHevcTilestream.h"
#include "../utils/data_type.h"
#include "TileSelectionPlugins_API.h"
#include "360SCVPViewportImpl.h"

#define MAX_TILE_NUM 1000
//!
//! \struct: MapFaceInfo
//! \brief:  define the information of one face from input
//!          multiple face which may not be the standard Cube-3x2
//!          defined in OMAF. The information includes mapped
//!          face id in standard Cube-3x2 and the transfrom
//!          type of the face
//!
typedef struct MapFaceInfo
{
    uint8_t mappedStandardFaceId; //the corresponding face id in standard Cube-3x2 projection
    E_TransformType transformType; //face transform type
}MapFaceInfo;

class TstitchStream
{
protected:
    //generate view port
    generateViewPortParam       m_pViewportParam;
    //according to the viewport information, stitch the tiles in the viewport
    param_mergeStream           m_mergeStreamParam;
    //generate the streamstitch
    param_gen_tiledStream       m_streamStitch;

    // handle for the view port, parsing, and merge instance
    void*           m_pViewport;
    void*           m_pMergeStream;
    void*           m_pSteamStitch;

    // the sei enabling
    bool                m_seiRWPK_enable;
    bool                m_seiProj_enable;
    bool                m_seiSphereRot_enable;
    bool                m_seiFramePacking_enable;
    bool                m_seiViewport_enable;
    int32_t             m_projType;
    RegionWisePacking*  m_pRWPK;
    SphereRotation*     m_pSphereRot;
    FramePacking*       m_pFramePacking;
    OMNIViewPort*       m_pSeiViewport;

    //the below variables are got from the parsing
    int32_t         m_tileWidthCountSel[2];
    int32_t         m_tileHeightCountSel[2];
    int32_t         m_tileWidthCountOri[2];
    int32_t         m_tileHeightCountOri[2];
    SliceType       m_sliceType;
    nal_info       *m_pNalInfo[2]; //support two bitstream parsing and stitch
    int32_t         m_specialDataLen[2];
    TileDef        *m_pOutTile;
    point          *m_pUpLeft;
    point          *m_pDownRight;
    int32_t         m_maxSelTiles;
    int32_t         m_bVPSReady; //used in the usetype = E_PARSER_ONENAL
    int32_t         m_bSPSReady; //used in the usetype = E_PARSER_ONENAL
    int32_t         m_bPPSReady; //used in the usetype = E_PARSER_ONENAL
    HEVCState      *m_hevcState;
    unsigned char * m_specialInfo[2];
    int32_t         m_lrTilesInCol;
    int32_t         m_lrTilesInRow;
    int32_t         m_hrTilesInRow;
    int32_t         m_hrTilesInCol;
    RegionWisePacking     m_dstRwpk;
    TileSelection  *m_pTileSelection;

public:
    uint16_t        m_nalType;
    uint8_t         m_startCodesSize;
    uint16_t        m_seiPayloadType; //SEI payload type if nalu is for SEI
    uint16_t        m_sliceHeaderLen; //slice header length if nalu is for slice
    int32_t         m_dataSize;       //total bytes number of the nalu
    uint8_t        *m_data;           //output data
    uint8_t         m_usedType;
    int32_t         m_viewportDestWidth;
    int32_t         m_viewportDestHeight;
    int32_t         m_dstWidthNet;
    int32_t         m_dstHeightNet;
    int32_t         m_xTopLeftNet;
    int32_t         m_yTopLeftNet;

    TstitchStream();
    TstitchStream(TstitchStream& other);
    TstitchStream& operator=(const TstitchStream& other);
    virtual ~TstitchStream();
    int32_t  init(param_360SCVP* pParamStitchStream);
    int32_t  uninit();
    int32_t  getViewPortTiles();
    int32_t  feedParamToGenStream(param_360SCVP* pParamStitchStream);
    int32_t  setViewPort(HeadPose *pose);
    int32_t  doMerge(param_360SCVP* pParamStitchStream);
    int32_t  getFixedNumTiles(TileDef* pOutTile);
    int32_t  getTilesInViewport(TileDef* pOutTile);
    int32_t  parseNals(param_360SCVP* pParamStitchStream, int32_t parseType, Nalu* pNALU, int32_t streamIdx);
    int32_t  GenerateRWPK(RegionWisePacking* pRWPK, uint8_t *pRWPKBits, int32_t* pRWPKBitsSize);
    int32_t  GenerateProj(int32_t projType, uint8_t *pProjBits, int32_t* pProjBitsSize);
    int32_t  GeneratePPS(param_360SCVP* pParamStitchStream, TileArrangement* pTileArrange);
    int32_t  GenerateSPS(param_360SCVP* pParamStitchStream);
    int32_t  GenerateSliceHdr(param_360SCVP* pParam360SCVP, int32_t newSliceAddr);
    int32_t  getPicInfo(Param_PicInfo* pPicInfo);
    int32_t  getBSHeader(Param_BSHeader * bsHeader);
    int32_t  getRWPKInfo(RegionWisePacking *pRWPK);
    int32_t  setViewPortInfo(Param_ViewPortInfo* pViewPortInfo);
    int32_t  setSEIProjInfo(int32_t projType);
    int32_t  setSEIRWPKInfo(RegionWisePacking* pRWPK);
    int32_t  setSphereRot(SphereRotation* pSphereRot);
    int32_t  setFramePacking(FramePacking* pFramePacking);
    int32_t  setViewportSEI(OMNIViewPort* pSeiViewport);
    int32_t  doStreamStitch(param_360SCVP* pParamStitchStream);
    int32_t  merge_one_tile(uint8_t **pBitstream, oneStream_info* pSlice, GTS_BitStream *bs, bool bFirstTile);
    int32_t  GenerateRwpkInfo(RegionWisePacking *dstRwpk);
    int32_t  EncRWPKSEI(RegionWisePacking* pRWPK, uint8_t *pRWPKBits, uint32_t* pRWPKBitsSize);
    int32_t  DecRWPKSEI(RegionWisePacking* pRWPK, uint8_t *pRWPKBits, uint32_t RWPKBitsSize);
    int32_t  getContentCoverage(CCDef* pOutCC);
    TileDef* getSelectedTile();
    int32_t  getTilesByLegacyWay(TileDef* pOutTile);
    int32_t  SetLogCallBack(LogFunction logFunction);

protected:
    int32_t initMerge(param_360SCVP* pParamStitchStream, int32_t sliceSize);
    int32_t initViewport(Param_ViewPortInfo* pViewPortInfo, int32_t tilecolCount, int32_t tilerowCount);
    int32_t merge_partstream_into1bitstream(int32_t totalInputLen);
    int32_t ConvertTilesIdx(uint16_t tilesNum);
    int32_t initTileInfo(param_360SCVP* pParamStitchStream);

private:
    void* m_pluginLibHdl;
    void* m_createPlugin;
    void* m_destroyPlugin;
    bool  m_bNeedPlugin;
    ITileInfo* m_tilesInfo;
    MapFaceInfo* m_mapFaceInfo;
};// END CLASS DEFINITION

#endif // _360SCVP_IMPL_H_
