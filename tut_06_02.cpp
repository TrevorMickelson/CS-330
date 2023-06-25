#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>      // Image loading Utility functions

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnOpengl/camera.h> // Camera class

using namespace std; // Standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    const char* const WINDOW_TITLE = "Tutorial 6.2"; // Macro for window title

    // Variables for window width and height
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;

    // Stores the GL data relative to a given mesh
    struct GLMesh
    {
        GLuint vao;         // Handle for the vertex array object
        GLuint vbo;         // Handle for the vertex buffer object
        GLuint nVertices;    // Number of indices of the mesh
    };

    // Main GLFW window
    GLFWwindow* gWindow = nullptr;
    // Triangle mesh data
    GLMesh gMesh;
    GLMesh gCylinder;
    GLMesh gSphere;
    // Shader programs
    GLuint gCubeProgramId;
    GLuint gLampProgramId;

    // Texture
    GLuint gTextureId;
    glm::vec2 gUVScale(5.0f, 5.0f);
    GLint gTexWrapMode = GL_REPEAT;

    // camera
    Camera gCamera(glm::vec3(0.0f, 0.0f, 7.0f));
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    // timing
    float gDeltaTime = 0.0f; // time between current frame and last frame
    float gLastFrame = 0.0f;

    // Subject position and scale
    glm::vec3 gRectanglePosition(0.0f, 0.0f, 0.0f);
    glm::vec3 gRectangleScale(4.5f, 2.0f, 0.5f);

    // Cube and light color
    glm::vec3 gObjectColor(1.0f, 0.2f, 0.0f);
    glm::vec3 gLightColor(2.0f, 2.0f, 2.0f); // Increase the RGB values for a brighter light

    // Light position and scale
    glm::vec3 gLightPosition(2.0f, 1.0f, 3.0f);
    glm::vec3 gLightScale(5.0f);

    // Cylinder position and scale
    glm::vec3 gCylinderPosition(0.0f, 0.0f, 0.0f); // Update the position as per your requirement
    glm::vec3 gCylinderScale(1.0f, 2.0f, 1.0f); // Update the scale as per your requirement

    // Lamp animation
    bool gIsLampOrbiting = true;

    // Perspective mode
    bool isIn3DMode = true;
}

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void UCreateMesh(GLMesh& mesh, GLMesh& cylinder, GLMesh& sphere);
void UDestroyMesh(GLMesh& mesh);
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);


/* Cube Vertex Shader Source Code*/
const GLchar* cubeVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data
layout(location = 1) in vec3 normal; // VAP position 1 for normals
layout(location = 2) in vec2 textureCoordinate;

out vec3 vertexNormal; // For outgoing normals to fragment shader
out vec3 vertexFragmentPos; // For outgoing color / pixels to fragment shader
out vec2 vertexTextureCoordinate;

//Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates

    vertexFragmentPos = vec3(model * vec4(position, 1.0f)); // Gets fragment / pixel position in world space only (exclude view and projection)

    vertexNormal = mat3(transpose(inverse(model))) * normal; // get normal vectors in world space only and exclude normal translation properties
    vertexTextureCoordinate = textureCoordinate;
}
);

