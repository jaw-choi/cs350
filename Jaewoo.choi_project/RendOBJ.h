/* Start Header -------------------------------------------------------
Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: RendOBJ.h
Purpose: <This file contains the declaration of class RendOBJ that encapsulates the
functionality required to init and draw models>
Language: <c++>
Platform: <Visual studio 2019, OpenGL 4.5, Window 64 bit>
Project: <jaewoo.choi_CS350_1>
Author: <Jaewoo Choi, jaewoo.choi, 55532>
Creation date: 04/11/2022
End Header --------------------------------------------------------*/
#pragma once

#include "model.h"

class Shader;

class RendOBJ
{
public:
    RendOBJ();
    ~RendOBJ();
    void init();
    void Update(float deltaTime);
    void Draw();
    void OnImGuiRender();
    void UnLoad();

    void SetView();
    void DrawBeforeImGui();
    void ObjectManager();

    void renderQuad();
    void renderCube();
    void ConfigureGbufferFramebuffer();
    std::vector<Mesh> meshes;
    GLuint FBO;
    GLuint gBuffer;
    GLuint rboDepth;
    GLuint gPosition, gNormal, diffuse_,specular_,gAmbient, gemissive;

    //const unsigned int NR_LIGHTS = 16;
    struct Light
    {
        float innerangle = 15.f;
        float outerangle = 45.f;
        float constant = 1.0f;
        float linear = 0.7f;
        //16
        float quadratic = 1.8f;
        float cutOff = 0;
        float outerCutOff = 0;
        float falloff = 1.f;
        //32
        float radius = 0;
        //40
        int type = 0;
        //48
        glm::vec3 lightPositions;
        //64
        glm::vec3 lightColors;
        //80
        glm::vec3 direction = { 1.f,1.f,1.f };
        //96
        glm::vec3 ambient = { 0.05f, 0.05f, 0.05f };
        //112
        glm::vec3 diffuse = { 0.8f, 0.8f, 0.8f };
        //128
        glm::vec3 specular = { 1.0f, 1.0f, 1.0f };
        //144

    };
    int NR_LIGHTS = 16;
    Light lights[16];

    





    glm::vec3 emissive = glm::vec3(0, 0, 0.1);
    glm::vec3 globalAmbient = glm::vec3(0, 0, 0.1);
    glm::vec3 gambient = { 0.20f, 0.1f, 0.05f };
    glm::vec3 gEmissive = { 0.01f, 0.01f, 0.01f };

    Shader shaderGeometryPass;
    Shader shaderLightingPass;
    Shader shaderLightBox;

    Model bunny;
    Model cube;
    Model sphere4;
    Model lucy_princeton;
    Model sphere;
    Model quad;

    std::vector<glm::vec3> objectPositions;

    GLuint quadVAO = 0;
    GLuint quadVBO;
    GLuint quadUBO;
    GLuint lightUBO;
    GLuint cubeVAO = 0;
    GLuint cubeVBO = 0;
    GLint blockSize;
    GLint light_blockSize;
    GLint MatricesLOC = 0;
    GLint LightLOC = 0;










    const GLfloat bgColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

    Vec4 useNormal = Vec4(0.9f, 0.9f, 0.9f, 1.0f);
    int numberLamp = 16;
    GLuint phongLightID = 0;
    GLuint phongShadingID = 0;
    GLuint blinnID = 0;

private:
    Camera camera;
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec3 eye;
    glm::vec3 light;
    bool vertxNrm[5] = { false,false,false,false,false };
    bool faceNrm[5] = { false,false,false,false,false };
    bool meshSwitch[5] = { true,false,false,false,false };
    bool materialSwitch[10] = { true,false,false,false,false,false,false,false,false,false };

    bool vtxNrm = false;
    bool facNrm = false;
    bool none = true;
    int indexFresnel = 0;
    std::vector<float> RefractiveIndex;
    std::vector<float> FresnelConstant;
    float FresnelPower = 0.5;
    bool shaderSwitch[3] = { true,false,false };
    bool lightSwitch[16] = { false,};
    std::vector<DirLight> dirlight;
    bool isRotationLamp = false;
    int currentLamp = 0;
    int typeMapping = 1;
    std::vector<lampSet> lampSetting;
    Global global{};
    Material mate{};
    bool reflection = true;
    bool refraction = true;
    int shaderType = 0;
    std::vector<GLuint> cubemapTexture;
    std::vector<std::string> faces
    {
            "../textures/right.jpg",
            "../textures/left.jpg",
            "../textures/top.jpg",
            "../textures/bottom.jpg",
            "../textures/front.jpg",
            "../textures/back.jpg"
    };

    bool CopyDepthInfo = true;
    Texture texture1;
    Texture texture2;
    bool modelSwitch[5] = { true,false,false,false,false, };
    //{ "bunny_high_poly","cube2", "4Sphere", "lucy_princeton","sphere"};
    //    1                 3         4            5                6
};


