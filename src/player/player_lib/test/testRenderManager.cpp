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
#include "gtest/gtest.h"
#include "../RenderManager.h"
#include "../FFmpegMediaSource.h"
#include "../DMABufferRenderSource.h"

#define WINDOW_WIDTH 960
#define WINDOW_HEIGHT 960

VCD_NS_BEGIN

namespace
{
class RenderManagerTest : public testing::Test
{
public:
    virtual void SetUp()
    {
    }
    virtual void TearDown()
    {
    }
    // struct RenderConfig config;
    GLFWwindow *window;
};

TEST_F(RenderManagerTest, ConstructorTest)
{
    struct RenderConfig config;
    config.windowWidth = WINDOW_WIDTH;
    config.windowHeight = WINDOW_HEIGHT;
    config.decoderType = SW_DECODER;
    config.sourceType = DASH_SOURCE;
    config.contextType = GLFW_CONTEXT;
    config.useDMABuffer = 0;
    config.url = "http://10.67.119.41:8080/4k_500frames_rc1/Test.mpd";
    config.cachePath = "/home/media/cache";
    config.viewportHeight = 960;
    config.viewportWidth = 960;
    config.viewportHFOV = 80;
    config.viewportVFOV = 80;
    RenderManager *renderManager = new RenderManager(config);
    ASSERT_FALSE(renderManager == NULL);
}

TEST_F(RenderManagerTest, InitializeTest)
{
    struct RenderConfig config;
    config.windowWidth = WINDOW_WIDTH;
    config.windowHeight = WINDOW_HEIGHT;
    config.decoderType = SW_DECODER;
    config.sourceType = DASH_SOURCE;
    config.contextType = GLFW_CONTEXT;
    config.useDMABuffer = 0;
    config.url = "http://10.67.119.41:8080/4k_500frames_rc1/Test.mpd";
    config.cachePath = "/home/media/cache";
    config.viewportHeight = 960;
    config.viewportWidth = 960;
    config.viewportHFOV = 80;
    config.viewportVFOV = 80;
    RenderManager *renderManager = new RenderManager(config);
    struct RenderConfig renderConfig;
    RenderStatus status = renderManager->Initialize();
    ASSERT_TRUE(status == RENDER_STATUS_OK);
    delete renderManager;
    renderManager = NULL;
}

TEST_F(RenderManagerTest, PrepareRenderTest)
{
    struct RenderConfig config;
    config.windowWidth = WINDOW_WIDTH;
    config.windowHeight = WINDOW_HEIGHT;
    config.decoderType = SW_DECODER;
    config.sourceType = DASH_SOURCE;
    config.contextType = GLFW_CONTEXT;
    config.useDMABuffer = 0;
    config.url = "http://10.67.119.41:8080/4k_500frames_rc1/Test.mpd";
    config.cachePath = "/home/media/cache";
    config.viewportHeight = 960;
    config.viewportWidth = 960;
    config.viewportHFOV = 80;
    config.viewportVFOV = 80;
    RenderManager *renderManager = new RenderManager(config);
    RenderStatus status1 = renderManager->Initialize();
    ASSERT_TRUE(status1 == RENDER_STATUS_OK);
    RenderStatus status2;
    float poseYaw, posePitch;
    renderManager->GetStatusAndPose(&poseYaw, &posePitch, NULL);
    renderManager->SetViewport(poseYaw, posePitch);
    for (int i = 0; i < 10; i++)
    status2 = renderManager->PrepareRender();
    // ASSERT_TRUE(status2 == RENDER_STATUS_OK);
    delete renderManager;
    renderManager = NULL;
}

} // namespace

VCD_NS_END
