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
//! \file:   CmafSegment.h
//! \brief:
//! \detail: Derived from OmafSegment, used for CMAF support
//! Created on May 24, 2019, 11:07 AM
//!

#ifndef CMAFSEGMENT_H
#define CMAFSEGMENT_H

#include "OmafSegment.h"
#include "OmafReader.h"
#include "OmafMP4VRReader.h"

VCD_OMAF_BEGIN

class CmafSegment : public OmafSegment {
 public:
  using Ptr = std::shared_ptr<CmafSegment>;

 public:
  //
  // @brief constructor with dash source
  //
  CmafSegment();

  CmafSegment(DashSegmentSourceParams ds_params, int segCnt, bool bInitSegment = false);

  //!
  //! \brief  de-construct
  //!
  virtual ~CmafSegment();

  //
  // @brief open this segment, which will trigger the dash download
  //
  // @param[in] ds
  // @brief dash source params
  //
  // @param[in] cb
  // @brief dash open state change callback
  //
  // @return int
  // @brief calling success or not
  virtual int Open(std::shared_ptr<OmafDashSegmentClient> dash_client) noexcept;

  virtual std::unique_ptr<StreamBlock> PopOneStreamBlock() noexcept {
    if (chunk_stream_.GetStreamBlockSize() != 0) {
      return chunk_stream_.pop_front();
    }
    return nullptr;
  }

  inline map<uint32_t, uint32_t> GetIndexRange() { return index_range_; };

  //!
  //! \brief  generate chunk stream from dash stream thread function
  //!
  int32_t GenerateChunkStream();

  virtual bool HasProcessDone() { if (processed_chunk_id_ == (int32_t)chunk_num_ - 1) return true; else return false; };

private:

  //!
  //! \brief  get segment index box length according to chunk num
  //!
  int32_t GetSegmentIndexLength(uint32_t chunk_num, uint64_t& size);

  //!
  //! \brief  update  header stream according to input stream block
  //!
  int32_t UpdateHeaderStream(std::unique_ptr<StreamBlock> sb);

 private:

  StreamBlocks header_stream_; //<! segment index stream
  StreamBlocks chunk_stream_; //<! output chunk stream

  int64_t index_length_ = 0; //<! index box size

  // omaf reader
  OmafReader *reader_;

  map<uint32_t, uint32_t> index_range_; //<! first: chunk id, second: chunk size

  uint32_t start_chunk_id_ = 0;

};

VCD_OMAF_END

#endif /* CMAFSEGMENT_H */