/* Cube Fragment Shader Source Code*/
const GLchar* cubeFragmentShaderSource = GLSL(440,

    in vec3 vertexNormal; // For incoming normals
in vec3 vertexFragmentPos; // For incoming fragment position
in vec2 vertexTextureCoordinate;

out vec4 fragmentColor; // For outgoing cube color to the GPU

// Uniform / Global variables for object color, light color, light position, and camera/view position
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPosition;
uniform sampler2D uTexture; // Useful when working with multiple textures
uniform vec2 uvScale;


void main()
{
    /*Phong lighting model calculations to generate ambient, diffuse, and specular components*/

    //Calculate Ambient lighting*/
    float ambientStrength = 0.1f; // Set ambient or global lighting strength
    vec3 ambient = ambientStrength * lightColor; // Generate ambient light color

    //Calculate Diffuse lighting*/
    vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit
    vec3 lightDirection = normalize(lightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
    float impact = max(dot(norm, lightDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light
    vec3 diffuse = impact * lightColor; // Generate diffuse light color

    //Calculate Specular lighting*/
    float specularIntensity = 0.8f; // Set specular light strength
    float highlightSize = 16.0f; // Set specular highlight size
    vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
    vec3 reflectDir = reflect(-lightDirection, norm);// Calculate reflection vector
    //Calculate specular component
    float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
    vec3 specular = specularIntensity * specularComponent * lightColor;

    // Texture holds the color to be used for all three components
    vec4 textureColor = texture(uTexture, vertexTextureCoordinate * uvScale);

    // Calculate phong result
    vec3 phong = (ambient + diffuse + specular) * textureColor.xyz;

    fragmentColor = vec4(phong, 1.0); // Send lighting results to GPU
}
);

/* Lamp Shader Source Code*/
const GLchar* lampVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data

//Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
}
);


/* Fragment Shader Source Code*/
const GLchar* lampFragmentShaderSource = GLSL(440,

    out vec4 fragmentColor; // For outgoing lamp color (smaller cube) to the GPU

void main()
{
    fragmentColor = vec4(1.0f); // Set color to white (1.0f,1.0f,1.0f) with alpha 1.0
}
);


// Images are loaded with Y axis going down, but OpenGL's Y axis goes up, so let's flip it
void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
    for (int j = 0; j < height / 2; ++j)
    {
        int index1 = j * width * channels;
        int index2 = (height - 1 - j) * width * channels;

        for (int i = width * channels; i > 0; --i)
        {
            unsigned char tmp = image[index1];
            image[index1] = image[index2];
            image[index2] = tmp;
            ++index1;
            ++index2;
        }
    }
}


int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    // Create the mesh
    UCreateMesh(gMesh, gCylinder, gSphere); // Calls the function to create the Vertex Buffer Object

    // Create the shader programs
    if (!UCreateShaderProgram(cubeVertexShaderSource, cubeFragmentShaderSource, gCubeProgramId))
        return EXIT_FAILURE;

    if (!UCreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource, gLampProgramId))
        return EXIT_FAILURE;

    // Load texture
    const char* texFilename = "../../resources/textures/smiley.png";
    if (!UCreateTexture(texFilename, gTextureId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gCubeProgramId);
    // We set the texture as texture unit 0
    glUniform1i(glGetUniformLocation(gCubeProgramId, "uTexture"), 0);

    // Sets the background color of the window to black (it will be implicitely used by glClear)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(gWindow))
    {
        // per-frame timing
        // --------------------
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        // input
        // -----
        UProcessInput(gWindow);

        // Render this frame
        URender();

        glfwPollEvents();
    }

    // Release mesh data
    UDestroyMesh(gMesh);

    // Release texture
    UDestroyTexture(gTextureId);

    // Release shader programs
    UDestroyShaderProgram(gCubeProgramId);
    UDestroyShaderProgram(gLampProgramId);

    exit(EXIT_SUCCESS); // Terminates the program successfully
}


// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    // GLFW: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW: window creation
    // ---------------------
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);
    glfwSetCursorPosCallback(*window, UMousePositionCallback);
    glfwSetScrollCallback(*window, UMouseScrollCallback);
    glfwSetMouseButtonCallback(*window, UMouseButtonCallback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLEW: initialize
    // ----------------
    // Note: if using GLEW version 1.13 or earlier
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }

    // Displays GPU OpenGL version
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{
    static const float cameraSpeed = 2.5f;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        gCamera.ProcessKeyboard(LEFT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        gCamera.ProcessKeyboard(RIGHT, gDeltaTime);

    // Up
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        gCamera.ProcessKeyboard(UP, gDeltaTime);

    // Down
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        gCamera.ProcessKeyboard(DOWN, gDeltaTime);

    // 2D
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
        isIn3DMode = false;
    }

    // 3D
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
        isIn3DMode = true;
    }

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && gTexWrapMode != GL_REPEAT)
    {
        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glBindTexture(GL_TEXTURE_2D, 0);

        gTexWrapMode = GL_REPEAT;

        cout << "Current Texture Wrapping Mode: REPEAT" << endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && gTexWrapMode != GL_MIRRORED_REPEAT)
    {
        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glBindTexture(GL_TEXTURE_2D, 0);

        gTexWrapMode = GL_MIRRORED_REPEAT;

        cout << "Current Texture Wrapping Mode: MIRRORED REPEAT" << endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && gTexWrapMode != GL_CLAMP_TO_EDGE)
    {
        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        gTexWrapMode = GL_CLAMP_TO_EDGE;

        cout << "Current Texture Wrapping Mode: CLAMP TO EDGE" << endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS && gTexWrapMode != GL_CLAMP_TO_BORDER)
    {
        float color[] = { 1.0f, 0.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);

        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glBindTexture(GL_TEXTURE_2D, 0);

        gTexWrapMode = GL_CLAMP_TO_BORDER;

        cout << "Current Texture Wrapping Mode: CLAMP TO BORDER" << endl;
    }

    if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS)
    {
        gUVScale += 0.1f;
        cout << "Current scale (" << gUVScale[0] << ", " << gUVScale[1] << ")" << endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS)
    {
        gUVScale -= 0.1f;
        cout << "Current scale (" << gUVScale[0] << ", " << gUVScale[1] << ")" << endl;
    }

    // Pause and resume lamp orbiting
    static bool isLKeyDown = false;
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS && !gIsLampOrbiting)
        gIsLampOrbiting = true;
    else if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS && gIsLampOrbiting)
        gIsLampOrbiting = false;

}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (gFirstMouse)
    {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouse = false;
    }

    float xoffset = xpos - gLastX;
    float yoffset = gLastY - ypos; // reversed since y-coordinates go from bottom to top

    gLastX = xpos;
    gLastY = ypos;

    gCamera.ProcessMouseMovement(xoffset, yoffset);
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    gCamera.ProcessMouseScroll(yoffset);
}

