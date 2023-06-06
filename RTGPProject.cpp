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



// dimensions of application's window
GLuint screenWidth = 1200, screenHeight = 900;

// callback functions for keyboard events
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

//////// Maybe camera.h?
void Do_Movement();

void mouse_callback(GLFWwindow* window, double xPos, double yPos);

// setup of Shader Programs - This could be worked on
void SetupShaders(int program);

// setup VAO for Portal
GLuint SetupPortal();

// Function for rendering Objects
void RenderObjects(Shader &mainShader, GLuint shaderIndex, int modelType, Model &planeModel, int render_pass, int loopIter = 0);

// Function dealing with the Rendering of the 4 Portals
void PortalRenderLoop(Shader &mainShader,GLuint shaderIndex, float signum, int modelType, Model &planeModel, GLuint VAO, int render_pass);

// manage the Shader and Modelindices that gets rendering in each Portal
void ManagePortalContent();

// print on console the name of current shader
void PrintCurrentShader(int shader);
void PrintCurrentModel(int model);

// set the Shader for Model rendered inside the Portals
void setInsideShader(GLuint rightFrontShader, GLuint leftBackShader);

//  Function that calls the IMGui if Space is pressed
void toggleIMGui();


// parameters for time calculation (for animations)
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// rotation angle on Y axis
GLfloat orientationY = 0.0f;
// rotation speed on Y axis
GLfloat spinspeed = 0.5f;
// boolean to start/stop animated rotation on Y angle
GLboolean spinning = GL_TRUE;

// boolean to activate/deactivate wireframe rendering
GLboolean wireframe = GL_FALSE;

enum render_passes{ SHADOWMAP, RENDER};
// enum data structure to manage indices for shaders swapping
enum available_ShaderPrograms{LambertianPlusShadow, PhongPlusShadow, BlinnPhongPlusShadow, GGXPlusShadow, Normal2ColorPlusLambertian, Normal2ColorPlusBlinnPhong, UV2ColorPlusLambertian,UV2ColorPlusBlinnPhong, FULLCOLOR };
const int NumShader = 4;
enum availabe_Models{Bunny, Cube, Sphere};
const int NumModel = 3;
// strings with shaders names to print the name of the current one on console
const char * print_available_ShaderPrograms[] = { "Lambertian", "Phong", "BlinnPhong", "GGX", "Normal2ColorPlusLambertian", "Normal2ColorPlusBlinnPhong", "UV2ColorPlusLambertian", "UV2ColorPlusBlinnPhong", "FULLCOLOR"};
const char * print_availabe_Models[] = {"Buny", "Cube", "Sphere"};

// index of the current shader (= 0 in the beginning)
GLuint currentProgramFrontRight = LambertianPlusShadow;
GLuint currentProgramBackLeft = BlinnPhongPlusShadow;
GLuint currentProgramInside = LambertianPlusShadow;
GLuint currentModelFrontRight = Bunny;
GLuint currentModelBackLeft = Sphere;
GLuint currentModelInside = Bunny;

// a vector for all the Shader Programs used and swapped in the application
vector<std::string> shader;
vector<Model> models;

// Uniforms to pass to shaders
// color to be passed to Fullcolor and Flatten shaders
GLfloat myColor[] = {1.0f,0.0f,0.0f};
GLfloat colorWhite[] = {0.8f,0.8f,0.8f};
GLfloat colorDarkRed[] = {0.35f,0.0f,0.0f};
GLfloat colorSandstone[] ={222.0f/255.0f,205.0f/255.0f,190.0f/255.0f};

// Light Informations
// Light Positions
glm::vec3 lightPos = glm::vec3(0.0f,2.0f,4.0f);
glm::vec3 lightRight = glm::vec3(0.0f,0.0f,-1.0f);
glm::vec3 lightPointsTo = glm::vec3(1.0f,1.0f,1.0f);

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
GLfloat F0 = 0.9f;

// weight and velocity for the animation of Wave shader
GLfloat currentFrame;
GLfloat weight = 0.2f;
GLfloat speed = 5.0f;

//Set up Camera Position and View Direction
bool keys[1024] = {0};

glm::vec3 cameraPos;
glm::vec3 lastCameraPos;
glm::vec3 cameraView;
glm::vec3 cameraRight;
glm::vec3 cameraUp;
glm::mat4 view;


