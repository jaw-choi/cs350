/* Start Header -------------------------------------------------------
Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Camera.cpp
Purpose: <This file contains definitions of member functions of class Camera.
Decide position and view of camera with camera vector>
Language: <c++>
Platform: <Visual studio 2019, OpenGL 4.5, Window 64bit>
Project: <jaewoo.choi_CS350_1>
Author: <Jaewoo Choi, jaewoo.choi, 55532>
Creation date: 25/09/2022
End Header --------------------------------------------------------*/
#include "Camera.h"
#include"glhelper.h"
#include "glm/gtc/matrix_transform.hpp"
#include "math.h"
Camera::Camera(glm::vec3 eye) :eye(eye), pitch(0), yaw(-90.0f), angle(0)
{
    cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    cameraDirection = glm::vec3(eye - cameraTarget);

    cameraRight = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), cameraDirection));
    cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    cameraUp = glm::cross(cameraDirection, cameraRight);
    view = glm::translate(view, eye);
}
Camera::Camera(glm::vec3 eye, glm::vec3 _direction) :eye(eye), cameraDirection(_direction), pitch(0), yaw(-90.0f), angle(0)
{
    cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    cameraRight = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), _direction));
    cameraUp = glm::cross(_direction, cameraRight);
}
void Camera::Update(float )
{
    float speed = 0.01f;

    if (glfwGetKey(GLHelper::ptr_window, GLFW_KEY_UP))
    {
        eye.y += speed ;
    }
    if (glfwGetKey(GLHelper::ptr_window, GLFW_KEY_DOWN))
    {
        eye.y -= speed ;
    }
    if (glfwGetKey(GLHelper::ptr_window, GLFW_KEY_W))
    {
        eye += speed *  cameraFront;
    }
    if (glfwGetKey(GLHelper::ptr_window, GLFW_KEY_S))
    {
        eye -= speed *  cameraFront;
    }
    if (glfwGetKey(GLHelper::ptr_window, GLFW_KEY_A))
    {
        eye -= glm::normalize(glm::cross(cameraFront, cameraUp)) * speed ;
    }
    if (glfwGetKey(GLHelper::ptr_window, GLFW_KEY_D))
    {
        eye += glm::normalize(glm::cross(cameraFront, cameraUp)) * speed ;
    }
    if (glfwGetKey(GLHelper::ptr_window, GLFW_KEY_E))
    {
        yaw += speed *10.f;
    }
    if (glfwGetKey(GLHelper::ptr_window, GLFW_KEY_Q))
    {
        yaw -= speed *10.f;
    }
    cameraFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront.y = sin(glm::radians(pitch));
    cameraFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    
    cameraFront = glm::normalize(cameraFront);
    cameraRight = glm::normalize(glm::cross(cameraFront, glm::vec3(0,1,0) ));
    cameraUp = glm::normalize(glm::cross(cameraRight, cameraFront));
    view = glm::lookAt(eye, eye + cameraFront, cameraUp);
   // view = glm::rotate(view, angle_, {0,1,0});
}
glm::vec3 Camera::mouse_update()
{
    double mouse_pos_x = 0;
    double mouse_pos_y = 0;
    static glm::vec2 start_pos{ 0 };
    static bool mouse_start = false;
    glfwGetCursorPos(GLHelper::ptr_window, &mouse_pos_x, &mouse_pos_y);

    glm::mat4 rotate = {
    1,0,0,0,
    0,1,0,0,
    0,0,1,0,
    0,0,0,1
    };
    if (GLHelper::mouse_pressed == true)
    {
        if (mouse_start == false)
        {
            start_pos.x = static_cast<float>(mouse_pos_x);
            start_pos.y = static_cast<float>(mouse_pos_y);
            mouse_start = true;
        }
        float xoffset = static_cast<float>(mouse_pos_x - start_pos.x);
        float yoffset = static_cast<float>(start_pos.y - mouse_pos_y);
        start_pos.x = static_cast<float>(mouse_pos_x);
        start_pos.y = static_cast<float>(mouse_pos_y);

        yaw += xoffset * 0.07f;
        pitch += yoffset * 0.07f;
    }
    else
    {
        mouse_start = false;
    }

    camDirection= cameraDirection;
    camDirection.x += cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    camDirection.y += sin(glm::radians(pitch));
    camDirection.z += sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(camDirection);
    cameraRight = glm::normalize(glm::cross(camDirection, cameraUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    cameraUp = glm::normalize(glm::cross(cameraRight, camDirection));
    return (camDirection);
}
glm::mat4& Camera::GetViewMatrix()
{
    return view;
}

glm::vec3 Camera::GetEye()
{
    return eye;
}

glm::vec3 Camera::GetFront()
{
    return cameraFront;
}

float Camera::GetAngle()
{
    return angle_;
}
