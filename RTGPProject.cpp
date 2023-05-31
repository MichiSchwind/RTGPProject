// Std. Includes
#include <string>

#ifdef _WIN32
    #define APIENTRY __stdcall
#endif

#include <glad/glad.h>

// GLFW library to create window and to manage I/O
#include <glfw/glfw3.h>

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

// dimensions of application's window
GLuint screenWidth = 1200, screenHeight = 900;

int room;
void ChangeRoomShader();
void ChangeModel();

// callback functions for keyboard events
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void Do_Movement();

void mouse_callback(GLFWwindow* window, double xPos, double yPos);

// setup of Shader Programs for the 5 shaders used in the application
void SetupShaders(int program);

// Function dealing with the Rendering of the 4 Portals
void PortalRenderLoop(Shader &mainShader,int shaderIndex[], float signum, Model &model, int modelType, Model &planeModel, GLuint VAO);

// print on console the name of current shader
void PrintCurrentShader(int shader);
void PrintCurrentModel(int model);

// parameters for time calculation (for animations)
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// rotation angle on Y axis
GLfloat orientationY = 0.0f;
// rotation speed on Y axis
GLfloat spin_speed = 30.0f;
// boolean to start/stop animated rotation on Y angle
GLboolean spinning = GL_TRUE;

// boolean to activate/deactivate wireframe rendering
GLboolean wireframe = GL_FALSE;

// enum data structure to manage indices for shaders swapping
enum available_ShaderPrograms{ FULLCOLOR, RANDOMNOISE, NORMAL2COLOR,UV2COLOR };
enum availabe_Models{Bunny, Cube, Sphere};
// strings with shaders names to print the name of the current one on console
const char * print_available_ShaderPrograms[] = { "FULLCOLOR", "RANDOMNOISE", "NORMAL2COLOR", "UV2COLOR" };
const char * print_availabe_Models[] = {"Buny", "Cube", "Sphere"};

// index of the current shader (= 0 in the beginning)
GLuint current_program = FULLCOLOR;
GLuint current_Model = Bunny;
// a vector for all the Shader Programs used and swapped in the application
vector<std::string> shader;
vector<Model> models;

// Uniforms to pass to shaders
// color to be passed to Fullcolor and Flatten shaders
GLfloat myColor[] = {1.0f,0.0f,0.0f};
GLfloat colorWhite[] = {0.8f,0.8f,0.8f};
GLfloat colorDarkRed[] = {0.35f,0.0f,0.0f};
GLfloat colorSandstone[] ={222.0f/255.0f,205.0f/255.0f,190.0f/255.0f};

// weight and velocity for the animation of Wave shader
GLfloat currentFrame;
GLfloat weight = 0.2f;
GLfloat speed = 5.0f;

//Set up Camera Position and View Direction
bool keys[1024];

glm::vec3 cameraPos;
glm::vec3 cameraView;
glm::vec3 cameraRight;
glm::vec3 cameraUp;
glm::mat4 view;
glm::vec3 lastCameraPos;
// Projection matrix: FOV angle, aspect ratio, near and far planes
glm::mat4 projection = glm::perspective(45.0f, (float)screenWidth/(float)screenHeight, 0.1f, 10000.0f);

float horizontalAngle;
float verticalAngle;
float lastxPos;
float lastyPos;
bool firstMouse;


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

    // we enable Z test
    glEnable(GL_DEPTH_TEST);

    // we enable face culling - this is important for the portals
    glEnable(GL_CULL_FACE);

    //the "clear" color for the frame buffer
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);

    // we create the Shader Programs used in the application
    Shader mainShader("vertexShader.vert", "fragmentSHader.frag");
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


    


