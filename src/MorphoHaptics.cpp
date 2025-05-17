//==============================================================================
/*
    MorphoHaptics: An Open-Source Tool for Visuohaptic Exploration of Morphological Image Datasets - Siqueira Rodrigues et al.
    This project was presented at the ACM KUI 2024 Conference in Florence, Italy
*/
//==============================================================================
#include "MorphoHaptics.h"
//------------------------------------------------------------------------------
#include "chai3d.h"

//------------------------------------------------------------------------------
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <windows.h>
#include <iostream>
#include <string>
#include <tinyfiledialogs.h>
#include "UI/UIUtils.h"
#include "UI/UIActions.h"
#include "UI/InputCallbacks.h"
#include "Utils/Utils.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

using namespace chai3d;

cStereoMode stereoMode = C_STEREO_DISABLED;

// fullscreen mode
bool fullscreen = true;

// mirrored display
bool mirroredDisplay = false;

enum HapticStates
{
    HAPTIC_IDLE,
    HAPTIC_SELECTION
};

//------------------------------------------------------------------------------
// DECLARED CONSTANTS
//------------------------------------------------------------------------------
// 

// maximum number of devices supported by this application
const int MAX_DEVICES = 16;

// number of haptic devices 
//const int NUM_HAPTIC_DEVICES = 1;

//------------------------------------------------------------------------------
// DECLARED VARIABLES
//------------------------------------------------------------------------------

// a world that contains all objects of the virtual environment
cWorld* world;

// a camera to render the world in the window display
cCamera* camera;

// a light source to illuminate the objects in the world
cDirectionalLight* light;

// a haptic device handler
cHapticDeviceHandler* handler;

// a pointer to the current haptic device
cGenericHapticDevicePtr hapticDevice[MAX_DEVICES];

// labels to display each haptic device model
cLabel* labelHapticDeviceModel[MAX_DEVICES];

// a virtual tool representing the haptic device in the scene
cToolCursor* tool[MAX_DEVICES];

// a sphere to show the projected point on the surface
//cShapeSphere* cursor;
cMultiMesh* cursor[MAX_DEVICES];

// a virtual heart object from a CT scan
cVoxelObject* object;

// color lookup tables for the volume
cImagePtr boneLUT;

// angular velocity of object
cVector3d angVel(0.0, 0.0, 0.1);

// a colored background
cBackground* background;

// a font for rendering text
cFontPtr font;

// a label to display the rate [Hz] at which the simulation is running
cLabel* labelRates;

// a label to explain what is happening
cLabel* labelMessage;

// a flag that indicates if the haptic simulation is currently running
bool simulationRunning = false;

// a flag that indicates if the haptic simulation has terminated
bool simulationFinished = true;

// a frequency counter to measure the simulation graphic rate
cFrequencyCounter freqCounterGraphics;

// a frequency counter to measure the simulation haptic rate
cFrequencyCounter freqCounterHaptics;

// mouse state
MouseStates mouseState = MOUSE_IDLE;

// last mouse position
double mouseX, mouseY;

// haptic thread
cThread* hapticsThread;
cThread* polyTaskThread;
cThread* exportVolumeThread;

// a handle to window display context
GLFWwindow* window = NULL;

// current width of window
int width = 0;

// current height of window
int height = 0;

// swap interval for the display context (vertical synchronization)
int swapInterval = 1;

cMaterial mat;
cMaterial transparentMat;

int toolOne = 0;
int toolTwo = 0;

cMutex mutexObject;

cMutex mutexVoxel;

// region of voxels being updated
cCollisionAABBBox volumeUpdate;

// flag that indicates that voxels have been updated
bool flagMarkVolumeForUpdate = false;

// 3D texture object
cTexture3dPtr texture;

double maxStiffness;
double stiffnessMultiplier = 0.9;

double rotationX = 0;
double rotationY = 0;
double rotationZ = 0;

// Custom cursor and multiple devices

// load an object file
bool fileload;

cMultiMesh* drills[NUM_HAPTIC_DEVICES];

