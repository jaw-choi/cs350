#version 450 core

/* Start Header -------------------------------------------------------
Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: line.vert
Purpose: <This file contains layout,uniform value,and set gl position>
Language: <c++>
Platform: <Visual studio 2019, OpenGL 4.5, Window 64 bit>
Project: <jaewoo.choi_CS300_1>
Author: <Jaewoo Choi, jaewoo.choi, 55532>
Creation date: 14/09/2022
End Header --------------------------------------------------------*/

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 nrm;

layout (location=1) out vec3 NRM;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos; 

void main(void) 
{
    gl_Position = projection * view * model * vec4(pos, 1.0);

}

