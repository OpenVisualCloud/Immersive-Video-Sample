// Copyright (C) <2019> Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0

#ifndef IMVIDEO_H
#define IMVIDEO_H

inline bool isHEVCMCTSVideoResolution(uint32_t width, uint32_t height) {
    return (width = 3840 && height == 2048)
        || (width = 7680 && height == 3840);
}

#endif /* IMVideo.h */
