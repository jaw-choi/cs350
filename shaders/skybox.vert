#version 450 core

/* Start Header -------------------------------------------------------
Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: PhongShading.vert
Purpose: <This file contains layout,uniform value,and set gl position with normal vector>
Language: <c++>
Platform: <Visual studio 2019, OpenGL 4.5, Window 64 bit>
Project: <jaewoo.choi_CS300_2>
Author: <Jaewoo Choi, jaewoo.choi, 55532>
Creation date: 04/11/2022
End Header --------------------------------------------------------*/

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 nrm;
layout (location = 2) in vec2 uv;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;
out vec3 mpos;
layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
    mat4 model;
};
uniform int typeMapping;

void main()
{
    mpos = pos;
    vec4 _pos = projection * view  * vec4(pos, 1.0f);
    gl_Position = _pos.xyww;
    FragPos = vec3(model * vec4(pos, 1.0f));
    Normal = mat3(transpose(inverse(model))) * nrm;
    
}