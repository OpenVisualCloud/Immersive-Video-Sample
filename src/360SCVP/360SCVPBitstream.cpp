/*
 * Copyright (c) 2018, Intel Corporation
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
#include "360SCVPBitstream.h"
#include "assert.h"
#include "../utils/safe_mem.h"

void* gts_malloc(size_t size)
{
    return malloc(size);
}

void *gts_realloc(void *ptr, size_t size)
{
    return realloc(ptr, size);
}

void gts_free(void *ptr)
{
    free(ptr);
}

uint64_t gts_ftell(FILE *fp)
{
    return (uint64_t)ftell(fp);
}

size_t gts_fwrite(const void *ptr, size_t size, size_t nmemb,
    FILE *stream)
{
    size_t result = fwrite(ptr, size, nmemb, stream);
    return result;
}

uint64_t gts_fseek(FILE *fp, int64_t offset, int32_t whence)
{
    return (uint64_t)fseek(fp, (long)offset, whence);
}

uint32_t gts_get_bit_size(uint32_t MaxVal)
{
    uint32_t k = 0;
    while ((int32_t)MaxVal > ((1 << k) - 1)) k += 1;
    return k;
}

GTS_BitStream *gts_bs_new(const int8_t *buffer, uint64_t BufferSize, uint32_t mode)
{
    GTS_BitStream *tmp;
    if ( (buffer && ! BufferSize)) return NULL;

    tmp = (GTS_BitStream *)gts_malloc(sizeof(GTS_BitStream));
    if (!tmp) return NULL;
    memset_s(tmp, sizeof(GTS_BitStream), 0);

    tmp->original = (int8_t*)buffer;
    tmp->size = BufferSize;

    tmp->position = 0;
    tmp->current = 0;
    tmp->bsmode = mode;
    tmp->stream = NULL;

    switch (tmp->bsmode) {
    case GTS_BITSTREAM_READ:
        tmp->nbBits = 8;
        tmp->current = 0;
        break;
    case  GTS_BITSTREAM_WRITE:
        tmp->nbBits = 0;
        if (! buffer) {
            /*if BufferSize is specified, use it. This is typically used when AvgSize of
            some buffers is known, but some exceed it.*/
            if (BufferSize) {
                tmp->size = BufferSize;
            } else {
                tmp->size = BS_MEM_BLOCK_ALLOC_SIZE;
            }
            tmp->original = (int8_t *) gts_malloc(sizeof(int8_t) * ((uint32_t) tmp->size));
            if (! tmp->original) {
                gts_free(tmp);
                return NULL;
            }
            tmp->bsmode = GTS_BITSTREAM_WRITE_DYN;
        } else {
            tmp->original = (int8_t*)buffer;
            tmp->size = BufferSize;
        }
        break;
    default:
        /*the stream constructor is not the same...*/
        gts_free(tmp);
        return NULL;
    }
    return tmp;
}


uint64_t gts_bs_get_size(GTS_BitStream *bs)
{
    if (!bs) return 0;
    if (bs->buffer_io)
        return bs->size + bs->buffer_written;
    return bs->size;
}

uint64_t gts_bs_get_position(GTS_BitStream *bs)
{
    if (!bs) return 0;
    if (bs->buffer_io)
        return bs->position + bs->buffer_written;
    return bs->position;
}

uint32_t gts_bs_get_bit_offset(GTS_BitStream *bs)
{
    if (!bs) return 0;
    if (bs->stream) return 0;
    if (bs->bsmode == GTS_BITSTREAM_READ) return (uint32_t)((bs->position - 1) * 8 + bs->nbBits);
    return (uint32_t)((bs->position) * 8 + bs->nbBits);
}


uint32_t gts_bs_get_bit_position(GTS_BitStream *bs)
{
    if (!bs) return 0;
    if (bs->stream) return 0;
    return bs->nbBits;
}

static void bs_flush_cache(GTS_BitStream *bs)
{
    if (bs->buffer_written) {
        uint32_t nb_write = (uint32_t) fwrite(bs->buffer_io, 1, bs->buffer_written, bs->stream);
        bs->size += nb_write;
        bs->position += nb_write;
        bs->buffer_written = 0;
    }
}

