#pragma once

#include "chai3d.h"
#include <GLFW/glfw3.h>

extern MouseStates mouseState;
extern double mouseX, mouseY;
extern chai3d::cCamera* camera;
extern chai3d::cToolCursor* tool[];

void mouseMotionCallback(GLFWwindow* a_window, double a_posX, double a_posY);
void mouseScrollCallback(GLFWwindow* a_window, double a_offsetX, double a_offsetY);
