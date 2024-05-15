// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow *window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

// #include <freetype/freetype.h>
// #include FT_FREETYPE_H

using namespace glm;
using namespace std;

#include <common/shader.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
#include <common/texture.hpp>
#include <TP1/Camera.hpp>
// #include <TP1/Terrain.hpp>
#include <TP1/Voiture.hpp>
#include <TP1/Cube.hpp>
#include <pthread.h>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <ctime>  
#include <map>

void processInput(GLFWwindow *window);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
bool collisionCarRoad(Object &voiture, Terrain &terrain);
void roadInfini(Terrain &planInfini);
void collisionCarCube(Voiture &car, Cube &cubeInfini);
void acceleration(Voiture &voiture, float v);
void desseleration(Voiture &voiture);
void resolveCollision(Voiture &voiture, Terrain &terrain);
void cubeInf(Cube &cubeInfini);
bool checkCollision(const std::vector<vec3>& aabb1, const std::vector<vec3>& aabb2);

bool firstMouse = true;
float yaw = -90.0f; // yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
float fov = 45.0f;


// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera *camera;

// timing
float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;
int nbFrames = 0;

Object sphereSky;

bool play = false;

/*******************************************************************************/

glm::mat4 viewMatrix;
glm::mat4 projMatrix;
std::vector<unsigned short> indices; // Triangles concaténés dans une liste
std::vector<glm::vec3> indexed_vertices;
std::vector<float> texCoords;


GLfloat fogColor[4] = {0.5f, 0.5f, 0.5f, 1.0f}; 

float vitesse = 0.05;
float rotateSpeed = 0.05;

Voiture voiture(1.5f,2.0f);
glm::vec3 sizeVoit = voiture.getSize();

Terrain planInfini;
int planToMove= 1;

Cube cubeInfini;
int cubeToMove = 1;

GLuint vertexbuffer;
GLuint elementbuffer;
GLuint uv;
GLuint hmap;


void *updateFPS(void *params)
{
    string s = "FPS - ";
    while (true)
    {
        glfwSetWindowTitle(window, (s + to_string(nbFrames)).c_str());
        nbFrames = 0;
        this_thread::sleep_for(1000ms);
    }
}

// Global score variable
double score = 0.0;

// Update the score based on the car's movement
void updateScore(Voiture &voiture) {
    vec3 speed = voiture.getSpeed();
    double distance = glm::length(speed); 
    score += distance; 
}