void gts_bs_del(GTS_BitStream *bs)
{
    if (!bs) return;
    /*if we are in dynamic mode (alloc done by the bitstream), free the buffer if still present*/
    if ((bs->bsmode == GTS_BITSTREAM_WRITE_DYN) && bs->original) gts_free(bs->original);
    if (bs->buffer_io)
        bs_flush_cache(bs);
    gts_free(bs);
}

/*returns 1 if aligned wrt current mode, 0 otherwise*/
bool gts_bs_is_align(GTS_BitStream *bs)
{
    switch (bs->bsmode) {
    case GTS_BITSTREAM_READ:
    case GTS_BITSTREAM_FILE_READ:
        return ( (8 == bs->nbBits) ? true : false);
    default:
        return (bool)!bs->nbBits;
    }
}


/*fetch a new byte in the bitstream switch between packets*/
static uint8_t BS_ReadByte(GTS_BitStream *bs)
{
    if (bs->bsmode == GTS_BITSTREAM_READ) {
        uint8_t res;
        if (bs->position >= bs->size) {
            if (bs->EndOfStream) bs->EndOfStream(bs->par);
            return 0;
        }
        res = bs->original[bs->position++];
        return res;
    }
    if (bs->buffer_io)
        bs_flush_cache(bs);

    /*we are in FILE mode, test for end of file*/
    if (!feof(bs->stream)) {
        uint8_t res;
        assert(bs->position<=bs->size);
        bs->position++;
        res = fgetc(bs->stream);
        return res;
    }
    if (bs->EndOfStream) bs->EndOfStream(bs->par);
    else {
        //GF_LOG(GF_LOG_ERROR, GF_LOG_CORE, ("[BS] Attempt to overread bitstream\n"));
    }
    assert(bs->position <= 1+bs->size);
    return 0;
}

uint8_t gf_bs_read_bit(GTS_BitStream *bs)
{
    if (bs->nbBits == 8) {
        bs->current = BS_ReadByte(bs);
        bs->nbBits = 0;
    }

    {
        int32_t ret;
        bs->current <<= 1;
        bs->nbBits++;
        ret = (bs->current & 0x100) >> 8;
        return (uint8_t) ret;
    }

}


uint32_t gts_bs_read_int(GTS_BitStream *bs, uint32_t nBits)
{
    uint32_t ret = 0;
    while (nBits-- > 0) {
        ret <<= 1;
        ret |= gf_bs_read_bit(bs);
    }
    return ret;
}

uint32_t gts_bs_read_U32(GTS_BitStream *bs)
{
    uint32_t ret;
    assert(bs->nbBits==8);
    ret = BS_ReadByte(bs);
    ret<<=8;
    ret |= BS_ReadByte(bs);
    ret<<=8;
    ret |= BS_ReadByte(bs);
    ret<<=8;
    ret |= BS_ReadByte(bs);
    return ret;
}


uint64_t gts_bs_read_long_int(GTS_BitStream *bs, uint32_t nBits)
{
    uint64_t ret = 0;
    if (nBits>64) {
        gts_bs_read_long_int(bs, nBits-64);
        ret = gts_bs_read_long_int(bs, 64);
    } else {
        while (nBits-- > 0) {
            ret <<= 1;
            ret |= gf_bs_read_bit(bs);
        }
    }
    return ret;
}


uint32_t gts_bs_read_data(GTS_BitStream *bs, int8_t *data, uint32_t nbBytes)
{
    uint64_t orig = 0;

    if (!bs || !data) return 0;
    if (bs->position + nbBytes > bs->size) return 0;

    orig = bs->position;
    if (gts_bs_is_align(bs)) {
        int32_t bytes_read;
        switch (bs->bsmode) {
        case GTS_BITSTREAM_FILE_READ:
        case GTS_BITSTREAM_FILE_WRITE:
            if (bs->buffer_io)
                bs_flush_cache(bs);
            bytes_read = (int32_t) fread(data, 1, nbBytes, bs->stream);
            if (bytes_read<0) return 0;
            bs->position += bytes_read;
            return bytes_read;
        case GTS_BITSTREAM_READ:
        case  GTS_BITSTREAM_WRITE:
        case GTS_BITSTREAM_WRITE_DYN:
            memcpy_s(data, nbBytes, bs->original + bs->position, nbBytes);
            bs->position += nbBytes;
            return nbBytes;
        default:
            return 0;
        }
    }

    while (nbBytes-- > 0) {
        *data++ = gts_bs_read_int(bs, 8);
    }
    return (uint32_t) (bs->position - orig);

}



