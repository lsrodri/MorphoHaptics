#include "UIUtils.h"
#include "chai3d.h" // Ensure CHAI3D headers are included
#include <string>
#include <iostream>
#include <GLFW/glfw3.h>

using namespace chai3d;

// Implementation of toggleStatusMessage
void toggleStatusMessage(bool on, const std::string& message)
{
    isStatusMessageVisible = on;
    statusMessageDisplayTime = 0; // Clear any display time
    if (isStatusMessageVisible)
    {
        statusMessage->setText(message);
        // Center the status message horizontally
        statusMessage->setLocalPos((width - statusMessage->getWidth()) / 2, 15);
    }
    statusMessage->setShowEnabled(isStatusMessageVisible);
}

// Implementation of isPointInsideLabel
bool isPointInsideLabel(cLabel* label, double x, double y)
{
    double labelX = label->getLocalPos().x();
    double labelY = label->getLocalPos().y();
    double labelWidth = label->getWidth();
    double labelHeight = label->getHeight();

    return (x >= labelX && x <= labelX + labelWidth &&
        y >= labelY && y <= labelY + labelHeight);
}

// Implementation of isPointInsidePanel
bool isPointInsidePanel(cPanel* panel, double x, double y)
{
    double panelX = panel->getLocalPos().x();
    double panelY = panel->getLocalPos().y();
    double panelWidth = panel->getWidth();
    double panelHeight = panel->getHeight();

    return (x >= panelX && x <= panelX + panelWidth &&
        y >= panelY && y <= panelY + panelHeight);
}

// Implementation of showStatusMessageForSeconds
void showStatusMessageForSeconds(double seconds, const std::string& message)
{
    isStatusMessageVisible = true;
    statusMessageDisplayTime = glfwGetTime() + seconds;
    statusMessage->setText(message);
    // Center the status message horizontally
    statusMessage->setLocalPos((width - statusMessage->getWidth()) / 2, 15);
    statusMessage->setShowEnabled(true);
}

void createPanelButton(
    chai3d::cLabel*& button,
    chai3d::cPanel* parentPanel,
    chai3d::cFontPtr font,
    const std::string& text,
    int yOffset
) {
    button = new chai3d::cLabel(font);
    parentPanel->addChild(button);
    button->setLocalPos(20, parentPanel->getHeight() - yOffset);
    button->setText(text);
    button->m_fontColor.setWhite();
}