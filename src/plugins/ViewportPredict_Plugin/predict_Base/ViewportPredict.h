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

//! \file:   ViewportPredict.h
//! \brief:  the class for viewport predict
//! \detail: it's class to predict viewport.
//!
//! Created on April 2, 2020, 1:18 PM
//!

#ifndef VIEWPORTPREDICT_H_
#define VIEWPORTPREDICT_H_

#include <stdlib.h>
#include <list>
#include "../../../utils/data_type.h"
#include "../../../utils/ns_def.h"

VCD_OMAF_BEGIN

class ViewportPredict
{
public:
    //!
    //! \brief  construct
    //!
    ViewportPredict();
    //!
    //! \brief  de-construct
    //!
    ~ViewportPredict();
    //! \brief Initialze the viewport prediction algorithm
    //!
    //! \param  [in] std::string
    //!              viewport prediction method
    //!         [in] uint32_t
    //!              pose interval
    //!         [in] uint32_t
    //!              previous pose count
    //!         [in] uint32_t
    //!              predict interval
    //!
    void Initialize(uint32_t pose_interval, uint32_t pre_pose_count, uint32_t predict_interval);
    //! \brief viewport prediction process
    //!
    //! \param  [in] std::list<ViewportAngle>
    //!              pose history
    //! \return ViewportAngle*
    //!              return predicted viewport pose
    //!
    virtual ViewportAngle* PredictPose(std::list<ViewportAngle> pose_history) = 0;

private:
    uint32_t    m_poseInterval;      //!< time interval between tow continous pose(ms)
    uint32_t    m_prePoseCount;      //!< size of pose history
    uint32_t    m_predictInterval;   //!< time interval of prediction(ms)

};

VCD_OMAF_END;

#endif /* VIEWPORTPREDICT_H_ */
