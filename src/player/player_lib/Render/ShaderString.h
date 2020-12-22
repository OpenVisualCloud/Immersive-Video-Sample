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
//! \file     ShaderString.h
//! \brief    Defines class for ShaderString.
//!
#ifndef _SHADERSTRING_H_
#define _SHADERSTRING_H_
#include <string>

std::string const shader_r2t_vs =
"#version 300 es\n"
"in vec3 vPosition;\n"
"in vec2 aTexCoord;\n"
"out vec2 texCoord;\n"
"void main()\n"
"{\n"
"    gl_Position = vec4(vPosition,1.0);\n"
"    texCoord = aTexCoord;\n"
"}\n";

std::string const shader_r2t_fs =
"#version 300 es\n"
"precision mediump float;\n"
"in vec2 texCoord;\n"
"out vec4 fragColor;\n"
"uniform sampler2D frameTex;\n"
"uniform sampler2D frameU;\n"
"uniform sampler2D frameV;\n"
"uniform int isNV12;\n"
"void main()\n"
"{\n"
"    if(isNV12 == 0)\n"
"    {\n"
"    vec4 c = vec4((texture(frameTex, texCoord).r - 16.0/255.0) * 1.164);\n"
"    vec4 U = vec4(texture(frameU, texCoord).r - 128.0/255.0);\n"
"    vec4 V = vec4(texture(frameV, texCoord).r - 128.0/255.0);\n"
"    c += V * vec4(1.596, -0.813, 0, 0);\n"
"    c += U * vec4(0, -0.392, 2.017, 0);\n"
"    c.a = 1.0;\n"
"    fragColor = c;\n"
"    }\n"
"    else\n"
"    {\n"
"    vec3 yuv;\n"
"    vec3 rgb;\n"
"    yuv.x = texture(frameTex, texCoord).r -16./256.;\n"
"    yuv.y = texture(frameU, texCoord).r - 128./256.;\n"
"    yuv.z = texture(frameU, texCoord).g - 128./256.;\n"
"    rgb = mat3( 1,       1,         1,\n"
"                0,       -0.39465,  2.03211,\n"
"                1.13983, -0.58060,  0) * yuv;\n"
"    fragColor = vec4(rgb, 1);\n"
"    }\n"
"}\n";

std::string const shader_screen_vs =
"#version 300 es\n"
"in vec3 vertex;\n"
"in vec2 texCoord0;\n"
"uniform mat4 uProjMatrix;\n"
"uniform mat4 uViewMatrix;\n"
"out vec2 texCoord;\n"
"void main()\n"
"{\n"
"    gl_Position = uProjMatrix * uViewMatrix *  vec4(vertex, 1.0);\n"
"    texCoord = texCoord0;\n"
"}\n";

std::string const shader_screen_fs =
"#version 300 es\n"
"precision mediump float;\n"
"uniform sampler2D frameTex_screen;\n"
"in vec2 texCoord;\n"
"out vec4 fragColor;\n"
"void main()\n"
"{\n"
"    fragColor = texture(frameTex_screen, texCoord);\n"
"}\n";

std::string const shader_skybox_vs =
"#version 300 es\n"
"in vec3 aPos;\n"
"in vec3 transPos;\n"
"out vec3 TexCoords;\n"
"uniform mat4 projection;\n"
"uniform mat4 view;\n"
"void main()\n"
"{\n"
"    TexCoords = transPos;\n"
"    gl_Position = projection * view * vec4(aPos, 1.0);\n"
"}\n";

std::string const shader_skybox_fs =
"#version 300 es\n"
"precision mediump float;\n"
"uniform samplerCube skybox;\n"
"in vec3 TexCoords;\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"    FragColor = texture(skybox, TexCoords);\n"
"}\n";

std::string const shader_vs_android =
"attribute vec4 anPosition;\n"
"attribute vec2 anTexCoords;\n"
"varying vec2 vTexCoords;\n"
"void main()\n"
"{\n"
"    gl_Position = vec4(anPosition);"
"    vTexCoords = anTexCoords;"
"}\n";

std::string const shader_fs_android =
"#extension GL_OES_EGL_image_external : require\n"
"precision mediump float;\n"
"uniform samplerExternalOES anTexture;\n"
"varying vec2 vTexCoords;\n"
"void main()\n"
"{\n"
"    gl_FragColor = texture2D(anTexture, vTexCoords);"
"}\n";
#endif /* _SHADERSTRING_H_ */
