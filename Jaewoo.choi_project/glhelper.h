/* !
@file    glhelper.h
@author  pghali@digipen.edu
@date    10/11/2016

This file contains the declaration of namespace Helper that encapsulates the
functionality required to create an OpenGL context using GLFW; use GLEW
to load OpenGL extensions; initialize OpenGL state; and finally initialize
the OpenGL application by calling initalization functions associated with
objects participating in the application.

*//*__________________________________________________________________________*/

/* Start Header -------------------------------------------------------
Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
File Name: glhelper.h
Purpose: <This file contains the declaration of namespace Helper that encapsulates the
functionality required to create an OpenGL context using GLFW; use GLEW
to load OpenGL extensions; initialize OpenGL state; and finally initialize
the OpenGL application by calling initalization functions associated with
objects participating in the application.>
Language: <c++>
Platform: <Visual studio 2019, OpenGL 4.5, Window 64bit>
Project: <jaewoo.choi_CS350_1>
Author: <Jaewoo Choi, jaewoo.choi, 55532>, Original author <pghali>
Creation date: 04/11/2022
End Header --------------------------------------------------------*/
/*                                                                      guard
----------------------------------------------------------------------------- */
#ifndef GLHELPER_H
#define GLHELPER_H

/*                                                                   includes
----------------------------------------------------------------------------- */
#include <GL/glew.h> // for access to OpenGL API declarations 
#include <GLFW/glfw3.h>
#include <iostream>
#include <sstream>
#include <string>
#include <cstdlib>

/*  _________________________________________________________________________ */
struct GLHelper
    /*! GLHelper structure to encapsulate initialization stuff ...
    */
{
    static bool init(GLint w, GLint h, std::string t);
    static void cleanup();

    // callbacks ...
    static void error_cb(int error, char const* description);
    static void fbsize_cb(GLFWwindow* ptr_win, int width, int height);
    // I/O callbacks ...
    static void key_cb(GLFWwindow* pwin, int key, int scancode, int action, int mod);
    static void mousebutton_cb(GLFWwindow* pwin, int button, int action, int mod);
    static void mousescroll_cb(GLFWwindow* pwin, double xoffset, double yoffset);
    static void mousepos_cb(GLFWwindow* pwin, double xpos, double ypos);

    static double update_time(double fpsCalcInt = 1.0);

    static GLint width, height;
    static GLdouble fps;
    static std::string title;
    static GLFWwindow* ptr_window;
    static bool mouse_pressed;

};

#endif /* GLHELPER_H */
