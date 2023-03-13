/* Start Header -------------------------------------------------------
Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: g_buffer.frag
Purpose: <This file contains gBuffer shader that draw value with debug draw>
Language: <c++>
Platform: <Visual studio 2019, OpenGL 4.5, Window 64 bit>
Project: <jaewoo.choi_CS350_1>
Author: <Jaewoo Choi, jaewoo.choi, 55532>
Creation date: 14/09/2022
End Header --------------------------------------------------------*/
#version 450 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gDiffuse;
layout (location = 3) out vec3 gSpecular;
layout (location = 4) out vec3 gAmbient;
layout (location = 5) out vec3 gEmissive;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture_diffuse;
uniform sampler2D texture_specular;

uniform vec3 UserAmbient;
uniform vec3 UserEmissive;

void main()
{    
    // store the fragment position vector in the first gbuffer texture
    gPosition = UserEmissive;
    // also store the per-fragment normals into the gbuffer
    gNormal = normalize(Normal);
    // and the diffuse per-fragment color
    gDiffuse.rgb = texture(texture_diffuse, TexCoords).rgb;
    // store specular intensity in gAlbedoSpec's alpha component
    gSpecular.rgb = texture(texture_specular, TexCoords).rgb;
    gAmbient = UserAmbient;
    gEmissive = vec3(0.08,0.01,0.03);
}
/*
TODO: change to UVs and depthValue
#version 430
in vec3 fragPos;
in vec3 fragNormal;
in vec3 fragUV;
// Writing the output data into our GBuffer
layout(location = 0) out vec3 vPos;
layout(location = 1) out vec3 normal;
layout(location = 2) out vec3 UVs;
layout(location = 3) out vec3 depthValue;
void main()
{
    vPos = fragPos;
    normal = normalize(fragNormal);
    UVs = fragUV;
    float distFromCamera = length(fragPos);
    depthValue = vec3(distFromCamera);
}
*/