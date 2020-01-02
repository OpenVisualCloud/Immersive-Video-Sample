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
//! \file     GLFWRenderContext.cpp
//! \brief    Implement class for GLFWRenderContext.
//!

#include "GLFWRenderContext.h"

VCD_NS_BEGIN

GLFWRenderContext::GLFWRenderContext()
{
    m_renderContextType = GLFW_CONTEXT;
}

GLFWRenderContext::GLFWRenderContext(uint32_t width, uint32_t height)
{
    m_renderContextType = GLFW_CONTEXT;
    m_windowWidth  = width;
    m_windowHeight = height;
}

GLFWRenderContext::~GLFWRenderContext()
{
}

RenderStatus GLFWRenderContext::SwapBuffers(void * window, int param)
{
    glfwSwapBuffers((GLFWwindow *)m_window);
    glfwPollEvents();

    return RENDER_STATUS_OK;
}

bool GLFWRenderContext::isRunning()
{
    return !glfwGetKey((GLFWwindow *)m_window, GLFW_KEY_ESCAPE) && !glfwWindowShouldClose((GLFWwindow *)m_window);
}

RenderStatus GLFWRenderContext::GetStatusAndPose(float *yaw, float *pitch, uint32_t* status)
{
    static glm::vec2 transfer(0, RENDER_PI);
    glm::vec3 direction(0.0f, 0.0f, 1.0f);
    double xpos, ypos;

    glfwGetCursorPos((GLFWwindow *)m_window, &xpos, &ypos);
    glfwSetCursorPos((GLFWwindow *)m_window, m_windowWidth / 2, m_windowHeight / 2);
    if (glfwGetMouseButton((GLFWwindow *)m_window, GLFW_MOUSE_BUTTON_LEFT))
    {
        m_horizontalAngle += m_mouseSpeed * float(m_windowWidth / 2 - xpos);
        m_verticalAngle += m_mouseSpeed * float(m_windowHeight / 2 - ypos);
        if (m_verticalAngle > RENDER_PI / 2)
            m_verticalAngle = RENDER_PI / 2;
        if (m_verticalAngle < -RENDER_PI / 2)
            m_verticalAngle = -RENDER_PI / 2;
    }
    if (glfwGetKey((GLFWwindow *)m_window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        m_verticalAngle = transfer.x;
        m_verticalAngle -= 3 * m_speed;

        if (m_verticalAngle > RENDER_PI / 2)
            m_verticalAngle = RENDER_PI / 2;
        if (m_verticalAngle < -RENDER_PI / 2)
            m_verticalAngle = -RENDER_PI / 2;
    }
    if (glfwGetKey((GLFWwindow *)m_window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        m_verticalAngle = transfer.x;
        m_verticalAngle += 3 * m_speed;
        if (m_verticalAngle > RENDER_PI / 2)
            m_verticalAngle = RENDER_PI / 2;
        if (m_verticalAngle < -RENDER_PI / 2)
            m_verticalAngle = -RENDER_PI / 2;
    }
    if (glfwGetKey((GLFWwindow *)m_window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        m_horizontalAngle = transfer.y;
        m_horizontalAngle -= 3 * m_speed;
    }
    if (glfwGetKey((GLFWwindow *)m_window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        m_horizontalAngle = transfer.y;
        m_horizontalAngle += 3 * m_speed;
    }
    if (glfwGetKey((GLFWwindow *)m_window, GLFW_KEY_S) == GLFW_PRESS)
    {
        *status = PLAY;
    }
    if (glfwGetKey((GLFWwindow *)m_window, GLFW_KEY_P) == GLFW_PRESS)
    {
        *status = PAUSE;
    }
    direction = glm::vec3(
        cos(m_verticalAngle) * sin(m_horizontalAngle), //1*0
        sin(m_verticalAngle),                        //0
        cos(m_verticalAngle) * cos(m_horizontalAngle)  //1*1
    );

    glm::vec3 right = glm::vec3(
        sin(m_horizontalAngle - RENDER_PI / 2.0f),
        0,
        cos(m_horizontalAngle - RENDER_PI / 2.0f));
    //    glm::vec3 right = glm::cross( direction, glm::vec3(0.0,1.0,0.0));
    glm::vec3 up = glm::cross(right, direction);
    float FoV = m_initialFoV; // - 5 * glfwGetMouseWheel();
    float aspect = float(m_windowWidth) / m_windowHeight;
    m_projectionMatrix = glm::perspective(glm::radians(-2 * FoV), aspect, 0.01f, 1000.0f);
    m_viewModelMatrix = glm::lookAt(
        //        position,           // Camera is here
        glm::vec3(0, 0, 0),
        //        position+direction, // and looks here : at the same position, plus "direction"
        direction,
        up // Head is up (set to 0,-1,0 to look upside-down)
    );
    transfer.x = m_verticalAngle;
    transfer.y = m_horizontalAngle;
    // eular angle and (longitude, latitude) transformation.
    float r = sqrt(direction[0] * direction[0] + direction[1] * direction[1] + direction[2] * direction[2]);
    float longitude = atan2(direction[0], -direction[2]);
    float latitude = asin(-direction[1] / r);
    float u = 0.5f - longitude / 2 / RENDER_PI;
    float v = 0.5f - latitude / RENDER_PI;
    *yaw = (u - 0.5f) * 360;
    *pitch = (0.5f - v) * 180;

    return RENDER_STATUS_OK;
}

RenderStatus GLFWRenderContext::InitContext()
{
    // initialize glfw
    if (!glfwInit())
    {
        LOG(ERROR)<< "glfw failed to init" << std::endl;
        glfwTerminate();
        return RENDER_ERROR;
    }
    //tranverse();
    // open a window
    //glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    m_window = glfwCreateWindow(m_windowWidth, m_windowHeight, "VR Player", NULL, NULL); //done
    if (!m_window)
    {
        LOG(ERROR)<< "failed to open window" << std::endl;
        glfwTerminate();
        return RENDER_ERROR;
    }
    glfwMakeContextCurrent((GLFWwindow *)m_window);
    glfwSetInputMode((GLFWwindow *)m_window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetInputMode((GLFWwindow *)m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwPollEvents();
    glfwSetCursorPos((GLFWwindow *)m_window, m_windowWidth / 2, m_windowHeight / 2); //done

    // initialize opengl
    // glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    // glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);

    return RENDER_STATUS_OK;
}

VCD_NS_END
