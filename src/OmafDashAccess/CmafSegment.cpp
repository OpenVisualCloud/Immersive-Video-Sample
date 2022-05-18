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

/*
 * File:   CmafSegment.cpp
 * Author: media
 *
 * Created on May 24, 2019, 11:07 AM
 */

#include "CmafSegment.h"
#include <algorithm>

VCD_OMAF_BEGIN

CmafSegment::CmafSegment(DashSegmentSourceParams ds_params, int segCnt, bool bInitSegment)
    : OmafSegment(ds_params, segCnt, bInitSegment) {
  reader_ =  new OmafMP4VRReader();
  index_length_ = 0;
}

CmafSegment::~CmafSegment() {
  chunk_stream_.clear();
  index_stream_.clear();
  index_range_.clear();
  SAFE_DELETE(reader_);
}

int CmafSegment::Open(std::shared_ptr<OmafDashSegmentClient> dash_client) noexcept {
  try {
    if (dash_client.get() == nullptr) {
      return ERROR_NULL_PTR;
    }

    dash_client_ = std::move(dash_client);

    state_ = State::CREATE;

    // get segment index size according to chunk num (including cloc and header(styp+sidx))
    chunk_num_ = ds_params_.chunk_num_;
    start_chunk_id_ = ds_params_.start_chunk_id_;
    processed_chunk_id_ = ds_params_.start_chunk_id_ - 1;

    GetSegmentIndexLength(ds_params_.chunk_num_, (uint64_t &)index_length_);

    if (index_length_ == 0) {
      OMAF_LOG(LOG_ERROR, "Index length is zero!\n");
      return ERROR_INVALID;
    }

    // open segment
    dash_client_->open(
        //dcb
        ds_params_, [this](std::unique_ptr<VCD::OMAF::StreamBlock> sb) {
          // LOG(INFO) << "Receive sb size " << sb->size() << "for " << this->ds_params_.dash_url_ << endl;
          this->dash_stream_.push_back(std::move(sb));
          this->GenerateChunkStream();
        },
        //cdcb
        [this](std::unique_ptr<VCD::OMAF::StreamBlock> sb, map<uint32_t, uint32_t> &indexRange) {
          this->UpdateIndexStream(std::move(sb));
          indexRange = this->GetIndexRange();
        },
        //scb
        [this](OmafDashSegmentClient::State s) {
          switch (s) {
            case OmafDashSegmentClient::State::SUCCESS:
              this->state_ = State::OPEN_SUCCES;
              break;
            case OmafDashSegmentClient::State::STOPPED:
              this->state_ = State::OPEN_STOPPED;
              break;
            case OmafDashSegmentClient::State::TIMEOUT:
              this->state_ = State::OPEN_TIMEOUT;
              break;
            case OmafDashSegmentClient::State::FAILURE:
              this->state_ = State::OPEN_FAILED;
              break;
            default:
              break;
          }
        });
    return ERROR_NONE;
  } catch (const std::exception& ex) {
    OMAF_LOG(LOG_ERROR, "Exception when start downloading the file: %s, ex: %s\n", ds_params_.dash_url_.c_str(), ex.what());
    return ERROR_INVALID;
  }
}

int32_t CmafSegment::GetSegmentIndexLength(uint32_t chunk_num, uint64_t& size) {
  if (reader_ == nullptr) return ERROR_NULL_PTR;
  int32_t res = ERROR_NONE;
  if (ds_params_.chunk_info_type_ == ChunkInfoType::CHUNKINFO_SIDX_ONLY) {
    res = reader_->getSegmentHeaderSize(true, chunk_num, size, 1);
    ds_params_.header_size_ = size;
  }
  else if (ds_params_.chunk_info_type_ == ChunkInfoType::CHUNKINFO_CLOC_ONLY || ds_params_.chunk_info_type_ == ChunkInfoType::CHUNKINFO_SIDX_AND_CLOC) {
    // 1. get the cloc size
    res = reader_->getSegmentClocSize(chunk_num, size, 1);
    ds_params_.cloc_size_ = size;
    // 2. get the header size
    uint64_t header_size = 0;
    bool hasSidx = (ds_params_.chunk_info_type_ == ChunkInfoType::CHUNKINFO_CLOC_ONLY ? false : true);
    res = reader_->getSegmentHeaderSize(hasSidx, chunk_num, header_size, 1);
    ds_params_.header_size_ = header_size;
  }
  else {
    OMAF_LOG(LOG_ERROR, "Not supported chunk type!\n");
    return ERROR_BAD_PARAM;
  }
  return res;
}