cVector3d position;

cMultiImagePtr image;

cHapticDeviceInfo hapticDeviceInfo;

// UI Layer and components
cTexture2dPtr sandwichIcon;
cPanel* panel;
cLabel* button1, *button2, *button3, *button4, *button5, *button6, *button7, *button8, *button9, *button10, *button11, *button12, *button13;
cPanel* sandwichButton;
bool isUILayerVisible = true;

// System status panel and components
cPanel* statusPanel;
cLabel* statusMessage;
bool isStatusMessageVisible = false;

double statusPanelDisplayTime = 0.0;
double statusMessageDisplayTime = 0.0;

bool isHapticsEnabled = true;
bool isSculptingEnabled = true;
bool isVoxelValueHapticsEnabled = true;
bool isHapticSmoothingEnabled = true;

float averageVoxelLuminosity;

float toolRadius = 0.023;

cColorb color(0x00, 0x00, 0x00, 0x00);

bool debugging = false;

int textureWidth;
int textureHeight;
int textureDepth;

// Number of voxels included in the calculation of the average luminosity for voxel value haptics
// Initially set to 9, but can be adjusted based on the size of the voxel object
int valueHapticsRadius = 9;
float previousLuminosity = 0.0f;

//------------------------------------------------------------------------------
// DECLARED MACROS
//------------------------------------------------------------------------------
// convert to resource path
#define RESOURCE_PATH(p)    (char*)((resourceRoot+string(p)).c_str())


//------------------------------------------------------------------------------
// DECLARED FUNCTIONS
//------------------------------------------------------------------------------

void windowSizeCallback(GLFWwindow* a_window, int a_width, int a_height);

void errorCallback(int error, const char* a_description);

void updateGraphics(void);

void updateHaptics(void);

void close(void);

int countFilesInDirectory(const  std::string& path, const  std::string& extension);

void polygonize();

void startPolygonize();

std::string selectFolder();

void exportVolume();

void startExportVolume();

void createVoxelObject(cVoxelObject* object, std::string path, char* argv[]);

void loadDataset();

