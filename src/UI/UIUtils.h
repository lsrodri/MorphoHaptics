#ifndef UI_UTILS_H
#define UI_UTILS_H

#include <string>
#include "chai3d.h" // Ensure CHAI3D headers are included

// Extern declarations for shared variables
extern bool isStatusMessageVisible;
extern double statusMessageDisplayTime;
extern chai3d::cLabel* statusMessage; // Explicitly use chai3d namespace
extern int width;

// Function declarations
void toggleStatusMessage(bool on, const std::string& message);
bool isPointInsideLabel(chai3d::cLabel* label, double x, double y);
bool isPointInsidePanel(chai3d::cPanel* panel, double x, double y);
void showStatusMessageForSeconds(double seconds, const std::string& message);

#endif // UI_UTILS_H
