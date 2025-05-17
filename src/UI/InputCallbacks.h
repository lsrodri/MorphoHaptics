#pragma once

#include "chai3d.h"
#include <GLFW/glfw3.h>

extern MouseStates mouseState;
extern double mouseX, mouseY;
extern chai3d::cCamera* camera;
extern chai3d::cToolCursor* tool[];
extern bool fullscreen;
extern GLFWwindow* window;
extern int swapInterval;
extern chai3d::cVoxelObject* object;
extern bool isUILayerVisible;
extern chai3d::cPanel* panel;
extern chai3d::cLabel* button1, * button2, * button3, * button4, * button5, * button6, * button7, * button8, * button9, * button10, * button11, * button12, * button13;
extern chai3d::cLabel* labelRates;
extern chai3d::cLabel* statusMessage;
extern int toolOne, toolTwo;

void startPolygonize();
void loadDataset();
void toggleHaptics();
void toggleSculpting();
void toggleVoxelValueHaptics();
void toggleForceSmoothing();
void toggleGhostMode();
void startExportVolume();
void updateValueHapticsRadius(int delta);
void updateProbeRadius(int delta);

void mouseMotionCallback(GLFWwindow* a_window, double a_posX, double a_posY);
void mouseScrollCallback(GLFWwindow* a_window, double a_offsetX, double a_offsetY);
void keyCallback(GLFWwindow* a_window, int a_key, int a_scancode, int a_action, int a_mods);