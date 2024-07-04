//==============================================================================
/*
    Bi-manual fossil demo

*/
//==============================================================================

//------------------------------------------------------------------------------
#include "chai3d.h"
//------------------------------------------------------------------------------
#include <GLFW/glfw3.h>


#include <windows.h>
#include <iostream>
#include <string>
#include <tinyfiledialogs.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

//------------------------------------------------------------------------------
//#include "COculus.h"
//------------------------------------------------------------------------------

using namespace chai3d;
using namespace std;
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// GENERAL SETTINGS
//------------------------------------------------------------------------------

// stereo Mode
/*
    C_STEREO_DISABLED:            Stereo is disabled
    C_STEREO_ACTIVE:              Active stereo for OpenGL NVDIA QUADRO cards
    C_STEREO_PASSIVE_LEFT_RIGHT:  Passive stereo where L/R images are rendered next to each other
    C_STEREO_PASSIVE_TOP_BOTTOM:  Passive stereo where L/R images are rendered above each other
*/
cStereoMode stereoMode = C_STEREO_DISABLED;

// fullscreen mode
bool fullscreen = true;

// mirrored display
bool mirroredDisplay = false;


//------------------------------------------------------------------------------
// STATES
//------------------------------------------------------------------------------
enum MouseStates
{
    MOUSE_IDLE,
    MOUSE_MOVE_CAMERA
};

enum HapticStates
{
    HAPTIC_IDLE,
    HAPTIC_SELECTION
};

// maximum number of devices supported by this application
const int MAX_DEVICES = 16;

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
cImagePtr softLUT;

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

/*

Voxel-Basic Volume Editing Variables

*/

// mutex to object
cMutex mutexObject;

// mutex to voxel
cMutex mutexVoxel;

// region of voxels being updated
cCollisionAABBBox volumeUpdate;

// flag that indicates that voxels have been updated
bool flagMarkVolumeForUpdate = false;

// 3D texture object
cTexture3dPtr texture;
cTexture3dPtr textureFoil;

// Making double maxStiffness global
double maxStiffness;

double stiffnessMultiplier = 0.9;

// Setting-up variables to discover the appropriate rotateExtrinsicEulerAnglesDeg
// Hand

double rotationX = 0;
double rotationY = 0;
double rotationZ = 0;


//double rotationX = 0;
//double rotationY = 0;
//double rotationZ = 0;

double shaderX = 0.0;
double shaderY = 0.0;
double shaderZ = 0.0;

// Custom cursor and multiple devices

// load an object file
bool fileload;

// number of haptic devices 
const int numHapticDevices = 1;

cMultiMesh* drills[numHapticDevices];

cShapeCylinder* stylus;

// Attempt to retrieve voxel color and print it to the label
cVector3d localPos;
cVector3d globalPos;
cVector3d position;
cColorb voxelColor;
int voxelIndexX;
int voxelIndexY;
int voxelIndexZ;
float unroundedX;

cMultiImagePtr image;
cMultiImagePtr imageFoil;

// a virtual object
cMultiMesh* tray;


cHapticDeviceInfo hapticDeviceInfo;

bool topView = true;

// UI Layer and components
cTexture2dPtr sandwichIcon;
cPanel* panel;
cLabel* button1;
cLabel* button2;
cLabel* button3;
cLabel* button4;
cLabel* button5;
cPanel* sandwichButton;
bool isUILayerVisible = true;

// System status panel and components
cPanel* statusPanel;
cLabel* statusMessage;
bool isStatusMessageVisible = false;

double statusPanelDisplayTime = 0.0;
double statusMessageDisplayTime = 0.0;

//------------------------------------------------------------------------------
// OCULUS RIFT
//------------------------------------------------------------------------------

// display context
//cOVRRenderContext renderContext;

// oculus device
//cOVRDevice oculusVR;

// added VR toggle
boolean renderVR = false;


//------------------------------------------------------------------------------
// DECLARED MACROS
//------------------------------------------------------------------------------
// convert to resource path
#define RESOURCE_PATH(p)    (char*)((resourceRoot+string(p)).c_str())


//------------------------------------------------------------------------------
// DECLARED FUNCTIONS
//------------------------------------------------------------------------------

// callback when the window display is resized
void windowSizeCallback(GLFWwindow* a_window, int a_width, int a_height);

// callback when an error GLFW occurs
void errorCallback(int error, const char* a_description);

// callback when a key is pressed
void keyCallback(GLFWwindow* a_window, int a_key, int a_scancode, int a_action, int a_mods);

