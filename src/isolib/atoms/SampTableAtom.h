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
//! \file:   SampTableAtom.h
//! \brief:  SampTableAtom class
//! \detail: "stbl" atom.
//!
//! Created on October 15, 2019, 13:39 PM
//!

#ifndef _SAMPLETABLEATOM_H_
#define _SAMPLETABLEATOM_H_

#include "Atom.h"
#include "Stream.h"
#include "ChunkOffsetAtom.h"
#include "CompOffsetAtom.h"
#include "CompToDecAtom.h"
#include "FormAllocator.h"
#include "SampDescAtom.h"
#include "SampGroupDescAtom.h"
#include "SampSizeAtom.h"
#include "SampToChunkAtom.h"
#include "SampToGroupAtom.h"
#include "SyncSampAtom.h"
#include "TimeToSampAtom.h"

VCD_MP4_BEGIN

class SampleTableAtom : public Atom
{
public:

    //!
    //! \brief Constructor
    //!
    SampleTableAtom();
    SampleTableAtom(const SampleTableAtom& obj) = delete;

    SampleTableAtom& operator=(const SampleTableAtom&) = default;

    //!
    //! \brief Destructor
    //!
    virtual ~SampleTableAtom() = default;

    //!
    //! \brief    Get SampleDescription Atom
    //!
    //! \return   SampleDescriptionAtom&
    //!           SampleDescription Atom
    //!
    SampleDescriptionAtom& GetSampleDescriptionAtom();

    //!
    //! \brief    Get SampleDescription Atom
    //!
    //! \return   const SampleDescriptionAtom&
    //!           SampleDescription Atom
    //!
    const SampleDescriptionAtom& GetSampleDescriptionAtom() const;

    //!
    //! \brief    Get TimeToSample Atom
    //!
    //! \return   TimeToSampleAtom&
    //!           TimeToSample Atom
    //!
    TimeToSampleAtom& GetTimeToSampleAtom();

    //!
    //! \brief    Get TimeToSample Atom
    //!
    //! \return   const TimeToSampleAtom&
    //!           TimeToSample Atom
    //!
    const TimeToSampleAtom& GetTimeToSampleAtom() const;

    //!
    //! \brief    Set CompositionOffset Atom
    //!
    //! \param    [in] const CompositionOffsetAtom&
    //!           CompositionOffset Atom
    //!
    //! \return   void
    //!
    void SetCompositionOffsetAtom(const CompositionOffsetAtom& compositionOffsetAtom);

    //!
    //! \brief    Get CompositionOffset Atom
    //!
    //! \return   std::shared_ptr<const CompositionOffsetAtom>
    //!           CompositionOffset Atom
    //!
    std::shared_ptr<const CompositionOffsetAtom> GetCompositionOffsetAtom() const;

    //!
    //! \brief    Set CompositionToDecode Atom
    //!
    //! \param    [in] const CompositionToDecodeAtom&
    //!           CompositionToDecode Atom
    //!
    //! \return   void
    //!
    void SetCompositionToDecodeAtom(const CompositionToDecodeAtom& compositionToDecodeAtom);

    //!
    //! \brief    Get CompositionToDecode Atom
    //!
    //! \return   std::shared_ptr<const CompositionToDecodeAtom>
    //!           CompositionToDecode Atom
    //!
    std::shared_ptr<const CompositionToDecodeAtom> GetCompositionToDecodeAtom() const;

    //!
    //! \brief    Set SyncSample Atom
    //!
    //! \param    [in] const SyncSampleAtom&
    //!           SyncSample Atom
    //!
    //! \return   void
    //!
    void SetSyncSampleAtom(const SyncSampleAtom& syncSampleAtom);

    //!
    //! \brief    Has SyncSample Atom or not
    //!
    //! \return   bool
    //!           has or not
    //!
    bool HasSyncSampleAtom();

    //!
    //! \brief    Get SyncSample Atom
    //!
    //! \return   std::shared_ptr<const SyncSampleAtom>
    //!           SyncSample Atom
    //!
    std::shared_ptr<const SyncSampleAtom> GetSyncSampleAtom() const;

