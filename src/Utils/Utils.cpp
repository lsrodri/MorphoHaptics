#include "Utils.h"
#include "chai3d.h" // Ensure CHAI3D headers are included
#include <string>
#include <iostream>
#include <GLFW/glfw3.h>
#include <windows.h>
#include <iostream>
#include <tinyfiledialogs.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

using namespace std; // Optional, to avoid writing std:: repeatedly

std::string selectFolder()
{
    const char* path = tinyfd_selectFolderDialog("Select Folder", nullptr);
    if (path)
    {
        HWND hwnd = glfwGetWin32Window(window);

        // Minimize and restore the window to ensure it regains focus
        ShowWindow(hwnd, SW_MINIMIZE);
        ShowWindow(hwnd, SW_RESTORE);

        SetForegroundWindow(hwnd);
        return std::string(path);
    }
    return std::string();
}

void createVoxelObject(cVoxelObject* object, string path, char* argv[])
{

    rotationX = -23;
    rotationY = 0;
    rotationZ = 93;

    object->setUseCulling(false, false);
    object->m_material->setStiffness(stiffnessMultiplier * maxStiffness);              // % of maximum linear stiffness

    // add object to world
    world->addChild(object);

    // rotate object
    object->rotateExtrinsicEulerAnglesDeg(rotationX, rotationY, rotationZ, C_EULER_ORDER_XYZ);

    // position object
    object->setLocalPos(-0.05, -0.05, 0.02);

    // set the dimensions by assigning the position of the min and max corners
    object->m_maxCorner.set(0.25, 0.25, 0.25);

    // set the texture coordinate at each corner.
    object->m_minTextureCoord.set(0, 0, 0);
    object->m_maxTextureCoord.set(1.0, 1.0, 1.0);


    object->m_material->setStaticFriction(0.0);
    object->m_material->setDynamicFriction(0.4);

    // enable materials
    object->setUseMaterial(true);

    // set quality of graphic rendering
    object->setQuality(1);

    object->setStiffness(maxStiffness, true);

    cShaderProgramPtr programShader = object->getShaderProgram();


    //--------------------------------------------------------------------------
    // LOAD VOXEL DATA
    //--------------------------------------------------------------------------

    // create multi image
    image = cMultiImage::create();

    // Extract the substring after the last backslash
    std::string dataset = path.substr(path.find_last_of('\\') + 1);

#if defined(_MSVC)
    //path = "resources/volumes/" + dataset + "/"; // Adjust path as necessary
# else
    path = RESOURCE_PATH("../resources/volumes/" + dataset + "/");
#endif

    int countFiles = countFilesInDirectory(path, ".png");

    std::replace(path.begin(), path.end(), '\\', '/');

    int filesloaded = image->loadFromFiles(path + "/" + dataset + "0", "png", countFiles);
    if (filesloaded == 0) {
#if defined(_MSVC)
        filesloaded = image->loadFromFiles(path + "/" + dataset + "0", "png", countFiles);

#endif
    }

    if (filesloaded == 0) {
        cout << "Error - Failed to load volume data." << endl;
        close();
        return;
    }


    // create texture
    texture = cTexture3d::create();

    // assign volumetric image to texture
    texture->setImage(image);

    // assign texture to voxel object
    object->setTexture(texture);

    // initially select an isosurface corresponding to the bone/heart level
    object->setIsosurfaceValue(0.2);

    // set optical density factor
    object->setOpticalDensity(1.2);

    // setting the texture's dimensions for other calculations
    textureWidth = texture->m_image->getWidth();
    textureHeight = texture->m_image->getHeight();
    textureDepth = texture->m_image->getImageCount();

    // Calculating the scale factors for width, height, and depth
    float widthScale = static_cast<float>(textureWidth) / textureWidth;
    float heightScale = static_cast<float>(textureHeight) / textureWidth;
    float depthScale = static_cast<float>(textureDepth) / textureWidth;

    // Setting the minCorner and maxCorner values
    object->m_minCorner.set(-0.5 * widthScale, -0.5 * heightScale, -0.5 * depthScale);
    object->m_maxCorner.set(0.5 * widthScale, 0.5 * heightScale, 0.5 * depthScale);

    //--------------------------------------------------------------------------
    // LOAD COLORMAPS
    //--------------------------------------------------------------------------

    boneLUT = cImage::create();
    bool fileLoaded = boneLUT->loadFromFile("resources/volumes/colormap_bone.png");
    if (!fileLoaded) {
#if defined(_MSVC)
        fileLoaded = boneLUT->loadFromFile("../resources/volumes/colormap_bone.png");
#endif
    }
    if (!fileLoaded)
    {
        cout << "Error - Failed to load colormap." << endl;
        close();
        return;
    }


    // tell the voxel object to load the colour look-up table as a texture
    object->m_colorMap->setImage(boneLUT);

    object->setRenderingModeDVRColorMap();            // high quality

    object->setShowEdges(false);
    object->clearAllEdges();
    object->clear();
    object->clearAllChildren();
    object->setUseLinearInterpolation(true);

}

void loadDataset()
{
    string path = selectFolder(); // Either use `std::string path` or `using namespace std`
    cout << path << endl;

    if (path.empty())
    {
        return;
    }
    else
    {
        createVoxelObject(object, path, nullptr);
    }
}


