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
//! \file:   Stream.h
//! \brief:  stream class for storing downloaded sub-streams
//!

#ifndef STREAM_H
#define STREAM_H

#include <fstream>
#include <mutex>  //std::mutex, std::unique_lock

#include "../OmafDashParser/Common.h"
#include "../common.h"
#include "../isolib/dash_parser/Mp4StreamIO.h"

namespace VCD {
namespace OMAF {
//!
//! \class  StreamBlock
//! \brief  Stream Information, including data and data size
//!
class StreamBlock : public VCD::NonCopyable {
 public:
  //!
  //! \brief Constructor
  //!
  StreamBlock() = default;

  StreamBlock(char *data, int64_t size) : data_(data), size_(size), capacity_(size), bOwner_(false) {}
  //!
  //! \brief Destructor
  //!
  ~StreamBlock() {
    if (bOwner_ && data_ != nullptr) {
      delete[] data_;
      data_ = nullptr;
    }
    size_ = 0;
  }
  char *buf() noexcept { return data_; }
  const char *cbuf() const noexcept { return data_; }
  int64_t size() const noexcept { return size_; }
  int64_t capacity() const noexcept { return capacity_; }
  bool size(int64_t size) {
    if (size <= capacity_ && size > 0) {
      size_ = size;
      return true;
    }
    return false;
  }
  void *resize(int64_t size) {
    if (bOwner_) {
      if (size > capacity_) {
        if (data_) {
          delete[] data_;
        }
        data_ = new char[size];
        capacity_ = size;
      }
      return data_;
    } else {
      return nullptr;
    }
  }

private:
    StreamBlock& operator=(const StreamBlock& other) { return *this; };
    StreamBlock(const StreamBlock& other) { /* do not create copies */ };

 private:
  // stream data
  char *data_ = nullptr;
  // length of data
  int64_t size_ = 0;
  int64_t capacity_ = 0;
  const bool bOwner_ = true;
};

class StreamBlocks : public VCD::NonCopyable, public VCD::MP4::StreamIO {
 public:
  StreamBlocks() = default;
  ~StreamBlocks() {}

 public:
  offset_t ReadStream(char *buffer, offset_t size) {
    std::lock_guard<std::mutex> lock(stream_mutex_);

    offset_t offset = offset_;

    std::list<std::unique_ptr<StreamBlock>>::const_iterator it = stream_blocks_.cbegin();
    while (it != stream_blocks_.cend()) {
      if (offset < (*it)->size()) {
        break;
      }
      offset -= (*it)->size();
      ++it;
    }

    offset_t readSize = 0;
    while (it != stream_blocks_.cend()) {
      if (readSize >= size) break;

      offset_t copySize = 0;
      offset_t dataSize = (*it)->size() - offset;
      if ((size - readSize) >= dataSize) {
        copySize = dataSize;
      } else {
        copySize = size - readSize;
      }

      memcpy_s(buffer + readSize, copySize, (*it)->cbuf() + offset, copySize);
      readSize += copySize;
      offset = 0;  // set offset to 0 for coming blocks
      ++it;
    }

    offset_ += readSize;

    return readSize;
  };

  bool SeekAbsoluteOffset(offset_t offset) {
    std::lock_guard<std::mutex> lock(stream_mutex_);
    offset_ = offset;  // FIXME same logic with old file solution
    return true;
  };

  offset_t TellOffset() { return offset_; };

  offset_t GetStreamSize() {
    std::lock_guard<std::mutex> lock(stream_mutex_);
    return stream_size_;
  };

 public:
  void push_back(std::unique_ptr<StreamBlock> sb) noexcept {
    std::lock_guard<std::mutex> lock(stream_mutex_);
    stream_size_ += sb->size();
    stream_blocks_.push_back(std::move(sb));
  }

  bool cacheToFile(std::string &filename) noexcept {
    std::ofstream of;  //<! file handle for writing
    try {
      of.open(filename, ios::out | ios::binary);

      for (auto &sb : stream_blocks_) {
        of.write(sb->cbuf(), sb->size());
      }

      of.close();
      return true;
    } catch (const std::exception &ex) {
      OMAF_LOG(LOG_ERROR, "Exception when cache the file: %s, ex: %s\n", filename.c_str(), ex.what());
      if (of.is_open()) {
        of.close();
      }
      return false;
    }
  }

 private:
  std::list<std::unique_ptr<StreamBlock>> stream_blocks_;

  std::mutex stream_mutex_;
  offset_t stream_size_ = 0;
  offset_t offset_ = 0;
};
}  // namespace OMAF
}  // namespace VCD

#endif  // STREAM_H
