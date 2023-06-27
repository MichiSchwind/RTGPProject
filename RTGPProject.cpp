// Std. Includes
#include <string>

#ifdef _WIN32
    #define APIENTRY __stdcall
#endif

#include <glad/glad.h>

// GLFW library to create window and to manage I/O
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

// another check related to OpenGL loader
// confirm that GLAD didn't include windows.h
#ifdef _WINDOWS_
    #error windows.h was included!
#endif

// classes developed during lab lectures to manage shaders and to load models
#include <utils/shader.h>
#include <utils/model.h>

// we load the GLM classes used in the application
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

// Load imgui for the User interface
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

// we include the library for images loading
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"



// dimensions of application's window
GLuint screenWidth = 800, screenHeight = 600;
const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

/////////////////////////////////// DEKLARE FUNCTIONS//////////////////////////////////////////////////////////////////////
// callback functions for keyboard events
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// function to move the camera by pressing w,a,s,d
void Do_Movement();

// here we do camera tilting, as well as saving mouse position that gets drawn in the paint texture
void mouse_callback(GLFWwindow* window, double xPos, double yPos);

// setup of Shader Programs
void SetupShaders(int program);

// setup VAO for Portal
GLuint SetupPortal();

// Function for rendering Objects
void RenderObjects(Shader &mainShader, GLint shaderIndex, GLint modelType, int render_pass);

// Function dealing with the Rendering of the 4 Portals
void PortalRenderLoop(Shader &mainShader,GLint shaderIndex[], GLint modelType[], GLuint VAO, std::vector<GLuint> shortestIndices, int render_pass);

// manage the Shader and Modelindices that gets rendering in each Portal
void ManagePortalContent(GLint &currentProgramBackLeft, GLint &currentProgramFrontRight, GLint &currentModelBackLeft, GLint &currentModelFrontRight);

// print on console the name of current shader
void PrintCurrentShader(int shader);
void PrintCurrentModel(int model);

// set the Shader for Model rendered inside the Portals
void setInsideShader(GLuint rightFrontShader, GLuint leftBackShader, GLint &currentProgramInside, GLint &currentModelInside, GLint &currentModelFrontRight, GLint &currentModelBackLeft);

// Function for creating VAO and VBO in order to draw a point
void drawLines(GLuint framebuffer);

// calculate the nearest two portals
std::vector<GLuint> nearestPortals(glm::vec3 cameraPos);

// we load the textures and them up as opengl textures
GLint LoadTexture(const char* path);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////SOME GLOBAL VARIABLES///////////////////////////////////////////////////////////////////////
// parameters for time calculation (for animations)
GLfloat deltaTime = 0.0f;
GLfloat currentFrame;
GLfloat lastFrame = 0.0f;

// rotation angle on Y axis
GLfloat orientationY = 0.0f;

// rotation speed on Y axis
GLfloat spinspeed = 0.5f;

// boolean to start/stop animated rotation on Y angle
GLboolean spinning = GL_TRUE;

// boolean to activate/deactivate wireframe rendering
GLboolean wireframe = GL_FALSE;

// the different Render passes
enum render_passes{ SHADOWMAP, RENDER, BAKE};

enum textureIDs {WOOD, MARPLE, WALL, CONCRETE};

// enum data structure to manage indices for shaders swapping
enum available_ShaderPrograms{LambertianPlusShadow, PhongPlusShadow, BlinnPhongPlusShadow, GGXPlusShadow, AnimatedCellsPlusGGX, AnimatedColorsPlusGGX, StripesSmoothstepPlusGGX, CirclesSmoothstepPlusGGX, FULLCOLOR, Bloom, Texture };
const int NumShader = 8;

enum availabe_Models{Bunny, Cube, Sphere};
const int NumModel = 3;

// Models we use for the enviroment
enum enviromentModels{Plane, Cylinder, Room, Lightbulb};

// strings with shaders names to print the name of the current one on console
const char * print_available_ShaderPrograms[] = { "Lambertian", "Phong", "BlinnPhong", "GGX", "Animated Cells Plus GGX", "Animated Colors Plus GGX", "Stripes Smoothstep Plus GGX", "Circles Smoothstep Plus GGX", "FULLCOLOR", "Bloom", "Texture"};
const char * print_availabe_Models[] = {"Bunny", "Cube", "Sphere"};

// a vector for all the Shader Programs, models and enviroment models used and swapped in the application
vector<std::string> shader;
vector<Model> models;
vector<Model> envModels;
vector<GLint> textureId;
// Uniforms to pass to shaders
// color to be passed to Fullcolor and Flatten shaders
GLfloat myColor[] = {1.0f,0.0f,0.0f};
GLfloat colorWhite[] = {0.8f,0.8f,0.8f};
GLfloat colorDarkRed[] = {0.35f,0.0f,0.0f};
GLfloat colorSandstone[] ={222.0f/255.0f,205.0f/255.0f,190.0f/255.0f};
GLfloat colorCylinder[] = {0.1f, 0.1f, 0.1f};


//Set up Camera Position and View Direction
bool keys[1024] = {0};

// position of the Light
glm::vec3 lightPos = glm::vec3(0.0f,6.0f,4.0f);

// initialise the camera 
glm::vec3 cameraPos;
glm::vec3 lastCameraPos;
glm::vec3 cameraView;
glm::vec3 cameraRight;
glm::vec3 cameraUp;
glm::mat4 view;
float horizontalAngle;
float verticalAngle;


// Projection matrix: FOV angle, aspect ratio, near and far planes
glm::mat4 projection = glm::perspective(45.0f, (float)screenWidth/(float)screenHeight, 0.1f, 10000.0f);
// Orthogonal Projection: we use this in baking Shader to get UV coordinates in screencoords
glm::mat4 OrthoProj = glm::ortho(0,1,0,1,-1,1);

float aspect = (float)SHADOW_WIDTH/(float)SHADOW_HEIGHT;
float near = 1.0f;
float far = 25.0f;
glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, near, far); 


// initialise Mouse informations
float mouseX;
float mouseY;
float lastxPos;
float lastyPos;
bool firstMouse;
std::vector<float> mouseHist;
GLint numMousePoints;

//initialise unsigned ints for the different Textures we use
GLuint depthCubemap[3];
GLuint bakeTexture;
GLuint paintTexture;
GLuint bakeDepthMap;


// initialise Informations for texture drawing
GLfloat brushColor[] = {0.0f,1.0f,0.0f};
float uvRep = 1.0;
float lineSize = 0.04;
bool bake = false;

