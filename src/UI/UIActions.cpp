#include "UIUtils.h"
#include "UIActions.h"
#include <iostream>

extern bool isHapticSmoothingEnabled;
extern chai3d::cLabel* button13;

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