// Projection matrix: FOV angle, aspect ratio, near and far planes
glm::mat4 projection = glm::perspective(45.0f, (float)screenWidth/(float)screenHeight, 0.1f, 10000.0f);

float horizontalAngle;
float verticalAngle;
float lastxPos;
float lastyPos;
bool firstMouse;

GLuint depthMap[3];

/////////////////// MAIN function ///////////////////////
int main()
{
    // Initialization of OpenGL context using GLFW
    glfwInit();
    // We set OpenGL specifications required for this application
    // In this case: 4.1 Core
    // It is possible to raise the values, in order to use functionalities of more recent OpenGL specs.
    // If not supported by your graphics HW, the context will not be created and the application will close
    // N.B.) creating GLAD code to load extensions, try to take into account the specifications and any extensions you want to use,
    // in relation also to the values indicated in these GLFW commands
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    // we set if the window is resizable
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // we create the application's window
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "MY_TESTING_KIT", nullptr, nullptr);
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

    // we enable face culling - this is important for the portals
    glEnable(GL_CULL_FACE);

    //the "clear" color for the frame buffer
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);

    // we create the Shader Programs used in the application
    Shader mainShader("vertexShader.vert", "fragmentSHader.frag");
    Shader shadowShader("19_shadowmap.vert", "20_shadowmap.frag");
    //Shader planeShader("00_basic.vert", "01_fullcolor.frag");
    SetupShaders(mainShader.Program);

    // we load the model(s) (code of Model class is in include/utils/model.h)
    Model cubeModel("models/cube.obj");
    Model sphereModel("models/sphere.obj");
    Model bunnyModel("models/bunny_lp.obj");
    Model planeModel("models/plane.obj");

    //Model david ("../../models/David.obj");
    models.push_back(std::move(bunnyModel));
    models.push_back(std::move(cubeModel));
    models.push_back(std::move(sphereModel));

    // we set up the Portalmesh
    GLuint PortalVAO = SetupPortal();

    // we print on console the name of the first shader used
    PrintCurrentShader(currentProgramInside);
    
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

    /////////////////// CREATION OF BUFFER FOR THE  DEPTH MAP /////////////////////////////////////////
    // buffer dimension: too large -> performance may slow down if we have many lights; too small -> strong aliasing
    const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    GLuint depthMapFBO[3];
    for (int i =0 ;  i < 3; i++) 
    {
        // we create a Frame Buffer Object: the first rendering step will render to this buffer, and not to the real frame buffer
        glGenFramebuffers(1, &depthMapFBO[i]);
        // we create a texture for the depth map
        glGenTextures(1, &depthMap[i]);
        glBindTexture(GL_TEXTURE_2D, depthMap[i]);
        // in the texture, we will save only the depth data of the fragments. Thus, we specify that we need to render only depth in the first rendering     step
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        // we set to clamp the uv coordinates outside [0,1] to the color of the border
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        // outside the area covered by the light frustum, everything is rendered in shadow (because we set GL_CLAMP_TO_BORDER)
        // thus, we set the texture border to white, so to render correctly everything not involved by the shadow map
        //*************
        GLfloat borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

        // we bind the depth map FBO
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap[i], 0);
        // we set that we are not calculating nor saving color data
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    ///////////////////////////////////////////////////////////////////


    // Rendering loop: this code is executed at each frame
    while(!glfwWindowShouldClose(window))
    {
        // we determine the time passed from the beginning
        // and we calculate time difference between current frame rendering and the previous one
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Check is an I/O event is happening
        glfwPollEvents();

        if (!keys[GLFW_KEY_SPACE])
            Do_Movement();
        
        // Manage the Model and Shader that are rendered in each Portal
        ManagePortalContent();
        // set the Model and Shader that gets renedered inside the Portalcube
        setInsideShader(currentProgramFrontRight, currentProgramBackLeft);


        //Update view Matrix
        view = glm::lookAt(cameraPos, cameraPos + cameraView, cameraUp); 
        
        // we set the rendering mode
        if (wireframe)
            // Draw in wireframe
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // if animated rotation is activated, then we increment the rotation angle using delta time and the rotation speed parameter
        if (spinning)
            lightPos = glm::mat3(glm::rotate(glm::mat4(1.0f), glm::radians(spinspeed), glm::vec3(0.0f,1.0f,0.0f))) * lightPos;
            lightRight = glm::mat3(glm::rotate(glm::mat4(1.0f), glm::radians(spinspeed), glm::vec3(0.0f,1.0f,0.0f))) * lightRight;


        /////////////////// STEP 1 - SHADOW MAP: RENDERING OF SCENE FROM LIGHT POINT OF VIEW ////////////////////////////////////////////////
        // we set view and projection matrix for the rendering using light as a camera
        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        // for a directional light, the projection is orthographic. For point lights, we should use a perspective projection
        lightProjection = glm::perspective(glm::radians(80.0f), 1.0f, 2.0f, 50.0f);
        // the light is directional, so technically it has no position. We need a view matrix, so we consider a position on the the directionvector    of the light
        lightView = glm::lookAt(lightPos, lightPointsTo, glm::normalize(glm::cross(lightPointsTo, lightRight)));
        // transformation matrix for the light
        lightSpaceMatrix = lightProjection * lightView;
        /// We "install" the  Shader Program for the shadow mapping creation
        shadowShader.Use();
        // we pass the transformation matrix as uniform
        glUniformMatrix4fv(glGetUniformLocation(shadowShader.Program, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
        // we set the viewport for the first rendering step = dimensions of the depth texture
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);

        for (int i:{currentModelFrontRight, currentModelBackLeft})
        {
            // we activate the FBO for the depth map rendering
            glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO[i]);
            glClear(GL_DEPTH_BUFFER_BIT);

            // we render the scene, using the shadow shader

            // Render the Inside of the Portalcube
            RenderObjects(shadowShader, 0, i, planeModel, SHADOWMAP);
        }

        /////// MAIN RENDERING LOOP /////////
        // we "clear" the frame and z buffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, width, height);
        glEnable(GL_STENCIL_TEST);
        glStencilMask(0xFF);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


        // activate the main Shader
        mainShader.Use();

        // Send the uniforms containing the light information
        glUniform3fv(glGetUniformLocation(mainShader.Program, "lightPos"), 1, glm::value_ptr(lightPos));    
        glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix)); 
        glUniform3fv(glGetUniformLocation(mainShader.Program, "ambientColor"), 1, ambientColor);
        glUniform3fv(glGetUniformLocation(mainShader.Program, "specularColor"), 1, specularColor);
        glUniform1f(glGetUniformLocation(mainShader.Program, "shininess"), shininess);
        glUniform1f(glGetUniformLocation(mainShader.Program, "alpha"), alpha);
        glUniform1f(glGetUniformLocation(mainShader.Program, "F0"), F0);
        glUniform1f(glGetUniformLocation(mainShader.Program, "Ka"), Ka);
        glUniform1f(glGetUniformLocation(mainShader.Program, "Kd"), Kd);
        glUniform1f(glGetUniformLocation(mainShader.Program, "Ks"), Ks);

        // Render Portals plus what's inside of them
        PortalRenderLoop(mainShader, currentProgramFrontRight, -1.0f, currentModelFrontRight, planeModel, PortalVAO, RENDER);
        PortalRenderLoop(mainShader, currentProgramBackLeft, 1.0f,currentModelBackLeft, planeModel, PortalVAO, RENDER);
        
        // Render the Inside of the Portalcube
        RenderObjects(mainShader, currentProgramInside, currentModelInside, planeModel, RENDER);

        if (keys[GLFW_KEY_SPACE])
            toggleIMGui();

        // Swapping back and front buffers
        glfwSwapBuffers(window);
        lastCameraPos = cameraPos;

    }

    // when I exit from the graphics loop, it is because the application is closing
    // we delete the Shader Programs
    mainShader.Delete();

    //Delete the IMGui context
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // we close and delete the created context
    glfwTerminate();
    return 0;
}