int main(int argc, char* argv[])
{
    //--------------------------------------------------------------------------
    // INITIALIZATION
    //--------------------------------------------------------------------------

    // parse first arg to try and locate resources
    std::string resourceRoot = std::string(argv[0]).substr(0, std::string(argv[0]).find_last_of("/\\") + 1);

    //--------------------------------------------------------------------------
    // OPEN GL - WINDOW DISPLAY
    //--------------------------------------------------------------------------

    // initialize GLFW library
    if (!glfwInit())
    {
        std::cout << "failed initialization" << std::endl;
        cSleepMs(1000);
        return 1;
    }

    // set error callback
    glfwSetErrorCallback(errorCallback);

    // compute desired size of window
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    int w = 1 * mode->height;
    int h = 0.5 * mode->height;
    int x = 0.5 * (mode->width - w);
    int y = 0.5 * (mode->height - h);

    // set OpenGL version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    // set active stereo mode
    if (stereoMode == C_STEREO_ACTIVE)
    {
        glfwWindowHint(GLFW_STEREO, GL_TRUE);
    }
    else
    {
        glfwWindowHint(GLFW_STEREO, GL_FALSE);
    }

    // create display context
    window = glfwCreateWindow(w, h, "MorphoHaptics - Voxel Sculpting", NULL, NULL);
    if (!window)
    {
        std::cout << "failed to create window" << std::endl;
        cSleepMs(1000);
        glfwTerminate();
        return 1;
    }

    glfwGetWindowSize(window, &width, &height);

    glfwSetWindowPos(window, x, y);

    glfwSetKeyCallback(window, keyCallback);

    glfwSetWindowSizeCallback(window, windowSizeCallback);

    glfwSetCursorPosCallback(window, mouseMotionCallback);

    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    glfwSetScrollCallback(window, mouseScrollCallback);

    // set current display context
    glfwMakeContextCurrent(window);

    // set fullscreen or window mode
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    if (fullscreen)
    {
        glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        glfwSwapInterval(swapInterval);
    }

    // sets the swap interval for the current display context
    glfwSwapInterval(swapInterval);

#ifdef GLEW_VERSION
    // initialize GLEW library
    if (glewInit() != GLEW_OK)
    {
        std::cout << "failed to initialize GLEW library" << std::endl;
        glfwTerminate();
        return 1;
    }
#endif

    //--------------------------------------------------------------------------
    // WORLD - CAMERA - LIGHTING
    //--------------------------------------------------------------------------

    // create a new world.
    world = new cWorld();

    // set the background color of the environment
    world->m_backgroundColor.setBlack();

    // create a camera and insert it into the virtual world
    camera = new cCamera(world);
    world->addChild(camera);

    // define a basis in spherical coordinates for the camera
    camera->setSphericalReferences(cVector3d(0, 0, 0),    // origin
        cVector3d(0, 0, 1),    // zenith direction
        cVector3d(1, 0, 0));   // azimuth direction

    camera->setSphericalDeg(2.0,    // spherical coordinate radius
        60,     // spherical coordinate polar angle
        10);    // spherical coordinate azimuth angle

    // set the near and far clipping planes of the camera
    // anything in front or behind these clipping planes will not be rendered
    camera->setClippingPlanes(0.1, 10.0);

    // set stereo mode
    camera->setStereoMode(stereoMode);

    // set stereo eye separation and focal length (applies only if stereo is enabled)
    camera->setStereoEyeSeparation(0.02);
    camera->setStereoFocalLength(2.0);

    // set vertical mirrored display mode
    camera->setMirrorVertical(mirroredDisplay);

    // create a light source
    light = new cDirectionalLight(world);

    // attach light to camera
    camera->addChild(light);

    // enable light source
    light->setEnabled(true);

    // define the direction of the light beam
    light->setDir(-3.0, -0.5, 0.0);

    // create a font
    font = NEW_CFONTCALIBRI20();

    /////////////////////////////////////////////////////////////////////////
    // OBJECT "DRILL"
    /////////////////////////////////////////////////////////////////////////

    // create and attach drill meshes to tools 
    for (int i = 0; i < NUM_HAPTIC_DEVICES; i++) {
        // create a new mesh 
        drills[i] = new cMultiMesh();

        // load a drill like mesh and attach it to the tool
        fileload = drills[i]->loadFromFile("resources/models/drill.3ds");
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
        drills[i]->scale(0.003);

        // remove the collision detector. we do not want to compute any
        // force feedback rendering on the object itself.
        drills[i]->deleteCollisionDetector(true);

        // define a material property for the mesh
        mat.m_ambient.set(0.5f, 0.5f, 0.5f);
        mat.m_diffuse.set(0.8f, 0.8f, 0.8f);
        mat.m_specular.set(1.0f, 1.0f, 1.0f);
        mat.setWhite();

        // Duplicate the material to create a copy
        transparentMat = cMaterial(mat);
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
            std::cout << "Error - Texture image failed to load correctly." << std::endl;
        }

        //apply texture to maskObject
        drills[i]->setTexture(maskTexture);

        //enable texture rendering 
        drills[i]->setUseTexture(true);

        drills[i]->computeAllNormals();

        cColorf colorRed;
        colorRed.setRed();

        drills[i]->setEdgeProperties(3, colorRed);
        drills[i]->setShowEdges(true);
        drills[i]->setEdgeLineWidth(3);

    }

    //--------------------------------------------------------------------------
    // HAPTIC DEVICES / TOOLS
    //--------------------------------------------------------------------------

    // create a haptic device handler
    handler = new cHapticDeviceHandler();

    cVector3d toolPosLeft(0.3, -0.2, 0);
    cVector3d toolPosRight(0.2, 0.5, 0);

    cVector3d toolPositions[2] = { toolPosLeft, toolPosRight };

    // setup each haptic device
    for (int i = 0; i < NUM_HAPTIC_DEVICES; i++)
    {
        // get access to the first available haptic device found
        handler->getDevice(hapticDevice[i], i);

        // retrieve information about the current haptic device
        hapticDeviceInfo = hapticDevice[i]->getSpecifications();

        // create a tool (cursor) and insert into the world
        cursor[i] = drills[i];

        tool[i] = new cToolCursor(world);
        world->addChild(tool[i]);

        // attach scope to tool
        tool[i]->m_image = drills[i];

        // connect the haptic device to the virtual tool
        tool[i]->setHapticDevice(hapticDevice[i]);

        // if the haptic device has a gripper, enable it as a user switch
        hapticDevice[i]->setEnableGripperUserSwitch(true);

        // define a radius for the virtual tool (sphere)
        tool[i]->setRadius(toolRadius);

        // map the physical workspace of the haptic device to a larger virtual workspace.
        tool[i]->setWorkspaceRadius(0.5);

        // oriente tool with camera
        tool[i]->setLocalRot(camera->getLocalRot());

        // haptic forces are enabled only if small forces are first sent to the device;
        // this mode avoids the force spike that occurs when the application starts when 
        // the tool is located inside an object for instance. 
        tool[i]->setWaitForSmallForce(true);

        // start the haptic tool
        tool[i]->start();
    }

    tool[toolOne]->setLocalPos(toolPosLeft);
    tool[toolTwo]->setLocalPos(toolPosRight);

    double workspaceScaleFactor = tool[0]->getWorkspaceScaleFactor();
    // stiffness properties
    maxStiffness = hapticDeviceInfo.m_maxLinearStiffness / workspaceScaleFactor;

    // added from the viscosity example
    double maxDamping = hapticDeviceInfo.m_maxLinearDamping / workspaceScaleFactor;

    double maxLinearForce = cMin(hapticDeviceInfo.m_maxLinearForce, 7.0);

    // create a volumetric model
    object = new cVoxelObject();

    // create a font
    font = NEW_CFONTCALIBRI20();

    // create a label to display the haptic and graphic rate of the simulation
    labelRates = new cLabel(font);
    //camera->m_frontLayer->addChild(labelRates);

    // set font color
    labelRates->m_fontColor.setWhite();

    // create a small message
    labelMessage = new cLabel(font);
    labelMessage->m_fontColor.setWhite();

    // create a background
    cColorf backgroundColor;
    backgroundColor.setBlack();
    background = new cBackground();
    camera->m_backLayer->addChild(background);
    background->setColor(backgroundColor);

    // Load the sandwich icon
    sandwichIcon = cTexture2d::create();
    bool fileload = sandwichIcon->loadFromFile("resources/images/menu.png");
    if (!fileload)
    {
        // Handle error if icon is not loaded
        std::cout << "Error - Sandwich icon image failed to load correctly." << std::endl;
        return -1;
    }

    // Create a panel for UI
    panel = new cPanel();
    camera->m_frontLayer->addChild(panel);
    panel->setLocalPos(10, 0);
    panel->setSize(230, height - 70); // Adjust width and height
    panel->setCornerRadius(10, 10, 10, 10);
    panel->setTransparencyLevel(0.5);
    panel->setColor(cColorf(0.3f, 0.3f, 0.3f, 0.5f)); // semi-transparent background

    createPanelButton(button1, panel, font, "Load Dataset (L)", 50);
    createPanelButton(button2, panel, font, "Export Model (M)", 100);
    createPanelButton(button3, panel, font, "Export Volume (V)", 150);
    createPanelButton(button4, panel, font, "(off) Ghost Mode (Space)", 200);
    createPanelButton(button5, panel, font, "(on) Haptics (H)", 250);
    createPanelButton(button6, panel, font, "Quit (Q)", 650);
    createPanelButton(button7, panel, font, "(on) Sculpting (S)", 300);
    createPanelButton(button8, panel, font, "(on) Voxel-Value Haptics (K)", 350);
    createPanelButton(button9, panel, font, "Increase Probe Radius (+)", 450);
    createPanelButton(button10, panel, font, "Decrease Probe Radius (-)", 500);
    createPanelButton(button11, panel, font, "Increase Voxel Radius (Up)", 550);
    createPanelButton(button12, panel, font, "Decrease Voxel Radius (Down)", 600);
    createPanelButton(button13, panel, font, "(on) Force Smoothing (X)", 400);

    // Displaying it inside the menu to avoid overlapping with system status panel
    panel->addChild(labelRates);
    labelRates->setLocalPos(20, 20);

    // Create sandwich button to toggle panel visibility
    sandwichButton = new cPanel();
    camera->m_frontLayer->addChild(sandwichButton);
    sandwichButton->setLocalPos(10, height - 50); // Position near top left corner
    sandwichButton->setSize(30, 30); // Set size of the button
    sandwichButton->setCornerRadius(5, 5, 5, 5);
    sandwichButton->setTexture(sandwichIcon);
    sandwichButton->setUseTexture(true);

    statusMessage = new cLabel(font);
    camera->m_frontLayer->addChild(statusMessage);
    statusMessage->m_fontColor.setWhite();
    statusMessage->setText("System Status");

    // Center the status message horizontally
    statusMessage->setLocalPos((width - statusMessage->getWidth()) / 2, 15);
    statusMessage->setShowEnabled(isStatusMessageVisible);

    // Added for development purposes
    if (debugging) {
        // Skipping the need to manually load the dataset during development
        createVoxelObject(object, "resources\\volumes\\tooth", argv);
    }

    //--------------------------------------------------------------------------
    // START SIMULATION
    //--------------------------------------------------------------------------

    // create a thread which starts the main haptics rendering loop
    hapticsThread = new cThread();
    hapticsThread->start(updateHaptics, CTHREAD_PRIORITY_HAPTICS);


    //--------------------------------------------------------------------------
    // MAIN GRAPHIC LOOP
    //--------------------------------------------------------------------------

    // call window size callback at initialization
    windowSizeCallback(window, width, height);

    // main graphic loop
    while (!glfwWindowShouldClose(window))
    {
        // get width and height of window
        glfwGetWindowSize(window, &width, &height);

        // render graphics
        updateGraphics();

        // swap buffers
        glfwSwapBuffers(window);

        // process events
        glfwPollEvents();

        // signal frequency counter
        freqCounterGraphics.signal(1);
    }

    // close window
    glfwDestroyWindow(window);

    // terminate GLFW library
    glfwTerminate();

    // exit
    return 0;
}

