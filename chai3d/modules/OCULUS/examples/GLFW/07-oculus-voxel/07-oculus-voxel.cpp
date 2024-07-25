//==============================================================================
/*
    Software License Agreement (BSD License)
    Copyright (c) 2003-2016, CHAI3D.
    (www.chai3d.org)

    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following
    disclaimer in the documentation and/or other materials provided
    with the distribution.

    * Neither the name of CHAI3D nor the names of its contributors may
    be used to endorse or promote products derived from this software
    without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
    COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
    ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

    \author    <http://www.chai3d.org>
    \author    Francois Conti
    \version   3.2.0 $Rev: 1659 $
*/
//==============================================================================

//------------------------------------------------------------------------------
#include "chai3d.h"
//------------------------------------------------------------------------------
#include <GLFW/glfw3.h>
//------------------------------------------------------------------------------
#include "COculus.h"
//------------------------------------------------------------------------------
using namespace chai3d;
using namespace std;
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// CHAI3D
//------------------------------------------------------------------------------

// a world that contains all objects of the virtual environment
cWorld* world;

// a camera to render the world in the window display
cCamera* camera;

// a light source
cSpotLight *light;

// a haptic device handler
cHapticDeviceHandler* handler;

// a pointer to the current haptic device
cGenericHapticDevicePtr hapticDevice;

// a virtual tool representing the haptic device in the scene
cToolCursor* tool;

// a virtual mesh like object
cVoxelObject* object;

// 3D texture
cTexture3dPtr texture;

// 3D image data
cMultiImagePtr image;

// Resolution of voxel model
int voxelModelResolution = 128;

// Region of voxels being updated
cCollisionAABBBox volumeUpdate;

// indicates if the haptic simulation currently running
bool simulationRunning = false;

// indicates if the haptic simulation has terminated
bool simulationFinished = false;

// frequency counter to measure the simulation haptic rate
cFrequencyCounter frequencyCounter;

// Flag that indicates that voxels have been updated
bool flagMarkVolumeForUpdate = false;

// Mutex to voxel
cMutex mutexVoxel;

// Mutex to object
cMutex mutexObject;

// haptic thread
cThread *hapticsThread;

cImagePtr boneLUT;

cMultiMesh* drills[1];


//------------------------------------------------------------------------------
// OCULUS RIFT
//------------------------------------------------------------------------------

// display context
cOVRRenderContext renderContext;

// oculus device
cOVRDevice oculusVR;

cMaterial mat;
cMaterial transparentMat;


//------------------------------------------------------------------------------
// DECLARED MACROS
//------------------------------------------------------------------------------

// convert to resource path
#define RESOURCE_PATH(p)    (char*)((resourceRoot+string(p)).c_str())


//------------------------------------------------------------------------------
// DECLARED FUNCTIONS
//------------------------------------------------------------------------------

// callback when an error GLFW occurs
void errorCallback(int error, const char* a_description);

// callback when a key is pressed
void keyCallback(GLFWwindow* a_window, int a_key, int a_scancode, int a_action, int a_mods);

// function that closes the application
void close(void);

// main haptics simulation loop
void updateHaptics(void);


//==============================================================================
/*
    DEMO:   07-clay.cpp

    This example illustrates the use of voxel data to simulate a clay material
*/
//==============================================================================