//////////////////////////////////////////
// we create and compile shaders (code of Shader class is in include/utils/shader.h), and we add them to the list of available shaders
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

//////////////////////////////////////////
// we print on console the name of the currently used shader
void PrintCurrentShader(int shader)
{
    std::cout << "Current shader: " << print_available_ShaderPrograms[shader]  << std::endl;

}

void PrintCurrentModel(int model)
{
    std::cout << "Current Model: " << print_availabe_Models[model] << std::endl;
}

//////////////////////////////////////////
// callback for keyboard events
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    // if ESC is pressed, we close the application
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    // if P is pressed, we start/stop the animated rotation of models
    if(key == GLFW_KEY_P && action == GLFW_PRESS)
        spinning=!spinning;

    // if L is pressed, we activate/deactivate wireframe rendering of models
    if(key == GLFW_KEY_L && action == GLFW_PRESS)
        wireframe=!wireframe;

    // pressing a key between 1 and 5, we change the shader applied to the models
    if((key >= GLFW_KEY_1 && key <= GLFW_KEY_8) && action == GLFW_PRESS)
    {
        // "1" to "5" -> ASCII codes from 49 to 57
        // we subtract 48 (= ASCII CODE of "0") to have integers from 1 to 5
        // we subtract 1 to have indices from 0 to 4 in the shaders list
        currentProgramInside= (key-'0'-1);
        PrintCurrentShader(currentProgramInside);
    }

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

