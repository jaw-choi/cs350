/* Start Header -------------------------------------------------------
Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: deffered_shading.frag
Purpose: <This file contains deffered shader info and lighting with deffered shading>
Language: <c++>
Platform: <Visual studio 2019, OpenGL 4.5, Window 64 bit>
Project: <jaewoo.choi_CS350_1>
Author: <Jaewoo Choi, jaewoo.choi, 55532>
Creation date: 14/09/2022
End Header --------------------------------------------------------*/
#version 450 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D diffuse_;
uniform sampler2D specular_;
uniform sampler2D gAmbient;
uniform sampler2D gEmissive;

struct Light {
    float innerAngle;
    float outerAngle;
    float Constant;
    float Linear;

    float Quadratic;
    float cutOff;
    float outerCutOff;
    float falloff;
    
    float Radius;
    int type;

    vec3 position;
    vec3 Color;
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    



};

//uniform Light lights[NR_LIGHTS];
uniform vec3 viewPos;
uniform vec3 emissive;
uniform vec3 globalAmbient;
uniform int NR_LIGHTS;
float shiness = 16.f;

layout (std140) uniform Lights
{
   Light lights[16];
};


// Function prototypes
vec3 CalcDirLight( Light light, vec3 normal, vec3 viewDir );
vec3 CalcPointLight( Light light, vec3 normal, vec3 fragPos, vec3 viewDir );
vec3 CalcSpotLight( Light light, vec3 normal, vec3 fragPos, vec3 viewDir );



// retrieve data from gbuffer
vec3 FragPos = texture(gPosition, TexCoords).rgb;
vec3 Normal = texture(gNormal, TexCoords).rgb;
vec3 Diffuse = texture(diffuse_, TexCoords).rgb;
vec3 Specular = texture(specular_, TexCoords).rgb;
vec3 Ambient = texture(gAmbient, TexCoords).rgb;
vec3 Emissive = texture(gEmissive, TexCoords).rgb;
/*
void main()
{         
    // then calculate lighting as usual
    vec3 lighting  = Diffuse * 0.1; // hard-coded ambient component
    vec3 viewDir  = normalize(viewPos - FragPos);
    for(int i = 0; i < NR_LIGHTS; ++i)
    {
        // calculate distance between light source and current fragment
        float distance = length(lights[i].position - FragPos);
        if(distance < lights[i].Radius)
        {
            // diffuse
            vec3 lightDir = normalize(lights[i].position - FragPos);
            vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * lights[i].Color;
            // specular
            vec3 halfwayDir = normalize(lightDir + viewDir);  
            float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
            vec3 specular = lights[i].Color * spec * Specular;
            // attenuation
            float attenuation = 1.0 / (1.0 + lights[i].Linear * distance + lights[i].Quadratic * distance * distance);
            diffuse *= attenuation;
            specular *= attenuation;
            lighting += diffuse + specular;
        }
    }    
    FragColor = vec4(lighting, 1.0);
}
*/
void main()
{             
    // retrieve data from gbuffer

    vec3 result={0.f,0.f,0.f};
    // then calculate lighting as usual
    vec3 lighting  = Diffuse * Ambient; // init hard-coded ambient component
    vec3 viewDir  = normalize(viewPos - FragPos);
    //result += CalcDirLight( lights[0], Normal, viewDir );
    for(int i = 0; i < NR_LIGHTS; ++i)
    {
        lighting  = Diffuse * lights[i].ambient;
        // calculate distance between light source and current fragment
        float distance = length(lights[i].position - FragPos);
        float distanceToObj = length( lights[i].position );
        //result += CalcDirLight( lights[i], Normal, viewDir );
        if(lights[i].type==0)
            result += CalcPointLight( lights[i], Normal, FragPos, viewDir );
        if(lights[i].type==1)
            result += CalcDirLight( lights[i], Normal, viewDir );
        //if(lights[i].type==2)
            //result += CalcSpotLight( lights[i], Normal, FragPos, viewDir ); 
    }
    
    FragColor = vec4(result, 1.0);
}
// Calculates the color when using a directional light.
vec3 CalcDirLight(Light light, vec3 normal, vec3 viewDir )
{
    vec3 lightDir = normalize( light.position - FragPos );
    
    // Diffuse shading
    float diff = max( dot( normal, lightDir ), 0.0 );
    
    // Specular shading
    vec3 reflectDir = normalize(2.f*(dot(normal,lightDir)*normal -lightDir));
    float spec = pow( max( dot( viewDir, reflectDir ), 0.0 ), 8.f );
    
    // Combine results
    vec3 ambient = light.ambient * reflectDir * Ambient;
    vec3 diffuse = light.diffuse * diff * Diffuse;
    vec3 specular = light.specular * spec * Specular;
    
    return (Emissive+ ambient + diffuse + specular );
}

// Calculates the color when using a point light.
vec3 CalcPointLight( Light light, vec3 normal, vec3 fragPos, vec3 viewDir )
{
    vec3 lightDir = normalize( light.position - fragPos );
    
    // Diffuse shading
    float diff = max( dot( normal, lightDir ), 0.0 );
    
    // Specular shading
    vec3 reflectDir = normalize(2.f*(dot(normal,lightDir)*normal -lightDir));
    float spec = pow( max( dot( viewDir, reflectDir ), 0.0 ), 8.f );
    
    // Attenuation
    float distance = length( light.position - fragPos );
    float Attenuation = min(1.0f / ( light.Constant + light.Linear * distance + light.Quadratic * ( distance * distance ) ),1.0);
    
    // Combine results
    vec3 ambient = light.ambient * Ambient;
    vec3 diffuse = light.diffuse * diff * Diffuse;
    vec3 specular = light.specular * spec * Specular;
    
    ambient *= Attenuation;
    diffuse *= Attenuation;
    specular *= Attenuation;
    
    return (Emissive+ambient+ diffuse+specular);
}

// Calculates the color when using a spot light.
vec3 CalcSpotLight( Light light, vec3 normal, vec3 fragPos, vec3 viewDir )
{
    vec3 lightDir = normalize( light.position - fragPos );
    vec3 lightDirToObj = normalize( -light.position);//-light.position
    // Diffuse shading
    float diff =  dot( normal, lightDir );
    
    // Specular shading
    vec3 reflectDir = normalize(2.f*(dot(normal,lightDir)*normal -lightDir));
    float spec = pow(  dot( viewDir, reflectDir ), 32.f );
    
    // Attenuation
    float distance = length( light.position - fragPos );
    float distanceToObj = length( light.position );
    float Attenuation = min(1.0f / ( light.Constant + light.Linear * distance + light.Quadratic * ( distance * distance ) ),1.0);
    
    // Spotlight intensity
    float theta = dot( lightDir, normalize( -light.direction ) );
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp( ( theta - light.outerCutOff ) / epsilon, 0.0, 1.0 );
    
    // Combine results
    vec3 ambient = light.ambient * Diffuse;
    vec3 diffuse = light.diffuse * diff * Diffuse;
    vec3 specular = light.specular * spec * Specular;
    
    ambient *= Attenuation * intensity;
    diffuse *= Attenuation * intensity;
    specular *= Attenuation * intensity;
    
    float LDAngle = dot((light.position - fragPos),light.position)/(distanceToObj*distance);
    float a = LDAngle-cos(radians(light.outerAngle));
    float b = cos(radians(light.innerAngle))-cos(radians(light.outerAngle));
    float spotLightEffect = pow(a/b,light.falloff);

    vec3 result = emissive + globalAmbient * Diffuse + Attenuation * ambient + Attenuation * spotLightEffect* (diffuse+specular);
    return result;

}