// callback to handle mouse click
void mouseButtonCallback(GLFWwindow* a_window, int a_button, int a_action, int a_mods);

// callback to handle mouse motion
void mouseMotionCallback(GLFWwindow* a_window, double a_posX, double a_posY);

// callback to handle mouse scroll
void mouseScrollCallback(GLFWwindow* a_window, double a_offsetX, double a_offsetY);

// this function renders the scene
void updateGraphics(void);

// this function contains the main haptics simulation loop
void updateHaptics(void);

// this function closes the application
void close(void);

int countFilesInDirectory(const std::string& path, const std::string& extension);

bool isPointInsideLabel(cLabel* label, double x, double y);

bool isPointInsidePanel(cPanel* panel, double x, double y);

void polygonize();

void showStatusMessageForSeconds(double seconds, const std::string& message);

void toggleStatusMessage(bool on, const std::string& message);

void startPolygonize();

std::string selectFolder();

int main(int argc, char* argv[])
{
    //--------------------------------------------------------------------------
    // INITIALIZATION
    //--------------------------------------------------------------------------

    //cout << endl;
    //cout << "-----------------------------------" << endl;
    //cout << "Keyboard Options:" << endl << endl;
    //cout << "[1] - View bones" << endl;
    //cout << "[2] - View hand" << endl;
    //cout << "[4,5] Adjust slicing along X axis" << endl;
    //cout << "[6,7] Adjust slicing along Y axis" << endl;
    //cout << "[8,9] Adjust slicing along Z axis" << endl;
    //cout << "[l,h] Adjust quality of graphic rendering" << endl;
    //cout << "[p] - Polygonize model and save to file (.stl)" << endl;
    //cout << "[f] - Enable/Disable full screen mode" << endl;
    //cout << "[m] - Enable/Disable vertical mirroring" << endl;
    //cout << "[q] - Exit application" << endl;
    //cout << endl << endl;

    // parse first arg to try and locate resources
    string resourceRoot = string(argv[0]).substr(0, string(argv[0]).find_last_of("/\\") + 1);


    //--------------------------------------------------------------------------
    // OPEN GL - WINDOW DISPLAY
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
    window = glfwCreateWindow(w, h, "HapticMorph - Voxel Sculpting", NULL, NULL);
    if (!window)
    {
        cout << "failed to create window" << endl;
        cSleepMs(1000);
        glfwTerminate();
        return 1;
    }

    // get width and height of window
    glfwGetWindowSize(window, &width, &height);

    // set position of window
    glfwSetWindowPos(window, x, y);

    // set key callback
    glfwSetKeyCallback(window, keyCallback);

    // set resize callback
    glfwSetWindowSizeCallback(window, windowSizeCallback);

    // set mouse position callback
    glfwSetCursorPosCallback(window, mouseMotionCallback);

    // set mouse button callback
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    // set mouse scroll callback
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
        cout << "failed to initialize GLEW library" << endl;
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
    for (int i = 0; i < numHapticDevices; i++) {
        // create a new mesh 
        drills[i] = new cMultiMesh();

        // load a drill like mesh and attach it to the tool
        fileload = drills[i]->loadFromFile("resources/models/drill.3ds");
        if (!fileload)
        {
#if defined(_MSVC)
            fileload = drills[i]->loadFromFile("resources/models/drill.3ds");
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
        //mat.setWhite();
        mat.setRedDark();


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

    cVector3d toolPosLeft(0.3, -0.2, 0);
    cVector3d toolPosRight(0.2, 0.5, 0);

    cVector3d toolPositions[2] = { toolPosLeft, toolPosRight };

    // setup each haptic device
    for (int i = 0; i < numHapticDevices; i++)
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

        //// define a radius for the virtual tool (sphere)
        //tool[i]->setRadius(0.03);

        //tool[i]->m_image->setUseTransparency(true);
        /*tool[i]->m_hapticPoint->m_sphereProxy->setUseTransparency(true, true);
        tool[i]->m_image->setUseTransparency(true, true);*/
        //tool[i]->m_hapticPoint->m_sphereProxy->m_material->setUseTransparency(true);

        //// map the physical workspace of the haptic device to a larger virtual workspace.
        //tool[i]->setWorkspaceRadius(0.55);

        // define a radius for the virtual tool (sphere)
        tool[i]->setRadius(0.04);

        // map the physical workspace of the haptic device to a larger virtual workspace.
        tool[i]->setWorkspaceRadius(0.5);

        // oriente tool with camera
        tool[i]->setLocalRot(camera->getLocalRot());

        //world->addChild(tool[i]);


        //tool[i]->setLocalPos(toolPositions[i]);

        // 
        // haptic forces are enabled only if small forces are first sent to the device;
        // this mode avoids the force spike that occurs when the application starts when 
        // the tool is located inside an object for instance. 
        tool[i]->setWaitForSmallForce(true);


        // start the haptic tool
        tool[i]->start();



        // read the scale factor between the physical workspace of the haptic
        // device and the virtual workspace defined for the tool

    }

    tool[toolOne]->setLocalPos(toolPosLeft);
    tool[toolTwo]->setLocalPos(toolPosRight);
    //tool[0]->m_hapticPoint->initialize(toolZero);
    //tool[1]->m_hapticPoint->initialize(toolOne);


    double workspaceScaleFactor = tool[0]->getWorkspaceScaleFactor();
    // stiffness properties
    maxStiffness = hapticDeviceInfo.m_maxLinearStiffness / workspaceScaleFactor;

    // added from the viscosity example
    double maxDamping = hapticDeviceInfo.m_maxLinearDamping / workspaceScaleFactor;

    double maxLinearForce = cMin(hapticDeviceInfo.m_maxLinearForce, 7.0);




    //--------------------------------------------------------------------------
    // CREATE OBJECT
    //--------------------------------------------------------------------------


    // create a volumetric model
    object = new cVoxelObject();

    int sampleNumber = 1;


    object->setUseCulling(false, false);
    object->m_material->setStiffness(stiffnessMultiplier * maxStiffness);              // % of maximum linear stiffness

    //object->setGhostEnabled(true);


    // add object to world
    world->addChild(object);



    // rotate object
    object->rotateExtrinsicEulerAnglesDeg(rotationX, rotationY, rotationZ, C_EULER_ORDER_XYZ);

    // position object
    object->setLocalPos(-0.05, -0.05, 0.02);

    // set the dimensions by assigning the position of the min and max corners
    /*object->m_minCorner.set(-0.25, -0.25, -0.25);
    object->m_maxCorner.set(0.25, 0.25, 0.25);*/

    // set the dimensions by assigning the position of the min and max corners
    // Hand dataset
    object->m_minCorner.set(-0.5, -0.5, -0.65);
    object->m_maxCorner.set(0.5, 0.5, 0.65);


    //Value for fossil dataset dimensions

    // set the texture coordinate at each corner.
    object->m_minTextureCoord.set(.0, 0.0, 0.0);
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

    string dataset = "tooth";

    //std::string dataset = "tooth";
    std::string path;
#if defined(_MSVC)
    path = "resources/volumes/" + dataset + "/"; // Adjust path as necessary
# else
    path = RESOURCE_PATH("../resources/volumes/" + dataset + "/");
#endif

    int countFiles = countFilesInDirectory(path, ".png");

    int filesloaded = image->loadFromFiles("resources/volumes/" + dataset + "/" + dataset + "0", "png", countFiles);
    if (filesloaded == 0) {
#if defined(_MSVC)
        filesloaded = image->loadFromFiles("../resources/volumes/" + dataset + "/" + dataset + "0", "png", countFiles);
#endif
    }



    if (filesloaded == 0) {
        cout << "Error - Failed to load volume data handXXXX.png." << endl;
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

    // set optical density factor
    object->setOpticalDensity(1.2);

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

    softLUT = cImage::create();
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
    }

    // tell the voxel object to load the colour look-up table as a texture
    object->m_colorMap->setImage(boneLUT);

    object->setRenderingModeDVRColorMap();            // high quality

    object->setShowEdges(false);
    object->clearAllEdges();
    object->clear();
    object->clearAllChildren();
    object->setUseLinearInterpolation(true);


    /*for (int i = 0; i < numHapticDevices; i++)
    {
        world->addChild(tool[i]);
    }*/

    //--------------------------------------------------------------------------
    // WIDGETS
    //--------------------------------------------------------------------------

    // create a font
    font = NEW_CFONTCALIBRI20();

    // create a label to display the haptic and graphic rate of the simulation
    labelRates = new cLabel(font);
    //camera->m_frontLayer->addChild(labelRates);

    // set font color
    //labelRates->m_fontColor.setWhite();

    // create a small message
    labelMessage = new cLabel(font);
    labelMessage->m_fontColor.setWhite();
    //labelMessage->setText("press keys [1,2] to toggle colormap and keys [4-9] to adjust slicing.");
    //camera->m_frontLayer->addChild(labelMessage);

    // create a background
    cColorf backgroundColor;
    backgroundColor.setBlack();
    //backgroundColor.set(0, 177, 64);
    background = new cBackground();
    camera->m_backLayer->addChild(background);
    background->setColor(backgroundColor);

    // Load the sandwich icon
    sandwichIcon = cTexture2d::create();
    bool fileload = sandwichIcon->loadFromFile("resources/images/menu.png");
    if (!fileload)
    {
        // Handle error if icon is not loaded
        cout << "Error - Sandwich icon image failed to load correctly." << endl;
        return -1;
    }

    // Create a panel for UI
    panel = new cPanel();
    camera->m_frontLayer->addChild(panel);
    panel->setLocalPos(10, 0);
    panel->setSize(190, height - 50); // Adjust width and height
    panel->setCornerRadius(10, 10, 10, 10);
    panel->setTransparencyLevel(0.5);
    panel->setColor(cColorf(0.3f, 0.3f, 0.3f, 0.5f)); // semi-transparent background

    // Create button 1 (vertical sequence)
    button1 = new cLabel(font);
    panel->addChild(button1);
    button1->setLocalPos(20, panel->getHeight() - 50); // Adjusted positions
    button1->setText("Load Dataset (L)");
    button1->m_fontColor.setWhite();

    // Create button 2 (vertical sequence)
    button2 = new cLabel(font);
    panel->addChild(button2);
    button2->setLocalPos(20, panel->getHeight() - 100); // Adjusted positions
    button2->setText("Export Model (M)");
    button2->m_fontColor.setWhite();

    // Create button 3 (vertical sequence)
    button3 = new cLabel(font);
    panel->addChild(button3);
    button3->setLocalPos(20, panel->getHeight() - 150); // Adjusted positions
    button3->setText("Export Volume (V)");
    button3->m_fontColor.setWhite();

    // Create button 4 (vertical sequence)
    button4 = new cLabel(font);
    panel->addChild(button4);
    button4->setLocalPos(20, panel->getHeight() - 200); // Adjusted positions
    button4->setText("Toggle Ghost Mode (Space)");
    button4->m_fontColor.setWhite();

    button5 = new cLabel(font);
    panel->addChild(button5);
    button5->setLocalPos(20, panel->getHeight() - 250); // Adjusted positions
    button5->setText("Toggle Haptics (H)");
    button5->m_fontColor.setWhite();

    // Create sandwich button to toggle panel visibility
    sandwichButton = new cPanel();
    camera->m_frontLayer->addChild(sandwichButton);
    sandwichButton->setLocalPos(10, height - 50); // Position near top left corner
    sandwichButton->setSize(30, 30); // Set size of the button
    sandwichButton->setCornerRadius(5, 5, 5, 5);
    sandwichButton->setTexture(sandwichIcon);
    sandwichButton->setUseTexture(true);

    //// Create a system status panel
    //statusPanel = new cPanel();
    //camera->m_frontLayer->addChild(statusPanel);
    //statusPanel->setSize(200, 50); // Adjust width and height
    //statusPanel->setCornerRadius(10, 10, 10, 10);
    //statusPanel->setLocalPos((int)(0.5 * (width - statusPanel->getWidth())), 40);
    //statusPanel->setTransparencyLevel(0.5);
    //statusPanel->setColor(cColorf(0.3f, 0.3f, 0.3f, 0.5f)); // semi-transparent background
    //statusPanel->setShowEnabled(isStatusPanelVisible);

    statusMessage = new cLabel(font);
    camera->m_frontLayer->addChild(statusMessage);
    statusMessage->m_fontColor.setWhite();
    statusMessage->setText("System Status");

    // Center the status message horizontally
    statusMessage->setLocalPos((width - statusMessage->getWidth()) / 2, 15);
    statusMessage->setShowEnabled(isStatusMessageVisible);

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

    // option - render bone and heart
    else if (a_key == GLFW_KEY_1)
    {
        object->m_colorMap->setImage(boneLUT);
        object->setIsosurfaceValue(0.32);
        object->m_material->setDynamicFriction(0.4);
        object->m_material->setStiffness(0.9 * maxStiffness);
        cout << "> Isosurface set to " << cStr(object->getIsosurfaceValue(), 3) << "                            \r";
    }

    // option - render soft tissue
    else if (a_key == GLFW_KEY_2)
    {
        object->m_colorMap->setImage(softLUT);
        object->m_material->setDynamicFriction(0);
        object->setIsosurfaceValue(0.18);
        object->m_material->setStiffness(0.2 * maxStiffness);
        cout << "> Isosurface set to " << cStr(object->getIsosurfaceValue(), 3) << "                            \r";
    }

    // Lucas: added isosurface changes
    else if (a_key == GLFW_KEY_DOWN)
    {
        /*if (stiffnessMultiplier > 0.1)
        {
            stiffnessMultiplier -= 0.1;
            object->m_material->setStiffness(stiffnessMultiplier * maxStiffness);
        }

        cout << "> Stiffness set to " << cStr(stiffnessMultiplier, 3) << "                            \r";*/
        cVector3d currentPosition = object->getLocalPos();

        //// Add 0.1 to the X-axis component
        currentPosition.sub(cVector3d(0.0, 0.0, 0.01));

        //// Set the updated local position
        object->setLocalPos(currentPosition);

        cout << "> object set to " << cStr(currentPosition.x()) << ", " << cStr(currentPosition.y()) << ", " << cStr(currentPosition.z()) << ",                             \r";
        /*shaderY -= 0.1;
        object->setShaderVars(shaderX, shaderY, shaderZ);*/
    }

    // Lucas: added isosurface changes
    else if (a_key == GLFW_KEY_UP)
    {
        /*if (stiffnessMultiplier < 1)
        {
            stiffnessMultiplier += 0.1;
            object->m_material->setStiffness(stiffnessMultiplier * maxStiffness);
        }*/

        //cout << "> Stiffness set to " << cStr(stiffnessMultiplier, 3) << "                            \r";

        cVector3d currentPosition = object->getLocalPos();

        //// Add 0.1 to the X-axis component
        currentPosition.add(cVector3d(0.0, 0.0, 0.01));

        //// Set the updated local position
        object->setLocalPos(currentPosition);

        cout << "> object set to " << cStr(currentPosition.x()) << ", " << cStr(currentPosition.y()) << ", " << cStr(currentPosition.z()) << ",                             \r";

        /*shaderY += 0.1;
        object->setShaderVars(shaderX, shaderY, shaderZ);
        */
    }

    // Lucas: added isosurface changes
    else if (a_key == GLFW_KEY_LEFT)
    {
        /*object->setIsosurfaceValue(object->getIsosurfaceValue() - 0.01);
        cout << "> Isosurface set to " << cStr(object->getIsosurfaceValue(), 3) << "                            \r";*/
        //float tempPos = object->getGlobalPos
        cVector3d currentPosition = object->getLocalPos();

        //// Add 0.1 to the X-axis component
        currentPosition.sub(cVector3d(0.0, 0.01, 0.0));

        //// Set the updated local position
        object->setLocalPos(currentPosition);

        cout << "> object set to " << cStr(currentPosition.x()) << ", " << cStr(currentPosition.y()) << ", " << cStr(currentPosition.z()) << ",                             \r";

        /*shaderX -= 0.1;
        object->setShaderVars(shaderX, shaderY, shaderZ);*/
    }

    // Lucas: added isosurface changes
    else if (a_key == GLFW_KEY_RIGHT)
    {
        /*object->setIsosurfaceValue(object->getIsosurfaceValue() + 0.01);
        cout << "> Isosurface set to " << cStr(object->getIsosurfaceValue(), 3) << "                            \r";*/
        cVector3d currentPosition = object->getLocalPos();

        // Add 0.1 to the X-axis component
        currentPosition.add(cVector3d(0.0, 0.01, 0.0));

        // Set the updated local position
        object->setLocalPos(currentPosition);

        cout << "> object set to " << cStr(currentPosition.x()) << ", " << cStr(currentPosition.y()) << ", " << cStr(currentPosition.z()) << ",                             \r";


        /*shaderX += 0.1;
        object->setShaderVars(shaderX, shaderY, shaderZ);*/
    }
    else if (a_key == GLFW_KEY_A)
    {

        cVector3d currentPosition = object->getLocalPos();

        // Add 0.1 to the X-axis component
        currentPosition.add(cVector3d(0.01, 0.0, 0.0));

        // Set the updated local position
        object->setLocalPos(currentPosition);

        cout << "> object set to " << cStr(currentPosition.x()) << ", " << cStr(currentPosition.y()) << ", " << cStr(currentPosition.z()) << ",                             \r";
        /*shaderZ += 0.1;
        object->setShaderVars(shaderX, shaderY, shaderZ);*/
    }
    else if (a_key == GLFW_KEY_Z)
    {
        cVector3d currentPosition = object->getLocalPos();

        //// Add 0.1 to the X-axis component
        currentPosition.sub(cVector3d(0.01, 0.0, 0.0));

        //// Set the updated local position
        object->setLocalPos(currentPosition);

        cout << "> object set to " << cStr(currentPosition.x()) << ", " << cStr(currentPosition.y()) << ", " << cStr(currentPosition.z()) << ",                             \r";

        /*shaderZ -= 0.1;
        object->setShaderVars(shaderX, shaderY, shaderZ);*/
    }

    // option - reduce size along X axis
    else if (a_key == GLFW_KEY_4)
    {
        double value = cClamp((object->m_maxCorner.x() - 0.005), 0.01, 0.5);
        object->m_maxCorner.x(value);
        object->m_minCorner.x(-value);
        object->m_maxTextureCoord.x(0.5 + value);
        object->m_minTextureCoord.x(0.5 - value);
        cout << "> Reduce size along X axis.                            \r";
    }

    // option - increase size along X axis
    else if (a_key == GLFW_KEY_5)
    {
        double value = cClamp((object->m_maxCorner.x() + 0.005), 0.01, 0.5);
        object->m_maxCorner.x(value);
        object->m_minCorner.x(-value);
        object->m_maxTextureCoord.x(0.5 + value);
        object->m_minTextureCoord.x(0.5 - value);
        cout << "> Increase size along X axis.                            \r";
    }

    // option - reduce size along Y axis
    else if (a_key == GLFW_KEY_6)
    {
        double value = cClamp((object->m_maxCorner.y() - 0.005), 0.01, 0.5);
        object->m_maxCorner.y(value);
        object->m_minCorner.y(-value);
        object->m_maxTextureCoord.y(0.5 + value);
        object->m_minTextureCoord.y(0.5 - value);
        cout << "> Reduce size along Y axis.                            \r";
    }

    // option - increase size along Y axis
    else if (a_key == GLFW_KEY_7)
    {
        double value = cClamp((object->m_maxCorner.y() + 0.005), 0.01, 0.5);
        object->m_maxCorner.y(value);
        object->m_minCorner.y(-value);
        object->m_maxTextureCoord.y(0.5 + value);
        object->m_minTextureCoord.y(0.5 - value);
        cout << "> Increase size along Y axis.                            \r";
    }

    // option - reduce size along Z axis
    else if (a_key == GLFW_KEY_8)
    {
        double value = cClamp((object->m_maxCorner.z() - 0.005), 0.01, 0.5);
        object->m_maxCorner.z(value);
        object->m_minCorner.z(-value);
        object->m_maxTextureCoord.z(0.5 + value);
        object->m_minTextureCoord.z(0.5 - value);
        cout << "> Reduce size along Z axis.                            \r";
    }

    // option - increase size along Z axis
    else if (a_key == GLFW_KEY_9)
    {
        double value = cClamp((object->m_maxCorner.z() + 0.005), 0.01, 0.5);
        object->m_maxCorner.z(value);
        object->m_minCorner.z(-value);
        object->m_maxTextureCoord.z(0.5 + value);
        object->m_minTextureCoord.z(0.5 - value);
        cout << "> Increase size along Z axis.                            \r";
    }
    // option - decrease quality of graphic rendering
    else if (a_key == GLFW_KEY_L)
    {
        double value = object->getQuality();
        object->setQuality(value - 0.1);
        cout << "> Quality set to " << cStr(object->getQuality(), 1) << "                            \r";
    }

    // option - increase quality of graphic rendering
    else if (a_key == GLFW_KEY_H)
    {
        double value = object->getQuality();
        object->setQuality(value + 0.1);
        cout << "> Quality set to " << cStr(object->getQuality(), 1) << "                            \r";
    }

    // option - polygonize model and save to file
    else if (a_key == GLFW_KEY_P)
    {
        startPolygonize();
    }

    // option - toggle fullscreen
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

    // option - toggle vertical mirroring
    else if (a_key == GLFW_KEY_M)
    {
        mirroredDisplay = !mirroredDisplay;
        camera->setMirrorVertical(mirroredDisplay);
    }
    else if (a_key == GLFW_KEY_G)
    {
        object->setGhostEnabled(!object->getGhostEnabled());
    }

    else if (a_key == GLFW_KEY_KP_1)
    {
        rotationX = rotationX - 1;
        object->rotateExtrinsicEulerAnglesDeg(rotationX, rotationY, rotationZ, C_EULER_ORDER_XYZ);
        cout << "> Rotation set to " << cStr(rotationX, 3) << "," << cStr(rotationY, 3) << "," << cStr(rotationZ, 3) << " \r";
    }

    else if (a_key == GLFW_KEY_KP_2)
    {
        rotationY = rotationY - 1;
        object->rotateExtrinsicEulerAnglesDeg(rotationX, rotationY, rotationZ, C_EULER_ORDER_XYZ);
        cout << "> Rotation set to " << cStr(rotationX, 3) << "," << cStr(rotationY, 3) << "," << cStr(rotationZ, 3) << " \r";
    }

    else if (a_key == GLFW_KEY_KP_3)
    {
        rotationZ = rotationZ - 1;
        object->rotateExtrinsicEulerAnglesDeg(rotationX, rotationY, rotationZ, C_EULER_ORDER_XYZ);
        cout << "> Rotation set to " << cStr(rotationX, 3) << "," << cStr(rotationY, 3) << "," << cStr(rotationZ, 3) << " \r";
    }

    else if (a_key == GLFW_KEY_KP_7)
    {
        rotationX = rotationX + 1;
        object->rotateExtrinsicEulerAnglesDeg(rotationX, rotationY, rotationZ, C_EULER_ORDER_XYZ);
        cout << "> Rotation set to " << cStr(rotationX, 3) << "," << cStr(rotationY, 3) << "," << cStr(rotationZ, 3) << " \r";
    }

    else if (a_key == GLFW_KEY_KP_8)
    {
        rotationY = rotationY + 1;
        object->rotateExtrinsicEulerAnglesDeg(rotationX, rotationY, rotationZ, C_EULER_ORDER_XYZ);
        cout << "> Rotation set to " << cStr(rotationX, 3) << "," << cStr(rotationY, 3) << "," << cStr(rotationZ, 3) << " \r";
    }

    else if (a_key == GLFW_KEY_KP_9)
    {
        rotationZ = rotationZ + 1;
        object->rotateExtrinsicEulerAnglesDeg(rotationX, rotationY, rotationZ, C_EULER_ORDER_XYZ);
        cout << "> Rotation set to " << cStr(rotationX, 3) << "," << cStr(rotationY, 3) << "," << cStr(rotationZ, 3) << " \r";
    }

    else if (a_key == GLFW_KEY_SPACE)
    {
        object->setGhostEnabled(!object->getGhostEnabled());
    }

    //cout << cStr(rotationX) << "," << cStr(rotationY) << "," << cStr(rotationZ) << "," << "                            \r";
    // option - save voxel data to disk
    else if (a_key == GLFW_KEY_S)
    {
        mutexObject.acquire();
        image->saveToFiles("output/volumes/volume", "png");
        mutexObject.release();
        cout << "> Volume image saved to disk                       \r";
    }
    else if (a_key == GLFW_KEY_TAB)
    {
        isUILayerVisible = !isUILayerVisible;
        panel->setShowEnabled(isUILayerVisible);
        toggleStatusMessage(isUILayerVisible, "Menu");
    }

    /*else if (a_key == GLFW_KEY_O)
    {
        toolOne = 0;
        toolTwo = 1;
    }

    else if (a_key == GLFW_KEY_P)
    {
        toolOne = 1;
        toolTwo = 0;
    }*/


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

        // Check if button 1 was clicked
        else if (isPointInsideLabel(button1, xpos, ypos)) {
            selectFolder();
        }

        // Check if button 2 was clicked
        else if (isPointInsideLabel(button2, xpos, ypos)) {
            cout << "Button 2 clicked" << endl;
        }

        // Check if button 3 was clicked
        else if (isPointInsideLabel(button3, xpos, ypos)) {
            cout << "Button 3 clicked" << endl;
        }

        // Check if button 4 was clicked
        else if (isPointInsideLabel(button4, xpos, ypos)) {
            cout << "Button 4 clicked" << endl;
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

//------------------------------------------------------------------------------

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

        cout << azimuthDeg << endl;
        cout << polarDeg << endl;

        for (int i = 0; i < numHapticDevices; i++)
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
    r = cClamp(r + 0.1 * a_offsetY, 0.5, 3.0);
    camera->setSphericalRadius(r);
}

//------------------------------------------------------------------------------


int countFilesInDirectory(const std::string& path, const std::string& extension) {
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

    for (int i = 0; i < numHapticDevices; i++)
    {
        // close haptic device
        tool[i]->stop();
    }


    // delete resources
    delete hapticsThread;
    delete polyTaskThread;
    delete world;
    delete handler;
}

//------------------------------------------------------------------------------

void updateGraphics(void)
{
    /////////////////////////////////////////////////////////////////////
    // UPDATE WIDGETS
    /////////////////////////////////////////////////////////////////////



    // update position of label
    //labelRates->setLocalPos((int)(0.5 * (width - labelRates->getWidth())), 15);

    // update position of message label
    //labelMessage->setLocalPos((int)(0.5 * (width - labelMessage->getWidth())), 40);


    //labelMessage->setText(cStr(unroundedX));
    //labelMessage->setText(cStr(voxelIndexX));
    //labelMessage->setText("X: " + cStr(voxelIndexX) + "Y: " + cStr(voxelIndexY) + "Z: " + cStr(voxelIndexZ) + " R: " + cStr(voxelColor.getR()));
    //labelMessage->setText(cStr(position.get(0)));

    // update haptic and graphic rate data
    //labelRates->setText(cStr(freqCounterGraphics.getFrequency(), 0) + " Hz / " +
        //cStr(freqCounterHaptics.getFrequency(), 0) + " Hz");

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
    if (err != GL_NO_ERROR) cout << "Error: " << gluErrorString(err) << endl;
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

        /*for (int i = 0; i < numHapticDevices; i++)
        {*/
        int i = 0;
        // update position and orientation of tool
        tool[i]->updateFromDevice();

        hapticDevice[i]->getPosition(position);

        //tool[i]->computeInteractionForces();

        //// send forces to haptic device
        //tool[i]->applyToDevice();
    //}

    // read user switch
        int userSwitches = tool[toolTwo]->getUserSwitches();

        // acquire mutex
        if (mutexObject.tryAcquire())
        {
            // compute interaction forces
            tool[toolTwo]->computeInteractionForces();

            // check if tool is in contact with voxel object
            if (tool[toolTwo]->isInContact(object) && (userSwitches > 0))
            {
                // retrieve contact event
                cCollisionEvent* contact = tool[toolTwo]->m_hapticPoint->getCollisionEvent(0);

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

        tool[i]->computeInteractionForces();

        // send forces to haptic device
        tool[i]->applyToDevice();

    }

    // exit haptics thread
    simulationFinished = true;
}

//------------------------------------------------------------------------------

bool isPointInsideLabel(cLabel* label, double x, double y)
{
    double labelX = label->getLocalPos().x();
    double labelY = label->getLocalPos().y();
    double labelWidth = label->getWidth();
    double labelHeight = label->getHeight();

    return (x >= labelX && x <= labelX + labelWidth &&
        y >= labelY && y <= labelY + labelHeight);
}

bool isPointInsidePanel(cPanel* panel, double x, double y)
{
    double panelX = panel->getLocalPos().x();
    double panelY = panel->getLocalPos().y();
    double panelWidth = panel->getWidth();
    double panelHeight = panel->getHeight();

    return (x >= panelX && x <= panelX + panelWidth &&
        y >= panelY && y <= panelY + panelHeight);
}

void startPolygonize()
{
    toggleStatusMessage(true, "Exporting Object...");
    glfwPollEvents();
    updateGraphics();

    polyTaskThread = new cThread();
    polyTaskThread->start(polygonize, CTHREAD_PRIORITY_GRAPHICS);
}

void polygonize()
{
    

    cMultiMesh* surface = new cMultiMesh;
    object->polygonize(surface, 0.004, 0.004, 0.010);
    double SCALE = 0.1;
    double METERS_TO_MILLIMETERS = 1000.0;
    surface->scale(SCALE * METERS_TO_MILLIMETERS);
    surface->saveToFile("output/models/model.stl");
    toggleStatusMessage(false,"");
    showStatusMessageForSeconds(3.0, "Object Exported");
    delete surface;
}



void showStatusMessageForSeconds(double seconds, const std::string& message)
{
    isStatusMessageVisible = true;
    statusMessageDisplayTime = glfwGetTime() + seconds;
    statusMessage->setText(message);
    // Center the status message horizontally
    statusMessage->setLocalPos((width - statusMessage->getWidth()) / 2, 15);
    statusMessage->setShowEnabled(true);
}

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