// frequency and power parameters for noise generation (for random pattern subroutines)
GLfloat frequency = 15.0;
GLfloat power = 2.5;
// number of harmonics (used in the turbulence-based subroutine)
GLfloat harmonics = 1.0;
GLfloat timer;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////// MAIN FUNCTION /////////////////////////////////////////////////////////////////////////////
int main()
{
    // Initialization of OpenGL context using GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    // we set if the window is resizable
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // we create the application's window
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "RTGPProject", nullptr, nullptr);
    if (!window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // we put in relation the window and the callbacks
    glfwSetKeyCallback(window, key_callback);

    //put mouse input and the window in relation
    glfwSetCursorPosCallback(window, mouse_callback);

    // we disable the mouse cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLAD tries to load the context set by GLFW
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }

    // we define the viewport dimensions
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    //imgui
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410");

    // we enable Z test
    glEnable(GL_DEPTH_TEST);

    // we enable face culling, so we dont see the backside of the portals
    glEnable(GL_CULL_FACE);

    //the "clear" color for the frame buffer
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);

    // we create the Shader Programs used in the application
    Shader mainShader("shaders/vertexShader.vert", "shaders/fragmentSHader.frag");
    Shader shadowShader("shaders/shadowmap.vert", "shaders/shadowmap.frag", "shaders/shadow.geo");
    Shader drawingShader("shaders/Drawing.vert", "shaders/Drawing.frag");
    Shader bakeShader("shaders/bakeShader.vert", "shaders/bakeShader.frag");
    SetupShaders(mainShader.Program);

    // we load the model(s) (code of Model class is in include/utils/model.h)
    Model cubeModel("models/cube.obj");
    Model sphereModel("models/sphere.obj");
    Model bunnyModel("models/bunny_lp.obj");
    Model planeModel("models/plane.obj");
    Model cylinderModel("models/cylinder.obj");
    Model roomModel("models/room.obj");
    Model lightbulbModel("models/lightbulb.obj");

    // we set up the models and enviroment models vector
    models.push_back(std::move(bunnyModel));
    models.push_back(std::move(cubeModel));
    models.push_back(std::move(sphereModel));

    envModels.push_back(std::move(planeModel));
    envModels.push_back(std::move(cylinderModel));
    envModels.push_back(std::move(roomModel));
    envModels.push_back(std::move(lightbulbModel));

    textureId.push_back(LoadTexture("textures/darkWood.png"));
    textureId.push_back(LoadTexture("textures/marple.jpg"));
    textureId.push_back(LoadTexture("textures/brickWall.jpg"));
    textureId.push_back(LoadTexture("textures/crackedConcrete.png"));


    // we set up the Portalmesh
    GLuint PortalVAO = SetupPortal();

    // we set the initial indices for the shaders and models shown in the FRONT/RIGHT, BACK/LEFT portal and what is inside
    GLint currentProgramFrontRight = LambertianPlusShadow;
    GLint currentProgramBackLeft = StripesSmoothstepPlusGGX;
    GLint currentProgramInside = LambertianPlusShadow;
    GLint currentModelFrontRight = Bunny;
    GLint currentModelBackLeft = Sphere;
    GLint currentModelInside = Bunny;

    
    // View matrix (=camera): position, view direction, camera "up" vector
    cameraPos = glm::vec3(0.0f,0.0f,7.0f);
    lastCameraPos = cameraPos;
    horizontalAngle = 3.14f;
    verticalAngle = 0.0f;
    firstMouse = true;
    cameraView = glm::vec3(cos(verticalAngle) * sin(horizontalAngle), sin(verticalAngle), cos(verticalAngle) * cos(horizontalAngle));
    cameraRight = glm::vec3(sin(horizontalAngle - 3.14f/2.0f), 0, cos(horizontalAngle - 3.14f/2.0f));
    cameraUp = glm::cross(cameraRight, cameraView);
    view = glm::lookAt(cameraPos, cameraPos + cameraView, cameraUp);

    // Light Informations
    // Light Positions
    std::vector<glm::mat4> shadowTransforms;
    shadowTransforms.push_back(shadowProj * 
                 glm::lookAt(lightPos, lightPos + glm::vec3( 1.0, 0.0, 0.0), glm::vec3(0.0,-1.0, 0.0)));
    shadowTransforms.push_back(shadowProj * 
                 glm::lookAt(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0,-1.0, 0.0)));
    shadowTransforms.push_back(shadowProj * 
                 glm::lookAt(lightPos, lightPos + glm::vec3( 0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
    shadowTransforms.push_back(shadowProj * 
                 glm::lookAt(lightPos, lightPos + glm::vec3( 0.0,-1.0, 0.0), glm::vec3(0.0, 0.0,-1.0)));
    shadowTransforms.push_back(shadowProj * 
                 glm::lookAt(lightPos, lightPos + glm::vec3( 0.0, 0.0, 1.0), glm::vec3(0.0,-1.0, 0.0)));
    shadowTransforms.push_back(shadowProj * 
                 glm::lookAt(lightPos, lightPos + glm::vec3( 0.0, 0.0,-1.0), glm::vec3(0.0,-1.0, 0.0)));

    // specular and ambient components
    GLfloat specularColor[] = {1.0,1.0,1.0};
    GLfloat ambientColor[] = {0.1,0.1,0.1};
    // weights for the diffusive, specular and ambient components
    GLfloat Kd = 0.8f;
    GLfloat Ks = 0.5f;
    GLfloat Ka = 0.1f;
    // shininess coefficient for Blinn-Phong shader
    GLfloat shininess = 25.0f;

    // roughness index for GGX shader
    GLfloat alpha = 0.2f;
    // Fresnel reflectance at 0 degree (Schlik's approximation)
    GLfloat F0 = 0.45f;

    /////////////////// CREATION OF BUFFER FOR THE  DEPTH MAP ///////////////////////////////////////////////////////
    GLuint depthCubemapFBO[3];
    for (int i =0 ;  i < 3; i++) 
    {
        glGenFramebuffers(1, &depthCubemapFBO[i]);
        
        glGenTextures(1, &depthCubemap[i]);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap[i]);
      
        for (unsigned int i = 0; i < 6; ++i)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); 
            GLfloat borderColor[] = {1.0f,1.0f,1.0f,1.0f} ;
            glTexParameterfv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BORDER_COLOR, borderColor);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, depthCubemapFBO[i]);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap[i], 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0); 
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    
    //////////////////Creation of the drawing Buffer and the texture to be drawn in///////////////////////////////
    GLuint paintTextureFBO;
    
    glGenFramebuffers(1, &paintTextureFBO);

    glGenTextures(1, &paintTexture);
    glBindTexture(GL_TEXTURE_2D, paintTexture);

    
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB,  width,  height, 0,GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glBindFramebuffer(GL_FRAMEBUFFER, paintTextureFBO);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, paintTexture, 0);
    
    glDrawBuffer(GL_FRONT_AND_BACK);
    glReadBuffer(GL_FRONT_AND_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /////////////create the Framebuffer and texture to bake the paint texture to the Mesh/////////////////////////
    GLuint bakeTextureFBO;

    glGenFramebuffers(1, &bakeTextureFBO);
    //// and the bake Texture
    glGenTextures(1, &bakeTexture);
    glBindTexture(GL_TEXTURE_2D, bakeTexture);

    std::vector<GLubyte> whiteTextureData( 4 *screenWidth *  screenHeight * 3, 255);
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0,GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glBindFramebuffer(GL_FRAMEBUFFER, bakeTextureFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bakeTexture, 0);

    glDrawBuffer(GL_FRONT_AND_BACK);
    glReadBuffer(GL_FRONT_AND_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //////////////create a Depth Map for the texture baking///////////////////////////////////////////////////////
    // We use this to not draw on both sides of the mesh or draw "through" the mesh 

    GLuint bakeDepthMapFBO;

    glGenFramebuffers(1, &bakeDepthMapFBO);
    glGenTextures(1, &bakeDepthMap);
    glBindTexture(GL_TEXTURE_2D, bakeDepthMap);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,  width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);


    glBindFramebuffer(GL_FRAMEBUFFER, bakeDepthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, bakeDepthMap, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Rendering loop
    while(!glfwWindowShouldClose(window))
    {
        // we determine the time passed from the beginning
        // and we calculate time difference between current frame rendering and the previous one
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Check is an I/O event is happening
        glfwPollEvents();

        // when not in draw Mode do movements
        if (!keys[GLFW_KEY_SPACE])
            Do_Movement();
        
        // Manage the Model and Shader that are rendered in each Portal
        ManagePortalContent(currentProgramBackLeft, currentProgramFrontRight, currentModelBackLeft, currentModelFrontRight);

        // set the Model and Shader that gets renedered inside the Portalcube
        // the Model in the front and right portal is the same (the same holds for the left portal and the one in the back)
        setInsideShader(currentProgramFrontRight, currentProgramBackLeft, currentProgramInside, currentModelInside, currentModelFrontRight, currentModelBackLeft);


        //Update view Matrix
        view = glm::lookAt(cameraPos, cameraPos + cameraView, cameraUp); 
        
        // we set the rendering mode
        if (wireframe)
            // Draw in wireframe
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


        // if animated rotation is activated, we rotate the light source around mesh 
        if (spinning && !keys[GLFW_KEY_SPACE])
        {
            lightPos = glm::mat3(glm::rotate(glm::mat4(1.0f), glm::radians(spinspeed), glm::vec3(0.0f,1.0f,0.0f))) * lightPos;
            shadowTransforms[0] = (shadowProj * 
                 glm::lookAt(lightPos, lightPos + glm::vec3( 1.0, 0.0, 0.0), glm::vec3(0.0,-1.0, 0.0)));
            shadowTransforms[1] = (shadowProj * 
                 glm::lookAt(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0,-1.0, 0.0)));
            shadowTransforms[2] = (shadowProj * 
                 glm::lookAt(lightPos, lightPos + glm::vec3( 0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
            shadowTransforms[3] = (shadowProj * 
                 glm::lookAt(lightPos, lightPos + glm::vec3( 0.0,-1.0, 0.0), glm::vec3(0.0, 0.0,-1.0)));
            shadowTransforms[4] = (shadowProj * 
                 glm::lookAt(lightPos, lightPos + glm::vec3( 0.0, 0.0, 1.0), glm::vec3(0.0,-1.0, 0.0)));
            shadowTransforms[5] = (shadowProj * 
                 glm::lookAt(lightPos, lightPos + glm::vec3( 0.0, 0.0,-1.0), glm::vec3(0.0,-1.0, 0.0)));
        }

        //////////////////////////////////////////////////// STEP 1 - SHADOW MAPPING ////////////////////////////////////////////////
        /// We "install" the  Shader Program for the shadow mapping creation
        shadowShader.Use();

        // we pass the transformation matrix as uniform
        glUniformMatrix4fv(glGetUniformLocation(shadowShader.Program, "shadowMatrices"), 6, GL_FALSE, glm::value_ptr(shadowTransforms[0]));
        glUniform3fv(glGetUniformLocation(shadowShader.Program, "lightPos"), 1, glm::value_ptr(lightPos));
        glUniform1f(glGetUniformLocation(shadowShader.Program,"far_plane"), far);


        // we set the viewport for the first rendering step = dimensions of the depth texture
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);

        // we calculate the shadow map for the Models currently loaded in the Portals
        for (int i:{currentModelFrontRight, currentModelBackLeft})
        {
            // we activate the FBO for the depth map rendering
            glBindFramebuffer(GL_FRAMEBUFFER, depthCubemapFBO[i]);
            glClear(GL_DEPTH_BUFFER_BIT);

            // we render the scene, using the shadow shader

            // Render the Inside of the Portalcube
            RenderObjects(shadowShader, 0, i, SHADOWMAP);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, width, height);
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        //////////////////////////////////////// STEP 2 - MAIN RENDERING LOOP /////////////////////////////////////////////////////
        // In this Step we render the 2 nearest portals in reference to the camera and the Model inside

        // we "clear" the frame and z buffer
        glEnable(GL_STENCIL_TEST);
        glStencilMask(0xFF);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


        // activate the main Shader
        mainShader.Use();

        // Send the uniforms containing the light information
        glUniform3fv(glGetUniformLocation(mainShader.Program, "lightPos"), 1, glm::value_ptr(lightPos));    
        glUniform1f(glGetUniformLocation(mainShader.Program, "far_plane"), far);
        glUniform3fv(glGetUniformLocation(mainShader.Program, "ambientColor"), 1, ambientColor);
        glUniform3fv(glGetUniformLocation(mainShader.Program, "specularColor"), 1, specularColor);
        glUniform1f(glGetUniformLocation(mainShader.Program, "shininess"), shininess);
        glUniform1f(glGetUniformLocation(mainShader.Program, "alpha"), alpha);
        glUniform1f(glGetUniformLocation(mainShader.Program, "F0"), F0);
        glUniform1f(glGetUniformLocation(mainShader.Program, "Ka"), Ka);
        glUniform1f(glGetUniformLocation(mainShader.Program, "Kd"), Kd);
        glUniform1f(glGetUniformLocation(mainShader.Program, "Ks"), Ks);

        // send the uniforms containing informations for the random patterns
        glUniform1f(glGetUniformLocation(mainShader.Program, "frequency"), frequency);
        glUniform1f(glGetUniformLocation(mainShader.Program, "power"), power);
        glUniform1f(glGetUniformLocation(mainShader.Program, "timer"), currentFrame);
        glUniform1f(glGetUniformLocation(mainShader.Program, "harmonics"), harmonics);

        
        GLint portalShader[] = {currentProgramFrontRight, currentProgramBackLeft};
        GLint portalModel[] = {currentModelFrontRight,currentModelBackLeft};

        // find the two nearest portals
        std::vector<GLuint> shortestIndices = nearestPortals(cameraPos);

        // Render Portals plus what's inside of them
        PortalRenderLoop(mainShader, portalShader, portalModel, PortalVAO, shortestIndices, RENDER);
        

        // Render the Inside of the Portalcube
        RenderObjects(mainShader, currentProgramInside, currentModelInside, RENDER);
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////// STEP 3 - DRAW THE TEXTURE//////////////////////////////////////////////////////
        // when we are in drawing Mode (by pressing SPACE), we first draw in a framebuffer/texture and when we are done drawing that framebuffer/texture that texture gets baked on the mesh.
        // This happens in the bake Shader. There we check for every vertex (in screencoordinates) of the mesh if there is a color stored in the texture where that vertex is drawn on the screen. Then we draw this color in the UV coordinates of the mesh. 
        // Hence we can look up the right color with the UV coordinates of the mesh.
        if (keys[GLFW_KEY_SPACE]) 
        {
            // First we draw in the first framebuffer/texture 
            if (keys[GLFW_KEY_E])
            {
                drawingShader.Use();
                glUniform1fv(glGetUniformLocation(drawingShader.Program, "colorIn"), 3 , brushColor);
                drawLines(paintTextureFBO);
                bake = true;
            }
            // then we bake that texture in UV coordinates
            else if (bake)
            {
                glBindFramebuffer(GL_FRAMEBUFFER, bakeDepthMapFBO);
                glClear(GL_DEPTH_BUFFER_BIT);
                // we need depth testing, so we dont draw through the texture 
                // so we draw the scene from cameras perspektive to get a depthmap 
                glEnable(GL_DEPTH_TEST);
                mainShader.Use();
                RenderObjects(mainShader, FULLCOLOR, currentModelInside, SHADOWMAP);

                // then we bake using the bakeShader
                glBindFramebuffer(GL_FRAMEBUFFER, bakeTextureFBO);
                bakeShader.Use();
                glUniformMatrix4fv(glGetUniformLocation(bakeShader.Program, "OrthoProj"), 1, GL_FALSE, glm::value_ptr(OrthoProj));
                
                // we have to disable face culling so we dont accidentally discard left facing triangles in UV coordinates
                glDisable(GL_CULL_FACE);
                RenderObjects(bakeShader, currentProgramInside, currentModelInside, BAKE);
                glEnable(GL_CULL_FACE);

                // we bind the first framebuffer to clear its color buffer bit
                glBindFramebuffer(GL_FRAMEBUFFER, paintTextureFBO);
                glClear(GL_COLOR_BUFFER_BIT);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                // set bake to false so we dont bake every frame
                bake = false;
            }
            
        }
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        /////////////////////////////////// IMGUI INTERFACE /////////////////////////////////////////////////////////////////////////////
        
        if (keys[GLFW_KEY_SPACE]) 
        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            // ImGUI window creation
            ImGui::Begin("Performance");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        
            ImGui::Text("Mouse position: (%.5f, %.5f)", mouseX, mouseY);
            ImGui::Text("Width, height: (%.0f, %.0f)", float(screenWidth), float(screenHeight));
           
            ImGui::Separator();
            ImGui::Text("Paintint Options: ");
            ImGui::ColorEdit4("Set Modelcolor", myColor);
            ImGui::SliderFloat("Repeat UV: ", &uvRep, 1.0f, 50.0f);
            ImGui::SliderFloat("Line size: ", &lineSize, 0.01f, 0.1f);
            ImGui::ColorEdit4("Set Brushcolor", brushColor);

            ImGui::Separator();
            ImGui::Text("Lightning Options: ");
            ImGui::SliderFloat("Light Height: ", &lightPos.y, 1.0f, 7.0f);
            ImGui::SliderFloat("Kd: ", &Kd, 0.0f, 1.0f);
            ImGui::SliderFloat("Ks: ", &Ks, 0.0f, 1.0f);
            ImGui::SliderFloat("Ka: ", &Ka, 0.0f, 1.0f);
            ImGui::SliderFloat("Shininess: ", &shininess, 10.0f, 100.0f);
            ImGui::SliderFloat("Roughness Index: ", &alpha, 0.0f, 1.0f);
            ImGui::SliderFloat("Fresnel: ", &F0, 0.0f, 1.0f);

            ImGui::Separator();
            ImGui::Text("Patterns: ");
            ImGui::SliderFloat("Power: ", &power, 0.0f, 5.0f);
            ImGui::SliderFloat("Frequency: ", &frequency, 1.0f, 20.0f);
            ImGui::SliderFloat("Harmonics: ", &harmonics, 1.0f, 7.0f);
        

            // Ends of imgui
            ImGui::End();
            // Renders the ImGUI elements
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }

        
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // Swapping back and front buffers
        glfwSwapBuffers(window);
        lastCameraPos = cameraPos;

    }

    // when I exit from the graphics loop, it is because the application is closing
    // we delete the Shader Programs
    mainShader.Delete();
    bakeShader.Delete();
    drawingShader.Delete();

    //Delete the IMGui context
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Delete the framebuffer
    glDeleteFramebuffers(3, depthCubemapFBO);
    glDeleteFramebuffers(1, &paintTextureFBO);
    glDeleteFramebuffers(1, &bakeTextureFBO);
    glDeleteFramebuffers(1, &bakeDepthMapFBO);


    // we close and delete the created context
    glfwTerminate();
    return 0;
}


//////////////////////////////////////////////////////// DEFINITION OF FUNCTIONS /////////////////////////////////////////////////////////////////////
void SetupShaders(int program)
{
    int maxSub, maxSubU, countActiveSU;
    GLchar name[256];
    int len, numComps;

    glGetIntegerv(GL_MAX_SUBROUTINES, &maxSub);
    glGetIntegerv(GL_MAX_SUBROUTINE_UNIFORM_LOCATIONS, &maxSubU);
    std::cout << "Max Subroutines: " << maxSub << " - Max Subroutine Uniform: " << maxSubU << std::endl;

    glGetProgramStageiv(program, GL_FRAGMENT_SHADER, GL_ACTIVE_SUBROUTINE_UNIFORMS, &countActiveSU);

    // print Info for every Subroutine uniform 
    for (int i = 0 ; i < countActiveSU; i++)
    {
        glGetActiveSubroutineUniformName(program, GL_FRAGMENT_SHADER, i , 256, &len, name);

        std::cout << "Subroutine Uniform: " << i << " - name: " << name << std::endl;

        glGetActiveSubroutineUniformiv(program, GL_FRAGMENT_SHADER, i, GL_NUM_COMPATIBLE_SUBROUTINES, &numComps);

        int *s = new int[numComps];

        glGetActiveSubroutineUniformiv(program, GL_FRAGMENT_SHADER, i, GL_COMPATIBLE_SUBROUTINES, s);
        std::cout << "Compatible Subroutines: " << std::endl;

        for (int j = 0; j <numComps; j++)
        {
            glGetActiveSubroutineName(program, GL_FRAGMENT_SHADER, s[j], 256, &len, name);
            std::cout << "\t" << s[j] << " - " << name << "\n";
            shader.push_back(name);
        }
        std:: cout << std:: endl;

        delete[] s;
    } 

}

void PrintCurrentShader(int shader)
{
    std::cout << "Current shader: " << print_available_ShaderPrograms[shader]  << std::endl;

}

void PrintCurrentModel(int model)
{
    std::cout << "Current Model: " << print_availabe_Models[model] << std::endl;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    // if ESC is pressed, we close the application
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    // if P is pressed, we start/stop the animated rotation of the Light source
    if(key == GLFW_KEY_P && action == GLFW_PRESS)
        spinning=!spinning;

    // if L is pressed, we activate/deactivate wireframe rendering of models
    if(key == GLFW_KEY_L && action == GLFW_PRESS)
        wireframe=!wireframe;

    //Let the camara "walk" using w,a,s,d
    if(key == GLFW_KEY_W && action == GLFW_PRESS)
    {
        keys[GLFW_KEY_W] = true;
    }
    if(key == GLFW_KEY_W && action == GLFW_RELEASE)
    {
        keys[GLFW_KEY_W] = false;
    }

    if(key == GLFW_KEY_A && action == GLFW_PRESS)
    {
        keys[GLFW_KEY_A] = true;
    }
    if(key == GLFW_KEY_A && action == GLFW_RELEASE)
    {
        keys[GLFW_KEY_A] = false;
    }

    if(key == GLFW_KEY_S && action == GLFW_PRESS)
    {
        keys[GLFW_KEY_S] = true;
    }
    if(key == GLFW_KEY_S && action == GLFW_RELEASE)
    {
        keys[GLFW_KEY_S] = false;
    }

    if(key == GLFW_KEY_D && action == GLFW_PRESS)
    {
        keys[GLFW_KEY_D] = true;
    }
    if(key == GLFW_KEY_D && action == GLFW_RELEASE)
    {
        keys[GLFW_KEY_D] = false;
    }

    // When pressing E we start drawing. Releasing E wipes the history of mousepoints that were drawn
    if(key == GLFW_KEY_E && action == GLFW_PRESS)
    {
        keys[GLFW_KEY_E] = true;
    }
    if(key == GLFW_KEY_E && action == GLFW_RELEASE)
    {
        keys[GLFW_KEY_E] = false;
        mouseHist.clear();
        numMousePoints = 0;
    }
    
    // pressing SPACE starts the draw Mode. It enables the cursor and sets firstMouse to true
    if(key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        keys[GLFW_KEY_SPACE] = !keys[GLFW_KEY_SPACE];

        if (keys[GLFW_KEY_SPACE])
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            firstMouse = true;
        }
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
}

void mouse_callback(GLFWwindow* window, double xPos, double yPos)
{   
    glfwGetCursorPos(window, &xPos, &yPos);
    // when not in draw Mode we adjust the angle of the camera by the differene of mouse positions from the last frame to the current frame
    if (!keys[GLFW_KEY_SPACE])
    {

        if (firstMouse)
        {
            lastyPos = yPos;
            lastxPos = xPos;
            firstMouse = false;
        }
        
        int height, width;
        glfwGetWindowSize(window, &height, &width);
        horizontalAngle = horizontalAngle + 0.1f * deltaTime * float(lastxPos - xPos);
        verticalAngle = verticalAngle + 0.1f * deltaTime * float(lastyPos - yPos);
        lastxPos = xPos;
        lastyPos = yPos;

        if (verticalAngle > 1.57f)
        {
            verticalAngle = 1.57f;
        }
        if (verticalAngle < -1.57f)
        {
            verticalAngle = -1.57f;
        }

        cameraView = glm::normalize(glm::vec3(cos(verticalAngle) * sin(horizontalAngle), sin(verticalAngle), cos(verticalAngle)*cos(horizontalAngle)));
        cameraRight = glm::normalize(glm::vec3(sin(horizontalAngle - 3.14f/2.0f), 0, cos(horizontalAngle - 3.14f/2.0f)));
        cameraUp = glm::cross(cameraRight, cameraView);
    }
    mouseX = (float)(2 * xPos) / screenWidth - 1.0f;
    mouseY = 1.0f - (float)(2 * yPos) / screenHeight;

    // when pressing E in Draw Mode we store the Mouse positions in the mouseHist vector, so we can use them later to draw lines
    if (keys[GLFW_KEY_SPACE] && keys[GLFW_KEY_E]) 
    {
        mouseHist.push_back(mouseX);
        mouseHist.push_back(mouseY);
        numMousePoints++;
    }
    
}

void Do_Movement()
{
     // Let the camara "walk" using w,a,s,d
     // we stop the camera movement when the new camera position is outside the outer walls
    if(keys[GLFW_KEY_W] == true)
    {
        glm::vec3 newCameraPos = cameraPos + glm::normalize(glm::vec3(cameraView.x,0,cameraView.z)) * 3.0f * deltaTime;
        if (std::abs(newCameraPos.x) < 13.8f && std::abs(newCameraPos.z) < 13.8f)
        {
            cameraPos = newCameraPos;
        }
    }
    if(keys[GLFW_KEY_A] == true)
    {
        glm::vec3 newCameraPos = cameraPos - glm::normalize(glm::vec3(cameraRight.x,0,cameraRight.z)) * 3.0f * deltaTime;
        if (std::abs(newCameraPos.x) < 13.8f && std::abs(newCameraPos.z) < 13.8f)
        {
            cameraPos = newCameraPos;
        }

    }
    if(keys[GLFW_KEY_S] == true)
    {
        glm::vec3 newCameraPos = cameraPos - glm::normalize(glm::vec3(cameraView.x,0,cameraView.z)) * 3.0f * deltaTime;
        if (std::abs(newCameraPos.x) < 13.8f && std::abs(newCameraPos.z) < 13.8f)
        {
            cameraPos = newCameraPos;
        }

    }
    if(keys[GLFW_KEY_D] == true)
    {
        glm::vec3 newCameraPos = cameraPos + glm::normalize(glm::vec3(cameraRight.x,0,cameraRight.z)) * 3.0f * deltaTime;
        if (std::abs(newCameraPos.x) < 13.8f && std::abs(newCameraPos.z) < 13.8f)
        {
            cameraPos = newCameraPos;
        }

    }
}

void ManagePortalContent(GLint &currentProgramBackLeft, GLint &currentProgramFrontRight, GLint &currentModelBackLeft, GLint &currentModelFrontRight) 
{
    // we rotate the camera, such that checking if we walk around the front-right and back-left corners is equal to checking if we cross the x - Axis
    glm::vec3 tempRotatetCameraPos = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0.0f,1.0f,0.0f)) * glm::vec4(cameraPos,1.0f);
    glm::vec3 lasttempRotatetCameraPos = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0.0f,1.0f,0.0f)) * glm::vec4(lastCameraPos,1.0f);

    // when crossing the x - Axis on the right side and walking from negativ to positiv we increase the shader program in the back left corner
    if (7.0f < tempRotatetCameraPos.x && tempRotatetCameraPos.x < 19.8f && tempRotatetCameraPos.z < 0.0f && lasttempRotatetCameraPos.z >= 0.0f)
    {
        currentProgramBackLeft += 4;
        if (currentProgramBackLeft >= NumShader)
        {
            currentProgramBackLeft -= NumShader;
            currentModelBackLeft += 1;
            if (currentModelBackLeft >= NumModel)
            {
                currentModelBackLeft -= NumModel;
            }
        }
    }

    // when crossing the x - Axis on the left side and walking from negativ to positiv we increase the shader program in the front right corner
    if (-19.8f < tempRotatetCameraPos.x && tempRotatetCameraPos.x < -7.0f && tempRotatetCameraPos.z > 0.0f && lasttempRotatetCameraPos.z <= 0.0f)
    {
        currentProgramFrontRight += 4;
        if (currentProgramFrontRight >= NumShader)
        {
            currentProgramFrontRight -= NumShader;
            currentModelFrontRight += 1;
            if (currentModelFrontRight >= NumModel)
            {
                currentModelFrontRight -= NumModel;
            }
        }
    }

    // when crossing the x - Axis on the right side and walking from negativ to positiv we decrease the shader program in the back left corner
    if (7.0f < tempRotatetCameraPos.x && tempRotatetCameraPos.x < 19.8f && lasttempRotatetCameraPos.z < 0.0f && tempRotatetCameraPos.z >= 0.0f)
    {
        currentProgramBackLeft -= 4;
        if (currentProgramBackLeft < 0)
        {
            currentProgramBackLeft += NumShader;
            currentModelBackLeft -= 1;
            if (currentModelBackLeft < 0)
            {
                currentModelBackLeft += NumModel;
            }
        }
    }

    // when crossing the x - Axis on the left side and walking from negativ to positiv we decrease the shader program in the front right corner
    if (-19.8f < tempRotatetCameraPos.x && tempRotatetCameraPos.x < -7.0f && lasttempRotatetCameraPos.z > 0.0f && tempRotatetCameraPos.z <= 0.0f)
    {
        currentProgramFrontRight -= 4;
        if (currentProgramFrontRight < 0)
        {
            currentProgramFrontRight += NumShader;
            currentModelFrontRight -= 1;
            if (currentModelFrontRight < 0 )
            {
                currentModelFrontRight += NumModel;
            }
        }
    }
}

