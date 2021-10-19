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
#ifndef __360SCVP_VIEWPORTIMPL__
#define __360SCVP_VIEWPORTIMPL__

#include "360SCVPAPI.h"
#include "360SCVPGeometry.h"

#include <sstream>
#include <vector>
#include <list>

/* The compare threshold of two double variables */
#define DOUBLE_COMPARE_THRESH (-double(1e-8))

///< for cubemap, given the facesize (960x960), the maxsimum viewport size is defined in the below table
typedef struct SIZE_DEF
{
    int32_t x;
    int32_t y;
}SIZE;

#define MAX_FOV_ANGLE 100
#define NORMAL_FACE_SIZE 960
#define NORMAL_PITCH_MIN -75
#define NORMAL_PITCH_MAX -15

enum FOVAngle
{
    FOV_100 = 0,
    FOV_90,
    FOV_80,
    FOV_70,
    FOV_Angle_NUM
};

enum CubeFace
{
    FACE_PX = 0,
    FACE_NX,
    FACE_PY,
    FACE_NY,
    FACE_PZ,
    FACE_NZ
};
// ====================================================================================================================
// Class definition,
// ====================================================================================================================
struct ITileInfo
{
    short x;
    short y;
    short tilewidth;
    short tileheight;
    int32_t   faceId;
    uint32_t  isOccupy;
    float vertPos;
    float horzPos;
    float horzPosBottomRight;
    float horzPosBottomLeft;
    float horzPosTopRight;
    float vertPosBottomRight; //Record all 4 endpoint position for Cubemap usage
    float vertPosBottomLeft;
    float vertPosTopRight;
};

/*  Point description on Sperial surface */
typedef struct SPHEREPOINT
{
    POSType alpha;
    POSType thita;
    POSType phi;
    SPos cord3D;
    SPos cord2D;
} SpherePoint;

