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
//! \file     VideoShader.cpp
//! \brief    Implement class for VideoShader.
//!

#include "VideoShader.h"

VCD_NS_BEGIN

static void CheckShaderError(GLuint shader, GLuint flag, bool isProgram, const std::string& errorMessage);
static GLuint CreateShader(const std::string& text, GLenum shaderType);

VideoShader::VideoShader(const std::string& vertex,const std::string& fragment)
{
    m_shaders[0] = CreateShader(vertex, GL_VERTEX_SHADER);
    m_shaders[1] = CreateShader(fragment, GL_FRAGMENT_SHADER);
    m_program = glCreateProgram();
    for (unsigned int i = 0; i < NUM_SHADERS; i++)
    {
        glAttachShader(m_program, m_shaders[i]);
    }

    //glBindAttribLocation(m_program, 0, "position");

    glLinkProgram(m_program);
    CheckShaderError(m_program, GL_LINK_STATUS, true, "Error: Program linking failed!");

    glValidateProgram(m_program);
    CheckShaderError(m_program, GL_VALIDATE_STATUS, true, "Error: Program invalid!");

}

VideoShader::~VideoShader()
{
    for (unsigned int i = 0; i < NUM_SHADERS; i++)
    {
        glDetachShader(m_program, m_shaders[i]);
        glDeleteShader(m_shaders[i]);
    }

    glDeleteProgram(m_program);
}

void VideoShader::Bind()
{
    glUseProgram(m_program);
}

static GLuint CreateShader(const std::string& text, GLenum shaderType)
{
    GLuint shader = glCreateShader(shaderType);
    if (shader == 0)
        std::cerr << "Error: Shader creation failed!" << std::endl;

    const GLchar* shaderSourceStrings[1];
    GLint shaderSourceStringLengths[1];

    shaderSourceStrings[0] = text.c_str();
    shaderSourceStringLengths[0] = text.length();
    glShaderSource(shader, 1, shaderSourceStrings, shaderSourceStringLengths);
    glCompileShader(shader);

    CheckShaderError(shader, GL_COMPILE_STATUS, false, "Error: Shader compilation failed!");

    return shader;
}

static void CheckShaderError(GLuint shader, GLuint flag, bool isProgram, const std::string& errorMessage)
{
    GLint success = 0;
    GLchar error[1024] = { 0 };

    if (isProgram)
        glGetProgramiv(shader, flag, &success);
    else
        glGetShaderiv(shader, flag, &success);

    if (success == GL_FALSE)
    {
        if (isProgram)
            glGetProgramInfoLog(shader, sizeof(error), NULL, error);
        else
            glGetShaderInfoLog(shader, sizeof(error), NULL, error);

        std::cerr << errorMessage << ": '" << error << "'" << std::endl;
    }
}

void VideoShader::SetUniform1i(const char* name, int32_t value)
{
    int32_t location = glGetUniformLocation(m_program, name);
    glUniform1i(location, value);
}

void VideoShader::SetUniform1f(const char* name, float value)
{
    int32_t location = glGetUniformLocation(m_program, name);
    glUniform1f(location, value);
}

#ifdef _LINUX_OS_
void VideoShader::SetUniformMatrix2f(const char* name, glm::mat2 matrix)
{
    int32_t location = glGetUniformLocation(m_program, name);
    glUniformMatrix2fv(location,1,GL_FALSE,glm::value_ptr(matrix));
}

void VideoShader::SetUniformMatrix3f(const char* name, glm::mat3 matrix)
{
    int32_t location = glGetUniformLocation(m_program, name);
    glUniformMatrix3fv(location,1,GL_FALSE,glm::value_ptr(matrix));
}

void VideoShader::SetUniformMatrix4f(const char* name, glm::mat4 matrix)
{
    int32_t location = glGetUniformLocation(m_program, name);
    glUniformMatrix4fv(location,1,GL_FALSE,glm::value_ptr(matrix));
}
#endif

uint32_t VideoShader::SetAttrib(const char* name)
{
    return glGetAttribLocation(m_program, name);
}

VCD_NS_END