// glfw: handle mouse button events
// --------------------------------
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
    {
        if (action == GLFW_PRESS)
            cout << "Left mouse button pressed" << endl;
        else
            cout << "Left mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_MIDDLE:
    {
        if (action == GLFW_PRESS)
            cout << "Middle mouse button pressed" << endl;
        else
            cout << "Middle mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_RIGHT:
    {
        if (action == GLFW_PRESS)
            cout << "Right mouse button pressed" << endl;
        else
            cout << "Right mouse button released" << endl;
    }
    break;

    default:
        cout << "Unhandled mouse button event" << endl;
        break;
    }
}

void URender()
{
    const float angularVelocity = glm::radians(45.0f);
    if (gIsLampOrbiting)
    {
        float angle = angularVelocity * gDeltaTime;
        glm::vec3 rotationAxis(0.0f, 1.0f, 0.0f);
        glm::vec4 newPosition = glm::rotate(angle, rotationAxis) * glm::vec4(gLightPosition, 1.0f);
        gLightPosition.x = newPosition.x;
        gLightPosition.y = newPosition.y;
        gLightPosition.z = newPosition.z;
    }

    glEnable(GL_DEPTH_TEST);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render the first rectangle
    glBindVertexArray(gMesh.vao);
    glUseProgram(gCubeProgramId);

    glm::mat4 rotation1 = glm::rotate(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 model1 = glm::translate(gRectanglePosition) * rotation1 * glm::scale(gRectangleScale);

    glUniformMatrix4fv(glGetUniformLocation(gCubeProgramId, "model"), 1, GL_FALSE, glm::value_ptr(model1));
    glUniformMatrix4fv(glGetUniformLocation(gCubeProgramId, "view"), 1, GL_FALSE, glm::value_ptr(gCamera.GetViewMatrix()));
    glUniformMatrix4fv(glGetUniformLocation(gCubeProgramId, "projection"), 1, GL_FALSE, glm::value_ptr(glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f)));
    glUniform3f(glGetUniformLocation(gCubeProgramId, "objectColor"), gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(glGetUniformLocation(gCubeProgramId, "lightColor"), gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(glGetUniformLocation(gCubeProgramId, "lightPos"), gLightPosition.x, gLightPosition.y, gLightPosition.z);
    glUniform3f(glGetUniformLocation(gCubeProgramId, "viewPosition"), gCamera.Position.x, gCamera.Position.y, gCamera.Position.z);

    // Activate texture unit 0 and bind the texture to it
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId);

    // Set the texture uniform in the shader program
    glUniform1i(glGetUniformLocation(gCubeProgramId, "texture1"), 0);

    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);

    // Render the second rectangle
    glBindVertexArray(gMesh.vao);
    glUseProgram(gCubeProgramId);

    if (isIn3DMode) {
        glm::mat4 rotation2 = glm::rotate(glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::vec3 secondRectanglePosition(-0.15f, 1.0f, 0.0f);
        glm::vec3 secondRectangleScale(2.0f, 0.75f, 1.0f);
        glm::mat4 model2 = glm::translate(secondRectanglePosition) * rotation2 * glm::scale(secondRectangleScale);
        glUniformMatrix4fv(glGetUniformLocation(gCubeProgramId, "model"), 1, GL_FALSE, glm::value_ptr(model2));
    }
    else {
        glm::mat4 rotation2 = glm::rotate(glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // Update the rotation axis
        glm::vec3 secondRectanglePosition(-0.15f, 1.0f, 0.0f); // Update the rectangle position
        glm::vec3 secondRectangleScale(2.0f, 0.75f, 0.0f); // Update the rectangle scale (set z-axis to 0)
        glm::mat4 model2 = glm::translate(secondRectanglePosition) * rotation2 * glm::scale(secondRectangleScale);
        glUniformMatrix4fv(glGetUniformLocation(gCubeProgramId, "model"), 1, GL_FALSE, glm::value_ptr(model2));
    }

    glUniformMatrix4fv(glGetUniformLocation(gCubeProgramId, "view"), 1, GL_FALSE, glm::value_ptr(gCamera.GetViewMatrix()));
    glUniformMatrix4fv(glGetUniformLocation(gCubeProgramId, "projection"), 1, GL_FALSE, glm::value_ptr(glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f)));
    glUniform3f(glGetUniformLocation(gCubeProgramId, "objectColor"), gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(glGetUniformLocation(gCubeProgramId, "lightColor"), gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(glGetUniformLocation(gCubeProgramId, "lightPos"), gLightPosition.x, gLightPosition.y, gLightPosition.z);
    glUniform3f(glGetUniformLocation(gCubeProgramId, "viewPosition"), gCamera.Position.x, gCamera.Position.y, gCamera.Position.z);

    // Activate texture unit 0 and bind the texture to it
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId);

    // Set the texture uniform in the shader program
    glUniform1i(glGetUniformLocation(gCubeProgramId, "texture1"), 0);

    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);

    // Render the cylinder
    glBindVertexArray(gCylinder.vao);
    glUseProgram(gCubeProgramId);

    if (isIn3DMode) {
        glm::vec3 cylinderPosition(1.5f, 0.85f, 0.0f); // Update the cylinder position
        glm::vec3 cylinderScale(1.0f, 2.5f, 1.0f); // Update the cylinder scale
        glm::mat4 cylinderRotation = glm::rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Make the cylinder stand vertically
        glm::mat4 cylinderModel = glm::translate(cylinderPosition) * cylinderRotation * glm::scale(cylinderScale);
        glUniformMatrix4fv(glGetUniformLocation(gCubeProgramId, "model"), 1, GL_FALSE, glm::value_ptr(cylinderModel));
    }
    else {
        glm::vec3 cylinderPosition(1.5f, 0.85f, 0.0f); // Update the cylinder position
        glm::vec3 cylinderScale(1.0f, 2.5f, 0.0f); // Update the cylinder scale (set z-axis to 0)
        glm::mat4 cylinderRotation = glm::rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Make the cylinder stand vertically
        glm::mat4 cylinderModel = glm::translate(cylinderPosition) * cylinderRotation * glm::scale(cylinderScale);
        glUniformMatrix4fv(glGetUniformLocation(gCubeProgramId, "model"), 1, GL_FALSE, glm::value_ptr(cylinderModel));
    }

    glDrawArrays(GL_TRIANGLES, 0, gCylinder.nVertices);

    // Render the sphere
    glBindVertexArray(gSphere.vao);
    glUseProgram(gCubeProgramId);

    glm::vec3 spherePosition(-1.5f, 1.0f, 0.0f); // Update the sphere position
    glm::mat4 sphereModel;

    if (isIn3DMode) {
        glm::vec3 sphereScale(1.5f);
        sphereModel = glm::translate(spherePosition) * glm::scale(sphereScale);
    }
    else {
        glm::vec3 sphereScale(1.5f, 1.5f, 0.01f);
        sphereModel = glm::translate(spherePosition) * glm::scale(sphereScale);
    }

    glUniformMatrix4fv(glGetUniformLocation(gCubeProgramId, "model"), 1, GL_FALSE, glm::value_ptr(sphereModel));
    glUniformMatrix4fv(glGetUniformLocation(gCubeProgramId, "view"), 1, GL_FALSE, glm::value_ptr(gCamera.GetViewMatrix()));
    glUniformMatrix4fv(glGetUniformLocation(gCubeProgramId, "projection"), 1, GL_FALSE, glm::value_ptr(glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f)));
    glUniform3f(glGetUniformLocation(gCubeProgramId, "objectColor"), gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(glGetUniformLocation(gCubeProgramId, "lightColor"), gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(glGetUniformLocation(gCubeProgramId, "lightPos"), gLightPosition.x, gLightPosition.y, gLightPosition.z);
    glUniform3f(glGetUniformLocation(gCubeProgramId, "viewPosition"), gCamera.Position.x, gCamera.Position.y, gCamera.Position.z);

    // Activate texture unit 0 and bind the texture to it
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId);

    // Set the texture uniform in the shader program
    glUniform1i(glGetUniformLocation(gCubeProgramId, "texture1"), 0);

    glDrawArrays(GL_TRIANGLES, 0, gSphere.nVertices);

    // Render the second light source
    glBindVertexArray(gCylinder.vao);
    glUseProgram(gLampProgramId);

    float angle = angularVelocity * gDeltaTime; // Calculate the angle based on the elapsed time or any other desired value
    glm::mat4 rotation2 = glm::rotate(angle, glm::vec3(0.0f, 1.0f, 0.0f)); // Update the rotation of the second light source

    glm::vec3 secondLightPosition(0.0f, 1.5f, 1.0f); // Update the position of the second light source
    glm::vec3 secondLightColor(0.0f, 1.0f, 0.0f); // Update the color of the second light source
    glm::vec3 secondLightScale(0.05f); // Update the scale of the second light source

    glm::mat4 secondLightModel = glm::translate(secondLightPosition) * rotation2 * glm::scale(secondLightScale);

    glUniformMatrix4fv(glGetUniformLocation(gLampProgramId, "model"), 1, GL_FALSE, glm::value_ptr(secondLightModel));
    glUniformMatrix4fv(glGetUniformLocation(gLampProgramId, "view"), 1, GL_FALSE, glm::value_ptr(gCamera.GetViewMatrix()));
    glUniformMatrix4fv(glGetUniformLocation(gLampProgramId, "projection"), 1, GL_FALSE, glm::value_ptr(glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f)));
    glUniform3f(glGetUniformLocation(gLampProgramId, "lightColor"), secondLightColor.r, secondLightColor.g, secondLightColor.b);
    glUniform3f(glGetUniformLocation(gLampProgramId, "lightPos"), secondLightPosition.x, secondLightPosition.y, secondLightPosition.z);

    glDrawArrays(GL_TRIANGLES, 0, gCylinder.nVertices);

    glBindVertexArray(0);
    glUseProgram(0);

    glfwSwapBuffers(gWindow);
}