void PortalRenderLoop(Shader &mainShader, GLint shaderIndex[], GLint modelType[], GLuint VAO, std::vector<GLuint> shortestIndices, int render_pass)
{
    // set up Position and Rotationaxis for the Portals
    vector<glm::vec3> PortalPos = {glm::vec3(0.0f,0.0f,5.0f), glm::vec3(5.0f,0.0f,0.0f), glm::vec3(0.0f,0.0f,-5.0f), glm::vec3(-5.0f,0.0f,0.0f)};
    vector<glm::vec3> PortalRotationAxis = {glm::vec3(-1.0f,0.0f,0.0f),glm::vec3(0.0f,0.0f,1.0f), glm::vec3(1.0f,0.0f,0.0f),glm::vec3(0.0f,0.0f,-1.0f)};

    for (int i :shortestIndices)
    {
        // Lets do Portals 
        // Step One: Disable Color and Depth Buffer. Enable Stencil Buffer
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_STENCIL_TEST);
        glStencilMask(0xFF);


        // Step Two: Set Stecnil Opereation for front facing triangles to Replace when Stencil test and depth test are succesful
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        //glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_REPLACE);
        //glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_KEEP);
    

        // Step Three: Set Stencil Test to ALWAYS, therefore it will always pass and replace the stencil value with i+1
        glStencilFunc(GL_ALWAYS, i+1, 0xFF);
        //glStencilFuncSeparate(GL_FRONT, GL_ALWAYS, i+1, 0xFF);
        //glStencilFuncSeparate(GL_BACK, GL_NEVER, 0, 0xFF);


        // Step Four: Draw Portal Frame in the stencil Buffer
        // Note that every Portal has its own stencil value 
        GLuint index = glGetSubroutineIndex(mainShader.Program, GL_FRAGMENT_SHADER, shader[FULLCOLOR].c_str());
        glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &index);

        // Set up ModelMatrix and Normalmatrix for the PortalFrame
        glm::mat4 planeModelMatrix = glm::mat4(1.0f);
        planeModelMatrix = glm::translate(planeModelMatrix, PortalPos[i]);
        planeModelMatrix = glm::rotate(planeModelMatrix, glm::radians(90.0f), PortalRotationAxis[i]);
        planeModelMatrix = glm::scale(planeModelMatrix, glm::vec3(5.0f,10.0f,5.0f));
        
        //Send the Matrizes and the color Uniform to our mainShader
        glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(planeModelMatrix));
        glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));

        // Draw the Portal
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        
        // Step Five: Disable writing to the Stencil Buffer and Enable Color and Depth Buffer
        glStencilMask(0x00);
        glEnable(GL_DEPTH_TEST);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);


        // Step Six: Set the stencil Function for front facing triangles such that we only draw if the value in the stencil buffer is i+1
        glStencilFunc(GL_EQUAL, i+1, 0xFF);
        //glStencilFuncSeparate(GL_FRONT, GL_EQUAL, i+1, 0xFF);
        

        // Step Seven: Draw what is inside of the Portal
        RenderObjects(mainShader, shaderIndex[i < 2 ? 0 : 1] + (i % 2), modelType[i < 2 ? 0 : 1], render_pass);

        // Step Eight: Disable Color Buffer and Stencil Test but enable writing to the depth buffer
        glDisable(GL_STENCIL_TEST);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glDepthMask(GL_TRUE);


        // Step Nine: Draw our Portal again. This time only in the Depth Buffer
        index = glGetSubroutineIndex(mainShader.Program, GL_FRAGMENT_SHADER, shader[FULLCOLOR].c_str());
        glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &index);

        //Set up ModelMatrix for the first Plane
        planeModelMatrix = glm::mat4(1.0f);
        planeModelMatrix = glm::translate(planeModelMatrix, PortalPos[i]);
        planeModelMatrix = glm::rotate(planeModelMatrix, glm::radians(90.0f), PortalRotationAxis[i]);
        planeModelMatrix = glm::scale(planeModelMatrix, glm::vec3(5.0f,10.0f,5.0f));

        //Send the Matrizes and the color Uniform to our mainSHader
        glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(planeModelMatrix));
        glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3fv(glGetUniformLocation(mainShader.Program, "colorIn"), 1, colorDarkRed);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

