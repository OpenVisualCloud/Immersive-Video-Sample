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
//! \file:   performance.h
//! \brief:  performance helper
//!

#ifndef PERFORMANCE_H_
#define PERFORMANCE_H_

#include "../common.h"

#include <string>
#include <mutex>
#include <list>
#include <chrono>
#include <sstream>
#include <memory>

namespace VCD {
namespace OMAF {

const long DEFAULT_TINE_WINDOW = 10000;  // 10s

template <class T>
class CountValue {
 public:
  std::chrono::steady_clock::time_point check_time_;
  size_t points_size_window_ = 0;
  size_t points_size_total_ = 0;
  float points_rate_window_ = 0.0;
  T sum_value_window_;
  T sum_value_total_;
  double avr_value_window_;
};

template <class T>
class WindowCounter : public VCD::NonCopyable {
 private:
  class Node {
   public:
    using UPtr = std::unique_ptr<Node>;

   public:
    Node(std::chrono::steady_clock::time_point start, T value)
        : start_(start),
          value_(value){

          };
    ~Node(){};
    std::chrono::steady_clock::time_point start_;
    T value_;
  };

 public:
  WindowCounter(std::string category = "CategoryUnknown", std::string object = "ObjectUnknown",
                std::string indicator = "Rate")
      : time_window_(DEFAULT_TINE_WINDOW) {
    std::stringstream ss;

    ss << (category.empty() ? "CategoryUnknown" : category) << "::" << (object.empty() ? "ObjectUnknown" : object)
       << "::" << (indicator.empty() ? "Rate" : indicator);

    description_ = ss.str();
  }

  ~WindowCounter(){};
  void setWindow(long bufferTime) { time_window_ = std::chrono::milliseconds(bufferTime); }
  void reset() {
    std::lock_guard<std::mutex> lock(mutex_);

    time_points_.clear();
  }

  void add(T value) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto now = std::chrono::steady_clock::now();

    if (!time_points_.empty()) {
      popExpired(now);
    }
    std::unique_ptr<Node> node = make_unique_vcd<Node>(now, value);
    data_.points_size_total_++;
    data_.sum_value_window_ += value;

    time_points_.emplace_back(std::move(node));
  }

  CountValue<T> count() {
    std::lock_guard<std::mutex> lock(mutex_);

    auto now = std::chrono::steady_clock::now();

    popExpired(now);

    data_.check_time_ = now;
    data_.points_size_window_ = time_points_.size();
    if (time_points_.size() > 0) {
      const auto &front = time_points_.front();
      const auto &front_time = front->start_;
      auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - front_time);
      if (duration > time_window_) {
        duration = time_window_;
      }

      data_.points_rate_window_ = (time_points_.size() * 1000.0f) / duration.count();
    }

    data_.sum_value_total_ = static_cast<T>(0);
    data_.avr_value_window_ = static_cast<double>(0);
    for (auto &node : time_points_) {
      data_.sum_value_total_ += node->value_;
    }
    uint32_t sizeofTP = time_points_.size();
    if (sizeofTP > 0) {
      data_.avr_value_window_ = static_cast<double>(data_.sum_value_total_) / sizeofTP;
    }

    return data_;
  }

 protected:
  void popExpired(std::chrono::steady_clock::time_point &now) {
    while (!time_points_.empty()) {
      auto &front = time_points_.front();
      auto front_time = front->start_;
      auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - front_time);
      if (duration < time_window_) {
        break;
      }

      time_points_.pop_front();
    }
  }

 private:
  std::mutex mutex_;
  std::string description_;
  std::chrono::milliseconds time_window_;
  std::list<std::unique_ptr<Node>> time_points_;
  CountValue<T> data_;
};

}  // namespace OMAF
}  // namespace VCD
#endif  // !PERFORMANCE_H_