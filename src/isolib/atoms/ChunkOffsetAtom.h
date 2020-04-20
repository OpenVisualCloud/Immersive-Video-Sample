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
//! \file:   ChunkOffsetAtom.h
//! \brief:  Chunk Offset Atom and Large Chunk Offset Atom class
//! \detail: 'stco' and 'co64' Atom implementation.
//!
//! Created on October 15, 2019, 13:39 PM
//!

#ifndef _CHUNKOFFSETATOM_H_
#define _CHUNKOFFSETATOM_H_

#include "FormAllocator.h"
#include "FullAtom.h"

VCD_MP4_BEGIN

class ChunkOffsetAtom : public FullAtom
{
public:

    //!
    //! \brief Constructor
    //!
    ChunkOffsetAtom();

    //!
    //! \brief Destructor
    //!
    virtual ~ChunkOffsetAtom() = default;

    //!
    //! \brief    Set Chunk Offsets
    //!
    //! \param    [in] const std::vector<std::uint64_t>&
    //!           Chunk Offsets
    //!
    //! \return   void
    //!
    void SetChunkOffsets(const std::vector<std::uint64_t>& chunkOffsets);

    //!
    //! \brief    Get Chunk Offsets
    //!
    //! \return   std::vector<std::uint64_t>
    //!           Chunk Offsets
    //!
    std::vector<std::uint64_t> GetChunkOffsets();
    const std::vector<std::uint64_t> GetChunkOffsets() const;

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
    std::vector<std::uint64_t> m_chunkOffsets;  //!< chunk Offsets
};

VCD_MP4_END;
#endif /* _CHUNKOFFSETATOM_H_ */