int main(int argc, char **argv)
{
  
    // parse first arg to try and locate resources
    string resourceRoot = string(argv[0]).substr(0, string(argv[0]).find_last_of("/\\") + 1);


    //--------------------------------------------------------------------------
    // SETUP DISPLAY CONTEXT
    //--------------------------------------------------------------------------

    // initialize GLFW library
    if (!glfwInit())
    {
        cout << "failed initialization" << endl;
        cSleepMs(1000);
        return 1;
    }

    // set error callback
    glfwSetErrorCallback(errorCallback);

    // set OpenGL version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // create display context
    //GLFWwindow* window = glfwCreateWindow(1920, 1080, "MorphoHaptics", NULL, NULL);
    GLFWwindow* window = glfwCreateWindow(640, 480, "MorphoHaptics", NULL, NULL);
    if (!window)
    {
        cout << "failed to create window" << endl;
        cSleepMs(1000);
        glfwTerminate();
        return 1;
    }

    // set key callback
    glfwSetKeyCallback(window, keyCallback);

    // set current display context
    glfwMakeContextCurrent(window);

    // sets the swap interval for the current display context
    glfwSwapInterval(0);

#ifdef GLEW_VERSION
    // initialize GLEW library
    if (glewInit() != GLEW_OK)
    {
        oculusVR.destroyVR();
        renderContext.destroy();
        glfwTerminate();
        return 1;
    }
#endif

    // initialize oculus
    if (!oculusVR.initVR())
    {
        cout << "failed to initialize Oculus" << endl;
        cSleepMs(1000);
        glfwTerminate();
        return 1;
    }

    // get oculus display resolution
    ovrSizei hmdResolution = oculusVR.getResolution();

    // setup mirror display on computer screen
    ovrSizei windowSize = { hmdResolution.w / 2, hmdResolution.h / 2 };
    //ovrSizei windowSize = { 2560 , 1440 };

    // inialize buffers
    if (!oculusVR.initVRBuffers(windowSize.w, windowSize.h))
    {
        cout << "failed to initialize Oculus buffers" << endl;
        cSleepMs(1000);
        oculusVR.destroyVR();
        renderContext.destroy();
        glfwTerminate();
        return 1;
    }

    // set window size
    glfwSetWindowSize(window, windowSize.w, windowSize.h);



    //--------------------------------------------------------------------------
    // WORLD - CAMERA - LIGHTING
    //--------------------------------------------------------------------------

    // create a new world.
    world = new cWorld();

    // set the background color of the environment
    // the color is defined by its (R,G,B) components.
    world->m_backgroundColor.setBlack();

    // create a camera and insert it into the virtual world
    camera = new cCamera(world);
    world->addChild(camera);

    // position and orient the camera
    camera->set(cVector3d(0.4, 0.0, 0.3),    // camera position (eye)
                cVector3d(0.0, 0.0, 0.3),    // lookat position (target)
                cVector3d(0.0, 0.0, 1.0));   // direction of the "up" vector

    // set the near and far clipping planes of the camera
    // anything in front/behind these clipping planes will not be rendered
    camera->setClippingPlanes(0.01, 10.0);

    // create a light source
    light = new cSpotLight(world);

    // add light to world
    world->addChild(light);

    // enable light source
    light->setEnabled(true);

    // position the light source
    light->setLocalPos(3.5, 2.0, 0.0);

    // define the direction of the light beam
    light->setDir(-3.5, -2.0, 0.0);

    // set light cone half angle
    light->setCutOffAngleDeg(50);

    /////////////////////////////////////////////////////////////////////////
    // OBJECT "DRILL"
    /////////////////////////////////////////////////////////////////////////

    // create and attach drill meshes to tools 
    for (int i = 0; i < 1; i++) {
        // create a new mesh 
        drills[i] = new cMultiMesh();

        // load a drill like mesh and attach it to the tool
        bool fileload = drills[i]->loadFromFile("resources/models/drill.3ds");
        if (!fileload)
        {
#if defined(_MSVC)
            fileload = drills[i]->loadFromFile("../../resources/models/drill.3ds");
#endif
        }
        if (!fileload)
        {
            printf("Error - 3D Model failed to load correctly.\n");
            close();
            return (-1);
        }

        // resize tool mesh model
        drills[i]->scale(0.0007);

        // remove the collision detector. we do not want to compute any
        // force feedback rendering on the object itself.
        drills[i]->deleteCollisionDetector(true);

        // define a material property for the mesh
        mat.m_ambient.set(0.5f, 0.5f, 0.5f);
        mat.m_diffuse.set(0.8f, 0.8f, 0.8f);
        mat.m_specular.set(1.0f, 1.0f, 1.0f);
        //mat.setWhite();
        mat.setWhite();


        // Duplicate the material to create a copy
        transparentMat = cMaterial(mat);
        // Trying to get it to 50% opacity, but it does not work
        transparentMat.m_diffuse.set(1.0, 1.0, 1.0, 0.5); // Set white color with 50% opacity
        transparentMat.m_ambient.set(0.5f, 0.5f, 0.5f, 0.5f);
        transparentMat.m_diffuse.set(0.8f, 0.8f, 0.8f, 0.5f);
        transparentMat.setTransparencyLevel(0.5f);

        drills[i]->setMaterial(mat, true);



        // create a texture
        cTexture2dPtr maskTexture = cTexture2d::create();

        fileload = maskTexture->loadFromFile("resources/images/white.png");
        if (!fileload)
        {
#if defined(_MSVC)
            fileload = maskTexture->loadFromFile("resources/images/white.png");
#endif
        }
        if (!fileload)
        {
            cout << "Error - Texture image failed to load correctly." << endl;
            // close();
            // return (-1);
        }

        //apply texture to maskObject
        drills[i]->setTexture(maskTexture);

        //enable texture rendering 
        drills[i]->setUseTexture(true);
        //drills[i]->setTexture(texture, true);


        drills[i]->computeAllNormals();

        cColorf colorRed;
        colorRed.setRed();

        drills[i]->setEdgeProperties(3, colorRed);
        drills[i]->setShowEdges(true);
        drills[i]->setEdgeLineWidth(3);

        // attach drill to tool
        // to-do, this might go under the haptic devices part as a cursor
        //tool->m_image->addChild(drills[i]);

    }


    //--------------------------------------------------------------------------
    // HAPTIC DEVICES / TOOLS
    //--------------------------------------------------------------------------

    // create a haptic device handler
    handler = new cHapticDeviceHandler();

    // get access to the first available haptic device
    handler->getDevice(hapticDevice, 0);

    // retrieve information about the current haptic device
    cHapticDeviceInfo hapticDeviceInfo = hapticDevice->getSpecifications();

    // if the device has a gripper, enable the gripper to simulate a user switch
    hapticDevice->setEnableGripperUserSwitch(true);

    // create a 3D tool and add it to the world
    tool = new cToolCursor(world);

    tool->m_image = drills[0];
    world->addChild(tool);

    // connect the haptic device to the tool
    tool->setHapticDevice(hapticDevice);

    // define the radius of the tool (sphere)
    double toolRadius = 0.01;

    // define a radius for the tool
    tool->setRadius(toolRadius);

    // enable if objects in the scene are going to rotate of translate
    // or possibly collide against the tool. If the environment
    // is entirely static, you can set this parameter to "false"
    tool->enableDynamicObjects(true);

    // map the physical workspace of the haptic device to a larger virtual workspace.
    tool->setWorkspaceRadius(0.4);

    // haptic forces are enabled only if small forces are first sent to the device;
    // this mode avoids the force spike that occurs when the application starts when 
    // the tool is located inside an object for instance. 
    tool->setWaitForSmallForce(true);

    // start the haptic tool
    tool->start();

    // read the scale factor between the physical workspace of the haptic
    // device and the virtual workspace defined for the tool
    double workspaceScaleFactor = tool->getWorkspaceScaleFactor();
    
    // stiffness properties
    double maxStiffness	= hapticDeviceInfo.m_maxLinearStiffness / workspaceScaleFactor;


    //--------------------------------------------------------------------------
    // CREATE ENVIRONMENT GLOBE
    //--------------------------------------------------------------------------

    // create a virtual mesh
    cMesh* globe = new cMesh();

    // add object to world
    //world->addChild(globe);

    // set the position of the object at the center of the world
    globe->setLocalPos(0.0, 0.0, 0.0);

    // create cube
    cCreateSphere(globe, 10.0, 36, 36);

    globe->setUseDisplayList(true);
    globe->deleteCollisionDetector();

    // create a texture
    cTexture2dPtr textureSpace = cTexture2d::create();

    bool fileload = textureSpace->loadFromFile(RESOURCE_PATH("../resources/images/shade.jpg"));
    if (!fileload)
    {
#if defined(_MSVC)
        fileload = textureSpace->loadFromFile("../../../bin/resources/images/shade.jpg");
#endif
    }
    if (!fileload)
    {
        cout << "Error - Texture image failed to load correctly." << endl;
        close();
        return (-1);
    }

    // apply texture to object
    globe->setTexture(textureSpace);

    // enable texture rendering 
    globe->setUseTexture(true);

    // Since we don't need to see our polygons from both sides, we enable culling.
    globe->setUseCulling(false);

    // disable material properties and lighting
    globe->setUseMaterial(false);
    

    //--------------------------------------------------------------------------
    // CREATE OBJECT
    //--------------------------------------------------------------------------


    // create a volumetric model
        // create a volumetric model
    object = new cVoxelObject();

    // add object to world
    world->addChild(object);

    // rotate object
    object->rotateAboutGlobalAxisDeg(0, 0, 1, 40);

    object->setUseCulling(false, false);

    //// set the dimensions by assigning the position of the min and max corners
    object->m_minCorner.set(-0.1, -0.1, -0.1 * 1.914);
    object->m_maxCorner.set(0.1, 0.1, 0.1 * 1.914);

    /*object->m_minCorner.set(-0.5, -0.5, -0.5);
    object->m_maxCorner.set(0.5, 0.5, 0.5);*/

    object->setUseMaterial(false);

    // set the texture coordinate at each corner.
    object->m_minTextureCoord.set(0.0, 0.0, 0.0);
    object->m_maxTextureCoord.set(1.0, 1.0, 1.0);

    // set material color
    //object->m_material->setOrangeCoral();

    // set stiffness property
    object->setStiffness(0.7 * maxStiffness);

    // show/hide boundary box
    object->setShowBoundaryBox(false);
    
    
        //--------------------------------------------------------------------------
        // LOAD VOXEL DATA
        //--------------------------------------------------------------------------
    
        // create multi image
        image = cMultiImage::create();
    
        string dataset = "tooth";
    
        //std::string dataset = "tooth";
        std::string path;
    #if defined(_MSVC)
        path = "resources/volumes/" + dataset + "/"; // Adjust path as necessary
    # else
        path = RESOURCE_PATH("../resources/volumes/" + dataset + "/");
    #endif
    
        int filesloaded = image->loadFromFiles("resources/volumes/tooth/tooth0", "png", 490);
        if (filesloaded == 0) {
    #if defined(_MSVC)
            filesloaded = image->loadFromFiles("../resources/volumes/tooth/tooth0", "png", 490);
    #endif
        }
    
    
    
        if (filesloaded == 0) {
            cout << "Error - Failed to load volume data tooth" << endl;
            close();
            return -1;
        }
    
    
        // create texture
        texture = cTexture3d::create();
    
        // assign volumetric image to texture
        texture->setImage(image);
    
        // assign texture to voxel object
        object->setTexture(texture);
    
        // initially select an isosurface corresponding to the bone/heart level
        object->setIsosurfaceValue(0.2);

        //object->setQuality(1);

        // set quality of graphic rendering
        object->setQuality(1);
    
        // set optical density factor
        object->setOpticalDensity(1.2);

        object->m_material->setStiffness(0.5 * maxStiffness);
    
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
            return -1;
        }
    
        /*softLUT = cImage::create();
        fileLoaded = softLUT->loadFromFile(RESOURCE_PATH("../resources/volumes/heart/colormap_soft.png"));
        if (!fileLoaded) {
    #if defined(_MSVC)
            fileLoaded = softLUT->loadFromFile("../../../bin/resources/volumes/heart/colormap_soft.png");
    #endif
        }
        if (!fileLoaded)
        {
            cout << "Error - Failed to load colormap." << endl;
            close();
            return -1;
        }*/
    
        // tell the voxel object to load the colour look-up table as a texture
        object->m_colorMap->setImage(boneLUT);
    
        //object->setRenderingModeDVRColorMap();            // high quality
        //object->setRenderingModeBasic();
        
        //object->scale(0.01);
        //object->scale(0.01);
        object->setRenderingModeIsosurfaceColorMap();
    
        object->setShowEdges(false);
        object->clearAllEdges();
        object->clear();
        object->clearAllChildren();
        object->setUseLinearInterpolation(true);
        object->createAABBCollisionDetector(toolRadius);

        //object->setUseMaterial(false);
        //object->setUseTransparency(false);


    //--------------------------------------------------------------------------
    // START SIMULATION
    //--------------------------------------------------------------------------

    // create a thread which starts the main haptics rendering loop
    hapticsThread = new cThread();
    hapticsThread->start(updateHaptics, CTHREAD_PRIORITY_HAPTICS);

    // setup callback when application exits
    atexit(close);


    //--------------------------------------------------------------------------
    // MAIN GRAPHIC LOOP
    //--------------------------------------------------------------------------

    // recenter oculus
    oculusVR.recenterPose();

    // main graphic rendering loop
    while (!glfwWindowShouldClose(window) && !simulationFinished)
    {
        // handle key presses
        //processEvents();

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        // start rendering
        oculusVR.onRenderStart();

        // render frame for each eye
        for (int eyeIndex = 0; eyeIndex < ovrEye_Count; eyeIndex++)
        {
            // retrieve projection and modelview matrix from oculus
            cTransform projectionMatrix, modelViewMatrix;
            oculusVR.onEyeRender(eyeIndex, projectionMatrix, modelViewMatrix);

            camera->m_useCustomProjectionMatrix = true;
            camera->m_projectionMatrix = projectionMatrix;

            camera->m_useCustomModelViewMatrix = true;
            camera->m_modelViewMatrix = modelViewMatrix;

            // render world
            ovrSizei size = oculusVR.getEyeTextureSize(eyeIndex);
            camera->renderView(size.w, size.h, C_STEREO_LEFT_EYE, false);

            // finalize rendering  
            oculusVR.onEyeRenderFinish(eyeIndex);


            /////////////////////////////////////////////////////////////////////
            // VOLUME UPDATE
            /////////////////////////////////////////////////////////////////////

            // update region of voxels to be updated
            if (flagMarkVolumeForUpdate)
            {
                mutexVoxel.acquire();
                cVector3d min = volumeUpdate.m_min;
                cVector3d max = volumeUpdate.m_max;
                volumeUpdate.setEmpty();
                mutexVoxel.release();
                texture->markForPartialUpdate(min, max);
                flagMarkVolumeForUpdate = false;
            }
        }

        // update frames
        oculusVR.submitFrame();
        oculusVR.blitMirror();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // cleanup
    oculusVR.destroyVR();
    renderContext.destroy();

    glfwDestroyWindow(window);

    // exit glfw
    glfwTerminate();

    return (0);
}

