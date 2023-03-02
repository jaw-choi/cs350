#version 450 core

/* Start Header -------------------------------------------------------
Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: triangle.frag
Purpose: <This file contains phong lighting, and uniform values>
Language: <c++>
Platform: <Visual studio 2019, OpenGL 4.5, Window 64 bit>
Project: <jaewoo.choi_CS300_1>
Author: <Jaewoo Choi, jaewoo.choi, 55532>
Creation date: 14/09/2022
End Header --------------------------------------------------------*/

layout(location=1) in vec3 NRM;


in vec3 FragPos; 
uniform vec4 color;
uniform vec2 triline;
float type = triline.x;
out vec4 outColor;

vec3 objectColor = {color.x,color.y,color.z};
uniform vec3 lightPos;
uniform vec3 viewPos;
vec3 norm = normalize(NRM);

vec3 lightDir = normalize(lightPos - FragPos);  
vec3 lightColor = vec3(1.0, 1.0, 1.0);
float diff = max(dot(norm, lightDir), 0.0);
vec3 diffuse = diff * lightColor;
vec3 viewDir = normalize(viewPos - FragPos);
vec3 reflectDir = reflect(-lightDir,norm);

vec3 halfwayDir = normalize(lightDir + viewDir);

float specularStrength = 0.5;
float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
vec3 specular = specularStrength * spec * lightColor; 

void main(void)
{
	if(type<0.5)
	{
	    float ambientStrength = 0.1;
    	vec3 ambient = ambientStrength * lightColor;

    	vec3 result = (ambient + diffuse + specular) * objectColor;
    	outColor = vec4(result, 1.0);
	}
	else
		outColor = vec4(1.f,1.f,1.f, 1.0);


}