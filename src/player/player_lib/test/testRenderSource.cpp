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
//! \file     testRenderSource.cpp
//! \brief    unit test for RenderSource.
//!
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdlib.h>
#include "gtest/gtest.h"
#include "../RenderSource.h"
#include "../SWRenderSource.h"
#include "../FFmpegMediaSource.h"
#include "../RenderBackend.h"
#include <vector>

VCD_NS_BEGIN

#define WINDOW_WIDTH 960
#define WINDOW_HEIGHT 960

namespace
{
class RenderSourceTest : public testing::Test
{
public:
    virtual void SetUp()
    {
        // initialize glfw
        if (!glfwInit())
        {
            LOG(ERROR)<< "glfw failed to init" << std::endl;
            glfwTerminate();
            return;
        }
        //tranverse();
        // open a window
        //glfwWindowHint(GLFW_SAMPLES, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "VR Player", NULL, NULL); //done
        if (!window)
        {
            LOG(ERROR)<< "failed to open window" << std::endl;
            glfwTerminate();
            return;
        }
        glfwMakeContextCurrent(window);
        glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        glfwPollEvents();
        glfwSetCursorPos(window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2); //done

        // initialize opengl
        // glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        // glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CCW);
        glCullFace(GL_BACK);

        mediaSource = new FFmpegMediaSource();
        renderBackend = new RenderBackend();
        renderSource = new SWRenderSource(renderBackend);
        struct RenderConfig renderConfig;
        renderConfig.url = "./packet_MultiBS.265";
        mediaSource->Initialize(renderConfig);
    }
    virtual void TearDown()
    {
    }
    struct RegionInfo regionInfo;
    struct SphereRegion sphereRegion;
    RenderSource *renderSource;
    MediaSource *mediaSource;
    RenderBackend *renderBackend;
    GLFWwindow *window;
};

TEST_F(RenderSourceTest, Initialize)
{
    struct MediaSourceInfo mediaSourceInfo;
    mediaSourceInfo = mediaSource->GetMediaSourceInfo();
    RenderStatus status = renderSource->Initialize(&mediaSourceInfo);
    ASSERT_EQ(status, RENDER_STATUS_OK);
}

TEST_F(RenderSourceTest, CreateRenderSource)
{
    struct MediaSourceInfo mediaSourceInfo;
    mediaSourceInfo = mediaSource->GetMediaSourceInfo();
    RenderStatus statusInit = renderSource->Initialize(&mediaSourceInfo);
    RenderStatus statusCreate = renderSource->CreateRenderSource(renderBackend);
    ASSERT_EQ(statusCreate, RENDER_STATUS_OK);
}

} // namespace

VCD_NS_END