GLuint SetupPortal()
{
    // Define Portals by hand and set up VAO, VBO, EBO
    GLfloat vertices[] = {
         1.0f,  0.0f,-1.0f,  // Top Right       
         1.0f,  0.0f, 1.0f,  // Bottom Right    
        -1.0f,  0.0f, 1.0f,  // Bottom Left     
        -1.0f,  0.0f,-1.0f,  // Top Left        
    };

    GLuint indices[] = {  
        0, 1, 2,  // First Triangle
        2, 3, 0,  // Second Triangle
    };

    GLuint VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO); 
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); 

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);  

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);   

    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    glBindVertexArray(0); 

    return VAO;
}

void setInsideShader(GLuint rightFrontShader, GLuint leftBackShader, GLint &currentProgramInside, GLint &currentModelInside, GLint &currentModelFrontRight, GLint &currentModelBackLeft)
{
    // set Inside Model and Shader equal to the FRONT FACING PORTAL, if we step through it (more precisely if we step through a barrier slietly infront of the portal)
    if(cameraPos.z < 5.2f && std::abs(cameraPos.x) <= 5.2f && lastCameraPos.z > 5.2f)
    {
        currentProgramInside = rightFrontShader;
        currentModelInside = currentModelFrontRight;
        PrintCurrentShader(currentProgramInside);
        PrintCurrentModel(currentModelInside);
    }

    // set Inside Model and Shader equal to the RIGHT FACING PORTAL, if we step through it
    if(cameraPos.x < 5.2f && std::abs(cameraPos.z) <= 5.2f && lastCameraPos.x > 5.2f)
    {
        currentProgramInside = rightFrontShader +1;
        currentModelInside = currentModelFrontRight;
        PrintCurrentShader(currentProgramInside);
        PrintCurrentModel(currentModelInside);
    }

    // set Inside Model and Shader equal to the BACK FACING PORTAL, if we step through it
    if(cameraPos.z > -5.2f && std::abs(cameraPos.x) <= 5.2f && lastCameraPos.z < -5.2f)
    {
        currentProgramInside = leftBackShader;
        currentModelInside = currentModelBackLeft;
        PrintCurrentShader(currentProgramInside);
        PrintCurrentModel(currentModelInside);
    }
    
    // set Inside Model and Shader equal to the LEFT FACING PORTAL, if we step through it
    if(cameraPos.x > -5.2f && std::abs(cameraPos.z) <= 5.2f && lastCameraPos.x < -5.2f)
    {
        currentProgramInside = leftBackShader +1;
        currentModelInside = currentModelBackLeft;
        PrintCurrentShader(currentProgramInside);
        PrintCurrentModel(currentModelInside);
    }
}

