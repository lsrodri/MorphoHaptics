#include "../MorphoHaptics.h"
#include "InputCallbacks.h"

template<typename T>
T clamp(T x, T min, T max) {
    return (x < min) ? min : (x > max) ? max : x;
}

void mouseMotionCallback(GLFWwindow* a_window, double a_posX, double a_posY)
{
    if (mouseState == MOUSE_MOVE_CAMERA)
    {
        // compute mouse motion
        int dx = a_posX - mouseX;
        int dy = a_posY - mouseY;
        mouseX = a_posX;
        mouseY = a_posY;

        // compute new camera angles
        double azimuthDeg = camera->getSphericalAzimuthDeg() - 0.5 * dx;
        double polarDeg = camera->getSphericalPolarDeg() - 0.5 * dy;

        //// assign new angles
        camera->setSphericalAzimuthDeg(azimuthDeg);
        camera->setSphericalPolarDeg(polarDeg);

        for (int i = 0; i < NUM_HAPTIC_DEVICES; i++)
        {
            // oriente tool with camera
            tool[i]->setLocalRot(camera->getLocalRot());
        }
    }
}

//------------------------------------------------------------------------------

void mouseScrollCallback(GLFWwindow* a_window, double a_offsetX, double a_offsetY)
{
    double r = camera->getSphericalRadius();
    r = clamp(r + 0.1 * a_offsetY, 0.5, 3.0);
    camera->setSphericalRadius(r);
}

//------------------------------------------------------------------------------

void keyCallback(GLFWwindow* a_window, int a_key, int a_scancode, int a_action, int a_mods)
{

    // filter calls that only include a key press
    if ((a_action != GLFW_PRESS) && (a_action != GLFW_REPEAT))
    {
        return;
    }

    // option - exit
    else if ((a_key == GLFW_KEY_ESCAPE) || (a_key == GLFW_KEY_Q))
    {
        glfwSetWindowShouldClose(a_window, GLFW_TRUE);
    }

    else if (a_key == GLFW_KEY_DOWN)
    {
        updateValueHapticsRadius(-1);
    }

    else if (a_key == GLFW_KEY_UP)
    {
        updateValueHapticsRadius(1);
    }

    // polygonize model and save to file
    else if (a_key == GLFW_KEY_M)
    {
        startPolygonize();
    }

    // load dataset
    else if (a_key == GLFW_KEY_L)
    {
        loadDataset();
    }

    // toggle fullscreen
    else if (a_key == GLFW_KEY_F)
    {
        // toggle state variable
        fullscreen = !fullscreen;

        // get handle to monitor
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();

        // get information about monitor
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        // set fullscreen or window mode
        if (fullscreen)
        {
            glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
            glfwSwapInterval(swapInterval);
        }
        else
        {
            int w = 0.8 * mode->height;
            int h = 0.5 * mode->height;
            int x = 0.5 * (mode->width - w);
            int y = 0.5 * (mode->height - h);
            glfwSetWindowMonitor(window, NULL, x, y, w, h, mode->refreshRate);
            glfwSwapInterval(swapInterval);
        }
    }

    else if (a_key == GLFW_KEY_G)
    {
        object->setGhostEnabled(!object->getGhostEnabled());
    }

    else if (a_key == GLFW_KEY_H)
    {
        toggleHaptics();
    }

    else if (a_key == GLFW_KEY_S)
    {
        toggleSculpting();
    }

    else if (a_key == GLFW_KEY_K)
    {
        toggleVoxelValueHaptics();
    }

    else if (a_key == GLFW_KEY_X)
    {
        toggleForceSmoothing();
    }

    else if (a_key == GLFW_KEY_SPACE)
    {
        toggleGhostMode();
    }

    else if (a_key == GLFW_KEY_V)
    {
        startExportVolume();
    }

    else if (a_key == GLFW_KEY_TAB)
    {
        isUILayerVisible = !isUILayerVisible;
        panel->setShowEnabled(isUILayerVisible);
    }

    else if (a_key == GLFW_KEY_EQUAL || a_key == GLFW_KEY_KP_ADD)
    {
        updateProbeRadius(1);
    }

    else if (a_key == GLFW_KEY_MINUS || a_key == GLFW_KEY_KP_SUBTRACT)
    {
        updateProbeRadius(-1);
    }

}

//------------------------------------------------------------------------------



void mouseButtonCallback(GLFWwindow* a_window, int a_button, int a_action, int a_mods)
{

    if (a_button == GLFW_MOUSE_BUTTON_LEFT && a_action == GLFW_PRESS)
    {
        // Get mouse position
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        // Adjust y position because of OpenGL's coordinate system
        ypos = height - ypos;

        // Check if sandwich button was clicked
        if (isPointInsidePanel(sandwichButton, xpos, ypos)) {
            isUILayerVisible = !isUILayerVisible;
            panel->setShowEnabled(isUILayerVisible);
        }

        else if (isPointInsideLabel(button1, xpos, ypos)) {
            loadDataset();
        }

        else if (isPointInsideLabel(button2, xpos, ypos)) {
            startPolygonize();
        }

        else if (isPointInsideLabel(button3, xpos, ypos)) {
            startExportVolume();
        }

        else if (isPointInsideLabel(button4, xpos, ypos)) {
            toggleGhostMode();
        }

        else if (isPointInsideLabel(button5, xpos, ypos)) {
            toggleHaptics();
        }

        else if (isPointInsideLabel(button6, xpos, ypos)) {
            glfwSetWindowShouldClose(a_window, GLFW_TRUE);
        }

        else if (isPointInsideLabel(button7, xpos, ypos)) {
            toggleSculpting();
        }

        else if (isPointInsideLabel(button8, xpos, ypos)) {
            toggleVoxelValueHaptics();
        }

        else if (isPointInsideLabel(button9, xpos, ypos)) {
            updateProbeRadius(1);
        }

        else if (isPointInsideLabel(button10, xpos, ypos)) {
            updateProbeRadius(-1);
        }

        else if (isPointInsideLabel(button11, xpos, ypos)) {
            updateValueHapticsRadius(1);
        }

        else if (isPointInsideLabel(button12, xpos, ypos)) {
            updateValueHapticsRadius(-1);
        }

        else if (isPointInsideLabel(button13, xpos, ypos)) {
            toggleForceSmoothing();
        }

    }

    if (a_button == GLFW_MOUSE_BUTTON_RIGHT && a_action == GLFW_PRESS)
    {
        // store mouse position
        glfwGetCursorPos(window, &mouseX, &mouseY);

        // update mouse state
        mouseState = MOUSE_MOVE_CAMERA;
    }

    else
    {
        // update mouse state
        mouseState = MOUSE_IDLE;
    }
}