void UCreateMesh(GLMesh& mesh, GLMesh& cylinder, GLMesh& sphere)
{
    GLfloat verts[] = {
        // First Rectangle
      // Front face
      -0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f,   // Top left
      0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f,   // Top right
      0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f,   // Bottom right
      0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f,   // Bottom right
      -0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f,   // Bottom left
      -0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f,   // Top left

      // Right face
      0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f,   // Top left
      0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f,   // Top right
      0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f,   // Bottom right
      0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f,   // Bottom right
      0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f,   // Bottom left
      0.5f,  0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f,   // Top left

      // Back face
      -0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f,   // Top left
      0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f,   // Top right
      0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f,   // Bottom right
      0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f,   // Bottom right
      -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f,   // Bottom left
      -0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f,   // Top left

      // Left face
      -0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f,   // Top right
      -0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 1.0f,   // Top left
      -0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 1.0f,   // Bottom left
      -0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 1.0f,   // Bottom left
      -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f,   // Bottom right
      -0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f,   // Top right

      // Top face
      -0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 1.0f, 1.0f,   // Bottom left
      0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 1.0f, 1.0f,   // Bottom right
      0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 1.0f, 1.0f,   // Top right
      0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 1.0f, 1.0f,   // Top right
      -0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 1.0f, 1.0f,   // Top left
      -0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 1.0f, 1.0f,   // Bottom left

      // Bottom face
      -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f,   // Top left
      0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f,   // Top right
      0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 1.0f,   // Bottom right
      0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 1.0f,   // Bottom right
      -0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 0.0f, 1.0f,   // Bottom left
      -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f    // Top left
    };

    // Generate VAO and VBO for rectangle
    glGenVertexArrays(1, &(mesh.vao));
    glGenBuffers(1, &(mesh.vbo));
    glBindVertexArray(mesh.vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    // Set vertex attribute pointers for rectangle
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);


    mesh.nVertices = sizeof(verts) / (7 * sizeof(GLfloat));

    // Generate vertices for the cylinder
    std::vector<GLfloat> cylVerts; // Holds cylinder vertices
    int numSegments = 100;
    float radius = 0.5f;
    float height = 1.0f;

    for (int i = 0; i <= numSegments; ++i)
    {
        float angle = glm::radians((float)i / (float)numSegments * 360.0f);
        float x = cos(angle) * radius;
        float z = sin(angle) * radius;

        // Top vertex
        cylVerts.push_back(x);
        cylVerts.push_back(0.5f * height);
        cylVerts.push_back(z);
        cylVerts.push_back(0.5f);
        cylVerts.push_back(0.5f);
        cylVerts.push_back(0.5f);
        cylVerts.push_back(1.0f);

        // Bottom vertex
        cylVerts.push_back(x);
        cylVerts.push_back(-0.5f * height);
        cylVerts.push_back(z);
        cylVerts.push_back(0.5f);
        cylVerts.push_back(0.5f);
        cylVerts.push_back(0.5f);
        cylVerts.push_back(1.0f);
    }

    // Generate VAO and VBO for cylinder
    glGenVertexArrays(1, &(cylinder.vao));
    glGenBuffers(1, &(cylinder.vbo));
    glBindVertexArray(cylinder.vao);
    glBindBuffer(GL_ARRAY_BUFFER, cylinder.vbo);
    glBufferData(GL_ARRAY_BUFFER, cylVerts.size() * sizeof(GLfloat), cylVerts.data(), GL_STATIC_DRAW);

    // Set vertex attribute pointers for cylinder
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    cylinder.nVertices = cylVerts.size() / 7;

    // Generate vertices for the sphere
    std::vector<GLfloat> sphereVerts; // Holds sphere vertices

    for (int lat = 0; lat <= numSegments; ++lat)
    {
        float theta = glm::radians((float)lat / (float)numSegments * 180.0f);
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);

        for (int lon = 0; lon <= numSegments; ++lon)
        {
            float phi = glm::radians((float)lon / (float)numSegments * 360.0f);
            float sinPhi = sin(phi);
            float cosPhi = cos(phi);

            float x = cosPhi * sinTheta;
            float y = cosTheta;
            float z = sinPhi * sinTheta;

            sphereVerts.push_back(radius * x);
            sphereVerts.push_back(radius * y);
            sphereVerts.push_back(radius * z);
            sphereVerts.push_back(0.5f);
            sphereVerts.push_back(0.5f);
            sphereVerts.push_back(0.5f);
            sphereVerts.push_back(1.0f);
        }
    }

    // Generate VAO and VBO for sphere
    glGenVertexArrays(1, &(sphere.vao));
    glGenBuffers(1, &(sphere.vbo));
    glBindVertexArray(sphere.vao);
    glBindBuffer(GL_ARRAY_BUFFER, sphere.vbo);
    glBufferData(GL_ARRAY_BUFFER, sphereVerts.size() * sizeof(GLfloat), sphereVerts.data(), GL_STATIC_DRAW);

    // Set vertex attribute pointers for sphere
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    sphere.nVertices = sphereVerts.size() / 7;

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}


void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(1, &mesh.vbo);
}


/*Generate and load the texture*/
bool UCreateTexture(const char* filename, GLuint& textureId)
{
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (image)
    {
        flipImageVertically(image, width, height, channels);

        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (channels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (channels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        else
        {
            cout << "Not implemented to handle image with " << channels << " channels" << endl;
            return false;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

        return true;
    }

    // Error loading the image
    return false;
}



void UDestroyTexture(GLuint textureId)
{
    glGenTextures(1, &textureId);
}


// Implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    // Compilation and linkage error reporting
    int success = 0;
    char infoLog[512];

    // Create a Shader program object.
    programId = glCreateProgram();

    // Create the vertex and fragment shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Retrive the shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Compile the vertex shader, and print compilation errors (if any)
    glCompileShader(vertexShaderId); // compile the vertex shader
    // check for shader compile errors
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glCompileShader(fragmentShaderId); // compile the fragment shader
    // check for shader compile errors
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Attached compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);   // links the shader program
    // check for linking errors
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glUseProgram(programId);    // Uses the shader program

    return true;
}


void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}
