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

vec2 TexCoords;
int DirTex;

out vec4 color;

uniform int NUMBER_OF_POINT_LIGHTS;
uniform vec3 viewPos;
uniform Material material;

uniform vec3 attenuation;
uniform vec3 globalAmbient;
uniform vec3 fogColor;
uniform float fogMin;
uniform float fogMax;

uniform float Reflection_bool;
uniform float Refraction_bool;

uniform float refractiveIndex;
uniform float fresnelConstant;
uniform float FresnelPower;

float Eta = refractiveIndex;
float F = ((1.0-Eta) * (1.0-Eta)) / ((1.0+Eta) * (1.0+Eta));



vec2 calculateReflect(vec3 vector);

vec3 result={0.f,0.f,0.f};
bool isMix = false;
void main( )
{
    vec4 color1 = vec4(1.f);
    vec4 color2 = vec4(1.f);
    vec3 I = normalize(viewPos - FragPos);
    vec3 N = normalize(Normal);
    float Ratio = F + (1.0 - F) * pow((1.0 - dot(-I, N)), FresnelPower);
    
    vec3 Reflect = 2.0f * N * dot(I, N) - I;
    vec3 Refract = (Ratio * dot(N,I) - sqrt(1-(Ratio*Ratio)*(1-(dot(N,I)*dot(N,I)))))*N - (Ratio*I);
    
    //float ratio = 1.f;
    //vec3 I = normalize(viewPos - FragPos);
    //vec3 N = normalize(Normal);
    //vec3 Reflect = reflect(normalize(FragPos - viewPos),N);
    //vec3 Refract_ = (ratio * dot(N,I) - sqrt(1-(ratio*ratio)*(1-(dot(N,I)*dot(N,I)))))*N - (ratio*I);
    //vec3 Refract_ = refract(normalize(FragPos - viewPos),N,ratio);

    //vec3 refractColor = vec3(texture(Cubemap, Refract));
    //vec3 reflectColor = vec3(texture(Cubemap, Reflect));
    //vec3 mixcolor = mix(refractColor, reflectColor, Ratio);

    TexCoords = vec2(0.f);

    if(Reflection_bool==1.f&&Refraction_bool==1.f)
    {
        isMix = true;
    }
    else if(Reflection_bool==1.f&&Refraction_bool==0.f)
    {
        TexCoords = calculateReflect(Reflect);
        isMix = false;
    }
    else if(Reflection_bool==0.f&&Refraction_bool==1.f)
    {
        TexCoords = calculateReflect(Refract);
        isMix = false;
    }
    else if(Reflection_bool==0.f&&Refraction_bool==0.f)
    {
        color = vec4(0.5,0.5,0.5,1);
    }
    if(isMix==true)
{
    TexCoords = calculateReflect(Reflect);
        if(DirTex==1)
        color1 = vec4(texture( material.right , TexCoords ).rgb,1.0);
    else if(DirTex==2)
        color1 = vec4(texture( material.left , TexCoords ).rgb,1.0);
    else if(DirTex==3)
        color1 = vec4(texture( material.bottom , TexCoords ).rgb,1.0);
    else if(DirTex==4)
        color1 = vec4(texture( material.top, TexCoords ).rgb,1.0);
    else if(DirTex==5)
        color1 = vec4(texture( material.back , TexCoords ).rgb,1.0);
    else if(DirTex==6)
        color1 = vec4(texture( material.front , TexCoords ).rgb,1.0);

    TexCoords = calculateReflect(Refract);
    if(DirTex==1)
        color2 = vec4(texture( material.right , TexCoords ).rgb,1.0);
    else if(DirTex==2)
        color2 = vec4(texture( material.left , TexCoords ).rgb,1.0);
    else if(DirTex==3)
        color2 = vec4(texture( material.bottom , TexCoords ).rgb,1.0);
    else if(DirTex==4)
        color2 = vec4(texture( material.top, TexCoords ).rgb,1.0);
    else if(DirTex==5)
        color2 = vec4(texture( material.back , TexCoords ).rgb,1.0);
    else if(DirTex==6)
        color2 = vec4(texture( material.front , TexCoords ).rgb,1.0);

    color = mix(color2, color1, Ratio);
}
else
{
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



}
vec2 calculateReflect(vec3 vector)
{
        vec2 UV=vec2(0,0);
        vec3 absVector = abs(vector);
        //+-X
        if(absVector.x >= absVector.y && absVector.x >= absVector.z)
        {
            if(vector.x > 0.0f)
            {
                DirTex=1;
                UV.x = vector.z/absVector.x;
            }
            else
            {
                DirTex=2;
                UV.x = -vector.z/absVector.x;
            }
            //pos.y
            UV.y = vector.y/absVector.x;
        }
        //+-Y
        else if(absVector.y >= absVector.x && absVector.y >= absVector.z)
        {
            if(vector.y < 0.0f)
            {
                DirTex=3;
                UV.y = vector.z/absVector.y;
            }
            else
            {
                DirTex=4;
                UV.y = -vector.z/absVector.y;
            }

            UV.x = -vector.x/absVector.y;
        }
        //+-Z
        else if(absVector.z >= absVector.x && absVector.z >= absVector.y)
        {
            if(vector.z < 0.0f){
                DirTex=6;
                UV.x = vector.x/absVector.z;
            }
            else{
                DirTex=5;
                UV.x = -vector.x/absVector.z;
            }

            UV.y = vector.y/absVector.z;
        }
        
        return (UV+vec2(1))/2.0;
}