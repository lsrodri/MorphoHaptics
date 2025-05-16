#include "UIUtils.h"
#include "UIActions.h"
#include <iostream>

extern bool isHapticSmoothingEnabled;
extern bool isVoxelValueHapticsEnabled;

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
