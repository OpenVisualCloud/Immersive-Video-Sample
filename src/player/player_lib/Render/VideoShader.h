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

 *
 */

//!
//! \file     VideoShader.h
//! \brief    Defines class for VideoShader.
//!

#ifndef _VIDEOSHADER_H_
#define _VIDEOSHADER_H_

#ifdef _LINUX_OS_
#include <GL/glu.h>
#include <GL/glu_mangle.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>
#include <GL/glcorearb.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <GLES3/gl3platform.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#endif

#ifdef _ANDROID_OS_
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <GLES3/gl3platform.h>
#endif

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include "../Common/Common.h"

VCD_NS_BEGIN

class VideoShader
{
public:
    VideoShader()=default;
    VideoShader(const std::string &vertexFileName, const std::string &fragmentFileName);
    void Bind();

    virtual ~VideoShader();

    //! \brief Set uniform for an integer
    //!
    //! \param  [in] name
    //!         The unifrom name
    //! \param  [in] value
    //!         The value which will be set
    //! \return void
    //!         no return
    //!
    void SetUniform1i(const char *name, int32_t value);

    //! \brief Set uniform for a float
    //!
    //! \param  [in] name
    //!         The unifrom name
    //! \param  [in] value
    //!         The value which will be set
    //! \return void
    //!         no return
    //!
    void SetUniform1f(const char *name, float value);

#ifdef _LINUX_OS_
    //! \brief Set uniform for a  2x2 matrix
    //!
    //! \param  [in] name
    //!         The unifrom name
    //! \param  [in] matrix
    //!         The value which will be set
    //! \return void
    //!         no return
    //!
    void SetUniformMatrix2f(const char *name, glm::mat2 matrix);

    //! \brief Set uniform for a 3x3 matrix
    //!
    //! \param  [in] name
    //!         The unifrom name
    //! \param  [in] matrix
    //!         The value which will be set
    //! \return void
    //!         no return
    //!
    void SetUniformMatrix3f(const char *name, glm::mat3 matrix);

    //! \brief Set uniform for a 4x4 matrix
    //!
    //! \param  [in] name
    //!         The unifrom name
    //! \param  [in] matrix
    //!         The value which will be set
    //! \return void
    //!         no return
    //!
    void SetUniformMatrix4f(const char *name, glm::mat4 matrix);
#endif
    //! \brief Set attrib for vertex and coords
    //!
    //! \param  [in] name
    //!         The unifrom name
    //! \return uint32_t
    //!         attrib value
    //!
    uint32_t SetAttrib(const char *name);

protected:
private:
    static const unsigned int NUM_SHADERS = 2;
    VideoShader(const VideoShader &other) {}
    void operator=(const VideoShader &other) {}
    GLuint m_program;
    GLuint m_shaders[NUM_SHADERS];
};

VCD_NS_END
#endif /* _VIDEOSHADER_H_ */
