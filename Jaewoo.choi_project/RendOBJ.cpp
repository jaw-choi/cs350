/* Start Header -------------------------------------------------------
Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: RendOBJ.cpp
Purpose: <This file contains the definitions of class RendOBJ that encapsulates the
functionality required to init and draw models>
Language: <c++>
Platform: <Visual studio 2019, OpenGL 4.5, Window 64 bit>
Project: <jaewoo.choi_CS350_1>
Author: <Jaewoo Choi, jaewoo.choi, 55532>
Creation date: 04/11/2022
End Header --------------------------------------------------------*/
#define _CRT_SECURE_NO_WARNINGS
#include "RendOBJ.h"
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include "Camera.h"



RendOBJ::RendOBJ() {}

void RendOBJ::init()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);


    shaderGeometryPass = Shader("../shaders/g_buffer.vert", "../shaders/g_buffer.frag");
    shaderLightingPass = Shader("../shaders/deferred_shading.vert", "../shaders/deferred_shading.frag");
    shaderLightBox = Shader("../shaders/deferred_light_box.vert", "../shaders/deferred_light_box.frag");


    part1_g0 = Model(("../object/part_a/g0.obj"));
    part1_g1 = Model(("../object/part_a/g1.obj"));
    part1_g2 = Model(("../object/part_a/g2.obj"));
    part2_g0 = Model(("../object/part_b/g0.obj"));
    part2_g1 = Model(("../object/part_b/g1.obj"));

    part1_g0_line = Model(("../object/part_a/g0.obj"), true);
    part1_g1_line = Model(("../object/part_a/g1.obj"), true);
    part1_g2_line = Model(("../object/part_a/g2.obj"), true);
    part2_g0_line = Model(("../object/part_b/g0.obj"), true);
    part2_g1_line = Model(("../object/part_b/g1.obj"), true);

    
    texture1.id = TextureFromFile("../textures/metal_roof_diff_512x512.png", "../textures", 0);
    texture1.path = "../textures/metal_roof_diff_512x512.png";
    texture1.type = "texture_diffuse";


    texture2.id = TextureFromFile("../textures/metal_roof_spec_512x512.png", "../textures", 0);
    texture2.path = "../textures/metal_roof_spec_512x512.png";
    texture2.type = "texture_specular";

    part1_g0.meshes[0].textures.push_back(texture1);
    part1_g0.meshes[0].textures.push_back(texture2);
    part1_g1.meshes[0].textures.push_back(texture1);
    part1_g1.meshes[0].textures.push_back(texture2);
    part1_g2.meshes[0].textures.push_back(texture1);
    part1_g2.meshes[0].textures.push_back(texture2);

    part2_g0.meshes[0].textures.push_back(texture1);
    part2_g0.meshes[0].textures.push_back(texture2);
    part2_g1.meshes[0].textures.push_back(texture1);
    part2_g1.meshes[0].textures.push_back(texture2);


    float theta = 0;
    float angle = TWO_PI / NR_LIGHTS;
    for (int i = 0; i < NR_LIGHTS; i++)
    {
        theta = angle * i;
        lampSetting.push_back(lampSet());
        dirlight.push_back(DirLight());

        dirlight[i].position = { 2 * cos(theta),0,2 * sin(theta) };
        lights[i].lightPositions = { 2 * cos(theta),0,2 * sin(theta) };
    }
    models.push_back(part1_g0);
    models.push_back(part1_g1);
    models.push_back(part1_g2);
    models.push_back(part2_g0);
    models.push_back(part2_g1);

    vector<BoundingBox> object_bboxes{
    BoundingBox(models[0].center,models[0].m),
    BoundingBox(models[1].center,models[1].m),
    BoundingBox(models[2].center,models[2].m),
    BoundingBox(models[3].center,models[3].m),
    BoundingBox(models[4].center,models[4].m),
    };
    vector<int> object_ids{ 0, 1, 2, 3, 4 };
    rootVolume = BVHNode::buildBVH(object_bboxes, object_ids,0);
    traverseBVH_pushbackAABB(rootVolume, BVHmeshes);
    traverseBVH_pushbackBsphere(rootVolume, BVHmeshesBSphere);
    rootDistance = BVHNode::buildBVH(object_bboxes, object_ids,1);
    traverseBVH_pushbackAABB(rootDistance, BVHmeshesDis);
    traverseBVH_pushbackBsphere(rootDistance, BVHmeshesBSphereDis);
    rootDeltaVolume = BVHNode::buildBVH(object_bboxes, object_ids, 2);
    traverseBVH_pushbackAABB(rootDeltaVolume, BVHmeshesDel);
    traverseBVH_pushbackBsphere(rootDeltaVolume, BVHmeshesBSphereDel);
    //BVHmeshes.push_back(CreateCube(root->bbox.center, root->bbox.m));

    float res10 = glm::length(MaxLength(part1_g0.m.min, part1_g0.m.max) - part1_g0_line.center);
    float res11 = glm::length(MaxLength(part1_g1.m.min, part1_g1.m.max) - part1_g1_line.center);
    float res12 = glm::length(MaxLength(part1_g2.m.min, part1_g2.m.max) - part1_g2_line.center);
    float res20 = glm::length(MaxLength(part2_g0.m.min, part2_g0.m.max) - part2_g0_line.center);
    float res21 = glm::length(MaxLength(part2_g1.m.min, part2_g1.m.max) - part2_g1_line.center);

    meshes.push_back(CreateSphere(60, 60, part1_g0_line.center, res10));//AABB
    meshes.push_back(CreateSphere(60, 60, part1_g0_line.center, part1_g0.CentroidRadius));//Centroid
    meshes.push_back(CreateSphere(60, 60, part1_g0_line.center, part1_g0.ritterRadius));//Ritter
    meshes.push_back(CreateSphere(60, 60, part1_g0_line.center, part1_g0.LarssonsRadius));//Larsson's
    meshes.push_back(CreateSphere(60, 60, part1_g0_line.center, part1_g0.PCARadius));//PCA

    meshes.push_back(CreateSphere(60, 60, part1_g1_line.center, res11));//AABB
    meshes.push_back(CreateSphere(60, 60, part1_g1_line.center, part1_g1.CentroidRadius));//Centroid
    meshes.push_back(CreateSphere(60, 60, part1_g1_line.center, part1_g1.ritterRadius));//Ritter
    meshes.push_back(CreateSphere(60, 60, part1_g1_line.center, part1_g1.LarssonsRadius));//Larsson's
    meshes.push_back(CreateSphere(60, 60, part1_g1_line.center, part1_g1.PCARadius));//PCA

    meshes.push_back(CreateSphere(60, 60, part1_g2_line.center, res12));//AABB
    meshes.push_back(CreateSphere(60, 60, part1_g2_line.center, part1_g2.CentroidRadius));//Centroid
    meshes.push_back(CreateSphere(60, 60, part1_g2_line.center, part1_g2.ritterRadius));//Ritter
    meshes.push_back(CreateSphere(60, 60, part1_g2_line.center, part1_g2.LarssonsRadius));//Larsson's
    meshes.push_back(CreateSphere(60, 60, part1_g2_line.center, part1_g2.PCARadius));//PCA

    meshes.push_back(CreateSphere(60, 60, part2_g0_line.center, res20));//AABB
    meshes.push_back(CreateSphere(60, 60, part2_g0_line.center, part2_g0.CentroidRadius));//Centroid
    meshes.push_back(CreateSphere(60, 60, part2_g0_line.center, part2_g0.ritterRadius));//Ritter
    meshes.push_back(CreateSphere(60, 60, part2_g0_line.center, part2_g0.LarssonsRadius));//Larsson's
    meshes.push_back(CreateSphere(60, 60, part2_g0_line.center, part2_g0.PCARadius));//PCA

    meshes.push_back(CreateSphere(60, 60, part2_g1_line.center, res21));//AABB
    meshes.push_back(CreateSphere(60, 60, part2_g1_line.center, part2_g1.CentroidRadius));//Centroid
    meshes.push_back(CreateSphere(60, 60, part2_g1_line.center, part2_g1.ritterRadius));//Ritter
    meshes.push_back(CreateSphere(60, 60, part2_g1_line.center, part2_g1.LarssonsRadius));//Larsson's
    meshes.push_back(CreateSphere(60, 60, part2_g1_line.center, part2_g1.PCARadius));//PCA

    ConfigureGbufferFramebuffer();



    srand(13);
    for (unsigned int i = 0; i < NR_LIGHTS; i++)
    {

        // calculate slightly random offsets
        //float xPos = static_cast<float>(((rand() % 100) / 100.0) * 6.0 - 3.0);
        //float yPos = static_cast<float>(((rand() % 100) / 100.0) * 6.0 - 4.0);
        //float zPos = static_cast<float>(((rand() % 100) / 100.0) * 6.0 - 3.0);
        //lights[i].lightPositions = (glm::vec3(xPos, yPos, zPos));
        // also calculate random color
        float rColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.)
        float gColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.)
        float bColor = static_cast<float>(((rand() % 100) / 200.0f) + 0.5); // between 0.5 and 1.)
        lights[i].lightColors = (glm::vec3(rColor, gColor, bColor));
        dirlight[i].ambient = { rColor,gColor,bColor };
        dirlight[i].diffuse = dirlight[i].ambient;
        dirlight[i].specular = dirlight[i].ambient;
        lampSetting[i].Diffuse = { dirlight[i].ambient ,1.0f };
    }

    shaderLightingPass.use();
    shaderLightingPass.setInt("gPosition", 0);
    shaderLightingPass.setInt("gNormal", 1);
    shaderLightingPass.setInt("diffuse_", 2);
    shaderLightingPass.setInt("specular_", 3);
    shaderLightingPass.setInt("gAmbient", 4);
    shaderLightingPass.setInt("gEmissive", 5);

    shaderGeometryPass.use();
    MatricesLOC = glGetUniformBlockIndex(shaderGeometryPass.ID, "Matrices");
    glUniformBlockBinding(shaderGeometryPass.ID, MatricesLOC, 0);//
    glGetActiveUniformBlockiv(shaderGeometryPass.ID, MatricesLOC, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);

    glGenBuffers(1, &quadUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, quadUBO);
    glBufferData(GL_UNIFORM_BUFFER, blockSize, NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, quadUBO);

    shaderLightingPass.use();
    LightLOC = glGetUniformBlockIndex(shaderLightingPass.ID, "Lights");
    glUniformBlockBinding(shaderLightingPass.ID, LightLOC, 0);//
    glGetActiveUniformBlockiv(shaderLightingPass.ID, LightLOC, GL_UNIFORM_BLOCK_DATA_SIZE, &light_blockSize);

    glGenBuffers(1, &lightUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, lightUBO);
    glBufferData(GL_UNIFORM_BUFFER, light_blockSize, NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, lightUBO);


    for (int i = 0; i < 5; i++)
    {
        meshes[0 + 5 * i].init("../shaders/PhongShading.vert", "../shaders/PhongShading.frag", { 0,0,0 }, { 1.f,1.f,1.f }, { 0,0,0 }, numberLamp);//AABB
        meshes[1 + 5 * i].init("../shaders/PhongShading.vert", "../shaders/PhongShading.frag", { 0,0,0 }, { 1.f,1.f,1.f }, { 0,0,0 }, numberLamp);//Centroid
        meshes[2 + 5 * i].init("../shaders/PhongShading.vert", "../shaders/PhongShading.frag", { 0,0,0 }, { 1.f,1.f,1.f }, { 0,0,0 }, numberLamp);//Ritter
        meshes[3 + 5 * i].init("../shaders/PhongShading.vert", "../shaders/PhongShading.frag", { 0,0,0 }, { 1.f,1.f,1.f }, { 0,0,0 }, numberLamp);////Larsson's
        meshes[4 + 5 * i].init("../shaders/PhongShading.vert", "../shaders/PhongShading.frag", { 0,0,0 }, { 1.f,1.f,1.f }, { 0,0,0 }, numberLamp);////PCA
    }
    for (int i = 0; i < BVHmeshes.size(); i++)
    {
        BVHmeshes[i].init("../shaders/PhongShading.vert", "../shaders/PhongShading.frag", { 0,0,0 }, { 1.f,1.f,1.f }, { 0,0,0 }, numberLamp);
    }
    for (int i = 0; i < BVHmeshesBSphere.size(); i++)
    {
        BVHmeshesBSphere[i].init("../shaders/PhongShading.vert", "../shaders/PhongShading.frag", { 0,0,0 }, { 1.f,1.f,1.f }, { 0,0,0 }, numberLamp);
    }
    for (int i = 0; i < BVHmeshesDis.size(); i++)
    {
        BVHmeshesDis[i].init("../shaders/PhongShading.vert", "../shaders/PhongShading.frag", { 0,0,0 }, { 1.f,1.f,1.f }, { 0,0,0 }, numberLamp);
    }
    for (int i = 0; i < BVHmeshesBSphereDis.size(); i++)
    {
        BVHmeshesBSphereDis[i].init("../shaders/PhongShading.vert", "../shaders/PhongShading.frag", { 0,0,0 }, { 1.f,1.f,1.f }, { 0,0,0 }, numberLamp);
    }
    for (int i = 0; i < BVHmeshesDel.size(); i++)
    {
        BVHmeshesDel[i].init("../shaders/PhongShading.vert", "../shaders/PhongShading.frag", { 0,0,0 }, { 1.f,1.f,1.f }, { 0,0,0 }, numberLamp);
    }
    for (int i = 0; i < BVHmeshesBSphereDel.size(); i++)
    {
        BVHmeshesBSphereDel[i].init("../shaders/PhongShading.vert", "../shaders/PhongShading.frag", { 0,0,0 }, { 1.f,1.f,1.f }, { 0,0,0 }, numberLamp);
    }
    camera = { {0.f, 40000.f, 80000.f} };

    SetView();

}

