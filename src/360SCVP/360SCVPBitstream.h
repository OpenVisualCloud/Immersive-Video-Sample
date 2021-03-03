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
#ifndef _360SCVP_BITSTREAM_H_
#define _360SCVP_BITSTREAM_H_

#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "stdint.h"
#include <errno.h>
#ifdef __cplusplus
extern "C" {
#endif
    /*we must explicitely export our functions...*/
#define  EXPORT_C

    /*!
    *
    *    values > 0 are warning and info
    *    0 means no error
    *    values < 0 are errors
    */
    typedef enum
    {
        /*!success */
        GTS_OK = 0,
        /*!\n*/
        /*!One of the input parameter is not correct or cannot be used */
        GTS_BAD_PARAM = -1,
        /*! Memory malloc failed.*/
        GTS_OUT_OF_MEM = -2,
        /*! IO failure (disk access, system call failures)*/
        GTS_IO_ERR = -3,

    } GTS_Err;


void* gts_malloc(size_t size);

void *gts_realloc(void *ptr, size_t size);

void gts_free(void *ptr);

uint64_t gts_ftell(FILE *fp);

size_t gts_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);

uint64_t gts_fseek(FILE *fp, int64_t offset, int32_t whence);

uint32_t gts_get_bit_size(uint32_t MaxVal);

enum
{
    GTS_BITSTREAM_READ = 0,
    GTS_BITSTREAM_WRITE
};


/*the default size for allocation, especailly for new streams...*/
#define BS_MEM_BLOCK_ALLOC_SIZE        4096

/*private types*/
enum
{
    GTS_BITSTREAM_FILE_READ =  GTS_BITSTREAM_WRITE + 1,
    GTS_BITSTREAM_FILE_WRITE,
    /*private mode if we own the buffer*/
    GTS_BITSTREAM_WRITE_DYN
};

struct __tag_bitstream
{
    /*original stream data*/
    FILE *stream;
    //original data
    int8_t *original;
    //the size of our buffer in bytes
    uint64_t size;
    //current position in BYTES
    uint64_t position;
    //the byte readen/written
    uint32_t current;

    void(*EndOfStream)(void *par);
    void *par;

    //the number of bits in the current byte
    uint32_t nbBits;
    //the bitstream mode
    uint32_t bsmode;

    uint8_t zeroCount;

    int8_t *buffer_io;
    uint32_t buffer_io_size;
    uint32_t buffer_written;
};

typedef struct __tag_bitstream GTS_BitStream;

/*!
 *    \brief Constructs a bitstream from a buffer (read or write mode)
 *
 *    \param const int8_t * buffer  input buffer to read or write. In WRITE mode, this can be NULL to let the bitstream object dynamically allocate memory, in which case the size param is ignored.
 *    \param uint64_t       size    input size of the buffer given.
 *    \param uint32_t       mode    operation mode for this bitstream: GF_BITSTREAM_READ for read, GF_BITSTREAM_WRITE for write.
 *
 *    \return GTS_BitStream * new bitstream object
 *
 *    \note In write mode on an existing data buffer, data overflow is never signaled but simply ignored, it is the caller responsability to ensure it
 *    does not write more than possible.
 */
GTS_BitStream *gts_bs_new(const int8_t *buffer, uint64_t size, uint32_t mode);

/*!
 *    \brief Deletes the bitstream object, bitstream destructor from file handle
 *    \param GTS_BitStream *bs  input which is created by gts_bs_new
 *
 *    If the buffer was created by the bitstream, it is deleted if still present.
 */
void gts_bs_del(GTS_BitStream *bs);

/*!
 *    \brief Checks if bitstream position is aligned to a byte boundary.
 *
 *    \param GTS_BitStream *bs             input the target bitstream
 *
 *    \return bool, GF_TRUE if aligned with regard to the read/write mode, GF_FALSE otherwise
 */
bool gts_bs_is_align(GTS_BitStream *bs);

/*!
 *    \brief Aligns bitstream to next byte boundary. In write mode, this will write 0 bit values until alignment.
 *
 *    \param GTS_BitStream *bs             input the target bitstream
 *
 *    \return uint8_t the number of bits read/written until alignment
 */
uint8_t gts_bs_align(GTS_BitStream *bs);
/*!
 *    \brief Returns the number of bytes still available in the bitstream in read mode.
 *
 *    \param GTS_BitStream *bs             input the target bitstream
 *
 *    \return uint64_t   the number of bytes still available in read mode, -1 in write modes.
 */
uint64_t gts_bs_available(GTS_BitStream *bs);

/*!
 *    \brief Reads an integer coded on a number of bit.
 *
 *    \param GTS_BitStream *bs   input  the target bitstream
 *    \param uint32_t      nBits input  the number of bits to read
 *
 *    \return uint32_t the integer value read.
 */
uint32_t gts_bs_read_int(GTS_BitStream *bs, uint32_t nBits);

/*!
 *    \brief Reads a large integer coded on a number of bit bigger than 32.
 *
 *    \param GTS_BitStream *bs   input  the target bitstream
 *    \param uint32_t      nBits input  the number of bits to read
 *
 *    \return uint64_t the large integer value read.
 */