//Mouse Inputs 
void mouse_callback(GLFWwindow* window, double xPos, double yPos)
{   
    if (!keys[GLFW_KEY_SPACE])
    {
        glfwGetCursorPos(window, &xPos, &yPos);

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

        // use glm::Radians to avoid turning the camera upside down i guess
        cameraView = glm::normalize(glm::vec3(cos(verticalAngle) * sin(horizontalAngle), sin(verticalAngle), cos(verticalAngle)*cos(horizontalAngle)));
        cameraRight = glm::normalize(glm::vec3(sin(horizontalAngle - 3.14f/2.0f), 0, cos(horizontalAngle - 3.14f/2.0f)));
        cameraUp = glm::cross(cameraRight, cameraView);
    }
}

void Do_Movement()
{
     //Let the camara "walk" using w,a,s,d
    if(keys[GLFW_KEY_W] == true)
    {
        cameraPos = cameraPos + glm::normalize(glm::vec3(cameraView.x,0,cameraView.z)) * 3.0f * deltaTime;
    }
    if(keys[GLFW_KEY_A] == true)
    {
        cameraPos = cameraPos - glm::normalize(glm::vec3(cameraRight.x,0,cameraRight.z)) * 3.0f * deltaTime;

    }
    if(keys[GLFW_KEY_S] == true)
    {
        cameraPos = cameraPos - glm::normalize(glm::vec3(cameraView.x,0,cameraView.z)) * 3.0f * deltaTime;

    }
    if(keys[GLFW_KEY_D] == true)
    {
        cameraPos = cameraPos + glm::normalize(glm::vec3(cameraRight.x,0,cameraRight.z)) * 3.0f * deltaTime;

    }
}

void ManagePortalContent() 
{
    glm::vec3 tempRotatetCameraPos = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0.0f,1.0f,0.0f)) * glm::vec4(cameraPos,1.0f);
    glm::vec3 lasttempRotatetCameraPos = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0.0f,1.0f,0.0f)) * glm::vec4(lastCameraPos,1.0f);

    if (7.0f < tempRotatetCameraPos.x && tempRotatetCameraPos.x < 14.14f && tempRotatetCameraPos.z < 0.0f && lasttempRotatetCameraPos.z >= 0.0f)
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

    if (-14.14f < tempRotatetCameraPos.x && tempRotatetCameraPos.x < -7.0f && tempRotatetCameraPos.z > 0.0f && lasttempRotatetCameraPos.z <= 0.0f)
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
}