    //!
    //! \brief    Get SampleToChunk Atom
    //!
    //! \return   SampleToChunkAtom&
    //!           SampleToChunk Atom
    //!
    SampleToChunkAtom& GetSampleToChunkAtom();

    //!
    //! \brief    Get SampleToChunk Atom
    //!
    //! \return   const SampleToChunkAtom&
    //!           SampleToChunk Atom
    //!
    const SampleToChunkAtom& GetSampleToChunkAtom() const;

    //!
    //! \brief    Get ChunkOffset Atom
    //!
    //! \return   ChunkOffsetAtom&
    //!           ChunkOffset Atom
    //!
    ChunkOffsetAtom& GetChunkOffsetAtom();

    //!
    //! \brief    Get ChunkOffset Atom
    //!
    //! \return   const ChunkOffsetAtom&
    //!           ChunkOffset Atom
    //!
    const ChunkOffsetAtom& GetChunkOffsetAtom() const;

    //!
    //! \brief    Get SampleSize Atom
    //!
    //! \return   SampleSizeAtom&
    //!           SampleSize Atom
    //!
    SampleSizeAtom& GetSampleSizeAtom();

    //!
    //! \brief    Get SampleSize Atom
    //!
    //! \return   const SampleSizeAtom&
    //!           SampleSize Atom
    //!
    const SampleSizeAtom& GetSampleSizeAtom() const;

    //!
    //! \brief    Set SampleGroupDescription Atom
    //!
    //! \param    [in] UniquePtr<SampleGroupDescriptionAtom>
    //!           SampleGroupDescription Atom
    //!
    //! \return   void
    //!
    void SetSampleGroupDescriptionAtom(UniquePtr<SampleGroupDescriptionAtom> sgpd);

    //!
    //! \brief    Get SampleToGroup Atom
    //!
    //! \return   SampleToGroupAtom&
    //!           SampleToGroup Atom
    //!
    SampleToGroupAtom& GetSampleToGroupAtom();

    //!
    //! \brief    Get SampleToGroup Atoms
    //!
    //! \return   const std::vector<SampleToGroupAtom>&
    //!           SampleToGroup Atoms
    //!
    const std::vector<SampleToGroupAtom>& GetSampleToGroupAtoms() const;

    //!
    //! \brief    Get SampleGroupDescription Atoms
    //!
    //! \param    [in] FourCCInt
    //!           grouping Type
    //!
    //! \return   const SampleGroupDescriptionAtom*
    //!           SampleGroupDescription Atoms
    //!
    const SampleGroupDescriptionAtom* GetSampleGroupDescriptionAtom(FourCCInt groupingType) const;

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
    //! \brief Reset Samples
    //!
    void ResetSamples();

private:

    SampleDescriptionAtom m_sampleDescrAtom;                    //!< Sample description Atom (mandatory)
    TimeToSampleAtom m_timeToSampAtom;                          //!< Time-to-sample Atom (mandatory)
    SampleToChunkAtom m_sampToChunkAtom;                        //!< Sample-to-chunk Atom (mandatory)
    ChunkOffsetAtom m_chunkOffsetAtom;                          //!< Chunk offset Atom (mandatory)
    SampleSizeAtom m_sampSizeAtom;                              //!< Sample size Atom (mandatory)
    std::shared_ptr<SyncSampleAtom> m_syncSampAtom;             //!< Sync sample Atom (optional)
    std::shared_ptr<CompositionOffsetAtom> m_compOffsetAtom;    //!< Composition offset Atom (optional)
    std::shared_ptr<CompositionToDecodeAtom> m_compToDecodeAtom;//!< Composition to decode Atom (optional)
    UniquePtr<SampleGroupDescriptionAtom> m_sampGroupDescrAtom; //!< Pointer to the sample group description Atom
    std::vector<SampleToGroupAtom> m_sampToGroupAtom;           //!< Vectory of sample-to-group Atoms
    bool m_hasSyncSampleAtom;                                   //!< has Sync Sample Atom
};

VCD_MP4_END;
#endif /* _SAMPLETABLEATOM_H_ */