//------------------------------------------------------------------------------

void windowSizeCallback(GLFWwindow* a_window, int a_width, int a_height)
{
    // update window size
    width = a_width;
    height = a_height;
}

//------------------------------------------------------------------------------

void errorCallback(int a_error, const char* a_description)
{
    std::cout << "Error: " << a_description << std::endl;
}


int countFilesInDirectory(const  std::string& path, const  std::string& extension) {
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile((path + "/*" + extension).c_str(), &findFileData);

    int count = 0;
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                ++count;
            }
        } while (FindNextFile(hFind, &findFileData) != 0);
        FindClose(hFind);
    }
    return count - 1;
}

void close(void)
{
    // stop the simulation
    simulationRunning = false;

    // wait for graphics and haptics loops to terminate
    while (!simulationFinished) { cSleepMs(100); }

    for (int i = 0; i < NUM_HAPTIC_DEVICES; i++)
    {
        // close haptic device
        tool[i]->stop();
    }

    // delete resources
    delete hapticsThread;
    delete polyTaskThread;
    delete exportVolumeThread;
    delete world;
    delete handler;
}

//------------------------------------------------------------------------------

void updateGraphics(void)
{
    // update haptic and graphic rate data
    labelRates->setText(cStr(freqCounterGraphics.getFrequency(), 0) + " Hz / " +
        cStr(freqCounterHaptics.getFrequency(), 0) + " Hz");

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

    // Check if the status message should be hidden (temporary messages)
    if (isStatusMessageVisible && statusMessageDisplayTime > 0 && glfwGetTime() > statusMessageDisplayTime)
    {
        isStatusMessageVisible = false;
        statusMessage->setShowEnabled(false);
        statusMessageDisplayTime = 0; // Reset display time
    }

    /////////////////////////////////////////////////////////////////////
    // RENDER SCENE
    /////////////////////////////////////////////////////////////////////

    // update shadow maps (if any)
    world->updateShadowMaps(false, mirroredDisplay);

    // render world
    camera->renderView(width, height);

    // wait until all GL commands are completed
    glFinish();

    // check for any OpenGL errors
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) std::cout << "Error: " << gluErrorString(err) << std::endl;
}

