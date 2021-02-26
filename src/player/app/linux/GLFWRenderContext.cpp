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

#ifdef _LINUX_OS_

#include "GLFWRenderContext.h"
#include "../../../utils/tinyxml2.h"
#include <math.h>

#define SPEED_CNT_THRESHOLD 150

VCD_NS_BEGIN
using namespace tinyxml2;

GLFWRenderContext::GLFWRenderContext()
{
    m_renderContextType = GLFW_CONTEXT;
    m_needMotionTest = false;
    m_motionConfig.freq = 0;
    m_motionConfig.timeInterval = 0;
    m_motionConfig.mode = NULL;
    m_zoomCnt = 0.0f;
    m_fullWidth = 0;
    m_fullHeight = 0;
    m_zoomFactor = 0.0f;
}

GLFWRenderContext::GLFWRenderContext(struct RenderConfig config)
{
    m_renderContextType = GLFW_CONTEXT;
    m_windowWidth  = config.windowWidth;
    m_windowHeight = config.windowHeight;
    m_hFOV         = config.viewportHFOV;
    m_vFOV         = config.viewportVFOV;
    m_motionConfig.mode = NULL;
    m_needMotionTest = (RENDER_STATUS_OK == GetMotionOptionParams()) ? true : false;
    m_zoomCnt = 0.0f;
    m_fullWidth = 0;
    m_fullHeight = 0;
    m_zoomFactor = 0.0f;
}