void RendOBJ::Update(float deltaTime)
{
    camera.Update(deltaTime);

}

void RendOBJ::Draw()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //Draw Bubby with UBO
    glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.f, 0.1f, 100000.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);

    /// <summary>
    /// send gAmbient value
    /// </summary>
    shaderGeometryPass.use();
    shaderGeometryPass.setVec3("UserAmbient", gambient);
    shaderGeometryPass.setVec3("UserEmissive", gEmissive);

    glBindBuffer(GL_UNIFORM_BUFFER, quadUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(1.f, 1.f, 1.f));
    //shaderGeometryPass.setMat4("model", model);
    glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(model));
    glBindBufferBase(GL_UNIFORM_BUFFER, MatricesLOC, quadUBO);


    part1_g0.Draw(shaderGeometryPass);
    part1_g1.Draw(shaderGeometryPass);
    part1_g2.Draw(shaderGeometryPass);
    part2_g0.Draw(shaderGeometryPass);
    part2_g1.Draw(shaderGeometryPass);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shaderLightingPass.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, diffuse_);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, specular_);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, gAmbient);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, gemissive);
    // send light relevant uniforms

    glBindBuffer(GL_UNIFORM_BUFFER, lightUBO);

    for (unsigned int i = 0; i < NR_LIGHTS; i++)
    {
        //TODO: Use UBO 
        const float maxBrightness = std::fmaxf(std::fmaxf(lights[i].lightColors.r, lights[i].lightColors.g), lights[i].lightColors.b);
        lights[i].radius = (-lights[i].linear + std::sqrt(lights[i].linear * lights[i].linear - 4 * lights[i].quadratic * (lights[i].constant - (256.0f / 5.0f) * maxBrightness))) / (2.0f * lights[i].quadratic);

        //glBufferSubData(GL_UNIFORM_BUFFER, i * 144, 144, &lights[i]);

        glBufferSubData(GL_UNIFORM_BUFFER, i * 144 + 0, 4, &lights[i].innerangle);
        glBufferSubData(GL_UNIFORM_BUFFER, i * 144 + 4, 4, &lights[i].outerangle);
        glBufferSubData(GL_UNIFORM_BUFFER, i * 144 + 8, 4, &lights[i].constant);
        glBufferSubData(GL_UNIFORM_BUFFER, i * 144 + 12, 4, &lights[i].linear);

        glBufferSubData(GL_UNIFORM_BUFFER, i * 144 + 16, 4, &lights[i].quadratic);
        glBufferSubData(GL_UNIFORM_BUFFER, i * 144 + 20, 4, &lights[i].cutOff);
        glBufferSubData(GL_UNIFORM_BUFFER, i * 144 + 24, 4, &lights[i].outerCutOff);
        glBufferSubData(GL_UNIFORM_BUFFER, i * 144 + 28, 4, &lights[i].falloff);

        glBufferSubData(GL_UNIFORM_BUFFER, i * 144 + 32, 8, &lights[i].radius);
        glBufferSubData(GL_UNIFORM_BUFFER, i * 144 + 40, 8, &lights[i].type);

        glBufferSubData(GL_UNIFORM_BUFFER, i * 144 + 48, 16, &lights[i].lightPositions);
        glBufferSubData(GL_UNIFORM_BUFFER, i * 144 + 64, 16, &lights[i].lightColors);
        glBufferSubData(GL_UNIFORM_BUFFER, i * 144 + 80, 16, &lights[i].direction);
        glBufferSubData(GL_UNIFORM_BUFFER, i * 144 + 96, 16, &lights[i].ambient);
        glBufferSubData(GL_UNIFORM_BUFFER, i * 144 + 112, 16, &lights[i].diffuse);
        glBufferSubData(GL_UNIFORM_BUFFER, i * 144 + 128, 16, &lights[i].specular);

    }
    glBindBufferBase(GL_UNIFORM_BUFFER, LightLOC, lightUBO);
    shaderLightingPass.setVec3("viewPos", camera.GetEye());
    shaderLightingPass.setVec3("emissive", emissive);
    shaderLightingPass.setVec3("globalAmbient", globalAmbient);
    shaderLightingPass.setInt("NR_LIGHTS", NR_LIGHTS);
    // finally render quad
    renderQuad();

    // 2.5. copy content of geometry's depth buffer to default framebuffer's depth buffer
        // ----------------------------------------------------------------------------------
    if (CopyDepthInfo)
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
        // blit to default framebuffer. Note that this may or may not work as the internal formats of both the FBO and default framebuffer have to match.
        // the internal formats are implementation defined. This works on all of my systems, but if it doesn't on yours you'll likely have to write to the 		
        // depth buffer in another shader stage (or somehow see to match the default framebuffer's internal format with the FBO's internal format).
        glBlitFramebuffer(0, 0, GLHelper::width, GLHelper::height, 0, 0, GLHelper::width, GLHelper::height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }


    // 3. render lights on top of scene
    // --------------------------------
    //TODO Draw others like Lamp sphere,Face & vertex normal line, plane,Orbit

    
    shaderGeometryPass.use();
    //shaderGeometryPass.setVec3("UserAmbient", gambient);
    shaderGeometryPass.setVec3("UserEmissive", gEmissive);

    glBindBuffer(GL_UNIFORM_BUFFER, quadUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(1.f, 1.f, 1.f));
    glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(model));
    glBindBufferBase(GL_UNIFORM_BUFFER, MatricesLOC, quadUBO);
    if (AABB_)
    {
        part1_g0_line.Draw(shaderGeometryPass, true);
        part1_g1_line.Draw(shaderGeometryPass, true);
        part1_g2_line.Draw(shaderGeometryPass, true);
        part2_g0_line.Draw(shaderGeometryPass, true);
        part2_g1_line.Draw(shaderGeometryPass, true);
    }

    if (sphere_)
    {
        if (TypeBV == 0)
            shaderGeometryPass.setVec3("UserEmissive", glm::vec3(0.2, 0.1, 0.1));
        else if (TypeBV == 1)
            shaderGeometryPass.setVec3("UserEmissive", glm::vec3(0.3, 0.3, 0.1));
        else if (TypeBV == 2)
            shaderGeometryPass.setVec3("UserEmissive", glm::vec3(0.2, 0.1, 0.3));
        else if (TypeBV == 3)
            shaderGeometryPass.setVec3("UserEmissive", glm::vec3(0.5, 0.2, 0.2));
        else if (TypeBV == 4)
            shaderGeometryPass.setVec3("UserEmissive", glm::vec3(0.8, 0.5, 0.1));

        meshes[TypeBV].drawSphere(shaderGeometryPass);//AABB
        meshes[5 + TypeBV].drawSphere(shaderGeometryPass);//Centroid
        meshes[10 + TypeBV].drawSphere(shaderGeometryPass);//Ritter
        meshes[15 + TypeBV].drawSphere(shaderGeometryPass);////Larsson's
        meshes[20 + TypeBV].drawSphere(shaderGeometryPass);////PCA
    }
    if(bvhbottomupAABB)
    for (int i = 0; i < BVHmeshes.size(); i++)
    {
        if (BVHmeshes[i].color == 0)
        {
            shaderGeometryPass.setVec3("UserEmissive", glm::vec3(0.8, 0.0, 0.0));
        }
        if (BVHmeshes[i].color == 1)
        {
            shaderGeometryPass.setVec3("UserEmissive", glm::vec3(1.0, 0.5, 0.0));
        }
        if (BVHmeshes[i].color == 2)
        {
            shaderGeometryPass.setVec3("UserEmissive", glm::vec3(0.0, 0.8, 0.0));
        }
        if (BVHmeshes[i].color == 3)
        {
            shaderGeometryPass.setVec3("UserEmissive", glm::vec3(0.0, 0.3, 0.8));
        }
            BVHmeshes[i].DrawLineBVH(shaderGeometryPass);
    }
    
    if (bvhbottomupBsphere)
        for (int i = 0; i < BVHmeshesBSphere.size(); i++)
        {
            if (BVHmeshesBSphere[i].color == 0)
            {
                shaderGeometryPass.setVec3("UserEmissive", glm::vec3(0.8, 0.0, 0.0));
            }
            if (BVHmeshesBSphere[i].color == 1)
            {
                shaderGeometryPass.setVec3("UserEmissive", glm::vec3(1.0, 0.5, 0.0));
            }
            if (BVHmeshesBSphere[i].color == 2)
            {
                shaderGeometryPass.setVec3("UserEmissive", glm::vec3(0.0, 0.8, 0.0));
            }
            if (BVHmeshesBSphere[i].color == 3)
            {
                shaderGeometryPass.setVec3("UserEmissive", glm::vec3(0.0, 0.3, 0.8));
            }
            
            BVHmeshesBSphere[i].drawSphere(shaderGeometryPass);
        }

    if (bvhbottomupAABBDis)
        for (int i = 0; i < BVHmeshesDis.size(); i++)
        {
            if (BVHmeshesDis[i].color == 0)
            {
                shaderGeometryPass.setVec3("UserEmissive", glm::vec3(0.8, 0.0, 0.0));
            }
            if (BVHmeshesDis[i].color == 1)
            {
                shaderGeometryPass.setVec3("UserEmissive", glm::vec3(1.0, 0.5, 0.0));
            }
            if (BVHmeshesDis[i].color == 2)
            {
                shaderGeometryPass.setVec3("UserEmissive", glm::vec3(0.0, 0.8, 0.0));
            }
            if (BVHmeshesDis[i].color == 3)
            {
                shaderGeometryPass.setVec3("UserEmissive", glm::vec3(0.0, 0.3, 0.8));
            }
            BVHmeshesDis[i].DrawLineBVH(shaderGeometryPass);
        }

    if (bvhbottomupBsphereDis)
        for (int i = 0; i < BVHmeshesBSphereDis.size(); i++)
        {
            if (BVHmeshesBSphereDis[i].color == 0)
            {
                shaderGeometryPass.setVec3("UserEmissive", glm::vec3(0.8, 0.0, 0.0));
            }
            if (BVHmeshesBSphereDis[i].color == 1)
            {
                shaderGeometryPass.setVec3("UserEmissive", glm::vec3(1.0, 0.5, 0.0));
            }
            if (BVHmeshesBSphereDis[i].color == 2)
            {
                shaderGeometryPass.setVec3("UserEmissive", glm::vec3(0.0, 0.8, 0.0));
            }
            if (BVHmeshesBSphereDis[i].color == 3)
            {
                shaderGeometryPass.setVec3("UserEmissive", glm::vec3(0.0, 0.3, 0.8));
            }

            BVHmeshesBSphereDis[i].drawSphere(shaderGeometryPass);
        }

    if (bvhbottomupAABBDel)
        for (int i = 0; i < BVHmeshesDel.size(); i++)
        {
            if (BVHmeshesDel[i].color == 0)
            {
                shaderGeometryPass.setVec3("UserEmissive", glm::vec3(0.8, 0.0, 0.0));
            }
            if (BVHmeshesDel[i].color == 1)
            {
                shaderGeometryPass.setVec3("UserEmissive", glm::vec3(1.0, 0.5, 0.0));
            }
            if (BVHmeshesDel[i].color == 2)
            {
                shaderGeometryPass.setVec3("UserEmissive", glm::vec3(0.0, 0.8, 0.0));
            }
            if (BVHmeshesDel[i].color == 3)
            {
                shaderGeometryPass.setVec3("UserEmissive", glm::vec3(0.0, 0.3, 0.8));
            }
            BVHmeshesDel[i].DrawLineBVH(shaderGeometryPass);
        }

    if (bvhbottomupBsphereDel)
        for (int i = 0; i < BVHmeshesBSphereDel.size(); i++)
        {
            if (BVHmeshesBSphereDel[i].color == 0)
            {
                shaderGeometryPass.setVec3("UserEmissive", glm::vec3(0.8, 0.0, 0.0));
            }
            if (BVHmeshesBSphereDel[i].color == 1)
            {
                shaderGeometryPass.setVec3("UserEmissive", glm::vec3(1.0, 0.5, 0.0));
            }
            if (BVHmeshesBSphereDel[i].color == 2)
            {
                shaderGeometryPass.setVec3("UserEmissive", glm::vec3(0.0, 0.8, 0.0));
            }
            if (BVHmeshesBSphereDel[i].color == 3)
            {
                shaderGeometryPass.setVec3("UserEmissive", glm::vec3(0.0, 0.3, 0.8));
            }

            BVHmeshesBSphereDel[i].drawSphere(shaderGeometryPass);
        }
    //


    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("ObjectManager");
    /*const char* ModelItem[5] = { "bunny_high_poly","cube2", "4Sphere", "lucy_princeton","sphere" };
    static const char* model_current_item = ModelItem[0];
    if (ImGui::BeginCombo("Model", model_current_item))
    {
        for (int n = 0; n < 5; n++)
        {
            bool is_selected = (model_current_item == ModelItem[n]);
            if (ImGui::Selectable(ModelItem[n], is_selected))
            {
                for (int i = 0; i < 5; i++)
                {
                    modelSwitch[i] = false;
                }
                modelSwitch[n] = true;
                model_current_item = ModelItem[n];
            }
            if (is_selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }*/


    //ObjectManager();
    ImGui::Checkbox("Sphere", &sphere_);
    ImGui::Checkbox("AABB", &AABB_);
    //if (ImGui::Button("reload shaders"))
    //{
    //    ////////////////////////////////////////////////
    //    init();
    //}

    if (ImGui::Button("AABB"))
    {
        TypeBV = 0;
    }
    if (ImGui::Button("Centroid"))
    {
        TypeBV = 1;
    }
    if (ImGui::Button("Ritter"))
    {
        TypeBV = 2;
    }
    if (ImGui::Button("Larsson"))
    {
        TypeBV = 3;
    }
    if (ImGui::Button("PCA"))
    {
        TypeBV = 4;
    }
    

    ImGui::Begin("Bottom-Up");
    if (ImGui::Button("Bottom-Up BVH AABB with Volume"))
    {
        bvhbottomupAABB = !bvhbottomupAABB;
    }
    if (ImGui::Button("Bottom-Up BVH Bsphere with Volume"))
    {
        bvhbottomupBsphere = !bvhbottomupBsphere;
    }
    if (ImGui::Button("Bottom-Up BVH AABB with Distance"))
    {
        bvhbottomupAABBDis = !bvhbottomupAABBDis;
    }
    if (ImGui::Button("Bottom-Up BVH Bsphere with Distance"))
    {
        bvhbottomupBsphereDis = !bvhbottomupBsphereDis;
    }
    if (ImGui::Button("Bottom-Up BVH AABB with minimum relative increase in volume"))
    {
        bvhbottomupAABBDel = !bvhbottomupAABBDel;
    }
    if (ImGui::Button("Bottom-Up BVH Bsphere with minimum relative increase in volume"))
    {
        bvhbottomupBsphereDel = !bvhbottomupBsphereDel;
    }

    ImGui::Begin("Top-down split point");
    if (ImGui::Button("Top-down BVH AABB with Median of BV centers"))
    {
        bvhbottomupAABB = !bvhbottomupAABB;
    }
    if (ImGui::Button("Top-down BVH Bsphere with Median of BV centers"))
    {
        bvhbottomupBsphere = !bvhbottomupBsphere;
    }
    if (ImGui::Button("Top-down BVH AABB with Median of BV extents"))
    {
        bvhbottomupAABBDis = !bvhbottomupAABBDis;
    }
    if (ImGui::Button("Top-down BVH Bsphere with Median of BV extents"))
    {
        bvhbottomupBsphereDis = !bvhbottomupBsphereDis;
    }
    if (ImGui::Button("Top-down BVH AABB with K-even splits in one axis"))
    {
        bvhbottomupAABBDel = !bvhbottomupAABBDel;
    }
    if (ImGui::Button("Top-down BVH Bsphere with K-even splits in one axis"))
    {
        bvhbottomupBsphereDel = !bvhbottomupBsphereDel;
    }

    ImGui::Begin("Top-down split plane");
    if (ImGui::Button("Top-down BVH AABB with Median of BV centers"))
    {
        bvhbottomupAABB = !bvhbottomupAABB;
    }
    if (ImGui::Button("Top-down BVH Bsphere with Median of BV centers"))
    {
        bvhbottomupBsphere = !bvhbottomupBsphere;
    }
    if (ImGui::Button("Top-down BVH AABB with Median of BV extents"))
    {
        bvhbottomupAABBDis = !bvhbottomupAABBDis;
    }
    if (ImGui::Button("Top-down BVH Bsphere with Median of BV extents"))
    {
        bvhbottomupBsphereDis = !bvhbottomupBsphereDis;
    }
    if (ImGui::Button("Top-down BVH AABB with K-even splits in one axis"))
    {
        bvhbottomupAABBDel = !bvhbottomupAABBDel;
    }
    if (ImGui::Button("Top-down BVH Bsphere with K-even splits in one axis"))
    {
        bvhbottomupBsphereDel = !bvhbottomupBsphereDel;
    }
    ImGui::Begin("Controls");
    OnImGuiRender();


    ImGui::Begin("Debug View");
    ImGui::Image((ImTextureID)gPosition, ImVec2(100, 100), ImVec2(0, 1), ImVec2(1, 0));
    ImGui::SameLine();
    ImGui::Image((ImTextureID)gNormal, ImVec2(100, 100), ImVec2(0, 1), ImVec2(1, 0));
    ImGui::SameLine();
    ImGui::Image((ImTextureID)diffuse_, ImVec2(100, 100), ImVec2(0, 1), ImVec2(1, 0));
    ImGui::SameLine();
    ImGui::Image((ImTextureID)specular_, ImVec2(100, 100), ImVec2(0, 1), ImVec2(1, 0));

    ImGui::Image((ImTextureID)gAmbient, ImVec2(100, 100), ImVec2(0, 1), ImVec2(1, 0));
    ImGui::SameLine();
    ImGui::Image((ImTextureID)gemissive, ImVec2(100, 100), ImVec2(0, 1), ImVec2(1, 0));
    //DrawBeforeImGui();


    //meshes[8].drawLight({1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1}, projection, light, camera.GetEye(), dirlight, numberLamp, global, mate, typeMapping, shaderType, phongShadingID, phongLightID, blinnID);


}