//------------------------------------------------------------------------------

void updateHaptics(void)
{

    HapticStates state = HAPTIC_IDLE;
    cGenericObject* selectedObject = NULL;
    cTransform tool_T_object;

    // simulation in now running
    simulationRunning = true;
    simulationFinished = false;

    // haptic force activation
    bool flagStart = true;
    int counter = 0;

    // A small threshold value to avoid calculating voxel-value haptics with zero forces
    double threshold = 1e-6; 
    int i = 0;

    // simulation in now running
    simulationRunning = true;
    simulationFinished = false;


    // main haptic simulation loop
    while (simulationRunning)
    {

        // signal frequency counter
        freqCounterHaptics.signal(1);

        /////////////////////////////////////////////////////////////////////
        // HAPTIC FORCE COMPUTATION
        /////////////////////////////////////////////////////////////////////

        // compute global reference frames for each object
        world->computeGlobalPositions(true);
        
        // update position and orientation of tool
        tool[i]->updateFromDevice();

        hapticDevice[i]->getPosition(position);

        // read user switch
        bool isFrontButtonPressed = tool[toolOne]->getUserSwitch(0);

        // Needs to be computed even if there are no haptics as it's needed for collision detection
        tool[i]->computeInteractionForces();

        if (tool[toolTwo]->isInContact(object) && isVoxelValueHapticsEnabled)
        {
            // retrieve contact event
            cCollisionEvent* contact = tool[toolTwo]->m_hapticPoint->getCollisionEvent(0);

            if (isHapticSmoothingEnabled)
            {
                // Calculate new luminosity
                float newLuminosity = getAverageLuminosity(contact->m_voxelIndexX, contact->m_voxelIndexY, contact->m_voxelIndexZ, valueHapticsRadius);

                // Smooth the luminosity value
                averageVoxelLuminosity = smoothAverageLuminosity(newLuminosity, previousLuminosity, 0.1f);
            }
            else
            {
                averageVoxelLuminosity = getAverageLuminosity(contact->m_voxelIndexX, contact->m_voxelIndexY, contact->m_voxelIndexZ, valueHapticsRadius);
            }

            // Update the previous luminosity for the next iteration, whether or not smoothing was activated to prevent hiccups upon activation
            previousLuminosity = averageVoxelLuminosity;

            cVector3d force = tool[i]->getDeviceGlobalForce();

            if (force.length() > threshold) {

                force.mul(averageVoxelLuminosity);

                tool[i]->setDeviceGlobalForce(force);
            }

        }

        if (isHapticsEnabled)
        {
            // send forces to haptic device
            tool[i]->applyToDevice();
        }

        // acquire mutex
        if (mutexObject.tryAcquire())
        {

            // retrieve contact event
            cCollisionEvent* contact = tool[toolTwo]->m_hapticPoint->getCollisionEvent(0);

            // check if tool is in contact with voxel object
            if (tool[toolTwo]->isInContact(object) && isFrontButtonPressed && isSculptingEnabled)
            {
                // update voxel color
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

        /////////////////////////////////////////////////////////////////////////
        // MANIPULATION
        /////////////////////////////////////////////////////////////////////////

        // compute transformation from world to tool (haptic device)
        cTransform world_T_tool = tool[toolOne]->getDeviceGlobalTransform();

        // get status of user switch
        bool button = tool[toolOne]->getUserSwitch(1);

        //
        // STATE 1:
        // Idle mode - user presses the user switch
        //
        if ((state == HAPTIC_IDLE) && (button == true))
        {
            // check if at least one contact has occurred
            if (tool[toolOne]->m_hapticPoint->getNumCollisionEvents() > 0)
            {
                // get contact event
                cCollisionEvent* collisionEvent = tool[toolOne]->m_hapticPoint->getCollisionEvent(0);

                // get object from contact event
                selectedObject = collisionEvent->m_object;
            }
            else
            {
                selectedObject = object;
            }

            // get transformation from object
            cTransform world_T_object = selectedObject->getGlobalTransform();

            // compute inverse transformation from contact point to object 
            cTransform tool_T_world = world_T_tool;
            tool_T_world.invert();

            // store current transformation tool
            tool_T_object = tool_T_world * world_T_object;

            // update state
            state = HAPTIC_SELECTION;
        }


        //
        // STATE 2:
        // Selection mode - operator maintains user switch enabled and moves object
        //
        else if ((state == HAPTIC_SELECTION) && (button == true))
        {
            // compute new transformation of object in global coordinates
            cTransform world_T_object = world_T_tool * tool_T_object;

            // compute new transformation of object in local coordinates
            cTransform parent_T_world = selectedObject->getParent()->getLocalTransform();
            parent_T_world.invert();
            cTransform parent_T_object = parent_T_world * world_T_object;

            // assign new local transformation to object
            selectedObject->setLocalTransform(parent_T_object);

            // set zero forces when manipulating objects
            tool[toolOne]->setDeviceGlobalForce(0.0, 0.0, 0.0);

            tool[toolOne]->initialize();
        }

        //
        // STATE 3:
        // Finalize Selection mode - operator releases user switch.
        //
        else
        {
            state = HAPTIC_IDLE;
        }

    }

    // exit haptics thread
    simulationFinished = true;
}

//------------------------------------------------------------------------------


void startPolygonize()
{
    toggleStatusMessage(true, "Exporting Model...");
    glfwPollEvents();
    updateGraphics();

    polyTaskThread = new cThread();
    polyTaskThread->start(polygonize, CTHREAD_PRIORITY_GRAPHICS);
}

void polygonize()
{
    std::string path = selectFolder();

    std::replace(path.begin(), path.end(), '\\', '/');

    cMultiMesh* surface = new cMultiMesh;
    object->polygonize(surface, 0.01, 0.01, 0.010);
    double SCALE = 0.1;
    double METERS_TO_MILLIMETERS = 1000.0;
    surface->scale(SCALE * METERS_TO_MILLIMETERS);
    surface->saveToFile(path + "/model.stl");
    toggleStatusMessage(false,"");
    showStatusMessageForSeconds(3.0, "Model Exported");
    delete surface;
}

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
        return  std::string(path);
    }
    return  std::string();
}

void exportVolume()
{
    std::string path = selectFolder();

    std::replace(path.begin(), path.end(), '\\', '/');

    mutexObject.acquire();
    image->saveToFiles(path + "/volume", "png");
    mutexObject.release();
    toggleStatusMessage(false, "");
    showStatusMessageForSeconds(3.0, "Volume Exported");
}

void startExportVolume()
{
    toggleStatusMessage(true, "Exporting Volume...");
    glfwPollEvents();
    updateGraphics();

    exportVolumeThread = new cThread();
    exportVolumeThread->start(exportVolume, CTHREAD_PRIORITY_GRAPHICS);
}

void createVoxelObject(cVoxelObject* object, std::string path, char* argv[])
{
    // Arbitrary number that convey three-dimensionality
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
        std::cout << "Error - Failed to load volume data." << std::endl;
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
        std::cout << "Error - Failed to load colormap." << std::endl;
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
    std::string path = selectFolder();
    if (path.empty())
    {
		return;
	}
    else
    {
		createVoxelObject(object, path, nullptr);
	}
}
