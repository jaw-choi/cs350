#version 450 core

/* Start Header -------------------------------------------------------
Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: PhongShading.frag
Purpose: <This file contains phong shading, and uniform values>
Language: <c++>
Platform: <Visual studio 2019, OpenGL 4.5, Window 64 bit>
Project: <jaewoo.choi_CS300_2>
Author: <Jaewoo Choi, jaewoo.choi, 55532>
Creation date: 04/11/2022
End Header --------------------------------------------------------*/

struct Material
{
    sampler2D right;
    sampler2D left;
    sampler2D top;
    sampler2D bottom;
    sampler2D front;
    sampler2D back;
};

in vec3 FragPos;
in vec3 Normal;
vec2 TexCoords=vec2(0);
float DirTex=0;
in vec3 mpos;
out vec4 color;
uniform Material material;

vec3 result={0.f,0.f,0.f};

void main( )
{
	vec2 UV=vec2(0,0);
        vec3 absVector = abs(mpos);
        //+-X
        if(absVector.x >= absVector.y && absVector.x >= absVector.z)
        {
            if(mpos.x < 0.0f)
            {
                DirTex=1;
                UV.x = mpos.z/absVector.x;
            }
            else
            {
                DirTex=2;
                UV.x = -mpos.z/absVector.x;
            }
            //pos.y
            UV.y = mpos.y/absVector.x;
        }
        //+-Y
        if(absVector.y >= absVector.x && absVector.y >= absVector.z)
        {
            if(mpos.y < 0.0f)
            {
                DirTex=3;
                UV.y = mpos.z/absVector.y;
            }
            else
            {
                DirTex=4;
                UV.y = -mpos.z/absVector.y;
            }

            UV.x = mpos.x/absVector.y;
        }
        //+-Z
        if(absVector.z >= absVector.x && absVector.z >= absVector.y)
        {
            if(mpos.z < 0.0f){
                DirTex=5;
                UV.x = -mpos.x/absVector.z;
            }
            else{
                DirTex=6;
                UV.x = mpos.x/absVector.z;
            }

            UV.y = mpos.y/absVector.z;
        }
        UV.x = -UV.x;
        TexCoords = (UV+vec2(1))/2.0;

    if(DirTex==1)
        color = vec4(texture( material.right , TexCoords ).rgb,1.0);
    else if(DirTex==2)
        color = vec4(texture( material.left , TexCoords ).rgb,1.0);
    else if(DirTex==3)
        color = vec4(texture( material.bottom , TexCoords ).rgb,1.0);
    else if(DirTex==4)
        color = vec4(texture( material.top, TexCoords ).rgb,1.0);
    else if(DirTex==5)
        color = vec4(texture( material.back , TexCoords ).rgb,1.0);
    else if(DirTex==6)
        color = vec4(texture( material.front , TexCoords ).rgb,1.0);

}