////////////
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

    glBindBuffer(GL_ARRAY_BUFFER, 0); // Note that this is allowed, the call to glVertexAttribPointer registered VBO as the currently bound vertex buffer object so afterwards we can safely unbind

    glBindVertexArray(0); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO, keep it bound to this VAO
 ////////////

    // we print on console the name of the first shader used
    PrintCurrentShader(current_program);

    // we set projection and view matrices
    // N.B.) in this case, the camera is fixed -> we set it up outside the rendering loop
    
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

    // Model and Normal transformation matrices for the objects in the scene: we set to identity
    glm::mat4 ModelMatrix = glm::mat4(1.0f);
    glm::mat3 NormalMatrix = glm::mat3(1.0f);
    //glm::mat4 cubeModelMatrix = glm::mat4(1.0f);
    //glm::mat3 cubeNormalMatrix = glm::mat3(1.0f);
    /*glm::mat4 bunnyModelMatrix = glm::mat4(1.0f);
    glm::mat3 bunnyNormalMatrix = glm::mat3(1.0f);*/
    glm::mat4 planeModelMatrix = glm::mat4(1.0f);
    glm::mat3 planeNormalMatrix = glm::mat3(1.0f);


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
        Do_Movement();
        //ChangeRoomShader();
        //ChangeModel();

        //Update view Matrix
        view = glm::lookAt(cameraPos, cameraPos + cameraView, cameraUp); 

        // we "clear" the frame and z buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        
        // we set the rendering mode
        if (wireframe)
            // Draw in wireframe
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // if animated rotation is activated, then we increment the rotation angle using delta time and the rotation speed parameter
        if (spinning)
            orientationY+=(deltaTime*spin_speed);

        glEnable(GL_STENCIL_TEST);

        glStencilMask(0xFF);
        glClear(GL_STENCIL_BUFFER_BIT);

        // temporal shader index Solution
        int rightFrontShader[] = {FULLCOLOR, RANDOMNOISE};
        int leftBackShader[] = {NORMAL2COLOR, UV2COLOR};

        PortalRenderLoop(mainShader, rightFrontShader, -1.0f, bunnyModel, Bunny, planeModel, VAO);
        PortalRenderLoop(mainShader, leftBackShader, 1.0f, bunnyModel, Bunny, planeModel, VAO);
        
        
        //Set up ModelMatrix for the first Plane
        planeModelMatrix = glm::mat4(1.0f);
        planeNormalMatrix = glm::mat3(1.0f);
        planeModelMatrix = glm::translate(planeModelMatrix, glm::vec3(0.0f,-1.0f,0.0f));
        //planeModelMatrix = glm::rotate(planeModelMatrix, glm::radians(0.0f), glm::vec3(0.0f,0.0f,0.0f));
        planeModelMatrix = glm::scale(planeModelMatrix, glm::vec3(2.0f,1.0f,2.0f));
        // and the NormalMatrix
        planeNormalMatrix = glm::inverseTranspose(glm::mat3(view*planeModelMatrix));

        //Send the Matrizes and the color Uniform to our planeShader
        glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(planeModelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(mainShader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(planeNormalMatrix));
        glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3fv(glGetUniformLocation(mainShader.Program, "colorIn"), 1, colorDarkRed);

        planeModel.Draw();

        //Set up ModelMatrix for the second Plane
        planeModelMatrix = glm::mat4(1.0f);
        planeNormalMatrix = glm::mat3(1.0f);
        planeModelMatrix = glm::translate(planeModelMatrix, glm::vec3(0.0f,-.999f,0.0f));
        //planeModelMatrix = glm::rotate(planeModelMatrix, glm::radians(0.0f), glm::vec3(0.0f,0.0f,0.0f));
        planeModelMatrix = glm::scale(planeModelMatrix, glm::vec3(1.0f,1.0f,1.0f));
        // and the NormalMatrix
        planeNormalMatrix = glm::inverseTranspose(glm::mat3(view*planeModelMatrix));

        //Send the Matrizes and the color Uniform to our planeShader
        glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(planeModelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(mainShader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(planeNormalMatrix));
        glUniform3fv(glGetUniformLocation(mainShader.Program, "colorIn"), 1, colorSandstone);

        planeModel.Draw();
        

        // Swapping back and front buffers
        glfwSwapBuffers(window);
        lastCameraPos = cameraPos;

    }

    // when I exit from the graphics loop, it is because the application is closing
    // we delete the Shader Programs
    mainShader.Delete();
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
    if((key >= GLFW_KEY_1 && key <= GLFW_KEY_5) && action == GLFW_PRESS)
    {
        // "1" to "5" -> ASCII codes from 49 to 57
        // we subtract 48 (= ASCII CODE of "0") to have integers from 1 to 5
        // we subtract 1 to have indices from 0 to 4 in the shaders list
        current_program = (key-'0'-1);
        PrintCurrentShader(current_program);
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
}

//Mouse Inputs 
void mouse_callback(GLFWwindow* window, double xPos, double yPos)
{
    if (firstMouse)
    {
        lastyPos = yPos;
        lastxPos = xPos;
        firstMouse = false;
    }
    glfwGetCursorPos(window, &xPos, &yPos);
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

void ChangeRoomShader() 
{
    if (cameraPos.x < 0.0f && cameraPos.z < -5.0f && lastCameraPos.x > 0 && lastCameraPos.z < -5.0f) 
    {
        current_program = (current_program + 1) % 5;
        PrintCurrentShader(current_program);
    }

    if (cameraPos.x > 0.0f && cameraPos.z < -5.0f && lastCameraPos.x < 0 && lastCameraPos.z < -5.0f) 
    {
        if(current_program == 0) current_program = 4;
        else current_program = (current_program - 1) % 5;
        PrintCurrentShader(current_program);
    }
}

void ChangeModel() 
{
    if (cameraPos.x < 0.0f && cameraPos.z > 5.0f && lastCameraPos.x > 0 && lastCameraPos.z > 5.0f) 
    {
        current_Model = (current_Model + 1) % 3;
        PrintCurrentModel(current_Model);
    }

    if (cameraPos.x > 0.0f && cameraPos.z > 5.0f && lastCameraPos.x < 0 && lastCameraPos.z > 5.0f) 
    {
        if(current_Model == 0) current_Model = 2;
        else current_Model = current_Model - 1;
        PrintCurrentModel(current_Model);
    }
}

void PortalRenderLoop(Shader &mainShader, int shaderIndex[], float signum, Model &model, int modelType, Model &planeModel, GLuint VAO)
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
        // Set up ModelMatrix for the first Plane
        mainShader.Use();
        GLuint index = glGetSubroutineIndex(mainShader.Program, GL_FRAGMENT_SHADER, shader[FULLCOLOR].c_str());
        glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &index);
        glm::mat4 planeModelMatrix = glm::mat4(1.0f);
        glm::mat3 planeNormalMatrix = glm::mat3(1.0f);
        planeModelMatrix = glm::translate(planeModelMatrix, glm::vec3(signum,signum,signum)*PortalPos[i]);
        planeModelMatrix = glm::rotate(planeModelMatrix, signum * glm::radians(90.0f), PortalRotationAxis[i]);
        planeModelMatrix = glm::scale(planeModelMatrix, glm::vec3(5.0f,1.0f,5.0f));
        // and the NormalMatrix
        planeNormalMatrix = glm::inverseTranspose(glm::mat3(view*planeModelMatrix));
        //Send the Matrizes and the color Uniform to our planeShader
        glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(planeModelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(mainShader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(planeNormalMatrix));
        glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3fv(glGetUniformLocation(mainShader.Program, "colorIn"), 1, colorDarkRed);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        // Step Five: Generate the virtual Camera

        
        // Step Six: Disable writing to the Stencil Buffer and Enable Color and Depth Buffer
        glStencilMask(0x00);
        glEnable(GL_DEPTH_TEST);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        // Step Seven: Set the stencil func to lEqual with reference value 1. Therefore we draw only on pixels with a stencil value equal to 1
        // Here is a mistake. We somehow write on pixels we should not write on 
        glStencilFuncSeparate(GL_FRONT, GL_EQUAL, i+1, 0xFF);
        
        // Step Eight: Draw what is inside of the Portal
        // For example:
        planeModelMatrix = glm::mat4(1.0f);
        planeNormalMatrix = glm::mat3(1.0f);
        planeModelMatrix = glm::translate(planeModelMatrix, glm::vec3(0.0f,-1.0f,0.0f));
        planeModelMatrix = glm::scale(planeModelMatrix, glm::vec3(2.0f,1.0f,2.0f));
        //planeModelMatrix = glm::rotate(planeModelMatrix, glm::radians(0.0f), glm::vec3(0.0f,0.0f,0.0f));
        // and the NormalMatrix
        planeNormalMatrix = glm::inverseTranspose(glm::mat3(view*planeModelMatrix));
        //Send the Matrizes and the color Uniform to our planeShader
        glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(planeModelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(mainShader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(planeNormalMatrix));
        glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3fv(glGetUniformLocation(mainShader.Program, "colorIn"), 1, colorDarkRed);
        planeModel.Draw();
        planeModelMatrix = glm::mat4(1.0f);
        planeNormalMatrix = glm::mat3(1.0f);
        planeModelMatrix = glm::translate(planeModelMatrix, glm::vec3(0.0f,-0.999f,0.0f));
        planeModelMatrix = glm::scale(planeModelMatrix, glm::vec3(1.0f,1.0f,1.0f));
        //planeModelMatrix = glm::rotate(planeModelMatrix, glm::radians(0.0f), glm::vec3(0.0f,0.0f,0.0f));
        // and the NormalMatrix
        planeNormalMatrix = glm::inverseTranspose(glm::mat3(view*planeModelMatrix));
        //Send the Matrizes and the color Uniform to our planeShader
        glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(planeModelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(mainShader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(planeNormalMatrix));
        glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3fv(glGetUniformLocation(mainShader.Program, "colorIn"), 1, colorSandstone);
        planeModel.Draw();


        //mainShader.Use();
        // Here we swap the subroutines in the fragment shader
        // first search in the shader program the index corresponding to the portal loop
        index = glGetSubroutineIndex(mainShader.Program, GL_FRAGMENT_SHADER, shader[shaderIndex[i]].c_str());
        // then change the subroutine accordingly
        glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &index);


        glm::mat4 ModelMatrix = glm::mat4(1.0f);
        glm::mat4 NormalMatrix = glm::mat3(1.0f);
        ModelMatrix = glm::translate(ModelMatrix, glm::vec3(0.0f,0.0f,0.0f));
        ModelMatrix = glm::rotate(ModelMatrix, glm::radians(orientationY), glm::vec3(0.0f, 1.0f, 0.0f));
        if (modelType == Bunny)
            ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.3f, 0.3f, 0.3f));
        else
            ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.8f, 0.8f, 0.8f));
        
        // if we cast a mat4 to a mat3, we are automatically considering the upper left 3x3 submatrix
        NormalMatrix = glm::inverseTranspose(glm::mat3(view*ModelMatrix));
        glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(ModelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(mainShader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(NormalMatrix));
        glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));
        glUniform3fv(glGetUniformLocation(mainShader.Program, "colorIn"), 1, myColor);
        glUniform1f(glGetUniformLocation(mainShader.Program, "weight"), weight);
        glUniform1f(glGetUniformLocation(mainShader.Program, "timer"), currentFrame*speed);
        models[modelType].Draw();

        // Step Nine: Disable Color Buffer and Stencil Test but enable writing to the depth buffer
        glDisable(GL_STENCIL_TEST);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glDepthMask(GL_TRUE);
        // STep Ten: CLear the Depth Buffer
        //glClear(GL_DEPTH_BUFFER_BIT);

        // Step Eleven: Draw our Portal againg. This time in the Depth Buffer
        index = glGetSubroutineIndex(mainShader.Program, GL_FRAGMENT_SHADER, shader[FULLCOLOR].c_str());
        glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &index);
        //Set up ModelMatrix for the first Plane
        planeModelMatrix = glm::mat4(1.0f);
        planeNormalMatrix = glm::mat3(1.0f);
        planeModelMatrix = glm::translate(planeModelMatrix, glm::vec3(signum,signum,signum)*PortalPos[i]);
        planeModelMatrix = glm::rotate(planeModelMatrix, signum * glm::radians(90.0f), PortalRotationAxis[i]);
        planeModelMatrix = glm::scale(planeModelMatrix, glm::vec3(5.0f,1.0f,5.0f));
        // and the NormalMatrix
        planeNormalMatrix = glm::inverseTranspose(glm::mat3(view*planeModelMatrix));
        //Send the Matrizes and the color Uniform to our planeShader
        glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(planeModelMatrix));
        glUniformMatrix3fv(glGetUniformLocation(mainShader.Program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(planeNormalMatrix));
        glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(mainShader.Program, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3fv(glGetUniformLocation(mainShader.Program, "colorIn"), 1, colorDarkRed);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        // Step Twelve: Enable Color Buffer Again
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        //glEnable(GL_STENCIL_TEST);
        //glStencilMask(0x00);
        //glStencilFunc(GL_EQUAL, 1, 0xFF);
    }

}