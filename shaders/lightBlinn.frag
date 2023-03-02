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
    sampler2D diffuse;
    sampler2D specular;
    //sampler2D emissive;
    float shininess;

    vec3 ambient;
    //vec3 diffuse;
    //vec3 specular;
    vec3 emissive;
};

struct PointLight
{
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    int type;
    float innerAngle;
    float outerAngle;
    float falloff;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 color;

uniform int NUMBER_OF_POINT_LIGHTS;
uniform vec3 viewPos;
uniform PointLight pointLights[16];
uniform Material material;

uniform vec3 attenuation;
uniform vec3 globalAmbient;
uniform vec3 fogColor;
uniform float fogMin;
uniform float fogMax;


// Function prototypes
vec3 CalcDirLight( PointLight light, vec3 normal, vec3 viewDir );
vec3 CalcPointLight( PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir );
vec3 CalcSpotLight( PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir );

vec3 result={0.f,0.f,0.f};

void main( )
{
    // Properties
    vec3 norm = normalize( Normal );
    vec3 viewDir = normalize( vec3(0.f,3.f,10.f) - FragPos);

    float dist = abs(viewPos.z - FragPos.z);
    float fogFactor = (fogMax- dist) /(fogMax - fogMin );
    //fogFactor = clamp( fogFactor, 0.0, 1.0 );

    for ( int i = 0; i < NUMBER_OF_POINT_LIGHTS; i++ )
    {

        if(pointLights[i].type==0)
            result += CalcPointLight( pointLights[i], norm, FragPos, viewDir );
        if(pointLights[i].type==1)
            result += CalcDirLight( pointLights[i], norm, viewDir );
        if(pointLights[i].type==2)
            result += CalcSpotLight( pointLights[i], norm, FragPos, viewDir );
    }
    

    color = vec4( result , 1.0 );
    color = vec4(mix(fogColor, color.xyz, fogFactor),1);
    //color = vec4(texture(material.diffuse, TexCoords).rgb,1.0);
}

// Calculates the color when using a directional light.
vec3 CalcDirLight( PointLight light, vec3 normal, vec3 viewDir )
{
    vec3 lightDir = normalize( light.position - FragPos );
    
    // Diffuse shading
    float diff = max( dot( normal, lightDir ), 0.0 );
    
    // Specular shading
    vec3 halfDir = normalize(lightDir + viewDir);
    float spec = pow( max( dot( viewDir, halfDir ), 0.0 ), 2.f );
    
    // Combine results
    vec3 ambient = light.ambient * halfDir * vec3( texture( material.diffuse, TexCoords ).rgb );
    vec3 diffuse = light.diffuse * diff * vec3( texture( material.diffuse, TexCoords ).rgb );
    vec3 specular = light.specular * spec * vec3( texture( material.specular, TexCoords ).rgb );
    
    return ( ambient + diffuse + specular );
}

// Calculates the color when using a point light.
vec3 CalcPointLight( PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir )
{
    vec3 lightDir = normalize( light.position - fragPos );
    
    // Diffuse shading
    float diff = max( dot( normal, lightDir ), 0.0 );
    
    // Specular shading
    vec3 halfDir = normalize(lightDir + viewDir);
    float spec = pow( max( dot( viewDir, halfDir ), 0.0 ), 2.f );
    
    // Attenuation
    float distance = length( light.position - fragPos );
    float Attenuation = min(1.0f / ( attenuation.x + attenuation.y * distance + attenuation.z * ( distance * distance ) ),1.0);
    
    // Combine results
    vec3 ambient = light.ambient * vec3( texture( material.diffuse, TexCoords ).rgb );
    vec3 diffuse = light.diffuse * diff * vec3( texture( material.diffuse, TexCoords ).rgb );
    vec3 specular = light.specular * spec * vec3( texture( material.specular, TexCoords ).rgb );
    
    ambient *= Attenuation;
    diffuse *= Attenuation;
    specular *= Attenuation;
    

    return (ambient+ diffuse+specular);
}

// Calculates the color when using a spot light.
vec3 CalcSpotLight( PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir )
{
    vec3 lightDir = normalize( light.position - fragPos );
    vec3 D = normalize( light.position - fragPos );
    vec3 L = normalize( light.position);//-light.position
    // Diffuse shading
    float L_D = dot(L,D);

    float diff =  dot( normal, lightDir ) ;
    float cos_phi = cos(radians(light.outerAngle));
    float cos_theta = cos(radians(light.innerAngle));

    float a = L_D - cos_phi;
    float b = cos_theta - cos_phi;
    float spotLightEffect = 0;

    if(L_D < cos_phi)
    {
        spotLightEffect =0;
    }
    else if(L_D > cos_theta)
    {
        spotLightEffect = 1;
    }
    else
    {
        spotLightEffect = pow(a/b,light.falloff);
    }

    // Specular shading
    vec3 halfDir = normalize(lightDir + viewDir);
    float spec = pow(  dot( viewDir, halfDir ), 2.f );
    
    // Attenuation
    float distance = length( light.position - fragPos );
    float distanceToObj = length( light.position );
    float Attenuation = min(1.0f / ( attenuation.x + attenuation.y * distance + attenuation.z * ( distance * distance ) ),1.0);
    
    // Spotlight intensity
    float theta = dot( lightDir, normalize( -light.direction ) );
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp( ( theta - light.outerCutOff ) / epsilon, 0.0, 1.0 );
    
    // Combine results
    vec3 ambient = light.ambient * vec3( texture( material.diffuse, TexCoords ).rgb );
    vec3 diffuse = light.diffuse * diff * vec3( texture( material.diffuse, TexCoords ).rgb );
    vec3 specular = light.specular * spec * vec3( texture( material.specular, TexCoords ).rgb );
    
    
    vec3 result = material.emissive + globalAmbient * vec3( texture( material.diffuse, TexCoords ).rgb ) + Attenuation * spotLightEffect* (diffuse+specular + ambient);
    return result;

}
