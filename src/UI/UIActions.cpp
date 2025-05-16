#include "UIUtils.h"
#include "UIActions.h"
#include <iostream>

extern bool isHapticSmoothingEnabled;
extern bool isVoxelValueHapticsEnabled;
extern bool isHapticsEnabled;

extern int valueHapticsRadius;

extern chai3d::cVoxelObject* object;

extern chai3d::cLabel* button4;
extern chai3d::cLabel* button5;
extern chai3d::cLabel* button7;
extern chai3d::cLabel* button8;
extern chai3d::cLabel* button13;

extern bool isSculptingEnabled;


void toggleForceSmoothing()
{
    if (isHapticSmoothingEnabled)
    {
        isHapticSmoothingEnabled = false;
        button13->setText("(off) Force Smoothing (X)");
        toggleStatusMessage(true, "Force Smoothing (off)");
        button13->m_fontColor.setRed();
    }
    else
    {
        isHapticSmoothingEnabled = true;
        button13->setText("(on) Force Smoothing (X)");
        showStatusMessageForSeconds(3.0, "Force Smoothings (on)");
        button13->m_fontColor.setWhite();
    }
}

void toggleSculpting()
{
    if (isSculptingEnabled)
    {
        isSculptingEnabled = false;
        button7->setText("(off) Sculpting (S)");
        toggleStatusMessage(true, "Sculpting (off)");
        button7->m_fontColor.setRed();
    }
    else
    {
        isSculptingEnabled = true;
        button7->setText("(on) Sculpting (S)");
        showStatusMessageForSeconds(3.0, "Sculpting (on)");
        button7->m_fontColor.setWhite();
    }
}

void toggleVoxelValueHaptics()
{
    if (isVoxelValueHapticsEnabled)
    {
        isVoxelValueHapticsEnabled = false;
        button8->setText("(off) Voxel-Value Haptics (K)");
        toggleStatusMessage(true, "Voxel-Value Haptics (off)");
        button8->m_fontColor.setRed();
    }
    else
    {
        isVoxelValueHapticsEnabled = true;
        button8->setText("(on) Voxel-Value Haptics (K)");
        showStatusMessageForSeconds(3.0, "Voxel-Value Haptics (on)");
        button8->m_fontColor.setWhite();
    }
}

void toggleGhostMode()
{
    object->setGhostEnabled(!object->getGhostEnabled());
    if (object->getGhostEnabled())
    {
        button4->setText("(on) Ghost Mode (Space)");
        button4->m_fontColor.setGreen();
        toggleStatusMessage(true, "Ghost Mode (on)");
    }
    else
    {
        button4->setText("(off) Ghost Mode (Space)");
        button4->m_fontColor.setWhite();
        showStatusMessageForSeconds(3.0, "Ghost Mode (off)");
    }
}

void toggleHaptics()
{
    if (isHapticsEnabled)
    {
        isHapticsEnabled = false;
        button5->setText("(off) Haptics (H)");
        toggleStatusMessage(true, "Haptics (off)");
        button5->m_fontColor.setRed();
    }
    else
    {
        isHapticsEnabled = true;
        button5->setText("(on) Haptics (H)");
        showStatusMessageForSeconds(3.0, "Haptics (on)");
        button5->m_fontColor.setWhite();
    }
}

void updateValueHapticsRadius(int value)
{
    if (valueHapticsRadius <= 1 && value == -1)
    {
        showStatusMessageForSeconds(3.0, "Minimum Radius Reached");
        return;
    }

    if (valueHapticsRadius >= 25 && value == 1)
    {
        showStatusMessageForSeconds(3.0, "Maximum Radius Reached");
        return;
    }

    valueHapticsRadius += value;
    showStatusMessageForSeconds(3.0, "Radius updated to " + std::to_string(valueHapticsRadius) + " voxels");

    #include <sstream> // Add this include for stringstream

    // Replace the problematic line with the following implementation
    showStatusMessageForSeconds(3.0, "Radius updated to " + std::to_string(valueHapticsRadius) + " voxels");
}