uint64_t gts_bs_read_long_int(GTS_BitStream *bs, uint32_t nBits);

/*!
 *    \brief Reads a data buffer
 *
 *    \param GTS_BitStream *bs      input  the target bitstream
 *    \param int8_t *       data    input  the data buffer to be filled
 *    \param uint32_t       nbBytes input   the amount of bytes to read
 *
 *    \return uint32_t the number of bytes actually read.
 *
 *    \warning the data buffer passed must be large enough to hold the desired amount of bytes.
 */
uint32_t gts_bs_read_data(GTS_BitStream *bs, int8_t *data, uint32_t nbBytes);

/*!
 *    \brief Reads an integer coded on 32 bits starting at a byte boundary in the bitstream.
 *
 *    \param GTS_BitStream *bs      input  the target bitstream
 *
 *    \return uint32_t  the integer value read.
 *
 *    \warning you must not use this function if the bitstream is not aligned
 */
uint32_t gts_bs_read_U32(GTS_BitStream *bs);

/*!
 *    \brief Returns current bit position in the bitstream - only works in memory mode.
 *
 *    \param GTS_BitStream *bs      input  the target bitstream
 *
 *    \return uint32_t the integer value read.
 */
uint32_t gts_bs_get_bit_offset(GTS_BitStream *bs);

/*!
 *    \brief Returns bit position in the current byte of the bitstream - only works in memory mode.
 *
 *    \param GTS_BitStream *bs      input  the target bitstream
 *
 *    \return uint32_t   the integer value read.
 */
uint32_t gts_bs_get_bit_position(GTS_BitStream *bs);

/*!
 *    \brief  Writes an integer on a given number of bits.
 *
 *    \param GTS_BitStream *bs      input  the target bitstream
 *    \param int32_t value          input  the integer to write
 *    \param int32_t nBits          input  number of bits used to code the integer
 */
void gts_bs_write_int(GTS_BitStream *bs, int32_t value, int32_t nBits);

/*!
 *    \brief Writes a data buffer.
 *
 *    \param GTS_BitStream *bs      input  the target bitstream
 *    \param const int8_t * data    input  the data to write
 *    \param uint32_t       nbBytes input  number of data bytes to write
 */
uint32_t gts_bs_write_data(GTS_BitStream *bs, const int8_t *data, uint32_t nbBytes);

/*!
 *    \brief Writes an integer on 8 bits starting at a byte boundary in the bitstream
 *
 *    \warning you must not use this function if the bitstream is not aligned
 *
 *    \param GTS_BitStream *bs      input  the target bitstream
 *    \param uint32_t       value   input  the int8_t value to write
 */
void gts_bs_write_U8(GTS_BitStream *bs, uint32_t value);
/*!
 *    \brief Writes an integer on 16 bits starting at a byte boundary in the bitstream.
 *
 *    \warning you must not use this function if the bitstream is not aligned
 *
 *    \param GTS_BitStream *bs             input    the target bitstream
 *    \param uint32_t       value          input    the short value to write
 */
void gts_bs_write_U16(GTS_BitStream *bs, uint32_t value);


/*!
 *    \brief Writes a give byte multiple times.
 *
 *    \param GTS_BitStream *bs             input the target bitstream
 *    \param uint8_t        byte           input the byte value to write
 *    \param uint32_t       count          input the number of times the byte should be written
 *
 *    \return the number of bytes written
 */
uint32_t gts_bs_write_byte(GTS_BitStream *bs, uint8_t byte, uint32_t count);


/*!
 *\brief Returns the reading/writting position in the buffer/file
 *
 *\param GTS_BitStream *bs   input the target bitstream
 *
 *\return uint64_t  the read/write position of the bitstream
 */
uint64_t gts_bs_get_position(GTS_BitStream *bs);

/*!
 *\brief Returns the size of the associated buffer/file.
 *
 *\param GTS_BitStream *bs   input the target bitstream
 *
 *\return uint64_t the size of the bitstream
 */
uint64_t gts_bs_get_size(GTS_BitStream *bs);

/*!
 *\brief Seeks the bitstream to a given offset after the beginning of the stream. This will perform alignment of the bitstream in all modes.
 *
 *\warning Results are unpredictable if seeking beyond the bitstream end is performed.
 *
 *\param GTS_BitStream *bs             input the target bitstream
 *\param uint64_t       offset         input buffer/file offset to seek to
 */
GTS_Err gts_bs_seek(GTS_BitStream *bs, uint64_t offset);

/*!
 *\brief Peeks a given number of bits (read without moving the position indicator) for read modes only
 *
 *\param GTS_BitStream *bs             input the target bitstream
 *\param uint32_t       numBits        input the number of bits to peek
 *\param uint64_t       byte_offset    input
    * if set, bitstream is aligned and moved from byte_offset before peeking (byte-aligned picking)
    * otherwise, bitstream is not aligned and bits are peeked from current state
 *
 *\return uint32_t the integer value read
*/
uint32_t gts_bs_peek_bits(GTS_BitStream *bs, uint32_t numBits, uint64_t byte_offset);

#ifdef __cplusplus
}
#endif

#endif        /*_360SCVP_BITSTREAM_H_*/