void PortalRenderLoop(Shader &mainShader, GLuint shaderIndex, float signum, int modelType, Model &planeModel, GLuint VAO, int render_pass)
{
    vector<glm::vec3> PortalPos = {glm::vec3(0.0f,0.0f,-5.0f), glm::vec3(-5.0f,0.0f,0.0f)};
    vector<glm::vec3> PortalRotationAxis = {glm::vec3(1.0f,0.0f,0.0f),glm::vec3(0.0f,0.0f,-1.0f)};

    for (int i = 0; i < 2; i++)
    {
        // Lets do Portals 
        // Step One: Disable Color and Depth Buffer. Enable Stencil Buffer
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_STENCIL_TEST);
        glStencilMask(0xFF);


        // Step Two: Set Stecnil Opereation to increasing when stencil test fails
        glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_KEEP);
    

        // Step Three: Set Stencil Test to ALWAYS, therefore it will always pass and increase the stencil value
        glStencilFuncSeparate(GL_FRONT, GL_ALWAYS, i+1, 0xFF);
        glStencilFuncSeparate(GL_BACK, GL_NEVER, 0, 0xFF);


        // Step Four: Draw Portal Frame
        // Set up ModelMatrix and Normalmatrix for the PortalFrame
        glm::mat4 planeModelMatrix = glm::mat4(1.0f);
        glm::mat3 planeNormalMatrix = glm::mat3(1.0f);
        planeModelMatrix = glm::translate(planeModelMatrix, glm::vec3(signum,signum,signum)*PortalPos[i]);
        planeModelMatrix = glm::rotate(planeModelMatrix, signum * glm::radians(90.0f), PortalRotationAxis[i]);
        planeModelMatrix = glm::scale(planeModelMatrix, glm::vec3(5.0f,1.0f,5.0f));
        // and the NormalMatrix
        planeNormalMatrix = glm::inverseTranspose(glm::mat3(view*planeModelMatrix));
        
        //Send the Matrizes and the color Uniform to our mainShader
        glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(planeModelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(mainShader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(planeNormalMatrix));
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
        glStencilFuncSeparate(GL_FRONT, GL_EQUAL, i+1, 0xFF);
        

        // Step Seven: Draw what is inside of the Portal
        RenderObjects(mainShader, shaderIndex + i, modelType, planeModel, render_pass);

        // Step Eight: Disable Color Buffer and Stencil Test but enable writing to the depth buffer
        glDisable(GL_STENCIL_TEST);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glDepthMask(GL_TRUE);


        // Step Nine: Draw our Portal againg. This time in the Depth Buffer
        //Set up ModelMatrix for the first Plane
        planeModelMatrix = glm::mat4(1.0f);
        planeNormalMatrix = glm::mat3(1.0f);
        planeModelMatrix = glm::translate(planeModelMatrix, glm::vec3(signum,signum,signum)*PortalPos[i]);
        planeModelMatrix = glm::rotate(planeModelMatrix, signum * glm::radians(90.0f), PortalRotationAxis[i]);
        planeModelMatrix = glm::scale(planeModelMatrix, glm::vec3(5.0f,1.0f,5.0f));
        // and the NormalMatrix
        planeNormalMatrix = glm::inverseTranspose(glm::mat3(view*planeModelMatrix));
        //Send the Matrizes and the color Uniform to our mainSHader
        glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(planeModelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(mainShader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(planeNormalMatrix));
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
    // Define Portals by hand 
    GLfloat vertices[] = {
         1.0f,  0.0f,-1.0f,  // Top Right
         1.0f,  0.0f, 1.0f,  // Bottom Right
        -1.0f,  0.0f, 1.0f,  // Bottom Left
        -1.0f,  0.0f,-1.0f   // Top Left
    };

    GLuint indices[] = {  // Note that we start from 0!
        0, 1, 2,  // First Triangle
        2, 3, 0   // Second Triangle
    };

    GLuint VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
    glBindVertexArray(VAO); 
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); 

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);  

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);   

    glBindBuffer(GL_ARRAY_BUFFER, 0); // Note that this is allowed, the call to glVertexAttribPointer registered VBO as the currently bound vertex  buffer object so afterwards we can safely unbind  

    glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO, keep it bound to this VAO

    return VAO;
}

void setInsideShader(GLuint rightFrontShader, GLuint leftBackShader)
{
    if(cameraPos.z < 5.2f && std::abs(cameraPos.x) <= 5.2f && lastCameraPos.z > 5.2f)
    {
        currentProgramInside = rightFrontShader;
        currentModelInside = currentModelFrontRight;
    }

    if(cameraPos.x < 5.2f && std::abs(cameraPos.z) <= 5.2f && lastCameraPos.x > 5.2f)
    {
        currentProgramInside = rightFrontShader +1;
        currentModelInside = currentModelFrontRight;
    }

    if(cameraPos.z > -5.2f && std::abs(cameraPos.x) <= 5.2f && lastCameraPos.z < -5.2f)
    {
        currentProgramInside = leftBackShader;
        currentModelInside = currentModelBackLeft;
    }
    
    if(cameraPos.x > -5.2f && std::abs(cameraPos.z) <= 5.2f && lastCameraPos.x < -5.2f)
    {
        currentProgramInside = leftBackShader +1;
        currentModelInside = currentModelBackLeft;
    }
}

