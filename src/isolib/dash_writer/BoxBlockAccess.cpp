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
//! \file:   BoxBlockAccess.cpp
//! \brief:  BoxBlockAccess class implementation
//!

#include <stdexcept>
#include "BoxBlockAccess.h"

using namespace std;

VCD_MP4_BEGIN

struct BoxInfo
{
    FourCCInt fourcc;
    bool hasSubBoxes;
};

DataBlock::DataBlock(size_t aBlockOffset, size_t aBlockSize)
    : offset(aBlockOffset)
    , size(aBlockSize)
{
}

std::vector<uint8_t> DataBlock::GetData(istream& aStream) const
{
    std::vector<uint8_t> buffer(size);
    aStream.seekg(streamoff(offset));
    aStream.read(reinterpret_cast<char*>(&buffer[0]), streamsize(size));
    return buffer;
}

Stream DataBlock::GetStream(istream& aStream) const
{
    std::vector<uint8_t> buffer(size);
    aStream.seekg(streamoff(offset));
    aStream.read(reinterpret_cast<char*>(&buffer[0]), streamsize(size));
    if (!aStream)
    {
        ISO_LOG(LOG_ERROR, "Unexpected EOS !\n");
        throw exception();
    }
    return Stream(buffer);
}

map<FourCCInt, BoxInfo> GenBoxInfos()
{
    const BoxInfo boxInfo = {FourCCInt("ftyp"), false};
    map<FourCCInt, BoxInfo> constructedMap;
    constructedMap.insert(make_pair(FourCCInt("ftyp"), boxInfo));
    return constructedMap;
}

const map<FourCCInt, BoxInfo> boxInfos = GenBoxInfos();

uint32_t ReadU32(istream& aStream)
{
    uint32_t v = 0;
    for (int c = 0; c < 4; ++c)
    {
        v = (v << 8) | static_cast<uint8_t>(aStream.get());
    }
    if (!aStream.good())
    {
        ISO_LOG(LOG_ERROR, "Unexpected EOS !\n");
        throw exception();
    }
    return v;
}

BoxBlock::BoxBlock(FourCCInt aFourcc, size_t aBlockOffset, size_t aBlockSize)
    : DataBlock(aBlockOffset, aBlockSize)
    , fourcc(aFourcc)
    , indexBuilt(false)
{
}

BoxIndex BoxBlockAccess::GenIndex(istream& aStream, const BoxBlock* aParent)
{
    BoxIndex index;

    streamoff endLimit = 0;
    if (aParent)
    {
        endLimit = streamoff(aParent->offset + aParent->size);
    }

    size_t offset = 0;
    aStream.get();
    if (aStream)
    {
        aStream.unget();
    }
    while (aStream && (!aParent || aStream.tellg() < endLimit))
    {
        size_t size      = size_t(ReadU32(aStream));
        FourCCInt fourcc = FourCCInt(ReadU32(aStream));
        aStream.seekg(aStream.tellg() + streamoff(size - 8));

        index[fourcc].push_back(BoxBlock(fourcc, offset, size));
        offset += size;
        aStream.get();
        if (aStream)
        {
            aStream.unget();
        }
    }

    aStream.clear();

    return index;
}

BoxBlockAccess::BoxBlockAccess(istream& aStream)
    : m_stream(aStream)
    , m_index(GenIndex(aStream, nullptr))
{
}

BoxBlockAccess::~BoxBlockAccess()
{
}

BoxBlock BoxBlockAccess::GetBoxBlock(FourCCInt aFourcc) const
{
    map<FourCCInt, list<BoxBlock>>::const_iterator iter = m_index.find(aFourcc);
    if (iter == m_index.end())
    {
        ISO_LOG(LOG_ERROR, "Couldn't find Boxes List!\n");
        throw exception();
    }

    list<BoxBlock> boxList = iter->second;
    if (boxList.size() == 0)
    {
        ISO_LOG(LOG_ERROR, "Couldn't find Box !\n");
        throw exception();
    }

    return *((iter->second).begin());
}

std::vector<uint8_t> BoxBlockAccess::GetData(const DataBlock& aBlock) const
{
    return aBlock.GetData(m_stream);
}

Stream BoxBlockAccess::GetStream(const DataBlock& aBlock) const
{
    return aBlock.GetStream(m_stream);
}

Stream BoxBlockAccess::GetStream(FourCCInt aFourcc) const
{
    return GetBoxBlock(aFourcc).GetStream(m_stream);
}

VCD_MP4_END
