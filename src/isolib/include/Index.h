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
//! \file:   Index.h
//! \brief:  Index class definition
//! \detail: Define the basic operation for index related numerical value
//!

#ifndef _IDBASE_H_
#define _IDBASE_H_

#include <iostream>
#include "Common.h"

VCD_MP4_BEGIN

template <typename T, typename Tag>
class Index
{
public:
    Index()
        : m_index()
    {
    }
    Index(T id)
        : m_index(id)
    {
    }

    T GetIndex() const
    {
        return m_index;
    }

protected:
    T m_index;
};

template <typename T, typename Tag>
class IndexExplicit : public Index<T, Tag>
{
public:
    IndexExplicit()
        : Index<T, Tag>()
    {
    }
    explicit IndexExplicit(T id)
        : Index<T, Tag>(id)
    {
    }
};

template <typename T, typename Tag>
class IndexCalculation : public Index<T, Tag>
{
public:
    IndexCalculation()
        : Index<T, Tag>()
    {
    }
    IndexCalculation(T id)
        : Index<T, Tag>(id)
    {
    }

    IndexCalculation<T, Tag>& operator++();
    ;
    IndexCalculation<T, Tag>& operator--();
};

template <typename T, typename Tag>
bool operator<(Index<T, Tag> a, Index<T, Tag> b)
{
    return a.GetIndex() < b.GetIndex();
}

template <typename T, typename Tag>
bool operator==(Index<T, Tag> a, Index<T, Tag> b)
{
    return a.GetIndex() == b.GetIndex();
}

template <typename T, typename Tag>
bool operator!=(Index<T, Tag> a, Index<T, Tag> b)
{
    return a.GetIndex() != b.GetIndex();
}

template <typename T, typename Tag>
bool operator>(Index<T, Tag> a, Index<T, Tag> b)
{
    return a.GetIndex() > b.GetIndex();
}

template <typename T, typename Tag>
bool operator<=(Index<T, Tag> a, Index<T, Tag> b)
{
    return a.GetIndex() <= b.GetIndex();
}

template <typename T, typename Tag>
bool operator>=(Index<T, Tag> a, Index<T, Tag> b)
{
    return a.GetIndex() >= b.GetIndex();
}

template <typename T, typename Tag>
IndexCalculation<T, Tag> operator+(IndexCalculation<T, Tag> a, IndexCalculation<T, Tag> b)
{
    return a.GetIndex() + b.GetIndex();
}

template <typename T, typename Tag>
IndexCalculation<T, Tag>& IndexCalculation<T, Tag>::operator++()
{
    ++Index<T, Tag>::m_index;
    return *this;
}

template <typename T, typename Tag>
IndexCalculation<T, Tag>& IndexCalculation<T, Tag>::operator--()
{
    --Index<T, Tag>::m_index;
    return *this;
}

template <typename T, typename Tag>
IndexCalculation<T, Tag> operator++(IndexCalculation<T, Tag>& a, int)
{
    auto orig = a;
    ++a;
    return orig;
}

template <typename T, typename Tag>
IndexCalculation<T, Tag> operator--(IndexCalculation<T, Tag>& a, int)
{
    auto orig = a;
    --a;
    return orig;
}

template <typename T, typename Tag>
IndexCalculation<T, Tag> operator-(IndexCalculation<T, Tag> a, IndexCalculation<T, Tag> b)
{
    return a.GetIndex() - b.GetIndex();
}

template <typename T, typename Tag>
IndexCalculation<T, Tag>& operator+=(IndexCalculation<T, Tag>& a, IndexCalculation<T, Tag> b)
{
    a = a.GetIndex() + b.GetIndex();
    return a;
}

template <typename T, typename Tag>
IndexCalculation<T, Tag>& operator-=(IndexCalculation<T, Tag>& a, IndexCalculation<T, Tag> b)
{
    a = a.GetIndex() - b.GetIndex();
    return a;
}

template <typename T, typename Tag>
std::ostream& operator<<(std::ostream& stream, Index<T, Tag> value)
{
    stream << value.GetIndex();
    return stream;
}

VCD_MP4_END;
#endif  // _IDBASE_H_
