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
//! \file:   SampToChunkAtom.h
//! \brief:  SampToChunkAtom class.
//! \detail: 'stsc' Atom
//!
//! Created on October 16, 2019, 13:39 PM
//!

#ifndef _SAMPLETOCHUNKATOM_H_
#define _SAMPLETOCHUNKATOM_H_

#include "Stream.h"
#include "FormAllocator.h"
#include "FullAtom.h"

VCD_MP4_BEGIN

class SampleToChunkAtom : public FullAtom
{
public:

    //!
    //! \brief Constructor
    //!
    SampleToChunkAtom();

    //!
    //! \brief Destructor
    //!
    ~SampleToChunkAtom() = default;

    //!
    //! \brief    Get Sample Descr Index
    //!
    //! \param    [in] std::uint32_t
    //!           samp Idx
    //! \param    [in] std::uint32_t&
    //!           sample Descr Idx
    //!
    //! \return   bool
    //!           success or not
    //!
    bool GetSampleDescrIndex(std::uint32_t sampIdx, std::uint32_t& sampleDescrIdx) const;

    //!
    //! \brief    Get Sample Chunk Index
    //!
    //! \param    [in] std::uint32_t
    //!           samp Idx
    //! \param    [in] std::uint32_t&
    //!           chunk Idx
    //!
    //! \return   bool
    //!           success or not
    //!
    bool GetSampleChunkIndex(std::uint32_t sampIdx, std::uint32_t& chunkIdx) const;

    //!
    //! \brief    Set Sample Num Max Safety
    //!
    //! \param    [in] int64_t
    //!           num value
    //!
    //! \return   void
    //!
    void SetSampleNumMaxSafety(int64_t num);

    struct ChunkEntry   //!< chunk entry
    {
        std::uint32_t firstChunk;
        std::uint32_t oneChunkSamples;
        std::uint32_t sampleDescrIndex;
    };

    //!
    //! \brief    Add Chunk Entry
    //!
    //! \param    [in] const ChunkEntry&
    //!           Chunk Entry value
    //!
    //! \return   void
    //!
    void AddChunkEntry(const ChunkEntry& chunkEntry);

    //!
    //! \brief    Write atom information to stream
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    void ToStream(Stream& str);

    //!
    //! \brief    Parse atom information from stream
    //!
    //! \param    [in,out] Stream&
    //!           bitstream that contains atom information
    //!
    //! \return   void
    //!
    void FromStream(Stream& str);

    //!
    //! \brief    Get Sample Num Lower Bound
    //!
    //! \param    [in] uint32_t
    //!           chunk Entry Count
    //!
    //! \return   uint32_t
    //!           Sample Num Lower Bound
    //!
    uint32_t GetSampleNumLowerBound(uint32_t chunkEntryCount) const;

    //!
    //! \brief    Decode Entries
    //!
    //! \param    [in] uint32_t
    //!           chunk Entry Count
    //!
    //! \return   void
    //!
    void DecodeEntries(std::uint32_t chunkEntryCount);

private:
    std::vector<ChunkEntry> m_runOfChunks;  //!< std::vector that contains the chunk entries

    struct DecEntry //!< decode entry parameter
    {
        std::uint32_t chunkIndex;
        std::uint32_t oneChunkSamples;
        std::uint32_t sampleDescrIndex;
    };

    std::vector<DecEntry> m_decodedEntries; //!< decoded Entry array
    int64_t m_maxSampleNum;                 //!< max Sample Num
};

VCD_MP4_END;
#endif /* _SAMPLETOCHUNKATOM_H_ */
