// Utils.cpp
#include "Utils.h"

float smoothAverageLuminosity(float newLuminosity, float previousLuminosity, float smoothingFactor) {
    return previousLuminosity * (1.0f - smoothingFactor) + newLuminosity * smoothingFactor;
}

// Implement other utility functions here
