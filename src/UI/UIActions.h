#pragma once
#include "chai3d.h" // Ensure CHAI3D headers are included

extern chai3d::cLabel* button13; // Use the namespace to resolve ambiguity
// Extern declarations for variables used in toggleForceSmoothing
extern bool isHapticSmoothingEnabled;

// Function declaration
void toggleForceSmoothing();

void toggleSculpting();

void toggleVoxelValueHaptics();

void toggleGhostMode();

void toggleHaptics();

void updateValueHapticsRadius(int value);

void updateProbeRadius(int value);