/// generate viewport class
class TgenViewport
{
public:
  //tile info
    uint32_t m_tileNumRow;
    uint32_t m_tileNumCol;
    ITileInfo *m_srd;
    int32_t m_numFaces;
    SPos *m_pUpLeft;
    SPos *m_pDownRight;
    // source specification
    int32_t       m_iFrameRate;                                     ///< source frame-rates (Hz)
    int32_t       m_iSourceWidth;                                   ///< source width in pixel
    int32_t       m_iSourceHeight;                                  ///< source height in pixel (when interlaced = field height)
    int32_t       m_iInputWidth;
    int32_t       m_iInputHeight;
    SVideoInfo m_sourceSVideoInfo;
    SVideoInfo m_codingSVideoInfo;
    int32_t       m_iCodingFaceWidth;
    int32_t       m_iCodingFaceHeight;
    int32_t       m_aiPad[2];                                       ///< number of padded pixels for width and height
    int32_t   m_faceSizeAlignment;
    int32_t       m_maxTileNum;
    UsageType     m_usageType;
    Param_VideoFPStruct m_paramVideoFP;
    SpherePoint   *m_pViewportHorizontalBoundaryPoints;
    SpherePoint   *m_pViewportVerticalBoundaryPoints;
    inline int32_t round(POSType t) { return (int32_t)(t+ (t>=0? 0.5 :-0.5)); }

public:
    TgenViewport();
    TgenViewport(TgenViewport& src);
    virtual ~TgenViewport();
    TgenViewport& operator=(const TgenViewport& src);

public:
    int32_t  create(uint32_t tileNumRow, uint32_t tileNumCol);  ///< create option handling class
    void     destroy();    ///< destroy option handling class
    int32_t  parseCfg(  );  ///< parse configuration file to fill member variables
    int32_t  convert();
    int32_t  ERPSelectRegion(short inputWidth, short inputHeight, short dstWidth, short dstHeight);
    int32_t  cubemapSelectRegion();
    //analysis;
    bool     isInside(int32_t x, int32_t y, int32_t width, int32_t height, int32_t faceId);
    //int32_t  CubemapIsInsideFaces();
    int32_t  calcTilesInViewport(ITileInfo* pTileInfo, int32_t tileCol, int32_t tileRow);
    int32_t  CalculateViewportBoundaryPoints();
    int32_t  CubemapCalcTilesGrid();
    int32_t  getContentCoverage(CCDef* pOutCC, int32_t coverageShapeType);

private:
    /* calculateLongitudeFromThita:                              *
     *    Param:                                                 *
     *        Latti: Point spherial lattitude                    *
     *        Phi:   The angle to the sphere center              *
     *        maxLongiOffset: The maximum longitude offset       *
     *    Return:                                                *
     *        The point longitude                                */
    double    calculateLongitudeFromThita(double Latti, double phi, double maxLongiOffset);
    /* calculateLattitudeFromPhi:                                *
     *    Param:                                                 *
     *        phi:   The angle to the sphere center              *
     *        pitch: The lattitude of the center point of the    *
     *               current great circle                        *
     *    Return:                                                *
     *        The point lattitude                                */
    double    calculateLattitudeFromPhi(double phi, double pitch);
    /* calculateLatti:                                *
     *    Param:                                                 *
     *        pitch: The lattitude of the center point of the    *
     *               current great circle                        *
     *        hFOV: The horizontal FOV of the viewport           *
     *    Return:                                                *
     *        The viewport's topleft point lattitude             */
    double    calculateLatti(double pitch, double hFOV);
    /* calculateLatti:                                *
     *    Param:                                                 *
     *        pitch: The lattitude of the center point of the    *
     *               current great circle                        *
     *        latti: The point lattitudehorizontal FOV of the    *
     *        viewport                                           *
     *    Return:                                                *
     *        The point longitude offset to the viewport center  */
    float    calculateLongiByLatti(float latti, float pitch);
    /* ERPselectTilesInsideOnOneRow: Choose tiles in the give row   *
     *    Param:                                                    *
     *        pTileInfo: Tile Info for output                       *
     *        leftCol: The most left tile index of current row      *
     *        rightCol: The most left tile index of current row     *
     *        tileNumCol: The tile number in one row                *
     *        row: The row number                                   *
     *        hFOV: The horizontal FOV of the viewport              *
     *    Return:                                                   *
     *        Error code.                                           */
    int32_t  ERPselectTilesInsideOnOneRow(ITileInfo* pTileInfo, int32_t tileNumCol, float leftCol, float rightCol, int32_t row);
    /* CubemapPolar2Cartesian:  Convert point coordinates from polar to *
     *                          cartesian expression in both 2D and 3D  *
     *    Param:                                                        *
     *        pPoint: Point with polar coordinates expression           *
     *    Return:                                                       *
     *        Error code                                                */
    int32_t CubemapPolar2Cartesian(SpherePoint* pPoint);
    /* CubemapGetFaceBoundaryCrossingPoints:                          *
     *                 Calculate cross point axis of the face         *
     *                 boundary and the connection line of two given  *
     *                 points                                         *
     *    Param:                                                      *
     *        upLeftPoint:         The first given point which is     *
     *                             offen in the up or left direction  *
     *        downRightPoint:      The second given point which is    *
     *                             in the down or right direction     *
     *        crossBoundaryPoints: The list which stores the crossing *
     *                             point of the cube's boundary and   *
     *                             connection line of the two input   *
     *                             points                             *
     *    Return:                                                     *
     *        Error code                                              */
    int32_t CubemapGetFaceBoundaryCrossingPoints(SpherePoint* upLeftPoint, SpherePoint* downRightPoint, std::list<SpherePoint>* crossBoundaryPoints);
     /* CubemapGetViewportProjInFace:  Calculate the projection of   *
      *                                the viewport on the give      *
      *                                cube face                     *
      *    Param:                                                    *
      *        faceId:            The face Id                        *
      *        refBoundaryPoints: The reference points in a list     *
      *                           which can provide the              *
      *                           projection area on each face       *
      *    Return:                                                   *
      *        error code                                            */
    int32_t CubemapGetViewportProjInFace(int32_t faceId, std::list<SpherePoint>* crossBoundaryPoints);
};// END CLASS DEFINITION

//! \}
#endif // __360SCVP_VIEWPORTIMPL__