void RenderObjects(Shader &mainShader, GLint shaderIndex, GLint modelType, int render_pass)
{
    // when baking we only set up the paintTexture, bakeTexture and the bakeDepthMap and afterwards render the Model
    if (render_pass == BAKE)
    {   
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, paintTexture);
        GLint paintTextureLoc = glGetUniformLocation(mainShader.Program, "paintTexture");
        glUniform1i(paintTextureLoc, 3); 

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, bakeTexture);
        GLint bakeTextureLoc = glGetUniformLocation(mainShader.Program, "bakeTexture");
        glUniform1i(bakeTextureLoc, 4);

        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, bakeDepthMap);
        GLint bakeDepthMapLoc = glGetUniformLocation(mainShader.Program, "bakeDepthMap");
        glUniform1i(bakeDepthMapLoc, 5);


    }

    // in the render pass, we also set up and render every enviroment Model
    if (render_pass==RENDER)
    {
        // pass the shadowMap texture to the shader
        glActiveTexture(GL_TEXTURE0 + modelType);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap[modelType]);
        GLint shadowLocation = glGetUniformLocation(mainShader.Program, "shadowMap");
        glUniform1i(shadowLocation, modelType);

        ////////////////////////////////// RENDER THE LIGHTBULB ////////////////////////////////////////////////////////////////////////
        GLuint index = glGetSubroutineIndex(mainShader.Program, GL_FRAGMENT_SHADER, shader[Bloom].c_str());
        glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &index);

        glm::mat4 lightbulbModelMatrix = glm::mat4(1.0f);
        lightbulbModelMatrix = glm::translate(lightbulbModelMatrix, lightPos);
        lightbulbModelMatrix = glm::scale(lightbulbModelMatrix, glm::vec3(0.1f,0.13f,0.1f));

        glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(lightbulbModelMatrix));
        models[Sphere].Draw();
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        
        // set the subroutine to Texture for the FLoorplanes
        index = glGetSubroutineIndex(mainShader.Program, GL_FRAGMENT_SHADER, shader[Texture].c_str());
        glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &index);

        /////////////////////////////////// RENDER THE LARGER FLOOR PLANE //////////////////////////////////////////////////////////////
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, textureId[WOOD]);
        GLint textureLocation = glGetUniformLocation(mainShader.Program, "textureID");
        glUniform1i(textureLocation, 6);

        // we set the Modelmatrix and Normalmatrix for the larger Floorplane
        glm::mat4 planeModelMatrix = glm::mat4(1.0f);
        glm::mat3 planeNormalMatrix = glm::mat3(1.0f);
        planeModelMatrix = glm::translate(planeModelMatrix, glm::vec3(0.0f,-1.0f,0.0f));
        planeModelMatrix = glm::scale(planeModelMatrix, glm::vec3(2.8f,1.0f,2.8f));
        planeNormalMatrix = glm::inverseTranspose(glm::mat3(view*planeModelMatrix));

        //Send the Matrizes and the color Uniform to our mainShader
        glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(planeModelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(mainShader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(planeNormalMatrix));
        glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform1f(glGetUniformLocation(mainShader.Program, "texRep"), 15.0);
        envModels[Plane].Draw();
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


        ////////////////////////////////// RENDER THE SMALLER FLOOR PLANE //////////////////////////////////////////////////////////////
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, textureId[MARPLE]);
        textureLocation = glGetUniformLocation(mainShader.Program, "textureID");
        glUniform1i(textureLocation, 7);

        // we set the Model and Normalmatrix for the smaller Floorplane
        planeModelMatrix = glm::mat4(1.0f);
        planeNormalMatrix = glm::mat3(1.0f);
        planeModelMatrix = glm::translate(planeModelMatrix, glm::vec3(0.0f,-0.999f,0.0f));
        planeModelMatrix = glm::scale(planeModelMatrix, glm::vec3(1.0f,1.0f,1.0f));
        planeNormalMatrix = glm::inverseTranspose(glm::mat3(view*planeModelMatrix));

        //Send the Matrizes and the color Uniform to our mainShader
        glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(planeModelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(mainShader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(planeNormalMatrix));
        glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform1f(glGetUniformLocation(mainShader.Program, "texRep"), 3.0);
        envModels[Plane].Draw();
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


        ///////////////////////////////// RENDER THE WALLS /////////////////////////////////////////////////////////////////////////////
        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_2D, textureId[WALL]);
        textureLocation = glGetUniformLocation(mainShader.Program, "textureID");
        glUniform1i(textureLocation, 8);

        //set up wall position, rotation axis and angle 
        glm::vec3 wallPos[] = {glm::vec3(14.0f,0.0f,0.0f), glm::vec3(0.0f,0.0f,14.0f), glm::vec3(-14.0f,0.0f,0.0f), glm::vec3(0.0f,0.0f,-14.0f)};
        glm::vec3 wallRot[] = {glm::vec3(0.0f,0.0f, 1.0f), glm::vec3(0.5773503f, -0.5773503f, 0.5773503f), glm::vec3(0.0f,0.0f, -1.0f), glm::vec3(-0.5773503f, 0.5773503f, 0.5773503f)};
        float wallRotations[] = {glm::radians(90.0f), glm::radians(240.0f), glm::radians(90.0f), glm::radians(240.0f)};

        // set up and send the Modelmatrix for the Walls to the shader and render them
        for (int i= 0; i<4; i++)
        {
            planeModelMatrix = glm::mat4(1.0f);
            planeModelMatrix = glm::translate(planeModelMatrix, wallPos[i]);
            planeModelMatrix = glm::rotate(planeModelMatrix, wallRotations[i], wallRot[i]);
            planeModelMatrix = glm::scale(planeModelMatrix, glm::vec3(3.0f,1.0f,3.0f));

            glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(planeModelMatrix));
            glUniform1f(glGetUniformLocation(mainShader.Program, "texRep"), 8.0);
            envModels[Plane].Draw();
        }
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


        ///////////////////////////////// RENDER THE CEILING //////////// //////////////////////////////////////////////////////////////
        glActiveTexture(GL_TEXTURE9);
        glBindTexture(GL_TEXTURE_2D, textureId[CONCRETE]);
        textureLocation = glGetUniformLocation(mainShader.Program, "textureID");
        glUniform1i(textureLocation, 9);

        // set up Modelmatrix for the ceiling 
        planeModelMatrix = glm::mat4(1.0f);
        planeModelMatrix = glm::translate(planeModelMatrix, glm::vec3(0.0f,10.0f,0.0f));
        planeModelMatrix = glm::rotate(planeModelMatrix, glm::radians(180.0f), glm::vec3(1.0f,0.0f,0.0f));
        planeModelMatrix = glm::scale(planeModelMatrix, glm::vec3(2.8f,1.0f,2.8f));

        // send the Modelmatrix to the Shader and render the ceiling
        glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(planeModelMatrix));
        glUniform1f(glGetUniformLocation(mainShader.Program, "texRep"), 5.0);
        envModels[Plane].Draw();
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, bakeTexture);
        GLint bakeTextureLoc = glGetUniformLocation(mainShader.Program, "bakeTexture");
        glUniform1i(bakeTextureLoc, 4);
    }

    ////////////////////////////////// RENDER THE MAIN MODEL ///////////////////////////////////////////////////////////////////////////
    // set up the subroutine
    GLuint index = glGetSubroutineIndex(mainShader.Program, GL_FRAGMENT_SHADER, shader[shaderIndex].c_str());
    glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &index);

    // set the Model and Normalmatrix for the model
    glm::mat4 ModelMatrix = glm::mat4(1.0f);
    glm::mat3 NormalMatrix = glm::mat3(1.0f);
    ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f,0.0f,0.0f));
    if (modelType == Bunny)
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.4f, 0.4f, 0.4f));
    else
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.9f, 0.9f, 0.9f));
    NormalMatrix = glm::inverseTranspose(glm::mat3(view*ModelMatrix));

    glUniform1f(glGetUniformLocation(mainShader.Program, "uvRep"), uvRep);
    glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(ModelMatrix));
    glUniformMatrix3fv(glGetUniformLocation(mainShader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(NormalMatrix));
    glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));
    glUniform3fv(glGetUniformLocation(mainShader.Program, "colorIn"), 1, myColor);
    models[modelType].Draw();
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    ////////////////////////////////////// RENDER THE PILLAR CYLINDERS ////////////////////////////////////////////////////////////////
    // set the subroutine to FULLCOLOR for the cylinders
    index = glGetSubroutineIndex(mainShader.Program, GL_FRAGMENT_SHADER, shader[FULLCOLOR].c_str());
    glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &index);

    // set up the Modelmatrix for every cylinder in each corner, send it to the shader and render
    glm::vec3 cylinderPos[] = {glm::vec3(-5.0f,-2.0f,-5.0f), glm::vec3(5.0f,-2.0f,-5.0f), glm::vec3(-5.0f,-2.0f,5.0f), glm::vec3(5.0f,-2.0f,5.0f)};
    for (int i =0; i<4; i++)
    {
        glm::mat4 cylinderModelMatrix = glm::mat4(1.0f);
        cylinderModelMatrix = glm::translate(cylinderModelMatrix, cylinderPos[i]);
        cylinderModelMatrix = glm::scale(cylinderModelMatrix, glm::vec3(0.001f, 0.02f, 0.001f));

        glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(cylinderModelMatrix));
        glUniform3fv(glGetUniformLocation(mainShader.Program, "colorIn"), 1, colorCylinder);
        envModels[Cylinder].Draw();
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////// RENDER THE SMALL CYLINDER FOR THE LIGHTBULB /////////////////////////////////////////////////
    // set the Modelmatrix for the "coord of the Lightbulb", send it to the shader and render
    glm::mat4 cylinderModelMatrix = glm::mat4(1.0f);
    cylinderModelMatrix = glm::translate(cylinderModelMatrix, lightPos + glm::vec3(0.0f,0.15f,0.0f));
    cylinderModelMatrix = glm::scale(cylinderModelMatrix, glm::vec3(0.0001f, 0.01f, 0.0001f));

    glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(cylinderModelMatrix));
    glUniform3fv(glGetUniformLocation(mainShader.Program, "colorIn"), 1, colorCylinder);
    envModels[Cylinder].Draw();
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
}

