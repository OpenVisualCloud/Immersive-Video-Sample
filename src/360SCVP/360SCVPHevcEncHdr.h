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
#ifndef _360SCVP_HEVCENC_HDR_H_
#define _360SCVP_HEVCENC_HDR_H_

#include "360SCVPHevcParser.h"
#include "360SCVPBitstream.h"
#include "360SCVPAPI.h"

void hevc_write_bitstream_aud(GTS_BitStream *stream,    HEVCState * const state);
void hevc_write_parameter_sets(GTS_BitStream *stream, HEVCState * const state);
void hevc_write_slice_header(GTS_BitStream * stream, HEVCState * state);
uint32_t hevc_write_RwpkSEI(GTS_BitStream * stream, const RegionWisePacking* pRegion, int32_t temporalIdPlus1);
uint32_t hevc_write_ProjectionSEI(GTS_BitStream * stream, int32_t projType, int32_t temporalIdPlus1);
uint32_t hevc_write_SphereRotSEI(GTS_BitStream * stream, const SphereRotation* pSphereRot, int32_t temporalIdPlus1);
uint32_t hevc_write_FramePackingSEI(GTS_BitStream * stream, const FramePacking* pFramePacking, int32_t temporalIdPlus1);
uint32_t hevc_write_ViewportSEI(GTS_BitStream * stream, const OMNIViewPort* pViewport, int32_t temporalIdPlus1);

void hevc_write_pps(GTS_BitStream *stream, HEVCState * const state);
void hevc_write_sps(GTS_BitStream *stream, HEVCState * const state);

#endif

