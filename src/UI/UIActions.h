#pragma once
#include "chai3d.h"

extern chai3d::cLabel* button13;

extern bool isHapticSmoothingEnabled;

// Function declaration
void toggleForceSmoothing();

void toggleSculpting();

void toggleVoxelValueHaptics();

void toggleGhostMode();

void toggleHaptics();

void updateValueHapticsRadius(int value);

void updateProbeRadius(int value);