void drawLines(GLuint framebuffer) 
{
    // set up the vertices Array
    GLfloat vertices [4*numMousePoints] ;

    // for every Mouse point in mouseHist we calculate the direction (difference from current mousepoint to last mousepoint) and store two mouse perpenticular to the direction with distance 2*linesize in vertices
    for (int i = 0; i < numMousePoints; i++ )
    {
        if (i == 0)
        {
            vertices[4*i] = mouseHist[i] + lineSize;
            vertices[4*i+1] = mouseHist[i+1] + lineSize;
            vertices[4*i+2] = mouseHist[i] - lineSize;
            vertices[4*i+3] = mouseHist[i+1] - lineSize;
        } 


        else 
        {
            glm::vec2 lastVert(mouseHist[2*i-2], mouseHist[2*i-1]);
            glm::vec2 currentVert(mouseHist[2*i], mouseHist[2*i+1]);
            glm::vec2 nextVert(mouseHist[2*i+2],mouseHist[2*i+3]);

            glm::vec4 leftPerp = glm::normalize(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f,0.0f,1.0f)) * glm::vec4(glm::normalize(currentVert - lastVert), 0.0f ,0.0f));

            vertices[4*i] = currentVert.x + lineSize * leftPerp.x;
            vertices[4*i+1] = currentVert.y + lineSize * leftPerp.y;
            vertices[4*i+2] = currentVert.x - lineSize * leftPerp.x;
            vertices[4*i+3] = currentVert.y - lineSize * leftPerp.y;
        }
    }

    // then we set up a VBO and VAO to render these vertices as triangle strips
    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    
    // we render them once in the paint framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 2 * numMousePoints - 2);

    // and once in the normal framebuffer, so the user can see the paint strokes
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 2 * numMousePoints - 2);
    glBindVertexArray(0);


    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
 
}

