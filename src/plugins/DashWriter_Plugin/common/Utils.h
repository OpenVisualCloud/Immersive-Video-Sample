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
//! \file:   Utils.h
//! \brief:  Basic data type utility operation
//!

#ifndef _UTILS_H_
#define _UTILS_H_

#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>
#include <string>

#include "Atom.h"
#include "Stream.h"
#include "AvcConfigAtom.h"

using namespace std;

VCD_MP4_BEGIN

template <typename T, typename... Args>
unique_ptr<T> MakeUnique(Args... args)
{
    return unique_ptr<T>(new T(forward<Args>(args)...));
}

template <typename T, typename U, typename Deleter>
unique_ptr<T, Deleter> StaticCast(unique_ptr<U, Deleter>&& aPtr)
{
    return unique_ptr<T, Deleter>{static_cast<T*>(aPtr.release())};
}

template <typename Container, typename Function>
auto ContMapSet(Function map, const Container& container) -> set<decltype(map(*container.begin()))>
{
    set<decltype(map(*container.begin()))> set;
    for (const auto& x : container)
    {
        set.insert(map(x));
    }
    return set;
}

template <typename Container>
auto Keys(const Container& cont) -> vector<typename Container::key_type>
{
    vector<typename Container::key_type> xs;

    for (const auto& x : cont)
    {
        xs.push_back(x.first);
    }

    return xs;
}

class InvertTrue
{
public:
    bool operator()()
    {
        if (m_first)
        {
            m_first = false;
            return true;
        }
        else
        {
            return false;
        }
    }

private:
    bool m_first = true;
};

VCD_MP4_END;
#endif //_UTILS_H_
