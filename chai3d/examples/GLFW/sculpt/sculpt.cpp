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
cLabel* button6;
cLabel* button7;
cLabel* button8;
cLabel* button9;
cLabel* button10;
cLabel* button11;
cLabel* button12;
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

float averageVoxelLuminosity;

float toolRadius = 0.04;

cColorb color(0x00, 0x00, 0x00, 0x00);

bool debugging = true;

int textureWidth;
int textureHeight;
int textureDepth;

// Number of voxels included in the calculation of the average luminosity for voxel value haptics
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

void exportVolume();

void startExportVolume();

void toggleGhostMode();

void toggleHaptics();

void toggleSculpting();

void toggleVirtualReality();

float  calculateLuminosity(const cColorb& color);

float getAverageLuminosity(int centerX, int centerY, int centerZ, int radius);

void createVoxelObject(cVoxelObject* object, string path, char* argv[]);

void loadDataset();

void toggleVoxelValueHaptics();

void updateProbeRadius(int value);

void updateValueHapticsRadius(int value);

float smoothAverageLuminosity(float newLuminosity, float previousLuminosity, float smoothingFactor);

int main(int argc, char* argv[])
{
    //--------------------------------------------------------------------------
    // INITIALIZATION
    //--------------------------------------------------------------------------


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
    window = glfwCreateWindow(w, h, "MorphoHaptics - Voxel Sculpting", NULL, NULL);
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
        tool[i]->setRadius(toolRadius);

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


    //--------------------------------------------------------------------------
    // WIDGETS
    //--------------------------------------------------------------------------

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
    panel->setSize(230, height - 70); // Adjust width and height
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
    button4->setText("(off) Ghost Mode (Space)");
    button4->m_fontColor.setWhite();

    button5 = new cLabel(font);
    panel->addChild(button5);
    button5->setLocalPos(20, panel->getHeight() - 250); // Adjusted positions
    button5->setText("(on) Haptics (H)");
    button5->m_fontColor.setWhite();
    
    button6 = new cLabel(font);
    panel->addChild(button6);
    button6->setLocalPos(20, panel->getHeight() - 600); // Adjusted positions
    button6->setText("Quit (Q)");
    button6->m_fontColor.setWhite();
    
    button7 = new cLabel(font);
    panel->addChild(button7);
    button7->setLocalPos(20, panel->getHeight() - 300); // Adjusted positions
    button7->setText("(on) Sculpting (S)");
    button7->m_fontColor.setWhite();

	button8 = new cLabel(font);
    panel->addChild(button8);
    button8->setLocalPos(20, panel->getHeight() - 350); // Adjusted positions
    button8->setText("(on) Voxel-Value Haptics (K)");
    button8->m_fontColor.setWhite();

    button9 = new cLabel(font);
    panel->addChild(button9);
    button9->setLocalPos(20, panel->getHeight() - 400); // Adjusted positions
    button9->setText("Increase Probe Radius (+)");
    button9->m_fontColor.setWhite();

    button10 = new cLabel(font);
    panel->addChild(button10);
    button10->setLocalPos(20, panel->getHeight() - 450); // Adjusted positions
    button10->setText("Decrease Probe Radius (-)");
    button10->m_fontColor.setWhite();

    button11 = new cLabel(font);
    panel->addChild(button11);
    button11->setLocalPos(20, panel->getHeight() - 500); // Adjusted positions
    button11->setText("Increase Voxel Radius (Up)");
    button11->m_fontColor.setWhite();

    button12 = new cLabel(font);
    panel->addChild(button12);
    button12->setLocalPos(20, panel->getHeight() - 550); // Adjusted positions
    button12->setText("Decrease Voxel Radius (Down)");
    button12->m_fontColor.setWhite();

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

    // Lucas: added isosurface changes
    else if (a_key == GLFW_KEY_DOWN)
    {
        updateValueHapticsRadius(-1);
    }

    // Lucas: added isosurface changes
    else if (a_key == GLFW_KEY_UP)
    {
        updateValueHapticsRadius(1);
    }

    // option - polygonize model and save to file
    else if (a_key == GLFW_KEY_M)
    {
        startPolygonize();
    }

    // option - load dataset
    else if (a_key == GLFW_KEY_L)
    {
		loadDataset();
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

    else if (a_key == GLFW_KEY_R)
    {
		toggleVirtualReality();
	}

    else if (a_key == GLFW_KEY_K)
    {
		toggleVoxelValueHaptics();
	}

    else if (a_key == GLFW_KEY_SPACE)
    {
        toggleGhostMode();
    }

    //cout << cStr(rotationX) << "," << cStr(rotationY) << "," << cStr(rotationZ) << "," << "                            \r";
    // option - save voxel data to disk
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
            loadDataset();
        }

        // Check if button 2 was clicked
        else if (isPointInsideLabel(button2, xpos, ypos)) {
            startPolygonize();
        }

        // Check if button 3 was clicked
        else if (isPointInsideLabel(button3, xpos, ypos)) {
            startExportVolume();
        }

        // Check if button 4 was clicked
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
    delete exportVolumeThread;
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
    labelRates->setText(cStr(freqCounterGraphics.getFrequency(), 0) + " Hz / " +
        cStr(freqCounterHaptics.getFrequency(), 0) + " Hz");

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

        if (isHapticsEnabled)
        {
            tool[i]->computeInteractionForces();

            if (tool[toolTwo]->isInContact(object) && isVoxelValueHapticsEnabled)
            {
                // retrieve contact event
                cCollisionEvent* contact = tool[toolTwo]->m_hapticPoint->getCollisionEvent(0);

                //averageVoxelLuminosity = getAverageLuminosity(contact->m_voxelIndexX, contact->m_voxelIndexY, contact->m_voxelIndexZ, valueHapticsRadius);

                // Calculate new luminosity
                float newLuminosity = getAverageLuminosity(contact->m_voxelIndexX, contact->m_voxelIndexY, contact->m_voxelIndexZ, valueHapticsRadius);

                // Smooth the luminosity value
                averageVoxelLuminosity = smoothAverageLuminosity(newLuminosity, previousLuminosity, 0.1f);

                // Update the previous luminosity for the next iteration
                previousLuminosity = averageVoxelLuminosity;

                cVector3d force = tool[i]->getDeviceGlobalForce();

                if (force.length() > threshold) {

                    force.mul(averageVoxelLuminosity);

                    tool[i]->setDeviceGlobalForce(force);
                }

            }

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
    toggleStatusMessage(true, "Exporting Model...");
    glfwPollEvents();
    updateGraphics();

    polyTaskThread = new cThread();
    polyTaskThread->start(polygonize, CTHREAD_PRIORITY_GRAPHICS);
}

void polygonize()
{
    string path = selectFolder();

    std::replace(path.begin(), path.end(), '\\', '/');

    cMultiMesh* surface = new cMultiMesh;
    object->polygonize(surface, 0.01, 0.01, 0.010);
    double SCALE = 0.1;
    double METERS_TO_MILLIMETERS = 1000.0;
    surface->scale(SCALE * METERS_TO_MILLIMETERS);
    //surface->saveToFile("output/models/model.stl");
    surface->saveToFile(path + "/model.stl");
    toggleStatusMessage(false,"");
    showStatusMessageForSeconds(3.0, "Model Exported");
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

void exportVolume()
{
    string path = selectFolder();

    std::replace(path.begin(), path.end(), '\\', '/');

    mutexObject.acquire();
    //image->saveToFiles("output/volumes/volume", "png");
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

void toggleGhostMode()
{
	object->setGhostEnabled(!object->getGhostEnabled());
    if (object->getGhostEnabled())
    {
		button4->setText("(on) Ghost Mode (Space)");
        button4->m_fontColor.setGreen();
        toggleStatusMessage(true, "Ghost Mode (on)");
	}
    else
    {
		button4->setText("(off) Ghost Mode (Space)");
        button4->m_fontColor.setWhite();
        showStatusMessageForSeconds(3.0, "Ghost Mode (off)");
	}
}

void toggleHaptics()
{
    if (isHapticsEnabled)
    {
		isHapticsEnabled = false;
		button5->setText("(off) Haptics (H)");
        toggleStatusMessage(true, "Haptics (off)");
		button5->m_fontColor.setRed();
	}
    else
    {
		isHapticsEnabled = true;
		button5->setText("(on) Haptics (H)");
        showStatusMessageForSeconds(3.0, "Haptics (on)");
		button5->m_fontColor.setWhite();
	}
}


void createVoxelObject(cVoxelObject* object, string path, char* argv[])
{
    
    rotationX = -23;
    rotationY = 0;
    rotationZ = 93;

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
    object->m_minCorner.set(-0.5, -0.5, -0.5);
    object->m_maxCorner.set(0.5, 0.5, 0.5);
    
    
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
    
    //string dataset = "tooth";
    
    //std::string dataset = "tooth";
    //std::string path;

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
    string path = selectFolder();
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

void toggleVirtualReality()
{

}

// Function to calculate luminosity including alpha to reduce the effect of fully transparent colors
float calculateLuminosity(const cColorb& color) {
    // Normalize the RGB components to floats
    float R = color.getR() / 255.0f;
    float G = color.getG() / 255.0f;
    float B = color.getB() / 255.0f;
    float A = color.getA() / 255.0f; // Normalize alpha component to float

    // Calculate luminosity with alpha consideration
    float luminosity = (0.2126f * R + 0.7152f * G + 0.0722f * B) * A; // Multiply by alpha to reduce luminosity for transparent colors

    return luminosity;
}

float getAverageLuminosity(int centerX, int centerY, int centerZ, int radius) {
    int voxelCount = 0;
    float totalLuminosity = 0;

    for (int x = centerX - radius; x <= centerX + radius; ++x) {
        for (int y = centerY - radius; y <= centerY + radius; ++y) {
            for (int z = centerZ - radius; z <= centerZ + radius; ++z) {
                // Check if the voxel is within the bounds of the texture
                if (x >= 0 && x < textureWidth && y >= 0 && y < textureHeight && z >= 0 && z < textureDepth) {
                    cColorb voxelColor;
                    texture->m_image->getVoxelColor(x, y, z, voxelColor);
                    totalLuminosity += calculateLuminosity(voxelColor);
                    ++voxelCount;
                }
            }
        }
    }

    return voxelCount > 0 ? totalLuminosity / voxelCount : 0;
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

void updateProbeRadius(int value)
{
    if (toolRadius <= 0.023 && value == -1)
    {
        showStatusMessageForSeconds(3.0, "Minimum Probe Size Reached");
		return;
	}
    if (toolRadius >= 0.06 && value == 1)
    {
		showStatusMessageForSeconds(3.0, "Maximum Probe Size Reached");
        return;
    }

    // value can be 1 or -1, so this will increase or decrease the radius by 0.001
    toolRadius += 0.001 * value;
    tool[0]->setRadius(toolRadius);
}

void updateValueHapticsRadius(int value)
{
    if (valueHapticsRadius <= 1 && value == -1)
    {
        showStatusMessageForSeconds(3.0, "Minimum Radius Reached");
        return;
    }

    if (valueHapticsRadius >= 25 && value == 1)
    {
        showStatusMessageForSeconds(3.0, "Maximum Radius Reached");
        return;
    }

    valueHapticsRadius += value;
    showStatusMessageForSeconds(3.0, "Radius updated to " + cStr(valueHapticsRadius) + " voxels");
    
}

// Smoothing function
float smoothAverageLuminosity(float newLuminosity, float previousLuminosity, float smoothingFactor = 0.1f) {
    return previousLuminosity * (1.0f - smoothingFactor) + newLuminosity * smoothingFactor;
}