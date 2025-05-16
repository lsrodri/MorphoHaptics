// Utils.cpp
#include "Utils.h"

extern int textureWidth;
extern int textureHeight;
extern int textureDepth;
extern chai3d::cTexture3dPtr texture;

float smoothAverageLuminosity(float newLuminosity, float previousLuminosity, float smoothingFactor) {
    return previousLuminosity * (1.0f - smoothingFactor) + newLuminosity * smoothingFactor;
}

float getAverageLuminosity(int centerX, int centerY, int centerZ, int radius) {
    int voxelCount = 0;
    float totalLuminosity = 0;

    for (int x = centerX - radius; x <= centerX + radius; ++x) {
        for (int y = centerY - radius; y <= centerY + radius; ++y) {
            for (int z = centerZ - radius; z <= centerZ + radius; ++z) {
                // Check if the voxel is within the bounds of the texture
                if (x >= 0 && x < textureWidth && y >= 0 && y < textureHeight && z >= 0 && z < textureDepth) {
                    chai3d::cColorb voxelColor;
                    texture->m_image->getVoxelColor(x, y, z, voxelColor);
                    totalLuminosity += calculateLuminosity(voxelColor);
                    ++voxelCount;
                }
            }
        }
    }

    return voxelCount > 0 ? totalLuminosity / voxelCount : 0;
}

float calculateLuminosity(const chai3d::cColorb& color) {
    // Normalize the RGB components to floats
    float R = color.getR() / 255.0f;
    float G = color.getG() / 255.0f;
    float B = color.getB() / 255.0f;
    float A = color.getA() / 255.0f; // Normalize alpha component to float

    // Calculate luminosity with alpha consideration
    float luminosity = (0.2126f * R + 0.7152f * G + 0.0722f * B) * A; // Multiply by alpha to reduce luminosity for transparent colors

    return luminosity;
}