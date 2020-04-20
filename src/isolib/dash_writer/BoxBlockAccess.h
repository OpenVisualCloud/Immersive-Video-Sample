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
//! \file:   BoxBlockAccess.h
//! \brief:  BoxBlockAccess class definition
//! \detail: Define the mp4 file access operation,
//!

#ifndef _MP4ACCES_H_
#define _MP4ACCES_H_

#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <stdexcept>
#include "Stream.h"
#include "FourCCInt.h"
#include "FormAllocator.h"

using namespace std;

VCD_MP4_BEGIN

struct DataBlock
{
    DataBlock()
        : offset(0)
        , size(0)
    {
    }
    DataBlock(size_t aBlockOffset, size_t aBlockSize);

    size_t offset;
    size_t size;

    std::vector<uint8_t> GetData(istream& aStream) const;
    Stream GetStream(istream& aStream) const;
};

struct BoxBlock;
typedef map<FourCCInt, list<BoxBlock>> BoxIndex;

struct BoxBlock : public DataBlock
{
    BoxBlock(FourCCInt aFourcc, size_t aBlockOffset, size_t aBlockSize);

    FourCCInt fourcc;

    bool operator<(const BoxBlock& other)
    {
        return fourcc < other.fourcc
                   ? true
                   : fourcc > other.fourcc
                         ? false
                         : offset < other.offset
                               ? true
                               : offset > other.offset
                                     ? false
                                     : size < other.size ? true
                                                                   : size > other.size ? false : false;
    }

    bool indexBuilt;
    BoxIndex index;
};

class BoxBlockAccess
{
public:
    BoxBlockAccess(istream& aStream);

    BoxBlock GetBoxBlock(FourCCInt aFourcc) const;
    std::vector<uint8_t> GetData(const DataBlock& aBlock) const;
    Stream GetStream(const DataBlock& aBlock) const;
    Stream GetStream(FourCCInt aFourcc) const;
    template <typename BoxType>
    void ParseBoxBlock(BoxType& aBox) const;

    virtual ~BoxBlockAccess();

private:
    static BoxIndex GenIndex(istream& aStream, const BoxBlock* aParent);

    istream& m_stream;
    BoxIndex m_index;
};

template <typename BoxType>
void BoxBlockAccess::ParseBoxBlock(BoxType& aBox) const
{
    Stream bs = GetStream(aBox.getType());
    aBox.parseBox(bs);
}

VCD_MP4_END;
#endif  // _MP4ACCESS_H_