//------------------------------------------------------------------------------

void errorCallback(int a_error, const char* a_description)
{
    cout << "Error: " << a_description << endl;
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

    // option - spacebar
    else if (a_key == GLFW_KEY_SPACE)
    {
        oculusVR.recenterPose();
    }

    else if (a_key == GLFW_KEY_G)
    {
        object->setGhostEnabled(!object->getGhostEnabled());
    }
}

//------------------------------------------------------------------------------

void close(void)
{
    // stop the simulation
    simulationRunning = false;

    // wait for graphics and haptics loops to terminate
    while (!simulationFinished) { cSleepMs(100); }

    // close haptic device
    tool->stop();

    // delete resources
    delete hapticsThread;
    delete world;
    delete handler;
}

//------------------------------------------------------------------------------

void updateHaptics(void)
{
    // reset clock
    cPrecisionClock clock;
    clock.reset();

    // simulation in now running
    simulationRunning = true;
    simulationFinished = false;

    // main haptic simulation loop
    while (simulationRunning)
    {
        /////////////////////////////////////////////////////////////////////
        // SIMULATION TIME
        /////////////////////////////////////////////////////////////////////

        // stop the simulation clock
        clock.stop();

        // read the time increment in seconds
        double timeInterval = clock.getCurrentTimeSeconds();

        // restart the simulation clock
        clock.reset();
        clock.start();

        // update frequency counter
        frequencyCounter.signal(1);


        /////////////////////////////////////////////////////////////////////
        // HAPTIC FORCE COMPUTATION
        /////////////////////////////////////////////////////////////////////

        // compute global reference frames for each object
        world->computeGlobalPositions(true);

        // update position and orientation of tool
        tool->updateFromDevice();

        // read user switch
        int userSwitches = tool->getUserSwitches();

        // acquire mutex
        if (mutexObject.tryAcquire())
        {
            // compute interaction forces
            tool->computeInteractionForces();

            // check if tool is in contact with voxel object
            if (tool->isInContact(object) && (userSwitches > 0))
            {
                // retrieve contact event
                cCollisionEvent* contact = tool->m_hapticPoint->getCollisionEvent(0);

                // update voxel color
                cColorb color(0x00, 0x00, 0x00, 0x00);
                object->m_texture->m_image->setVoxelColor(contact->m_voxelIndexX, contact->m_voxelIndexY, contact->m_voxelIndexZ, color);

                // mark voxel for update
                mutexVoxel.acquire();
                volumeUpdate.enclose(cVector3d(contact->m_voxelIndexX, contact->m_voxelIndexY, contact->m_voxelIndexZ));
                mutexVoxel.release();
                flagMarkVolumeForUpdate = true;
            }

            // release mutex
            mutexObject.release();
        }

        // send forces to haptic device
        tool->applyToDevice();
    }

    // exit haptics thread
    simulationFinished = true;
}

//------------------------------------------------------------------------------