static void BS_WriteByte(GTS_BitStream *bs, uint8_t val)
{
    if (!bs)
        return;
    if ( (bs->bsmode == GTS_BITSTREAM_READ) || (bs->bsmode == GTS_BITSTREAM_FILE_READ) )
    {
        return;
    }
    if (!bs->original && !bs->stream)
    {
        return;
    }
    /*we are in MEM mode*/
    if ( (bs->bsmode ==  GTS_BITSTREAM_WRITE) || (bs->bsmode == GTS_BITSTREAM_WRITE_DYN) ) {
        if (bs->position == bs->size) {
            /*no more space...*/
            if (bs->bsmode != GTS_BITSTREAM_WRITE_DYN) return;
            /*gf_realloc if enough space...*/
            if (bs->size > 0xFFFFFFFF) return;
            bs->size = bs->size ? (bs->size * 2) : BS_MEM_BLOCK_ALLOC_SIZE;
            bs->original = (int8_t*)gts_realloc(bs->original, (uint32_t)bs->size);
            if (!bs->original) return;
        }
        if (bs->original)
            bs->original[bs->position] = val;
        bs->position++;
        return;
    }
    if (bs->buffer_io) {
        if (bs->buffer_written == bs->buffer_io_size) {
            bs_flush_cache(bs);
        }
        bs->buffer_io[bs->buffer_written] = val;
        bs->buffer_written++;
        if (bs->buffer_written == bs->buffer_io_size) {
            bs_flush_cache(bs);
        }
        return;
    }
    /*we are in FILE mode, no pb for any gf_realloc...*/
    fputc(val, bs->stream);
    /*check we didn't rewind the stream*/
    if (bs->size == bs->position) bs->size++;
    bs->position += 1;
}

static void BS_WriteBit(GTS_BitStream *bs, uint32_t bit)
{
    const uint8_t emulation_prevention_three_byte = 0x03;

    bs->current <<= 1;
    bs->current |= bit;
    if (++ bs->nbBits == 8) {
        bs->nbBits = 0;

        if((bs->zeroCount == 2) && ((uint8_t) bs->current < 4))
        {
            BS_WriteByte(bs, emulation_prevention_three_byte);
            bs->zeroCount = 0;
        }
        bs->zeroCount = ((uint8_t) bs->current) == 0 ? bs->zeroCount+1 : 0;

        BS_WriteByte(bs, (uint8_t) bs->current);
        bs->current = 0;
    }
}


void gts_bs_write_int(GTS_BitStream *bs, int32_t _value, int32_t nBits)
{
    if (!bs) return;
    uint32_t value, nb_shift;
    if (!nBits) return;
    value = (uint32_t) _value;
    nb_shift = sizeof (int32_t) * 8 - nBits;
    if (nb_shift)
        value <<= nb_shift;

    while (--nBits >= 0) {
        //but check value as signed
        BS_WriteBit (bs, ((int32_t)value) < 0);
        value <<= 1;
    }
}


void gts_bs_write_U8(GTS_BitStream *bs, uint32_t value)
{
    if (!bs) return;
    assert(!bs->nbBits);
    BS_WriteByte(bs, (uint8_t) value);
}


void gts_bs_write_U16(GTS_BitStream *bs, uint32_t value)
{
    if (!bs) return;
    assert(!bs->nbBits);
    BS_WriteByte(bs, (uint8_t) ((value>>8)&0xff));
    BS_WriteByte(bs, (uint8_t) ((value)&0xff));
}


