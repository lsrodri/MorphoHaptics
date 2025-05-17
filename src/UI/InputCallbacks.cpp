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
