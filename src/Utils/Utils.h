// Utils.h
#pragma once
#include "chai3d.h"

float smoothAverageLuminosity(float newLuminosity, float previousLuminosity, float smoothingFactor = 0.1f);

float calculateLuminosity(const chai3d::cColorb& color);

float getAverageLuminosity(int centerX, int centerY, int centerZ, int radius);