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
//! \file     ERPRender.h
//! \brief    Defines base class for ERPRender.
//!

#ifndef _AUTOVIEWSELECTOR_H_
#define _AUTOVIEWSELECTOR_H_

#include "../Common/Common.h"
#include "../../../utils/pose.h"
#include <math.h>

VCD_NS_BEGIN

enum class SelectorMode {
        FULLMOVE = 1,
        HALFMOVEHALFSTOP = 2,
};

class AutoViewSelector
{
public:
    AutoViewSelector();
    AutoViewSelector(SelectorMode mode, uint32_t view_num, uint32_t group_num) {
        mode_ = mode;
        view_num_in_group_ = view_num;
        group_num_ = group_num;
        pose_ = new HeadPose();
        frames_in_each_view_ = 0;
        stop_frames_ = 0;
        move_frames_ = 0;
    }
    virtual ~AutoViewSelector() {
        SAFE_DELETE(pose_);
    }

    void SetSamplesInSeg(uint32_t samples) {
        samples_in_seg_ = samples;
        frames_in_each_view_ = round((float(samples_in_seg_) / 2) / (view_num_in_group_ -1));
        stop_frames_ = samples_in_seg_ - frames_in_each_view_ * (view_num_in_group_ - 1);
        move_frames_ = samples_in_seg_ - stop_frames_;
    }

    HeadPose* GetPose(uint64_t pts) {
        if (mode_ == SelectorMode::FULLMOVE) {
            return ViewSelectorWithFullMode(pts);
        }
        else if (mode_ == SelectorMode::HALFMOVEHALFSTOP) {
            if (group_num_ == 2)
                return ViewSelectorWithHalfMode2(pts);
            else if (group_num_ == 3)
                return ViewSelectorWithHalfMode3(pts);
        }
        else {
            // LOG(ERROR) << "selector mode is not supported!" << endl;
            return nullptr;
        }
    }

private:

    int32_t FromMoveToStop_LeftToRight(uint32_t start_view_id, uint32_t offset_samples) {
        return offset_samples < move_frames_ ? offset_samples / frames_in_each_view_ + start_view_id : start_view_id + view_num_in_group_ - 1;
    }

    int32_t FromStopToMove_RightToLeft(uint32_t start_view_id, uint32_t offset_samples) {
        return offset_samples < stop_frames_ ? start_view_id : start_view_id - ((offset_samples - stop_frames_) / frames_in_each_view_ + 1);
    }

    int32_t FromStopToMove_LeftToRight(uint32_t start_view_id, uint32_t offset_samples) {
        return offset_samples < stop_frames_ ? start_view_id : start_view_id + ((offset_samples - stop_frames_) / frames_in_each_view_ + 1);
    }

    int32_t FromMoveToStop_RightToLeft(uint32_t start_view_id, uint32_t offset_samples) {
        return offset_samples < move_frames_ ? start_view_id - offset_samples / frames_in_each_view_ : start_view_id + 1 - view_num_in_group_;
    }

    HeadPose* ViewSelectorWithHalfMode3(uint64_t pts) {
        uint32_t seg_id = pts / samples_in_seg_ + 1;
        uint32_t offset_samples = pts % samples_in_seg_;
        if (seg_id % (group_num_ * 2) == 2) {// second group from left to right
            pose_->hViewId = FromMoveToStop_LeftToRight(view_num_in_group_, offset_samples);
        }
        else if (seg_id % (group_num_ * 2) == 5) {// second group from right to left
            pose_->hViewId = FromStopToMove_RightToLeft(view_num_in_group_ * 2 - 1, offset_samples);
        }
        else if (seg_id % (group_num_ * 2) == 1) {// first group from left to right
            pose_->hViewId = FromStopToMove_LeftToRight(0, offset_samples);
        }
        else if (seg_id % (group_num_ * 2) == 0) {// first group from right to left
            pose_->hViewId = FromMoveToStop_RightToLeft(view_num_in_group_ - 1, offset_samples);
        }
        else if (seg_id % (group_num_ * 2) == 3) {// third group from left to right
            pose_->hViewId = FromMoveToStop_LeftToRight(view_num_in_group_ * 2, offset_samples);
        }
        else if (seg_id % (group_num_ * 2) == 4) {// third group from right to left
            pose_->hViewId = FromStopToMove_RightToLeft(view_num_in_group_ * 3 - 1, offset_samples);
        }
        pose_->pts = pts;
        pose_->vViewId = 0;
        return pose_;
    }

    HeadPose* ViewSelectorWithHalfMode2(uint64_t pts) {
        uint32_t seg_id = pts / samples_in_seg_ + 1;
        uint32_t offset_samples = pts % samples_in_seg_;
        if (seg_id % (group_num_ * 2) == 1) {// first group from left to right
            pose_->hViewId = FromStopToMove_LeftToRight(0, offset_samples);
        }
        else if (seg_id % (group_num_ * 2) == 0) {// first group from right to left
            pose_->hViewId = FromMoveToStop_RightToLeft(view_num_in_group_ - 1, offset_samples);
        }
        else if (seg_id % (group_num_ * 2) == 2) {// second group from left to right
            pose_->hViewId = FromMoveToStop_LeftToRight(view_num_in_group_, offset_samples);
        }
        else if (seg_id % (group_num_ * 2) == 3) {// second group from right to left
            pose_->hViewId = FromStopToMove_RightToLeft(view_num_in_group_ * 2 - 1, offset_samples);
        }
        pose_->pts = pts;
        pose_->vViewId = 0;
        return pose_;
    }

    HeadPose* ViewSelectorWithFullMode(uint64_t pts) {
        // TBD
        return pose_;
    }

private:
    SelectorMode mode_;
    uint32_t view_num_in_group_;
    uint32_t group_num_;
    uint32_t samples_in_seg_;

    uint32_t frames_in_each_view_;
    uint32_t stop_frames_;
    uint32_t move_frames_;
    HeadPose *pose_;
};

VCD_NS_END
#endif /* _AUTOVIEWSELECTOR_H_ */