void RendOBJ::ObjectManager()
{
    ImGui::Checkbox("Vertex Normal", &vtxNrm);
    ImGui::Checkbox("Face Normal", &facNrm);
    if (vtxNrm)
    {
        meshes[1].drawLine(useNormal, view, projection, light, camera.GetEye(), { 1,0 });
    }
    else if (facNrm)
    {
        meshes[1].drawFaceLine(useNormal, view, projection, light, camera.GetEye(), { 1,0 });
    }
}

void RendOBJ::OnImGuiRender()
{
    ImGui::Checkbox("CopyDepthInfo", &CopyDepthInfo);

    const char* Lightitems[16] = { "Light#1","Light#2", "Light#3", "Light#4",
    "Light#5","Light#6", "Light#7", "Light#8",
    "Light#9","Light#10", "Light#11", "Light#12",
    "Light#13","Light#14", "Light#15", "Light#16", };
    static const char* light_current_item = Lightitems[0];

    const char* LightType[3] = { "Point","Direction", "Spot" };
    static const char* type_current_item = LightType[0];

    //if (ImGui::CollapsingHeader("Light"))
    //{

    //    ImGui::SliderInt("Light Number", &NR_LIGHTS, 1, 16, "%d", 0);
    //    NR_LIGHTS = NR_LIGHTS;

    //    ImGui::Checkbox("Pause Rotation", &isRotationLamp);
    //    if (ImGui::Button("Scenario 1(Point Light)")) {
    //        int j = 0;
    //        for (int i = 0; i < 16; i++)
    //        {
    //            lightSwitch[i] = false;
    //        }
    //        for (int i = 0; i < 6; i++)
    //        {
    //            lightSwitch[i] = true;
    //            dirlight[i].type = 0;
    //            dirlight[i].ambient = { 1.f,1.f,1.f };
    //            dirlight[i].diffuse = { 1.f,1.f,1.f };
    //            dirlight[i].specular = { 1.f,1.f,1.f };
    //            lampSetting[i].Diffuse = { 1.f,1.f,1.f,1.f };

    //            lightSwitch[i] = true;
    //            lights[i].type = 0;
    //            lights[i].ambient = { 1.f,1.f,1.f };
    //            lights[i].diffuse = { 1.f,1.f,1.f };
    //            lights[i].specular = { 1.f,1.f,1.f };
    //            //lampSetting[i].Diffuse = { 1.f,1.f,1.f,1.f };
    //            j++;
    //        }
    //        type_current_item = LightType[0];
    //        global.attenuation.z = 2.f;
    //        NR_LIGHTS = 6;
    //    }
    //    if (ImGui::Button("Scenario 2(Dir Light)")) {
    //        int j = 0;
    //        float a = 1.f / 255.f;
    //        for (int i = 0; i < 16; i++)
    //        {
    //            lightSwitch[i] = false;
    //        }
    //        for (int i = 0; i < 7; i++)
    //        {
    //            lightSwitch[i] = true;
    //            lights[i].type = 1;
    //            j++;
    //        }
    //        lights[0].ambient = { 59 * a,234 * a,21 * a };
    //        lights[0].diffuse = lights[0].ambient;
    //        lights[0].specular = lights[0].ambient;
    //        lampSetting[0].Diffuse = { lights[0].ambient ,1.0f };
    //        lights[1].ambient = { 22 * a,89 * a,248 * a };
    //        lights[1].diffuse = lights[1].ambient;
    //        lights[1].specular = lights[1].ambient;
    //        lampSetting[1].Diffuse = { lights[1].ambient ,1.0f };
    //        lights[2].ambient = { 244 * a,15 * a,15 * a };
    //        lights[2].diffuse = lights[2].ambient;
    //        lights[2].specular = lights[2].ambient;
    //        lampSetting[2].Diffuse = { lights[2].ambient ,1.0f };
    //        lights[3].ambient = { 204 * a,28 * a,225 * a };
    //        lights[3].diffuse = lights[3].ambient;
    //        lights[3].specular = lights[3].ambient;
    //        lampSetting[3].Diffuse = { lights[3].ambient ,1.0f };
    //        lights[4].ambient = { 29 * a,227 * a,179 * a };
    //        lights[4].diffuse = lights[4].ambient;
    //        lights[4].specular = lights[4].ambient;
    //        lampSetting[4].Diffuse = { lights[4].ambient ,1.0f };
    //        lights[5].ambient = { 226 * a,215 * a,28 * a };
    //        lights[5].diffuse = lights[5].ambient;
    //        lights[5].specular = lights[5].ambient;
    //        lampSetting[5].Diffuse = { lights[5].ambient ,1.0f };

    //        lights[6].ambient = { 226 * a,184 * a,28 * a };
    //        lights[6].diffuse = lights[6].ambient;
    //        lights[6].specular = lights[6].ambient;
    //        lampSetting[6].Diffuse = { lights[6].ambient ,1.0f };
    //        global.attenuation.z = 0.45f;
    //        type_current_item = LightType[2];
    //        NR_LIGHTS = j;


    //    }
    //    if (ImGui::Button("Scenario 3(Point Light)")) {
    //        int j = 0;
    //        float a = 1.f / 255.f;
    //        for (int i = 0; i < 16; i++)
    //        {
    //            lightSwitch[i] = false;
    //        }
    //        for (int i = 0; i < 11; i++)
    //        {
    //            lightSwitch[i] = true;
    //            lights[i].type = 0;
    //            j++;
    //        }
    //        lights[0].ambient = { 59 * a,234 * a,21 * a };
    //        lights[0].diffuse = lights[0].ambient;
    //        lights[0].specular = lights[0].ambient;
    //        lampSetting[0].Diffuse = { lights[0].ambient ,1.0f };

    //        lights[1].ambient = { 22 * a,89 * a,248 * a };
    //        lights[1].diffuse = lights[1].ambient;
    //        lights[1].specular = lights[1].ambient;
    //        lampSetting[1].Diffuse = { lights[1].ambient ,1.0f };

    //        lights[2].ambient = { 244 * a,15 * a,15 * a };
    //        lights[2].diffuse = lights[2].ambient;
    //        lights[2].specular = lights[2].ambient;
    //        lampSetting[2].Diffuse = { lights[2].ambient ,1.0f };

    //        lights[3].ambient = { 204 * a,28 * a,225 * a };
    //        lights[3].diffuse = lights[3].ambient;
    //        lights[3].specular = lights[3].ambient;
    //        lampSetting[3].Diffuse = { lights[3].ambient ,1.0f };

    //        lights[4].ambient = { 29 * a,227 * a,179 * a };
    //        lights[4].diffuse = lights[4].ambient;
    //        lights[4].specular = lights[4].ambient;
    //        lampSetting[4].Diffuse = { lights[4].ambient ,1.0f };

    //        lights[5].ambient = { 226 * a,215 * a,28 * a };
    //        lights[5].diffuse = lights[5].ambient;
    //        lights[5].specular = lights[5].ambient;
    //        lampSetting[5].Diffuse = { lights[5].ambient ,1.0f };

    //        lights[6].ambient = { 226 * a,184 * a,28 * a };
    //        lights[6].diffuse = lights[6].ambient;
    //        lights[6].specular = lights[6].ambient;
    //        lampSetting[6].Diffuse = { lights[6].ambient ,1.0f };

    //        lights[7].ambient = { 59 * a,234 * a,21 * a };
    //        lights[7].diffuse = lights[7].ambient;
    //        lights[7].specular = lights[7].ambient;
    //        lampSetting[7].Diffuse = { lights[7].ambient ,1.0f };

    //        lights[8].ambient = { 22 * a,89 * a,248 * a };
    //        lights[8].diffuse = lights[8].ambient;
    //        lights[8].specular = lights[8].ambient;
    //        lampSetting[1].Diffuse = { lights[8].ambient ,1.0f };

    //        lights[9].ambient = { 244 * a,15 * a,15 * a };
    //        lights[9].diffuse = lights[9].ambient;
    //        lights[9].specular = lights[9].ambient;
    //        lampSetting[9].Diffuse = { lights[9].ambient ,1.0f };

    //        lights[10].ambient = { 204 * a,28 * a,225 * a };
    //        lights[10].diffuse = lights[10].ambient;
    //        lights[10].specular = lights[10].ambient;
    //        lampSetting[10].Diffuse = { lights[10].ambient ,1.0f };

    //        lights[11].ambient = { 29 * a,227 * a,179 * a };
    //        lights[11].diffuse = lights[11].ambient;
    //        lights[11].specular = lights[11].ambient;
    //        lampSetting[11].Diffuse = { lights[11].ambient ,1.0f };
    //        global.attenuation.z = 2.f;
    //        type_current_item = LightType[0];
    //        NR_LIGHTS = j;
    //    }
    //    //if (ImGui::BeginCombo("Light Count", light_current_item)) // The second parameter is the label previewed before opening the combo.
    //    //{
    //    //    for (int n = 0; n < 16; n++)
    //    //    {
    //    //        bool is_selected = (light_current_item == Lightitems[n]);
    //    //        if (ImGui::Selectable(Lightitems[n], is_selected))
    //    //        {
    //    //            for (int i = 0; i < 16; i++)
    //    //            {
    //    //                lightSwitch[i] = false;
    //    //            }
    //    //            lightSwitch[n] = true;
    //    //            light_current_item = Lightitems[n];
    //    //            currentLamp = n;
    //    //        }
    //    //        if (is_selected)
    //    //        {
    //    //            ImGui::SetItemDefaultFocus();
    //    //        }
    //    //    }
    //    //    ImGui::EndCombo();
    //    //}
    //    //if (ImGui::BeginCombo("Light type", type_current_item)) // The second parameter is the label previewed before opening the combo.
    //    //{
    //    //    for (int n = 0; n < 3; n++)
    //    //    {
    //    //        bool is_selected = (lights[currentLamp].type == n);
    //    //        if (ImGui::Selectable(LightType[n], is_selected))
    //    //        {
    //    //            lights[currentLamp].type = n;
    //    //            type_current_item = LightType[n];
    //    //        }
    //    //        if (is_selected)
    //    //        {
    //    //            ImGui::SetItemDefaultFocus();
    //    //        }
    //    //    }
    //    //    ImGui::EndCombo();
    //    //}

    //    ImGui::ColorEdit3("Ambient", &lights[currentLamp].ambient.x);
    //    ImGui::ColorEdit3("diffuse", &lights[currentLamp].diffuse.x);
    //    lampSetting[currentLamp].Diffuse.x = lights[currentLamp].diffuse.x;
    //    lampSetting[currentLamp].Diffuse.y = lights[currentLamp].diffuse.y;
    //    lampSetting[currentLamp].Diffuse.z = lights[currentLamp].diffuse.z;
    //    ImGui::ColorEdit3("Specular", &lights[currentLamp].specular.x);
    //    //if (type_current_item == LightType[2])
    //    //{
    //    //    ImGui::SliderFloat("Inner Angle", &lights[currentLamp].innerangle, 0.f, 90.f);
    //    //    ImGui::SliderFloat("Outer Angle", &lights[currentLamp].outerangle, 0.f, 90.f);
    //    //    if (lights[currentLamp].innerangle > lights[currentLamp].outerangle)
    //    //        lights[currentLamp].innerangle = lights[currentLamp].outerangle;
    //    //    ImGui::SliderFloat("Falloff", &lights[currentLamp].falloff, 0.f, 10.f);
    //    //}
    //}


    ImGui::Button("Camera can move with 'w','a','s','d','up','down'");
    ImGui::Button("Camera can move with 'up','down'");
}

