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
//! \file:   ChunkLocationAtom.h
//! \brief:  Chunk Location Atom to describe the chunk location in the segment
//! \detail: 'cloc' Atom implementation.
//!
//! Created on October 15, 2019, 13:39 PM
//!

#ifndef _CHUNKLOCATIONATOM_H_
#define _CHUNKLOCATIONATOM_H_

#include <vector>
#include "FormAllocator.h"
#include "FullAtom.h"

VCD_MP4_BEGIN

class ChunkLocationAtom : public FullAtom
{
public:
    //!
    //! \brief Constructor
    //!
    ChunkLocationAtom(uint8_t version = 0);

    //!
    //! \brief Destructor
    //!
    virtual ~ChunkLocationAtom() = default;

    struct ChunkLocation
    {
        uint16_t chunkIndex;    //<! provides the index of the chunk within the segment and begins from 0
        uint32_t chunkOffset;   //<! the distance in bytes from the beginning of the segment to the first byte of current chunk
        uint32_t chunkSize;     //<! the distance in bytes from the beginning of current chunk to the first byte of the next chunk or to the end of the segment
    };

    //!
    //! \brief    Set Space Reserve
    //!
    //! \param    [in] size_t
    //!           reserve Total
    //!
    //! \return   void
    //!
    void SetSpaceReserve(size_t reserveTotal);

    //!
    //! \brief    Set and Get function for m_chunksNum member
    //!
    //! \param    [in] uint32_t
    //!           value to set
    //! \param    [in] m_chunksNum
    //!           m_chunksNum member in class
    //! \param    [in] ChunksNum
    //!           m_chunksNum name in class
    //! \param    [in] const or blank
    //!           return const type or not
    //!
    //! \return   void
    //!
    MEMBER_SETANDGET_FUNC_WITH_OPTION(uint32_t, m_chunksNum, ChunksNum, const);

    //!
    //! \brief    Add chunk location structure
    //!
    //! \param    [in] const ChunkLocation&
    //!           chunkLocation
    //!
    //! \return   void
    //!
    void AddChunkLocation(const ChunkLocation& chunkLocation);

    //!
    //! \brief    Get chunk location
    //!
    //! \return   std::vector<ChunkLocation>
    //!           ChunkLocation array
    //!
    std::vector<ChunkLocation> GetChunksLocation() const;

    //!
    //! \brief    Write atom information to stream
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    virtual void ToStream(Stream& str);

    //!
    //! \brief    Parse atom information from stream
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    virtual void FromStream(Stream& str);

private:
    uint16_t m_chunksNum;                           //!< total number of chunks in segment
    std::vector<ChunkLocation>  m_chunksLocation;   //!< all chunks location within the segment
    size_t   m_reserveTotal;                        //!< reserve Total
};

VCD_MP4_END;
#endif /* _CHUNKLOCATIONATOM_H_ */