std::vector<GLuint> nearestPortals(glm::vec3 cameraPos)
{
    // We find the nearest two portals to the camera position 
    std::vector<GLuint> index;
    GLuint tempIndex;
    glm::vec3 portalPos[] = {glm::vec3(0.0f,0.0f,5.0f), glm::vec3(5.0f,0.0f,0.0f), glm::vec3(0.0f,0.0f,-5.0f), glm::vec3(-5.0f,0.0f,0.0f)};
    GLfloat compareDist = std::numeric_limits<GLfloat>::max();

    for (int i = 0; i<4 ;i++)
    {
        GLfloat tempDist = glm::length(cameraPos - portalPos[i]);
        if (tempDist < compareDist)
        {
            compareDist = tempDist;
            tempIndex = i;
        }
    }
    index.push_back(tempIndex);

    compareDist = std::numeric_limits<GLfloat>::max();
    for (int i = 0; i<4 ;i++)
    {
        GLfloat tempDist = glm::length(cameraPos - portalPos[i]);
        if (tempDist < compareDist && i != index[0])
        {
            compareDist = tempDist;
            tempIndex = i;
        }
            
    }
    index.push_back(tempIndex);
    
    return index;
}

GLint LoadTexture(const char* path)
{
    GLuint textureImage;
    int w, h, channels;
    unsigned char* image;
    image = stbi_load(path, &w, &h, &channels, STBI_rgb);

    if (image == nullptr)
        std::cout << "Failed to load texture!" << std::endl;

    glGenTextures(1, &textureImage);
    glBindTexture(GL_TEXTURE_2D, textureImage);

    // 3 channels = RGB ; 4 channel = RGBA
    if (channels==3)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    else if (channels==4)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);

    // we set how to consider UVs outside [0,1] range
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // we set the filtering for minification and magnification
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);

    // we free the memory once we have created an OpenGL texture
    stbi_image_free(image);

    // we set the binding to 0 once we have finished
    glBindTexture(GL_TEXTURE_2D, 0);

    return textureImage;

}