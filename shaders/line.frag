#version 450 core

/* Start Header -------------------------------------------------------
Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: line.frag
Purpose: <This file contains frament shader that draw all white>
Language: <c++>
Platform: <Visual studio 2019, OpenGL 4.5, Window 64 bit>
Project: <jaewoo.choi_CS300_2>
Author: <Jaewoo Choi, jaewoo.choi, 55532>
Creation date: 14/09/2022
End Header --------------------------------------------------------*/

layout(location=1) in vec3 NRM;

in vec3 FragPos; 
uniform vec4 color;
out vec4 outColor;

//Light calculate
uniform vec3 lightPos;
uniform vec3 viewPos;
vec3 norm = normalize(NRM);
vec3 lightDir = normalize(lightPos - FragPos);  
vec3 lightColor = vec3(0.3, 0.5, 0.8);
float diff = max(dot(norm, lightDir), 0.0);
vec3 viewDir = normalize(viewPos - FragPos);


void main(void)
{
			//outColor =  color * vec4(lightColor,1.0);
			//outColor = diff * outColor ;
			outColor = vec4(1.0,1.0,1.0,1.0);

}