uint32_t gts_bs_write_byte(GTS_BitStream *bs, uint8_t byte, uint32_t countLoop)
{
    if (!bs) return 0;
    if (!gts_bs_is_align(bs) || bs->buffer_io) {
        uint32_t count = 0;
        while (count<countLoop) {
            gts_bs_write_int(bs, byte, 8);
            count++;
        }
        return count;
    }
    uint32_t totalSize = 0;
    uint32_t writeSize = 0;
    switch (bs->bsmode) {
    case  GTS_BITSTREAM_WRITE:
        totalSize = bs->position + countLoop;
        if (totalSize  > bs->size)
            return 0;
        memset_s(bs->original + bs->position, countLoop, byte);
        bs->position += countLoop;
        return countLoop;
    case GTS_BITSTREAM_WRITE_DYN:
        totalSize = bs->position + countLoop;
        if (totalSize > bs->size)
        {
            uint32_t dynSize = (uint32_t) (bs->size*2);
            if (!dynSize)
                dynSize = BS_MEM_BLOCK_ALLOC_SIZE;

            if (bs->size + countLoop > 0xFFFFFFFF)
                return 0;
            while (dynSize < (uint32_t) ( bs->size + countLoop))
                dynSize *= 2;
            bs->original = (int8_t*)gts_realloc(bs->original, sizeof(uint32_t)*dynSize);
            if (!bs->original)
                return 0;
            bs->size = dynSize;
        }
        memset_s(bs->original + bs->position, countLoop, byte);
        bs->position += countLoop;
        return countLoop;
    case GTS_BITSTREAM_FILE_READ:
    case GTS_BITSTREAM_FILE_WRITE:
        writeSize = gts_fwrite(&byte, 1, countLoop, bs->stream);
        if (writeSize != countLoop)
            return 0;
        if (bs->size == bs->position)
            bs->size += countLoop;
        bs->position += countLoop;
        return countLoop;
    default:
        return 0;
    }
}

uint32_t gts_bs_write_data(GTS_BitStream *bs, const int8_t *data, uint32_t nbBytes)
{
    if (!bs || !data) return 0;
    if (!nbBytes) return 0;
    uint64_t begin = bs->position;
    uint32_t bufferSize = 0;
    uint32_t neededSize = 0;
    uint32_t newSize = 0;
    if (gts_bs_is_align(bs)) {
        switch (bs->bsmode) {
        case GTS_BITSTREAM_WRITE_DYN:
            /*need to gf_realloc ...*/
            if (bs->position+nbBytes > bs->size) {
                uint32_t new_size = (uint32_t) (bs->size*2);
                if (!new_size) new_size = BS_MEM_BLOCK_ALLOC_SIZE;

                if (bs->size + nbBytes > 0xFFFFFFFF)
                    return 0;

                while (new_size < (uint32_t) ( bs->size + nbBytes))
                    new_size *= 2;
                bs->original = (int8_t*)gts_realloc(bs->original, sizeof(uint32_t)*new_size);
                if (!bs->original)
                    return 0;
                bs->size = new_size;
            }
            memcpy_s(bs->original + bs->position, nbBytes, data, nbBytes);
            bs->position += nbBytes;
            return nbBytes;
        case GTS_BITSTREAM_FILE_READ:
        case GTS_BITSTREAM_FILE_WRITE:
            if (!bs->buffer_io) return 0;
            bufferSize = bs->buffer_io_size;
            neededSize = bs->buffer_written + nbBytes;
            if (neededSize > bufferSize)
            {
                bs_flush_cache(bs);
                if (nbBytes > bufferSize) {
                    newSize = 2 * nbBytes;
                    bs->buffer_io = (int8_t*)gts_realloc(bs->buffer_io, newSize);
                    if (!bs->buffer_io)
                        return GTS_OUT_OF_MEM;
                    bs->buffer_io_size = newSize;
                }
            }
            memcpy_s(bs->buffer_io+bs->buffer_written, nbBytes, data, nbBytes);
            bs->buffer_written += nbBytes;
            if (gts_fwrite(data, nbBytes, 1, bs->stream) != 1) return 0;
            if (bs->size == bs->position) bs->size += nbBytes;
            bs->position += nbBytes;
            return nbBytes;
        case  GTS_BITSTREAM_WRITE:
            if (bs->position + nbBytes > bs->size)
                return 0;
            memcpy_s(bs->original + bs->position, nbBytes, data, nbBytes);
            bs->position += nbBytes;
            return nbBytes;
        default:
            return 0;
        }
    }

    while (nbBytes) {
        gts_bs_write_int(bs, (int32_t) *data, 8);
        data++;
        nbBytes--;
    }
    return (uint32_t) (bs->position - begin);
}

