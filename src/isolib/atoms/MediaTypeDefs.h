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
//! \file:   MediaTypeDefs.h
//! \brief:  Media Type Definition class
//! \detail: defines media type
//!
//! Created on October 16, 2019, 13:39 PM
//!

#ifndef _MEDIATYPEDEFS_H_
#define _MEDIATYPEDEFS_H_

#include <stdexcept>
#include <string>
#include "FormAllocator.h"

VCD_MP4_BEGIN

enum class MediaType    //!< Supported bitstream media types
{
    AVCSTR,
    HEVCSTR,
    UNKNOWN
};

namespace MediaTypeTool
{
    //!
    //! \brief    Get Type
    //!
    //! \param    [in] const std::string
    //!           type
    //! \param    [in] const std::string
    //!           file Name For Error Msg
    //!
    //! \return   MediaType
    //!           media type
    //!
    inline MediaType GetType(const std::string pType, const std::string fileNameForErrorMsg)
    {
        if (pType == "avc1" || pType == "avc3")
        {
            return MediaType::AVCSTR;
        }
        else if (pType == "hvc1" || pType == "hev1")
        {
            return MediaType::HEVCSTR;
        }
        else
        {
            // Unsupported code type
            std::string fileInfo = (fileNameForErrorMsg.empty()) ? "" : " (" + fileNameForErrorMsg + ")";

            if (pType.empty())
            {
                ISO_LOG(LOG_ERROR, "Failed to define media type, code_type not set %s\n", fileInfo.c_str());
                throw Exception();
            }
            else
            {
                ISO_LOG(LOG_ERROR, "Failed to define media type for unsupported code_type '%s', %s\n", pType.c_str(), fileInfo.c_str());
                throw Exception();
            }
        }
    }

    //!
    //! \brief    Stream Type Name
    //!
    //! \param    [in] MediaType
    //!           media type
    //!
    //! \return   const std::string
    //!           media type string
    //!
    inline const std::string GetMP4VRImpl::StreamTypeName(MediaType mediaType)
    {
        switch (mediaType)
        {
        case MediaType::AVCSTR:
        {
            return "AVC";
            break;
        }
        case MediaType::HEVCSTR:
        {
            return "HEVC";
            break;
        }
        default:
        {
            // Invalid media type
            return "INVALID";
            break;
        }
        }
    }
}

VCD_MP4_END;
#endif /* _MEDIATYPEDEFS_H_ */