void RendOBJ::UnLoad()
{
    meshes.clear();

}


void RendOBJ::SetView()
{
    view = {
    1,0,0,0,
    0,1,0,0,
    0,0,1,0,
    0,0,0,1
    };
    projection = {
    1,0,0,0,
    0,1,0,0,
    0,0,1,0,
    0,0,0,1
    };
    //eye = { 0.f, 0.f, 10.0f };
    light = { 0.0f, 0.0f, 0.0f };

    //view = glm::rotate(view, PI, glm::vec3(0.0f, 1.0f, 0.0f));

    //view = glm::rotate(view, PI, glm::vec3(1.0f, 0.0f, 0.0f));
    view = glm::translate(view, camera.eye);
    projection = glm::perspective(glm::radians(90.0f), 1.f, 0.1f, 1000.0f);

}

void RendOBJ::DrawBeforeImGui()
{

    const char* items[] = { "sphere", "cube2", "bunny_high_poly", "4Sphere", "bunny" };
    static const char* current_item = items[0];


    if (ImGui::CollapsingHeader("Model"))
    {
        if (ImGui::BeginCombo("Loaded Model", current_item)) // The second parameter is the label previewed before opening the combo.
        {
            for (int n = 0; n < 5; n++)
            {
                bool is_selected = (current_item == items[n]);
                if (ImGui::Selectable(items[n], is_selected))
                {
                    meshSwitch[0] = false;
                    meshSwitch[1] = false;
                    meshSwitch[2] = false;
                    meshSwitch[3] = false;
                    meshSwitch[4] = false;
                    meshSwitch[n] = true;
                    current_item = items[n];
                }
                if (is_selected)
                {
                    vertxNrm[0] = false;
                    vertxNrm[1] = false;
                    vertxNrm[2] = false;
                    vertxNrm[3] = false;
                    vertxNrm[4] = false;
                    faceNrm[0] = false;
                    faceNrm[1] = false;
                    faceNrm[2] = false;
                    faceNrm[3] = false;
                    faceNrm[4] = false;
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
    }
    //if (meshSwitch[0])
    //    meshes[1].draw(camera.GetViewMatrix(), projection, light, camera.GetEye(), dirlight, numberLamp, global, mate, cubemapTexture, reflection, refraction, indexFresnel,RefractiveIndex, FresnelConstant, FresnelPower);
    //else if (meshSwitch[1])
    //    meshes[3].draw(camera.GetViewMatrix(), projection, light, camera.GetEye(), dirlight, numberLamp, global, mate, cubemapTexture, reflection, refraction, indexFresnel, RefractiveIndex, FresnelConstant, FresnelPower);
    //else if (meshSwitch[2])
    //    meshes[4].draw(camera.GetViewMatrix(), projection, light, camera.GetEye(), dirlight, numberLamp, global, mate, cubemapTexture, reflection, refraction, indexFresnel, RefractiveIndex, FresnelConstant, FresnelPower);
    //else if (meshSwitch[3])
    //    meshes[5].draw(camera.GetViewMatrix(), projection, light, camera.GetEye(), dirlight, numberLamp, global, mate, cubemapTexture, reflection, refraction, indexFresnel, RefractiveIndex, FresnelConstant, FresnelPower);
    //else if (meshSwitch[4])
    //    meshes[6].draw(camera.GetViewMatrix(), projection, light, camera.GetEye(), dirlight, numberLamp, global, mate, cubemapTexture, reflection, refraction, indexFresnel, RefractiveIndex, FresnelConstant, FresnelPower);

    const char* shade_items[] = { "phongShading", "blinnShading" };
    static const char* shade_current_item = shade_items[0];
    if (ImGui::CollapsingHeader("Shader"))
    {
        if (ImGui::BeginCombo("Current Shader", shade_current_item)) // The second parameter is the label previewed before opening the combo.
        {
            for (int n = 0; n < 2; n++)
            {
                bool is_selected = (shade_current_item == shade_items[n]);
                if (ImGui::Selectable(shade_items[n], is_selected))
                {
                    shaderType = n;
                    shade_current_item = shade_items[n];
                }
                if (is_selected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        if (ImGui::Button("reload shaders"))
        {
            ////////////////////////////////////////////////
            init();
        }
    }
    const char* items_material[] = { "Air", "Hydrogen", "water", "Olive Oil", "Ice","Quartz", "Diamond", "Acrylic", "PlexiglasOil", "Lucite" };
    static const char* current_item_material = items_material[0];
    if (ImGui::CollapsingHeader("Material"))
    {
        ImGui::Checkbox("Visualize Reflection", &reflection);
        ImGui::Checkbox("Visualize Refraction", &refraction);

        if (ImGui::BeginCombo("Loaded Model", current_item_material)) // The second parameter is the label previewed before opening the combo.
        {
            for (int n = 0; n < 10; n++)
            {
                bool is_selected = (current_item_material == items_material[n]);
                if (ImGui::Selectable(items_material[n], is_selected))
                {
                    materialSwitch[0] = false;
                    materialSwitch[1] = false;
                    materialSwitch[2] = false;
                    materialSwitch[3] = false;
                    materialSwitch[4] = false;
                    materialSwitch[5] = false;
                    materialSwitch[6] = false;
                    materialSwitch[7] = false;
                    materialSwitch[8] = false;
                    materialSwitch[9] = false;
                    materialSwitch[n] = true;
                    current_item_material = items_material[n];
                }
            }
            ImGui::EndCombo();
        }
        for (int i = 0; i < 10; i++)
        {
            if (materialSwitch[i] == true)
            {
                indexFresnel = i;
                ImGui::SliderFloat("Refractive Index", &RefractiveIndex[i], 1.f, 100.f, "%.4f", 0);
                ImGui::SliderFloat("FresnelConstant", &FresnelConstant[i], 0.01f, 1.f, "%.4f", 0);
            }
        }
        ImGui::SliderFloat("Fresnel Power", &FresnelPower, 0.f, 1.f, "%.6f", 0);
    }

    if (ImGui::CollapsingHeader("Global Constant"))
    {
        ImGui::SliderFloat3("Attenuation", &global.attenuation.x, 0.f, 3.f);
        ImGui::ColorEdit3("Global Ambient", &global.ambient.x);
        ImGui::ColorEdit3("Fog Color", &global.fogColor.x);
        ImGui::SliderFloat("Fog Min", &global.fogMin, 0.1f, 10.f);
        ImGui::SliderFloat("Fog Max", &global.fogMax, 0.2f, 40.1f);

    }
    if (global.fogMin > global.fogMax)
        global.fogMin = global.fogMax;
}

void RendOBJ::renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void RendOBJ::renderCube()
{

    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void RendOBJ::ConfigureGbufferFramebuffer()
{
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

    // position color buffer
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, GLHelper::width, GLHelper::height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

    // normal color buffer
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, GLHelper::width, GLHelper::height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

    // diffuse
    glGenTextures(1, &diffuse_);
    glBindTexture(GL_TEXTURE_2D, diffuse_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, GLHelper::width, GLHelper::height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, diffuse_, 0);

    glGenTextures(1, &specular_);
    glBindTexture(GL_TEXTURE_2D, specular_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, GLHelper::width, GLHelper::height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, specular_, 0);

    glGenTextures(1, &gAmbient);
    glBindTexture(GL_TEXTURE_2D, gAmbient);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, GLHelper::width, GLHelper::height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, gAmbient, 0);

    glGenTextures(1, &gemissive);
    glBindTexture(GL_TEXTURE_2D, gemissive);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, GLHelper::width, GLHelper::height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT5, GL_TEXTURE_2D, gemissive, 0);

    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
    unsigned int attachments[6] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3,GL_COLOR_ATTACHMENT4,GL_COLOR_ATTACHMENT5 };
    glDrawBuffers(6, attachments);
    // create and attach depth buffer (renderbuffer)
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
        GL_FRAMEBUFFER_COMPLETE)
    {
        printf("gbuffer is incomplete!");
        return;
    }
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, GLHelper::width, GLHelper::height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RendOBJ::traverseBVH_pushbackAABB(const BVHNode* node, std::vector<Mesh>& meshes)
{
    if (node == nullptr) {
        return;
    }
    meshes.push_back(CreateCube(node->bbox.center, node->bbox.m, node->getlevel()));
    traverseBVH_pushbackAABB(node->left_child, meshes);
    traverseBVH_pushbackAABB(node->right_child, meshes);
}

void RendOBJ::traverseBVH_pushbackBsphere(const BVHNode* node, std::vector<Mesh>& meshes)
{
    if (node == nullptr) {
        return;
    }
    meshes.push_back(CreateSphere(60,60,node->bbox.center, glm::length(node->bbox.m.max - node->bbox.center), node->getlevel()));
    traverseBVH_pushbackBsphere(node->left_child, meshes);
    traverseBVH_pushbackBsphere(node->right_child, meshes);
}

glm::vec3 RendOBJ::MaxLength(glm::vec3 min, glm::vec3 max)
{
    glm::vec3 res{};
    res.x = sqrt(min.x) > sqrt(max.x) ? min.x : max.x;
    res.y = sqrt(min.y) > sqrt(max.y) ? min.y : max.y;
    res.z = sqrt(min.z) > sqrt(max.z) ? min.z : max.z;
    return res;
}