void toggleIMGui()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    // ImGUI window creation
    ImGui::Begin("Settings");
    // Text that appears in the window
    ImGui::ColorEdit4("Set Color", myColor);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);
    ImGui::Text("Mouse position: (%.5f, %.5f)", lastxPos, lastyPos);
    ImGui::Text("Width, height: (%.0f, %.0f)", float(screenWidth), float(screenHeight));
    // Ends of imgui
    ImGui::End();
    // Renders the ImGUI elements
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void RenderObjects(Shader &mainShader, GLuint shaderIndex, int modelType, Model &planeModel, int render_pass, int loopIter)
{
    if (render_pass==RENDER)
    {
        glActiveTexture(GL_TEXTURE0 + modelType);
        glBindTexture(GL_TEXTURE_2D, depthMap[modelType]);
        GLint shadowLocation = glGetUniformLocation(mainShader.Program, "shadowMap");
        glUniform1i(shadowLocation, modelType);
    }

    // set the subroutine to FULLCOLOR for the FLoorplanes
    GLuint index = glGetSubroutineIndex(mainShader.Program, GL_FRAGMENT_SHADER, shader[FULLCOLOR].c_str());
    glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &index);

    // we set the Modelmatrix and Normalmatrix for the larger Floorplane
    glm::mat4 planeModelMatrix = glm::mat4(1.0f);
    glm::mat3 planeNormalMatrix = glm::mat3(1.0f);
    planeModelMatrix = glm::translate(planeModelMatrix, glm::vec3(0.0f,-1.0f,0.0f));
    planeModelMatrix = glm::scale(planeModelMatrix, glm::vec3(2.0f,1.0f,2.0f));
    //planeModelMatrix = glm::rotate(planeModelMatrix, glm::radians(0.0f), glm::vec3(0.0f,0.0f,0.0f));
    // and the NormalMatrix
    planeNormalMatrix = glm::inverseTranspose(glm::mat3(view*planeModelMatrix));

    //Send the Matrizes and the color Uniform to our mainShader
    glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(planeModelMatrix));
    glUniformMatrix3fv(glGetUniformLocation(mainShader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(planeNormalMatrix));
    glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(glGetUniformLocation(mainShader.Program, "colorIn"), 1, colorDarkRed);
    planeModel.Draw();


    // we set the Model and Normalmatrix for the smaller Floorplane
    planeModelMatrix = glm::mat4(1.0f);
    planeNormalMatrix = glm::mat3(1.0f);
    planeModelMatrix = glm::translate(planeModelMatrix, glm::vec3(0.0f,-0.999f,0.0f));
    planeModelMatrix = glm::scale(planeModelMatrix, glm::vec3(1.0f,1.0f,1.0f));
    //planeModelMatrix = glm::rotate(planeModelMatrix, glm::radians(0.0f), glm::vec3(0.0f,0.0f,0.0f));
    // and the NormalMatrix
    planeNormalMatrix = glm::inverseTranspose(glm::mat3(view*planeModelMatrix));

    //Send the Matrizes and the color Uniform to our mainShader
    glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(planeModelMatrix));
    glUniformMatrix3fv(glGetUniformLocation(mainShader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(planeNormalMatrix));
    glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(glGetUniformLocation(mainShader.Program, "colorIn"), 1, colorSandstone);
    planeModel.Draw();


    // Here we swap the subroutines in the fragment shader
    // first search in the shader program the index corresponding to the portal loop
    index = glGetSubroutineIndex(mainShader.Program, GL_FRAGMENT_SHADER, shader[shaderIndex+loopIter].c_str());
    // then change the subroutine accordingly
    glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &index);

    // set the Model and Normalmatrix for the model
    glm::mat4 ModelMatrix = glm::mat4(1.0f);
    glm::mat3 NormalMatrix = glm::mat3(1.0f);
    ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f,0.0f,0.0f));
    //ModelMatrix = glm::rotate(ModelMatrix, glm::radians(orientationY), glm::vec3(0.0f, 1.0f, 0.0f));
    if (modelType == Bunny)
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.3f, 0.3f, 0.3f));
    else
        ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.8f, 0.8f, 0.8f));
    NormalMatrix = glm::inverseTranspose(glm::mat3(view*ModelMatrix));

    glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(ModelMatrix));
    glUniformMatrix3fv(glGetUniformLocation(mainShader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(NormalMatrix));
    glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));
    glUniform3fv(glGetUniformLocation(mainShader.Program, "colorIn"), 1, myColor);
    glUniform1f(glGetUniformLocation(mainShader.Program, "weight"), weight);
    glUniform1f(glGetUniformLocation(mainShader.Program, "timer"), currentFrame*speed);
    models[modelType].Draw();
}