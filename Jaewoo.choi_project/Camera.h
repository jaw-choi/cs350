/* Start Header -------------------------------------------------------
Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Camera.h
Purpose: <This file contains declaration of class Camera.>
Language: <c++>
Platform: <Visual studio 2019, OpenGL 4.5, Window 64bit>
Project: <jaewoo.choi_CS350_1>
Author: <Jaewoo Choi, jaewoo.choi, 55532>
Creation date: 25/09/2022
End Header --------------------------------------------------------*/
#pragma once
#include <GL/glew.h> 
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
class Camera
{
public:
    Camera() = default;
    Camera(glm::vec3 eye);
    Camera(glm::vec3 eye, glm::vec3 _direction);
    void Update(float );
    glm::vec3 mouse_update();
    glm::mat4& GetViewMatrix();
    glm::vec3 GetEye();
    glm::vec3 GetFront();
    float GetAngle();

    glm::mat4      view = {
    1,0,0,0,
    0,1,0,0,
    0,0,1,0,
    0,0,0,1
    };
    glm::vec3 eye{ 0 }; //camera position
    glm::vec3 cameraTarget{ 0 }; //view target
    glm::vec3 cameraDirection{ 0 }; //view direction
    glm::vec3 cameraUp{ 0 };
    glm::vec3 cameraRight{ 0 };
    glm::vec3 cameraFront{ 0 };
    glm::vec3 camDirection{ 0 };
    glm::vec2 direction{ 0 };
    float pitch{ 0 };
    float yaw{ 0 };
    glm::vec3 angle{ 0 };
    float angle_{ 0 };


};