GLFWRenderContext::~GLFWRenderContext()
{
    SAFE_DELETE(m_motionConfig.mode);
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

void GLFWRenderContext::AutoChangePos(float *hPos, float *vPos)
{
    m_verticalAngle = *vPos;
    m_horizontalAngle = *hPos;
    if (!strcmp(m_motionConfig.mode, "smooth"))
    {
        *hPos += 2 * RENDER_PI / m_motionConfig.freq;
    }
    else if (!strcmp(m_motionConfig.mode, "sharp"))
    {
        static int timeInterval = 0;
        timeInterval++;
        if (timeInterval % m_motionConfig.timeInterval == 0)
        {
            *hPos +=  2 * RENDER_PI / m_motionConfig.freq;
        }
    }
    else if (!strcmp(m_motionConfig.mode, "quicksin"))
    {
        static int timeInterval = 0;
        timeInterval++;
        if (timeInterval > 10 && timeInterval % m_motionConfig.timeInterval == 0)
        {
            *hPos += 2 * RENDER_PI / m_motionConfig.freq;
            *vPos = RENDER_PI / 2 * sin(*hPos*1.5);
        }
    }
    else if (!strcmp(m_motionConfig.mode, "swing"))
    {
        static int32_t rightOrient = 1;
        if (*hPos > RENDER_PI / 2 + RENDER_PI)
        {
            rightOrient = -1;
        }
        else if (*hPos < -RENDER_PI / 2 + RENDER_PI)
        {
            rightOrient = 1;
        }
        *hPos += 2 * RENDER_PI / m_motionConfig.freq * rightOrient;
    }
    else if (!strcmp(m_motionConfig.mode, "dynamicSpeed"))
    {
        static int speedCnt = 0;
        static bool isTurn = false;
        if (speedCnt >= SPEED_CNT_THRESHOLD) isTurn = true;
        if (!isTurn)
        {
            speedCnt+=4;
        }
        else
        {
            speedCnt--;
        }
        if (speedCnt > 0)
            *hPos -= 2 * RENDER_PI / m_motionConfig.freq * speedCnt;
    }
    else //xml input check
    {
        LOG(ERROR)<<"test motion option support only smooth and sharp mode!"<<std::endl;
        m_needMotionTest = false;
    }
}

RenderStatus GLFWRenderContext::GetMotionOptionParams()
{
    XMLDocument config;
    config.LoadFile("config.xml");
    XMLElement *info = config.RootElement();
    if (NULL == info)
    {
        LOG(ERROR) << " XML parse failed! " << std::endl;
        return RENDER_ERROR;
    }
    XMLElement *motionElem = info->FirstChildElement("testMotionOption");
    if (NULL == motionElem)
    {
        return RENDER_ERROR;
    }
    const XMLAttribute *attOfMotion = motionElem->FirstAttribute();
    if (NULL == attOfMotion)
    {
        LOG(ERROR) << "XML parse failed!" << std::endl;
        return RENDER_ERROR;
    }
    m_motionConfig.mode = new char[128]();
    if (m_motionConfig.mode != NULL)
    {
        memcpy_s(m_motionConfig.mode, strlen(attOfMotion->Value()), attOfMotion->Value(), strlen(attOfMotion->Value()));
    }
    else
    {
        LOG(ERROR) << " Mode is invalid! " << std::endl;
        return RENDER_ERROR;
    }
    XMLElement* freqElem = motionElem->FirstChildElement("freq");
    XMLElement* interElem = motionElem->FirstChildElement("timeInterval");
    if (freqElem != NULL && interElem != NULL)
    {
        m_motionConfig.freq = atoi(freqElem->GetText());
        m_motionConfig.timeInterval = atoi(interElem->GetText());
    }
    else
    {
        LOG(ERROR) << " freq OR timeInterval is invalid! " << std::endl;
        return RENDER_ERROR;
    }

    return RENDER_STATUS_OK;
}

RenderStatus GLFWRenderContext::GetStatusAndPoseFor3D(HeadPose *pose, uint32_t* status)
{
    static glm::vec2 transfer(RENDER_PI, 0);
    glm::vec3 direction(0.0f, 0.0f, 1.0f);
    static bool isInitialSetting = true;
    double xpos, ypos;

    if (m_needMotionTest)
    {
        AutoChangePos(&transfer.x, &transfer.y);
    }
    if (isInitialSetting) // init horizon angle for 2D view model
    {
        m_horizontalAngle = transfer.x;
        isInitialSetting = false;
    }
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
        m_verticalAngle = transfer.y;
        m_verticalAngle += m_speed;

        if (m_verticalAngle > RENDER_PI / 2)
            m_verticalAngle = RENDER_PI / 2;
        if (m_verticalAngle < -RENDER_PI / 2)
            m_verticalAngle = -RENDER_PI / 2;
    }
    if (glfwGetKey((GLFWwindow *)m_window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        m_verticalAngle = transfer.y;
        m_verticalAngle -= m_speed;
        if (m_verticalAngle > RENDER_PI / 2)
            m_verticalAngle = RENDER_PI / 2;
        if (m_verticalAngle < -RENDER_PI / 2)
            m_verticalAngle = -RENDER_PI / 2;
    }
    if (glfwGetKey((GLFWwindow *)m_window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        m_horizontalAngle = transfer.x;
        m_horizontalAngle -= m_speed;
    }
    if (glfwGetKey((GLFWwindow *)m_window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        m_horizontalAngle = transfer.x;
        m_horizontalAngle += m_speed;
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
    float aspect = m_hFOV / m_vFOV;
    m_projectionMatrix = glm::perspective(glm::radians(m_vFOV), aspect, 0.01f, 1000.0f);
    m_viewModelMatrix = glm::lookAt(
        //        position,           // Camera is here
        glm::vec3(0, 0, 0),
        //        position+direction, // and looks here : at the same position, plus "direction"
        direction,
        up // Head is up (set to 0,-1,0 to look upside-down)
    );
    if (!m_needMotionTest)
    {
        transfer.y = m_verticalAngle;
        transfer.x = m_horizontalAngle;
    }
    // eular angle and (longitude, latitude) transformation.
    float r = sqrt(direction[0] * direction[0] + direction[1] * direction[1] + direction[2] * direction[2]);
    float longitude = atan2(direction[0], -direction[2]);
    float latitude = asin(-direction[1] / r);
    float u = 0.5f - longitude / 2 / RENDER_PI;
    float v = 0.5f - latitude / RENDER_PI;
    pose->yaw = (0.5f - u) * 360;
    pose->pitch = (v - 0.5f) * 180;
    return RENDER_STATUS_OK;
}

RenderStatus GLFWRenderContext::GetStatusAndPoseFor2D(HeadPose *pose, uint32_t* status)
{
    static glm::vec2 transfer(-RENDER_PI / 2, 0);
    static bool isInitialSetting = true;
    // front is always against z axis
    glm::vec3 front(0.0f, 0.0f, -1.0f);
    double xpos, ypos;
    // bool isStopZoomOut = false;
    if (m_needMotionTest)
    {
        AutoChangePos(&transfer.x, &transfer.y);
    }
    if (isInitialSetting) // init horizon angle for 2D view model
    {
        m_horizontalAngle = transfer.x;
    }
    // position x y are view point in axis (-1 ~ 1)
    float position_x = (m_horizontalAngle + RENDER_PI / 2);
    float position_y = m_verticalAngle;

    //boundary limiation
    float halfXLenInMesh = tan(m_hFOV / 180.0f * RENDER_PI / 2);
    float halfYLenInMesh = tan(m_vFOV / 180.0f * RENDER_PI / 2);
    // if (position_x - halfXLenInMesh <= -1.0f)
    if ((position_x - halfXLenInMesh < -1.0f) && fabs(position_x - halfXLenInMesh + 1.0f) > 1e-6 || fabs(position_x - halfXLenInMesh + 1.0f) <= 1e-6)
    {
        position_x = -1.0f + halfXLenInMesh;
        m_horizontalAngle = position_x - RENDER_PI / 2;
        // isStopZoomOut = true;
    }
    // if (position_x + halfXLenInMesh >= 1.0f)
    if ((position_x + halfXLenInMesh > 1.0f) && fabs(position_x + halfXLenInMesh - 1.0f) > 1e-6 || fabs(position_x + halfXLenInMesh - 1.0f) <= 1e-6)
    {
        position_x = 1.0f - halfXLenInMesh;
        m_horizontalAngle = position_x - RENDER_PI / 2;
        // isStopZoomOut = true;
    }
    // if (position_y - halfYLenInMesh <= -1.0f)
    if ((position_y - halfYLenInMesh < -1.0f) && fabs(position_y - halfYLenInMesh + 1.0f) > 1e-6 || fabs(position_y - halfYLenInMesh + 1.0f) <= 1e-6)
    {
        position_y = -1.0f + halfYLenInMesh;
        m_verticalAngle = position_y;
        // isStopZoomOut = true;
    }
    // if (position_y + halfYLenInMesh >= 1.0f)
    if ((position_y + halfYLenInMesh > 1.0f) && fabs(position_y + halfYLenInMesh - 1.0f) > 1e-6 || fabs(position_y + halfYLenInMesh - 1.0f) <= 1e-6)
    {
        position_y = 1.0f - halfYLenInMesh;
        m_verticalAngle = position_y;
        // isStopZoomOut = true;
    }

    glm::vec3 right = glm::cross( front, glm::vec3(0.0,1.0,0.0));
    glm::vec3 up = glm::cross(right, front);

    // 1. get aspect.
    //tanHalAspect = tanHalfFovx / tanHalfFovy = (w/W) / (h/H) is a constant.
    float tanHalAspect = float(m_windowWidth) / m_fullWidth / (float(m_windowHeight) / m_fullHeight);
    // 2. set fov( in degrees )
    if (isInitialSetting) // initial to set zoom factor is 1
    {
        m_vFOV = 2 * atan(float(m_windowHeight) / m_fullHeight) * 180 / RENDER_PI;
        m_hFOV = 2 * atan(tan(m_vFOV * RENDER_PI / 180.0f / 2) * tanHalAspect) * 180 / RENDER_PI;
        m_zoomFactor = 1.0f;
        m_zoomCnt = tan(m_vFOV * RENDER_PI / 180.0f - RENDER_PI / 2) / 0.05;
        isInitialSetting = false;
    }
    else
    {
        float v_tmpFOV = 180.0 / RENDER_PI * (atan(0.05 * m_zoomCnt) + 0.5 * RENDER_PI);
        v_tmpFOV = v_tmpFOV >= 90.0f ? 90.0f : v_tmpFOV;
        float aspectHV = m_hFOV / m_vFOV;
        if (aspectHV >= 1)// h >= v
        {
            m_hFOV = 2 * atan(tan(v_tmpFOV * RENDER_PI / 180.0f / 2) * tanHalAspect) * 180 / RENDER_PI;
            if (m_hFOV > 90.0f) // vFOV is greater than 90.0f
            {
                m_hFOV = 90.0f;
                m_vFOV = 2 * atan(tan(m_hFOV * RENDER_PI / 180.0f / 2) / tanHalAspect) * 180 / RENDER_PI;
            }
            else
            {
                m_vFOV = v_tmpFOV;
            }
        }
        else // h < v
        {
            m_vFOV = v_tmpFOV;
            m_hFOV = 2 * atan(tan(m_vFOV * RENDER_PI / 180.0f / 2) * tanHalAspect) * 180 / RENDER_PI;
        }
    }

    // 3. get even key and get m_horizontalAngle & m_verticalAngle
    glfwGetCursorPos((GLFWwindow *)m_window, &xpos, &ypos);
    glfwSetCursorPos((GLFWwindow *)m_window, m_windowWidth / 2, m_windowHeight / 2);
    float frameMove = m_speed * m_renderInterval / 1000 * 2; // speed * interval * range of [-1, 1]
    pose->speed = 0;
    pose->viewOrient.orientation = 0;
    pose->viewOrient.mode = ORIENT_NONE;
    if (glfwGetMouseButton((GLFWwindow *)m_window, GLFW_MOUSE_BUTTON_LEFT))
    {
        m_horizontalAngle -= m_mouseSpeed * float(m_windowWidth / 2 - xpos);
        m_verticalAngle += m_mouseSpeed * float(m_windowHeight / 2 - ypos);
    }
    if (glfwGetKey((GLFWwindow *)m_window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        m_verticalAngle = transfer.y;
        m_verticalAngle += frameMove * m_fullWidth / m_fullHeight;
        pose->viewOrient.orientation = RENDER_PI / 2;
        pose->viewOrient.mode = ORIENT_MOVE;
        pose->speed = m_speed;
    }
    if (glfwGetKey((GLFWwindow *)m_window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        m_verticalAngle = transfer.y;
        m_verticalAngle -= frameMove * m_fullWidth / m_fullHeight;
        pose->viewOrient.orientation = RENDER_PI / 2 * 3;
        pose->viewOrient.mode = ORIENT_MOVE;
        pose->speed = m_speed;
    }
    if (glfwGetKey((GLFWwindow *)m_window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        m_horizontalAngle = transfer.x;
        m_horizontalAngle += frameMove;
        pose->viewOrient.orientation = 0;
        pose->viewOrient.mode = ORIENT_MOVE;
        pose->speed = m_speed;
    }
    if (glfwGetKey((GLFWwindow *)m_window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        m_horizontalAngle = transfer.x;
        m_horizontalAngle -= frameMove;
        pose->viewOrient.orientation = RENDER_PI;
        pose->viewOrient.mode = ORIENT_MOVE;
        pose->speed = m_speed;
    }

    if (glfwGetKey((GLFWwindow *)m_window, GLFW_KEY_S) == GLFW_PRESS) // zoom out
    {
        // if (!isStopZoomOut)
        if (m_vFOV < 90.0f && m_hFOV < 90.0f)
        {
            m_zoomCnt = m_zoomCnt + 1;
            pose->viewOrient.orientation = 0;
            pose->viewOrient.mode = ORIENT_ZOOMOUT;
            pose->speed = m_speed;
        }
    }
    if (glfwGetKey((GLFWwindow *)m_window, GLFW_KEY_W) == GLFW_PRESS) // zoom in
    {
        if (m_zoomFactor <= 3.0f) // zoom in limitation
        {
            m_zoomCnt = m_zoomCnt - 1;
            pose->viewOrient.orientation = 0;
            pose->viewOrient.mode = ORIENT_ZOOMIN;
            pose->speed = m_speed;
        }
    }
    //4. update project matrix and view matrix
    m_projectionMatrix = glm::perspective(glm::radians(m_vFOV), tanHalAspect, 0.01f, 100.0f);
    m_viewModelMatrix = glm::lookAt(
        //        position,           // Camera is here
        glm::vec3(position_x, position_y, 1),
        //        position+front, // and looks here : at the same position, plus "front"
        glm::vec3(position_x, position_y, 1) + front,
        up // Head is up (set to 0,-1,0 to look upside-down)
    );

    // 5. calculate zoom factor and x y
    m_zoomFactor = float(m_windowWidth) * float(m_windowHeight) / m_fullWidth / m_fullHeight / (tan(float(m_hFOV) / 2 / 180 * RENDER_PI) * tan(float(m_vFOV) / 2 / 180 * RENDER_PI));
    // x y is view point in picture
    pose->centerX = (position_x + 1) / 2 * m_fullWidth;
    pose->centerY = (-position_y + 1) / 2 * m_fullHeight;

    pose->zoomFactor = m_zoomFactor;

    if (!m_needMotionTest)
    {
        transfer.y = m_verticalAngle;
        transfer.x = m_horizontalAngle;
    }

    return RENDER_STATUS_OK;
}

void GLFWRenderContext::SetMaxSpeed()
{
    // set max speed that is 1 tiles unit pixels / second.
    float x_speed = 1 / (float)m_col;
    float y_speed = (float)m_fullHeight / m_fullWidth /  (float)m_row;
    m_speed = x_speed < y_speed ? x_speed : y_speed;
    if (m_projFormat == VCD::OMAF::PF_ERP || m_projFormat == VCD::OMAF::PF_CUBEMAP)
    {
        uint32_t slow_times = 5;
        m_speed /= slow_times;
    }
}

RenderStatus GLFWRenderContext::GetStatusAndPose(HeadPose *pose, uint32_t* status)
{
    if (m_projFormat == VCD::OMAF::PF_UNKNOWN)
    {
        LOG(INFO) << " projection format in render context is unknown!" << std::endl;
        return RENDER_ERROR;
    }
    else if (m_projFormat == VCD::OMAF::PF_ERP || m_projFormat == VCD::OMAF::PF_CUBEMAP)
    {
        return GetStatusAndPoseFor3D(pose, status);
    }
    else if (m_projFormat == VCD::OMAF::PF_PLANAR)
    {
        return GetStatusAndPoseFor2D(pose, status);
    }
    return RENDER_ERROR;
}

void* GLFWRenderContext::InitContext()
{
    // initialize glfw
    if (!glfwInit())
    {
        LOG(ERROR)<< "glfw failed to init" << std::endl;
        glfwTerminate();
        return NULL;
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
        return NULL;
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

    return m_window;
}

VCD_NS_END
#endif