
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

/*
 * File:   common.h
 * Author: media
 *
 * Created on 2020/04/22
 */
#ifndef VCD_UTILS_COMMON_H
#define VCD_UTILS_COMMON_H
#include <memory>
#include "../utils/safe_mem.h"

namespace VCD {
class NonCopyable {
 protected:
  NonCopyable() = default;

  // Non-moveable.
  NonCopyable(NonCopyable &&) noexcept = delete;
  NonCopyable &operator=(NonCopyable &&) noexcept = delete;

  // Non-copyable.
  NonCopyable(const NonCopyable &) = delete;
  NonCopyable &operator=(const NonCopyable &) = delete;
};
}  // namespace VCD

//
// @template make_unique_vcaa
// @brief std11 not support this feature
//
template <typename T, typename... Args>
std::unique_ptr<T> make_unique_vcd(Args &&... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

#ifndef UNUSED
#define UNUSED(prama) (void)prama
#endif  // !UNUSED

#ifndef IN
#define IN
#endif  // !IN

#ifndef OUT
#define OUT
#endif  // !OUT

#ifndef INOUT
#define INOUT
#endif  // !INOUT

#endif  // !VCD_UTILS_COMMON_H