uint8_t gts_bs_align(GTS_BitStream *bs)
{
    if (!bs) return 0;
    uint8_t res = 8 - bs->nbBits;
    if ( (bs->bsmode == GTS_BITSTREAM_READ) || (bs->bsmode == GTS_BITSTREAM_FILE_READ)) {
        if (res > 0) {
            gts_bs_read_int(bs, res);
        }
        return res;
    }
    if (bs->nbBits > 0) {
        gts_bs_write_int (bs, 0, res);
        return res;
    }
    return 0;
}

uint64_t gts_bs_available(GTS_BitStream *bs)
{
    if (!bs) return 0;

    if ( (bs->bsmode ==  GTS_BITSTREAM_WRITE)
            || (bs->bsmode == GTS_BITSTREAM_WRITE_DYN)
       )
        return (uint64_t) -1;

    //we are in MEM mode
    if (bs->bsmode == GTS_BITSTREAM_READ) {
        if (bs->size < bs->position)
            return 0;
        else
            return (bs->size - bs->position);
    }

    if (bs->bsmode==GTS_BITSTREAM_FILE_READ) {
        if (bs->position>bs->size) return 0;
        return (bs->size - bs->position);
    }
    if (bs->buffer_io)
        bs_flush_cache(bs);

    int64_t cur, end;
    cur = gts_ftell(bs->stream);
    gts_fseek(bs->stream, 0, SEEK_END);
    end = gts_ftell(bs->stream);
    gts_fseek(bs->stream, cur, SEEK_SET);
    return (uint64_t) (end - cur);
}

static GTS_Err BS_SeekIntern(GTS_BitStream *bs, uint64_t offset)
{
    uint32_t i;
    if (!bs) return GTS_BAD_PARAM;
    if ((bs->bsmode == GTS_BITSTREAM_READ) || (bs->bsmode ==  GTS_BITSTREAM_WRITE)) {
        if (offset > 0xFFFFFFFF) return GTS_IO_ERR;
        if (!bs->original) return GTS_BAD_PARAM;
        if (offset >= bs->size) {
            if ( (bs->bsmode == GTS_BITSTREAM_READ) || (bs->bsmode ==  GTS_BITSTREAM_WRITE) ) {
                if (offset > bs->size) {
                    ;
                }
                bs->position = bs->size;
                bs->nbBits = (bs->bsmode == GTS_BITSTREAM_READ) ? 8 : 0;
                return GTS_OK;
            }
            /*in DYN, gf_realloc ...*/
            bs->original = (int8_t*)gts_realloc(bs->original, (uint32_t) (offset + 1));
            if (!bs->original)
                return GTS_OUT_OF_MEM;
            for (i = 0; i < (uint32_t) (offset + 1 - bs->size); i++) {
                bs->original[bs->size + i] = 0;
            }
            bs->size = offset + 1;
        }
        bs->current = bs->original[offset];
        bs->position = offset;
        bs->nbBits = (bs->bsmode == GTS_BITSTREAM_READ) ? 8 : 0;
        return GTS_OK;
    }

    if (bs->buffer_io)
        bs_flush_cache(bs);

    gts_fseek(bs->stream, offset, SEEK_SET);

    bs->position = offset;
    bs->current = 0;
    bs->nbBits = (bs->bsmode == GTS_BITSTREAM_FILE_READ) ? 8 : 0;
    return GTS_OK;
}

GTS_Err gts_bs_seek(GTS_BitStream *bs, uint64_t offset)
{
    if (!bs) return GTS_BAD_PARAM;
    if (offset > bs->size) return GTS_BAD_PARAM;

    gts_bs_align(bs);
    return BS_SeekIntern(bs, offset);
}

uint32_t gts_bs_peek_bits(GTS_BitStream *bs, uint32_t numBits, uint64_t byte_offset)
{
    uint64_t curPos;
    uint32_t curBits, ret, current;
 
    if (!bs) return 0;

    if ( (bs->bsmode != GTS_BITSTREAM_READ) && (bs->bsmode != GTS_BITSTREAM_FILE_READ)) return 0;
    if (!numBits || (bs->size < bs->position + byte_offset)) return 0;

    /*store our state*/
    curPos = bs->position;
    curBits = bs->nbBits;
    current = bs->current;

    if (byte_offset) gts_bs_seek(bs, bs->position + byte_offset);
    ret = gts_bs_read_int(bs, numBits);

    gts_bs_seek(bs, curPos);
    bs->nbBits = curBits;
    bs->current = current;
    return ret;
}