int32_t CmafSegment::GetSegmentIndexRange(char* indexBuf, size_t size, std::map<uint32_t, uint32_t> &indexRange)
{
    if (reader_ == nullptr) return ERROR_NULL_PTR;
    if (ds_params_.chunk_info_type_ == ChunkInfoType::CHUNKINFO_SIDX_ONLY) {
        return reader_->getSegmentIndexRangeFromSidx(indexBuf, size, indexRange);
    }
    else if (ds_params_.chunk_info_type_ == ChunkInfoType::CHUNKINFO_CLOC_ONLY || ds_params_.chunk_info_type_ == ChunkInfoType::CHUNKINFO_SIDX_AND_CLOC) {
        return reader_->getSegmentIndexRangeFromCloc(indexBuf, size, indexRange);
    }
    else {
        OMAF_LOG(LOG_ERROR, "Not supported chunk type!\n");
        return ERROR_BAD_PARAM;
    }
}

int32_t CmafSegment::GenerateChunkStream() {
  // 1. check data validation
  if (index_range_.empty()) {
    OMAF_LOG(LOG_WARNING, "Index range map is empty!\n");
    return ERROR_INVALID;
  }

  if (processed_chunk_id_ >= (int32_t)chunk_num_ - 1) {
    OMAF_LOG(LOG_WARNING, "available chunk id %d is greater than chunk num %d\n", processed_chunk_id_, chunk_num_);
    return ERROR_INVALID;
  }
  // 2. generate chunk stream from dash stream
  // 2.1 get processed bytes
  size_t bytesProcessed = 0;
  for (int32_t i = start_chunk_id_; i <= processed_chunk_id_; i++) {
    bytesProcessed += index_range_[i];
  }

  // 2.2 generate chunk stream
  while (processed_chunk_id_ < (int32_t)chunk_num_ - 1 &&
          dash_stream_.GetStreamSize() - bytesProcessed >= index_range_[processed_chunk_id_ + 1] &&
          index_range_[processed_chunk_id_ + 1] != 0) {

    uint32_t cur_chunk_size = index_range_[processed_chunk_id_ + 1];
    char *buf = new char[cur_chunk_size];
    size_t readSize = dash_stream_.ReadStreamFromOffset(buf, bytesProcessed, cur_chunk_size);
    // LOG(INFO) << "readSize " << readSize <<" cur_chunk_size " << cur_chunk_size << endl;
    if (readSize != cur_chunk_size) {
      DELETE_ARRAY(buf);
      OMAF_LOG(LOG_WARNING, "dash stream has not enough data\n");
      return ERROR_NO_VALUE;
    }
    std::unique_ptr<StreamBlock> chunk_sb = make_unique_vcd<StreamBlock>(std::move(buf), cur_chunk_size);
    chunk_stream_.push_back(std::move(chunk_sb));
    bytesProcessed += readSize;
    processed_chunk_id_++;
    // chunk state change: generate node from chunk stream
    if (this->state_change_cb_) {
      this->state_change_cb_(this->shared_from_this(), State::OPEN_SUCCES);
    }
  }

  if (processed_chunk_id_ == (int32_t)chunk_num_ - 1) {
    dash_stream_.clear();
  }

  return ERROR_NONE;
}

int32_t CmafSegment::UpdateIndexStream(std::unique_ptr<StreamBlock> sb)
{
  if (reader_ == nullptr) return ERROR_NULL_PTR;
  int64_t sb_size = sb->size();
  index_stream_.clear();
  index_stream_.push_back(std::move(sb));
  // if stream block size is enough, then do GetSegmentIdxRange
  if (sb_size == index_length_) {
    // 1. read stream to index buf
    char *indexBuf = new char[index_length_];
    index_stream_.ReadStream(indexBuf, index_length_);
    // check dirty data
    if (!CheckIndexBuf(indexBuf, index_length_)) {
        OMAF_LOG(LOG_WARNING, "Dirty data happen in index stream!\n");
        return ERROR_INVALID;
    }
    // 2. get index_range_
    if (GetSegmentIndexRange(indexBuf, index_length_, index_range_) != ERROR_NONE) {
      OMAF_LOG(LOG_ERROR, "Get segment index range failed!\n");
      return ERROR_INVALID;
    }
    // for (auto index : index_range_) {
    //   LOG(INFO) << "Index range " << index.first << " " << index.second << " segid " << GetSegID()<< endl;
    // }
    // 3. clear header stream
    SAFE_DELARRAY(indexBuf);
    index_stream_.clear();
  }
  return ERROR_NONE;
}

bool CmafSegment::CheckIndexBuf(char *index_buf, size_t index_size)
{
    if (index_buf == nullptr || index_size < (size_t)index_length_) return false;

    if (ds_params_.chunk_info_type_ == ChunkInfoType::CHUNKINFO_CLOC_ONLY
        || ds_params_.chunk_info_type_ == ChunkInfoType::CHUNKINFO_SIDX_AND_CLOC) {
        if ((index_buf[4] == 'c' && index_buf[5] == 'l' && index_buf[6] == 'o' && index_buf[7] == 'c')) {
            return true;
        }
        else return false;
    }
    else return true;
}

VCD_OMAF_END