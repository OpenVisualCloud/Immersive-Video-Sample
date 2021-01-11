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

/*
 *  File:   pose.h
 *  Author: media
 *
 *  Created on January 5, 2021, 10:11 AM
 */

#ifndef _POSE_H_
#define _POSE_H_

#ifdef __cplusplus
extern "C" {
#endif

//!
//! \brief  This enum expresses the planar video viewport mode
//!
//! ORIENT_NONE,    no moving or scaling
//! ORIENT_ZOOMIN,  zoom-in mode
//! ORIENT_ZOOMOUT, zoom-out mode
//! ORIENT_MOVE,    move viewport in parallel to the image surface
//!
typedef enum {
  ORIENT_NONE = 0,
  ORIENT_ZOOMIN = 1,
  ORIENT_ZOOMOUT = 2,
  ORIENT_MOVE = 3,
} ViewOrientMode;

//!
//! \brief  This structure is for the planar video orientation
//!
//! \param    orientation,    input,     moving direction vector in angle counter clockwise. Zero is for the right
//! \param    mode,           input,     orientation mode to indicate which viewport mode is currently applied
//!
typedef struct VIEWORIENT {
  float orientation; //[0,2 * PI)
  ViewOrientMode mode;
} ViewOrient;

//!
//! \brief  This structure is for the head pose parameters
//!
//! \param    yaw,           input,     the viewing angle yaw in degree
//! \param    pitch,         input,     the viewing angle yaw in degree
//! \param    centerX,       input,     the viewport center X-axis coordinate
//! \param    centerY,       input,     the viewport center Y-axis coordinate
//! \param    zoomFactor,    input,     the viewport zoom factor calculated by the viewport area divided by the maximum frame area
//! \param    ViewOrient,    input,     the viewport orientation mode and moving direction as defined
//! \param    speed,         input,     the viewport moving speed expressed by the portion of the maximum frame width
//!
typedef struct HEADPOSE {
  float    yaw;
  float    pitch;
  int32_t  centerX;
  int32_t  centerY;
  float    zoomFactor;
  ViewOrient viewOrient;
  float    speed;
  uint64_t pts;
} HeadPose;

typedef struct HEADSETINFO {
  HeadPose* pose;
  float viewPort_hFOV;
  float viewPort_vFOV;
  int32_t viewPort_Width;
  int32_t viewPort_Height;
} HeadSetInfo;

typedef struct Viewport {
  int32_t x;
  int32_t y;
  int32_t height;
  int32_t width;
  int32_t faceId;
} Viewport;

typedef enum {
  HIGH = 0,
  LOW = 1,
  END = 2,
}ViewportPriority;

typedef struct VIEWPORTANGLE {
  // Euler angle
  float yaw;
  float pitch;
  float roll;
  uint64_t pts;
  ViewportPriority priority;
} ViewportAngle;

#ifdef __cplusplus
}
#endif

#endif /* POSE_H */
