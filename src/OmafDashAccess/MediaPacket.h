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

//! \file:   MediaPacket.h
//! \brief:  the class for media packet
//! \detail: it's class to hold buffer for streams.
//!
//! Created on May 22, 2019, 1:18 PM
//!

#ifndef MEDIAPACKET_H_
#define MEDIAPACKET_H_

#include "general.h"
#include "OmafMediaStream.h"

VCD_OMAF_BEGIN

class MediaPacket {
public:
    //!
    //! \brief  construct
    //!
    MediaPacket(){
        m_pPayload = NULL;
        m_nAllocSize = 0;
        m_type = -1;
        mPts = 0;
        m_nRealSize = 0;
        m_rwpk = NULL;
    };

    //!
    //! \brief  construct
    //!
    MediaPacket(char* buf, int size) : MediaPacket() {
        m_nAllocSize = AllocatePacket( size );
        memcpy(m_pPayload, buf, size);
        m_type = -1;
        mPts = 0;
        m_rwpk = NULL;
    };

    //!
    //! \brief  de-construct
    //!
    virtual ~MediaPacket(){
        if( NULL != m_pPayload ){
            free(m_pPayload);
            m_pPayload = NULL;
            m_nAllocSize = 0;
            m_type = -1;
            mPts = 0;
            m_nRealSize = 0;
        }
        if (m_rwpk != NULL)
            deleteRwpk();
    };

    //!
    //! \brief  Allocate the packet buffer, and fill the buffer with fill
    //!
    //! \param  [in] size
    //!         the buffer size to be allocated
    //! \param  [in] fill
    //!         the init value for the buffer
    //!
    //! \return
    //!         size of new allocated packet
    //!
    int AllocatePacket(int size, char fill = 0){
        if( NULL != m_pPayload ){
            free(m_pPayload);
            m_pPayload = NULL;
            m_nAllocSize = 0;
        }

        m_pPayload = (char*)malloc( size );

        if(NULL == m_pPayload) return -1;

        m_nAllocSize = size;
        memset(m_pPayload, fill, m_nAllocSize );
        m_nRealSize = 0;
        return size;
    };

    //!
    //! \brief  get the buffer pointer of the packet
    //!
    //! \return
    //!         the buffer pointer
    //!
    char* Payload(){ return m_pPayload; };

    //!
    //! \brief  get the size of the buffer
    //!
    //! \return
    //!         size of the packet's payload
    //!
    int Size(){ return m_nRealSize; };

    //!
    //! \brief  reallocate the payload buffer. if size > m_nAllocSize, keep the data
    //!         in old buf to new buf;
    //!
    //! \return
    //!         size of new allocated packet
    //!
    int ReAllocatePacket(int size){
        if(NULL==m_pPayload)
            return AllocatePacket(size);

        if( size < m_nAllocSize )
            return AllocatePacket(size);

        char* buf = m_pPayload;

        m_pPayload = (char*)malloc( size );

        if(NULL == m_pPayload) return -1;

        memcpy(m_pPayload, buf, m_nAllocSize);

        free(buf);

        m_nAllocSize = size;
        m_nRealSize = 0;
        return 0;
    };

    //!
    //! \brief  Set the type for packet
    //!
    //! \return
    //!
    //!
    void SetType(int type){m_type = type;};

    //!
    //! \brief  Get the type of the payload
    //!
    //! \return
    //!         the type of the Packet payload
    //!
    int GetType(){return m_type;};

    uint64_t GetPTS() { return mPts; };
    void SetPTS(uint64_t pts){ mPts = pts; };

    void SetRealSize(uint64_t realSize) { m_nRealSize = realSize; };
    uint64_t GetRealSize() { return m_nRealSize; };

    void SetRwpk(RegionWisePacking *rwpk) { m_rwpk = rwpk; };
    RegionWisePacking* GetRwpk() { return m_rwpk; };

private:
    char* m_pPayload;                    //!<the payload buffer of the packet
    int   m_nAllocSize;                  //!<the allocated size of packet
    uint64_t m_nRealSize;                //!< real size of packet
    int   m_type;                        //!<the type of the payload
    uint64_t mPts;
    RegionWisePacking *m_rwpk;

    void deleteRwpk()
    {
        if (m_rwpk != NULL)
        {
            if (m_rwpk->rectRegionPacking != NULL)
            {
                delete []m_rwpk->rectRegionPacking;
                m_rwpk->rectRegionPacking = NULL;
            }
            delete m_rwpk;
            m_rwpk = NULL;
        }
    }
};

VCD_OMAF_END;

#endif /* MEDIAPACKET_H */

