/* Start Header -------------------------------------------------------
Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: Engine.cpp
Purpose: <This file contains definitions of member functions of class Engine.
Manage whole demo and start the window>
Language: <c++>
Platform: <Visual studio 2019, OpenGL 4.5, Window 64bit>
Project: <jaewoo.choi_CS350_1>
Author: <Jaewoo Choi, jaewoo.choi, 55532>
Creation date: 04/11/2022
End Header --------------------------------------------------------*/
#include "Engine.h"
#include "glhelper.h"
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include "RendOBJ.h"
Engine::Engine()
{
    obj = new RendOBJ();
    if (!GLHelper::init(1600, 1000, "Jaewoo.choi_Project"))
    {
	std::cout << "Unable to create OpenGL context" << std::endl;
	std::exit(EXIT_FAILURE);
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(GLHelper::ptr_window, true);
    const char* glsl_version = "#version 450";
    ImGui_ImplOpenGL3_Init(glsl_version);
    ImGui::StyleColorsDark();
    
}

Engine::~Engine()
{
    GLHelper::cleanup();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

}

void Engine::init()
{
    obj->init();
}

void Engine::Update()
{
    glfwPollEvents();
    obj->Update((float)glfwGetTime());

}

void Engine::Draw()
{

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);



    obj->Draw();


    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(GLHelper::ptr_window);
}

bool Engine::ShouldClose()
{
    return glfwWindowShouldClose(GLHelper::ptr_window);
}