int main(void)
{
    // Initialise GLFW
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
        return -1;
    }
    srand(time(NULL));
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); 
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    srand(time(NULL)); 
    // Open a window and create its OpenGL context
    window = glfwCreateWindow(1920, 1080, "Projet", NULL, NULL);
    if (window == NULL)
    {
        fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // tell GLFW to capture our mouse

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK)
    {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Hide the mouse and enable unlimited mouvement

    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, 1024 / 2, 768 / 2);

    // Dark blue background
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);

    // Enable depth test
    glDisable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);


    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders("vertex_shader.glsl", "fragment_shader.glsl");


    /*****************TODO***********************/
    // Get a handle for our "Model View Projection" matrices uniforms
    viewMatrix = glm::mat4(1.f);
    projMatrix = glm::mat4(1.f);
    /****************************************/
    std::vector<std::vector<unsigned short>> triangles;

    // Chargement du fichier de maillage

    // Get a handle for our "LightPosition" uniform
    glUseProgram(programID);
    GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

    // For speed computation
    double lastTime = glfwGetTime();
    pthread_t thread;

    if (pthread_create(&thread, NULL, updateFPS, (void *)0) != 0)
    {
        perror("erreur creation thread");
        exit(1);
    }

    /* ___________________________________________________________________________________ */

    camera = new Camera(vec3(0.0f, 2.0f, 6.0f), vec3(), vec3(0, 1, 0), vec3(0, 0, -1));

    int resolutionX = 2;
    int resolutionY = 2;
    int sizeRoadX = 16;
    int sizeY = 16;

    // Routes
    Terrain plan1(resolutionX, resolutionY, sizeRoadX, sizeY, (float)(sizeRoadX / 2), 0, 1, false, "../textures/road4.png");
    plan1.loadTexture("../textures/road4.jpg");
    plan1.transform.Translate(vec3(0, 0, -(float)(sizeY / 2)));
    Terrain plan2(resolutionX, resolutionY, sizeRoadX, sizeY, (float)(sizeRoadX / 2), 0, 1, false, "../textures/road4.png");
    plan2.loadTexture("../textures/road4.jpg");
    plan2.transform.Translate(vec3(0, 0, -(float)((sizeY / 2) + sizeY)));
    Terrain plan3(resolutionX, resolutionY, sizeRoadX, sizeY, (float)(sizeRoadX / 2), 0, 1, false, "../textures/road4.png");
    plan3.loadTexture("../textures/road4.jpg");
    plan3.transform.Translate(vec3(0, 0, -(float)((sizeY / 2) + 2 * sizeY)));
    Terrain plan4(resolutionX, resolutionY, sizeRoadX, sizeY, (float)(sizeRoadX / 2), 0, 1, false, "../textures/road4.png");
    plan4.loadTexture("../textures/road4.jpg");
    plan4.transform.Translate(vec3(0, 0, -(float)((sizeY / 2) + 3 * sizeY)));
    Terrain plan5(resolutionX, resolutionY, sizeRoadX, sizeY, (float)(sizeRoadX / 2), 0, 1, false, "../textures/road4.png");
    plan5.loadTexture("../textures/road4.jpg");
    plan5.transform.Translate(vec3(0, 0, -(float)((sizeY / 2) + 4 * sizeY)));
    Terrain plan6(resolutionX, resolutionY, sizeRoadX, sizeY, (float)(sizeRoadX / 2), 0, 1, false, "../textures/road4.png");
    plan6.loadTexture("../textures/road4.jpg");
    plan6.transform.Translate(vec3(0, 0, -(float)((sizeY / 2) + 5 * sizeY)));
    Terrain plan7(resolutionX, resolutionY, sizeRoadX, sizeY, (float)(sizeRoadX / 2), 0, 1, false, "../textures/road4.png");
    plan7.loadTexture("../textures/road4.jpg");
    plan7.transform.Translate(vec3(0, 0, -(float)((sizeY / 2) + 6 * sizeY)));
    Terrain plan8(resolutionX, resolutionY, sizeRoadX, sizeY, (float)(sizeRoadX / 2), 0, 1, false, "../textures/road4.png");
    plan8.loadTexture("../textures/road4.jpg");
    plan8.transform.Translate(vec3(0, 0, -(float)((sizeY / 2) + 7 * sizeY)));
    Terrain plan9(resolutionX, resolutionY, sizeRoadX, sizeY, (float)(sizeRoadX / 2), 0, 1, false, "../textures/road4.png");
    plan9.loadTexture("../textures/road4.jpg");
    plan9.transform.Translate(vec3(0, 0, -(float)((sizeY / 2) + 8 * sizeY)));
    Terrain plan10(resolutionX, resolutionY, sizeRoadX, sizeY, (float)(sizeRoadX / 2), 0, 1, false, "../textures/road4.png");
    plan10.loadTexture("../textures/road4.jpg");
    plan10.transform.Translate(vec3(0, 0, -(float)((sizeY / 2) + 9 * sizeY)));
    Terrain plan11(resolutionX, resolutionY, sizeRoadX, sizeY, (float)(sizeRoadX / 2), 0, 1, false, "../textures/road4.png");
    plan11.loadTexture("../textures/road4.jpg");
    plan11.transform.Translate(vec3(0, 0, -(float)((sizeY / 2) + 10 * sizeY)));
    Terrain plan12(resolutionX, resolutionY, sizeRoadX, sizeY, (float)(sizeRoadX / 2), 0, 1, false, "../textures/road4.png");
    plan12.loadTexture("../textures/road4.jpg");
    plan12.transform.Translate(vec3(0, 0, -(float)((sizeY / 2) + 11 * sizeY)));

    // Voiture
    voiture.loadObject("../models/Porsche_911_GT2.obj", 1);
    voiture.loadTexture("../textures/textureCar.png");
    voiture.transform.Translate(vec3(0, 0.62, -4));


    int sizeCube = 16; // espace entre les cubes...

    // Cubes
    Cube cube1;
    cube1.loadObject("../models/cube.obj", 1);
    cube1.loadTexture("../textures/Heightmap_Mountain.png");
    cube1.transform.Translate(vec3((static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 8 - 4, 1, -16));
    cube1.setSizeX(sizeCube);
    cube1.setSizeY(sizeCube);
    Cube cube2;
    cube2.loadObject("../models/cube.obj", 1);
    cube2.loadTexture("../textures/Heightmap_Mountain.png");
    cube2.transform.Translate(vec3((static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 8 - 4, 1, -30));
    cube2.setSizeX(sizeCube);
    cube2.setSizeY(sizeCube);
    Cube cube3;
    cube3.loadObject("../models/cube.obj", 1);
    cube3.loadTexture("../textures/Heightmap_Mountain.png");
    cube3.transform.Translate(vec3((static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 8 - 4, 1, -44));
    cube3.setSizeX(sizeCube);
    cube3.setSizeY(sizeCube);
    Cube cube4;
    cube4.loadObject("../models/cube.obj", 1);
    cube4.loadTexture("../textures/Heightmap_Mountain.png");
    cube4.transform.Translate(vec3((static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 8 - 4, 1, -58));
    cube4.setSizeX(sizeCube);
    cube4.setSizeY(sizeCube);
    Cube cube5;
    cube5.loadObject("../models/cube.obj", 1);
    cube5.loadTexture("../textures/Heightmap_Mountain.png");
    cube5.transform.Translate(vec3((static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 8 - 4, 1, -72));
    cube5.setSizeX(sizeCube);
    cube5.setSizeY(sizeCube);
    Cube cube6;
    cube6.loadObject("../models/cube.obj", 1);
    cube6.loadTexture("../textures/Heightmap_Mountain.png");
    cube6.transform.Translate(vec3((static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 8 - 4, 1, -86));
    cube6.setSizeX(sizeCube);
    cube6.setSizeY(sizeCube);

    voiture.add(camera);

    cubeInfini.add(&cube1);
    cubeInfini.add(&cube2);
    cubeInfini.add(&cube3);
    cubeInfini.add(&cube4);
    cubeInfini.add(&cube5);
    cubeInfini.add(&cube6);

    planInfini.add(&plan1);
    planInfini.add(&plan2);
    planInfini.add(&plan3);
    planInfini.add(&plan4);
    planInfini.add(&plan5);
    planInfini.add(&plan6);
    planInfini.add(&plan7);
    planInfini.add(&plan8);
    planInfini.add(&plan9);
    planInfini.add(&plan10);
    planInfini.add(&plan11);
    planInfini.add(&plan12);

    sphereSky.loadObject("../models/sphere.off", 0);
    sphereSky.loadTexture("../textures/test3.jpg");
    sphereSky.transform.Scale(vec3(150, 150, 150));
    sphereSky.transform.Rotation(vec3(0, 1, 0), radians(-21.0));


    Object racine;

    racine.add(&planInfini);
    racine.add(&cubeInfini);
    racine.add(&voiture);
    racine.add(&sphereSky);

    voiture.setVelocity(glm::vec3(0));
    voiture.setAcceleration(glm::vec3(0.0f, -0.0f, 0.0f));
    voiture.setMass(1.0f);
    voiture.setForce(glm::vec3(0.0f, 0.0f, 0.0f));
    voiture.setk(0.3);

    bool isFirstFrame = true;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_START, 10.0f);
    glFogf(GL_FOG_END, 50.0f);
    glFogfv(GL_FOG_COLOR, fogColor);

    do
    {

        //  Measure speed
        //  per-frame time logic
        //  --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        nbFrames++;

        // input
        // -----
        processInput(window);

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use our shader
        glUseProgram(programID);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_FOG);
        glFogi(GL_FOG_MODE, GL_LINEAR);
        glFogf(GL_FOG_START, -10.0f);
        glFogf(GL_FOG_END, -50.0f);
        glFogfv(GL_FOG_COLOR, fogColor);


        if (play){
            planInfini.transform.Translate(-(voiture.getSpeed() * camera->getFront()));
            cubeInfini.transform.Translate(-(voiture.getSpeed() * camera->getFront()));
        
            roadInfini(planInfini);
            cubeInf(cubeInfini);
        
            if(!isFirstFrame){
                voiture.updatePhysics(deltaTime);
                if(collisionCarRoad(voiture, planInfini)){                    
                    resolveCollision(voiture, planInfini);
                }else{
                    play = false;
                }
            }

            // updateScore(voiture);

            desseleration(voiture);
            collisionCarCube(voiture, cubeInfini);
        }

        isFirstFrame = false;

        racine.updateTree();

        viewMatrix = glm::lookAt(camera->getPosition(), camera->getPosition() + camera->getFront(), camera->getUp());

        // Projection matrix : 45 Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
        projMatrix = glm::perspective(glm::radians(45.f), 4.0f / 3.0f, 0.1f, 250.0f);

        // Send our transformation to the currently bound shader,
        // in the "Model View Projection" to the shader uniforms
        glUniformMatrix4fv(glGetUniformLocation(programID, "viewM"), 1, GL_FALSE, &viewMatrix[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(programID, "projM"), 1, GL_FALSE, &projMatrix[0][0]);

        racine.draw(programID);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0);

    // Cleanup VBO and shader
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &elementbuffer);
    glDeleteProgram(programID);
    glDeleteVertexArrays(1, &VertexArrayID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}

bool keyAlreadyPressed = false;
bool libre = false;

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    /********* Souris *********/
    // Get the current cursor position
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    // Check if the left mouse button is pressed
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        if (firstMouse) {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        double xoffset = lastX - xpos; 
        double yoffset = lastY - ypos; 
        lastX = xpos;
        lastY = ypos;

        float sensitivity = 0.05f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        yaw += xoffset;
        pitch += yoffset;

    
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
 
        camera->setFront(normalize(front));

    } else {
        firstMouse = true;
    }

    if(glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS){
        libre = false;
    }

    if(glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS){
        libre = true;
    }

    float cameraSpeed = 5. * deltaTime;

    if(libre){
        voiture.setStop(true);
        // Camera zoom in and out
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera->addPosition(cameraSpeed * camera->getFront());
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera->addPosition(- cameraSpeed * camera->getFront());

        // Camera move left and right
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera->addPosition(glm::normalize(glm::cross(camera->getFront(), camera->getUp())) * cameraSpeed);
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            camera->addPosition(-glm::normalize(glm::cross(camera->getFront(), camera->getUp())) * cameraSpeed);

        // move up and down
        if (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
            camera->addPosition(cameraSpeed * camera->getUp());
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            camera->addPosition(-cameraSpeed * camera->getUp());
    }


    if(glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS){
        camera->setFirstPerson(true);
    }
    if(glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS){
        camera->setFirstPerson(false);
        camera->setPosition(vec3(0.0f, 2.3f, 6.0f));
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
    
        if (!voiture.getStop())
        {
            acceleration(voiture, voiture.getVMax());
        }
        acceleration(voiture, voiture.getVMax());
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        voiture.addSpeed(vec3(0, 0, -0.02));
        if (voiture.getSpeed().z < 0.1)
        {
            voiture.setSpeed(vec3(0, 0, 0.1));
        }
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && !voiture.getStop())
    {
        vec3 move = glm::normalize(glm::cross(camera->getFront(), camera->getUp())) * (10 * deltaTime);
        voiture.transform.Translate(-move);
        voiture.setTurn(1);
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && !voiture.getStop())
    {
        vec3 move = glm::normalize(glm::cross(camera->getFront(), camera->getUp())) * (10 * deltaTime);
        voiture.transform.Translate(move);
        voiture.setTurn(2);
    
    }

    if (!(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) && !(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS))
    {
        voiture.setTurn(0);
    }

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        rotateSpeed += 0.001;
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        play = true;
    }


    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
    {
        if (!keyAlreadyPressed){
            play = false;
            keyAlreadyPressed = true;
        
            Transform t;
            Transform t1;
            voiture.setStop(false);
            voiture.transform = t;
            voiture.transform.Translate(vec3(0, 0.62, -4));
            planInfini.transform = t1;
            for (size_t i = 0; i < planInfini.getChilds().size(); i++)
            {
                Transform t2;
                Terrain *currentPlan = dynamic_cast<Terrain *>(planInfini.getChilds()[i]);

                currentPlan->transform = t2;
                currentPlan->transform.Translate(vec3(0, 0, -((float)(currentPlan->getsizeY() / 2) + i * currentPlan->getsizeY())));
                camera->setPosition(camera->getStartPos());
                planToMove = 1;
            }
            Transform t4;
            cubeInfini.transform = t4;
            for (size_t i = 0; i < cubeInfini.getChilds().size(); i++)
            {
                Transform t3;
                Cube *currentCube = dynamic_cast<Cube *>(cubeInfini.getChilds()[i]);
                std::cout<<"Cube "<<i<<" x :"<<currentCube->transform.getPosition().x<<", y : "<<currentCube->transform.getPosition().y <<", z : "<<currentCube->transform.getPosition().z<<std::endl;

                currentCube->transform = t3;
                currentCube->transform.Translate(vec3((static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 8 - 4, 1,(-16 - 14 * (float)i)));
                camera->setPosition(camera->getStartPos());
                cubeToMove = 1;
            }
        }
        else{
            keyAlreadyPressed = false;
        }
    }
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

bool collisionCarRoad(Object &voiture, Terrain &terrain){
    glm::vec3 voiturePos = voiture.transform.getPosition();

    bool collisionX = (voiturePos.x > -6) &&
                      (voiturePos.x < 6);

    return collisionX;
}

void roadInfini(Terrain &planInfini)
{

    for (size_t i = 1; i <= planInfini.getChilds().size(); i++)
    {
        Terrain *currentPlan = dynamic_cast<Terrain *>(planInfini.getChilds()[i - 1]);
        if (planInfini.transform.t.z > (float)(currentPlan->getsizeY() / 2) + (i - 1) * currentPlan->getsizeY() + 2 && planToMove == i)
        {

            currentPlan->transform.Translate(vec3(0, 0, -(currentPlan->getsizeY() * (int)planInfini.getChilds().size())));
            planToMove = i + 1;
            if (i == (int)planInfini.getChilds().size())
            {
                planToMove = 1;
                planInfini.transform.t.z -= currentPlan->getsizeY() * (int)planInfini.getChilds().size();
            }
        }
    }
}


void collisionCarCube(Voiture &car, Cube &cubeInfini) {
    std::vector<vec3> carAABB = car.boiteEnglobante();

    vec4 min = vec4(carAABB[0], 1);
    min = car.transform.model * min;
    vec4 max = vec4(carAABB[1], 1);
    max = car.transform.model * max;

    for (size_t i = 0; i < cubeInfini.getChilds().size(); i++) {
        Cube *currentCube = dynamic_cast<Cube *>(cubeInfini.getChilds()[i]);

        std::vector<vec3> cubeAABB = currentCube->boiteEnglobante();

        vec4 cubeMin = vec4(cubeAABB[0], 1);
        cubeMin = currentCube->transform.model * cubeMin;
        vec4 cubeMax = vec4(cubeAABB[1], 1);
        cubeMax = currentCube->transform.model * cubeMax;

        if ((min.x > cubeMin.x && min.x < cubeMax.x && min.z > cubeMin.z && min.z < cubeMax.z) ||
            (max.x > cubeMin.x && max.x < cubeMax.x && min.z > cubeMin.z && min.z < cubeMax.z) ||
            (min.x > cubeMin.x && min.x < cubeMax.x && max.z > cubeMin.z && max.z < cubeMax.z) ||
            (max.x > cubeMin.x && max.x < cubeMax.x && max.z > cubeMin.z && max.z < cubeMax.z)) {
            // std::cout << "Collision detecté" << i + 1 << std::endl;
            // React to the collision...
            play = false;
            voiture.setStop(true);
        }
    }
}

void resolveCollision(Voiture &voiture, Terrain &terrain){
    voiture.transform.Translate(vec3(voiture.transform.getPosition().x, 0.62, voiture.transform.getPosition().z) - voiture.transform.getPosition());
    voiture.setVelocity(glm::vec3(voiture.getVelocity().x, -voiture.getVelocity().y,voiture.getVelocity().z));
}

void desseleration(Voiture &voiture)
{
    voiture.addSpeed(vec3(0, 0, -(0.009 * voiture.getSpeed().z * voiture.getSpeed().z)));
    if (voiture.getSpeed().z < 0.1 && !voiture.getStop())
    {
        voiture.addSpeed(vec3(0, 0, 0.08));
    }
}


void cubeInf(Cube &cubeInfini)
{
    for (size_t i = 1; i <= cubeInfini.getChilds().size(); i++)
    {
        Cube *currentCube = dynamic_cast<Cube *>(cubeInfini.getChilds()[i - 1]);
        if (cubeInfini.transform.t.z > (float)(currentCube->getsizeY() / 2) + (i - 1) * currentCube->getsizeY() + 2 && cubeToMove== i)
        {
            float randomX = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * (4 - (-4)) - 4;
            if(currentCube->transform.getPosition().x+randomX<-6 || currentCube->transform.getPosition().x+randomX>6){
                randomX = -randomX;
            }
            currentCube->transform.Translate(vec3(randomX, 0, -(currentCube->getsizeY() * (int)cubeInfini.getChilds().size())));
            // std::cout<<"Cube "<<i<<" x :"<<currentCube->transform.getPosition().x<<", y : "<<currentCube->transform.getPosition().y <<", z : "<<currentCube->transform.getPosition().z<<std::endl;
            // std::cout<<"Voiture "<<i<<" x :"<<voiture.transform.getPosition().x<<", y : "<<voiture.transform.getPosition().y <<", z : "<<voiture.transform.getPosition().z<<std::endl;
            cubeToMove = i + 1;
            if (i == (int)cubeInfini.getChilds().size())
            {
                cubeToMove = 1;
                cubeInfini.transform.t.z -= currentCube->getsizeY() * (int)cubeInfini.getChilds().size();
            }
        }
    }
}


void acceleration(Voiture &voiture, float vitesseMax)
{
    vec3 vitesse = voiture.getSpeed();
    if (vitesse.z < vitesseMax * 2 / 3)
    {
        voiture.setSpeed(vec3(vitesse.x, vitesse.y, vitesse.z + 0.005));
    }
    else if (vitesse.z < vitesseMax)
    {
        voiture.setSpeed(vec3(vitesse.x, vitesse.y, vitesse.z + 0.001));
    }
}


bool checkCollision(const std::vector<vec3>& aabb1, const std::vector<vec3>& aabb2) {
    // on check si l'une des boîtes englobantes est à l'intérieur de l'autre sur tous les axes
    bool collisionX = aabb1[0].x < aabb2[1].x && aabb1[1].x > aabb2[0].x;
    bool collisionY = aabb1[0].y < aabb2[1].y && aabb1[1].y > aabb2[0].y;
    bool collisionZ = aabb1[0].z < aabb2[1].z && aabb1[1].z > aabb2[0].z;

    // si les boîtes englobantes s'intersectent sur tous les axes, il y a une collision
    return collisionX && collisionY && collisionZ;
}


