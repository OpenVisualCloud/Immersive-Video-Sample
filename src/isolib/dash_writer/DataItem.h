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
//! \file:   DataItem.h
//! \brief:  DataItem class definition
//! \detail: DataItem class is the basic operation class
//!

#ifndef _DATAITEM_H_
#define _DATAITEM_H_

#include <memory>
#include "../include/Common.h"

using namespace std;

VCD_MP4_BEGIN

class EmptyData
{
};

extern EmptyData none;

template <typename T>
class DataItem
{
public:
    DataItem();
    DataItem(EmptyData);
    DataItem(const DataItem<T>&);
    DataItem(DataItem<T>&&);

    DataItem(const T&);
    DataItem(T&&);

    DataItem& operator=(const DataItem<T>&);
    DataItem& operator=(DataItem<T>&&);

    ~DataItem();

    bool operator==(const DataItem<T>& other) const;
    bool operator!=(const DataItem<T>& other) const;
    bool operator<=(const DataItem<T>& other) const;
    bool operator>=(const DataItem<T>& other) const;
    bool operator<(const DataItem<T>& other) const;
    bool operator>(const DataItem<T>& other) const;

    explicit operator bool() const;

    T& operator*();
    const T& operator*() const;
    T& get();
    const T& get() const;
    T* operator->();
    const T* operator->() const;

private:
    unique_ptr<T> m_value;
};

template <typename T>
DataItem<T> GenDataItem(const T& value);

template <typename T>
DataItem<T> GenDataItem(T&& value);

template <typename First, typename... Rest>
First CoalesceData(First value, Rest... rest);

#include "DataItem.icc"

VCD_MP4_END;
#endif
