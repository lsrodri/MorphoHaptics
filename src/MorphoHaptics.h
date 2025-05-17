// MorphoHaptics.h
#pragma once

// Enum for mouse states
enum MouseStates
{
    MOUSE_IDLE,
    MOUSE_MOVE_CAMERA
};

constexpr int NUM_HAPTIC_DEVICES = 1;

extern MouseStates mouseState;