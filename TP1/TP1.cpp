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

using namespace glm;
using namespace std;

#include <common/shader.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
#include <common/texture.hpp>
#include <TP1/Camera.hpp>
#include <TP1/Terrain.hpp>
#include <TP1/Voiture.hpp>
#include <pthread.h>
#include <chrono>
#include <thread>

void processInput(GLFWwindow *window);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
bool collisionDetection(Object &voiture, Terrain &terrain);
void roadInfini(Terrain &planInfini);
// void acceleration(Vehicule &voiture, float v);


// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera *camera;

// timing
float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;
int nbFrames = 0;

// bool firstMouse = true;
// float yaw = -90.0f; // yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
// float pitch = 0.0f;
// float lastX = 800.0f / 2.0;
// float lastY = 600.0 / 2.0;
// float fov = 45.0f;
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
int planToMove = 1;

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


void resolveCollision(Voiture &voiture, Terrain &terrain){
    // voiture.transform.Translate(vec3(voiture.transform.getPosition().x, 0.62, voiture.transform.getPosition().z));
    // voiture.transform.Translate(vec3(0, 0.62, -8));
    voiture.transform.setPosition(vec3(voiture.transform.getPosition().x, 0.62, voiture.transform.getPosition().z));
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

void desseleration(Voiture &voiture)
{
    voiture.addSpeed(vec3(0, 0, -(0.009 * voiture.getSpeed().z * voiture.getSpeed().z)));
    if (voiture.getSpeed().z < 0.1 && !voiture.getStop())
    {
        voiture.addSpeed(vec3(0, 0, 0.08));
    }
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

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); 
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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
    int ObjectState = 1;

    int resolutionX = 2;
    int resolutionY = 2;
    int sizeX = 16;
    int sizeY = 16;

    Terrain plan1(resolutionX, resolutionY, sizeX, sizeY, (float)(sizeX / 2), 0, 1, false, "../textures/road4.png");
    plan1.loadTexture("../textures/road4.jpg");
    plan1.transform.Translate(vec3(0, 0, -(float)(sizeY / 2)));
    Terrain plan2(resolutionX, resolutionY, sizeX, sizeY, (float)(sizeX / 2), 0, 1, false, "../textures/road4.png");
    plan2.loadTexture("../textures/road4.jpg");
    plan2.transform.Translate(vec3(0, 0, -(float)((sizeY / 2) + sizeY)));
    Terrain plan3(resolutionX, resolutionY, sizeX, sizeY, (float)(sizeX / 2), 0, 1, false, "../textures/road4.png");
    plan3.loadTexture("../textures/road4.jpg");
    plan3.transform.Translate(vec3(0, 0, -(float)((sizeY / 2) + 2 * sizeY)));
    Terrain plan4(resolutionX, resolutionY, sizeX, sizeY, (float)(sizeX / 2), 0, 1, false, "../textures/road4.png");
    plan4.loadTexture("../textures/road4.jpg");
    plan4.transform.Translate(vec3(0, 0, -(float)((sizeY / 2) + 3 * sizeY)));
    Terrain plan5(resolutionX, resolutionY, sizeX, sizeY, (float)(sizeX / 2), 0, 1, false, "../textures/road4.png");
    plan5.loadTexture("../textures/road4.jpg");
    plan5.transform.Translate(vec3(0, 0, -(float)((sizeY / 2) + 4 * sizeY)));
    Terrain plan6(resolutionX, resolutionY, sizeX, sizeY, (float)(sizeX / 2), 0, 1, false, "../textures/road4.png");
    plan6.loadTexture("../textures/road4.jpg");
    plan6.transform.Translate(vec3(0, 0, -(float)((sizeY / 2) + 5 * sizeY)));
    Terrain plan7(resolutionX, resolutionY, sizeX, sizeY, (float)(sizeX / 2), 0, 1, false, "../textures/road4.png");
    plan7.loadTexture("../textures/road4.jpg");
    plan7.transform.Translate(vec3(0, 0, -(float)((sizeY / 2) + 6 * sizeY)));
    Terrain plan8(resolutionX, resolutionY, sizeX, sizeY, (float)(sizeX / 2), 0, 1, false, "../textures/road4.png");
    plan8.loadTexture("../textures/road4.jpg");
    plan8.transform.Translate(vec3(0, 0, -(float)((sizeY / 2) + 7 * sizeY)));
    Terrain plan9(resolutionX, resolutionY, sizeX, sizeY, (float)(sizeX / 2), 0, 1, false, "../textures/road4.png");
    plan9.loadTexture("../textures/road4.jpg");
    plan9.transform.Translate(vec3(0, 0, -(float)((sizeY / 2) + 8 * sizeY)));
    Terrain plan10(resolutionX, resolutionY, sizeX, sizeY, (float)(sizeX / 2), 0, 1, false, "../textures/road4.png");
    plan10.loadTexture("../textures/road4.jpg");
    plan10.transform.Translate(vec3(0, 0, -(float)((sizeY / 2) + 9 * sizeY)));
    Terrain plan11(resolutionX, resolutionY, sizeX, sizeY, (float)(sizeX / 2), 0, 1, false, "../textures/road4.png");
    plan11.loadTexture("../textures/road4.jpg");
    plan11.transform.Translate(vec3(0, 0, -(float)((sizeY / 2) + 10 * sizeY)));
    Terrain plan12(resolutionX, resolutionY, sizeX, sizeY, (float)(sizeX / 2), 0, 1, false, "../textures/road4.png");
    plan12.loadTexture("../textures/road4.jpg");
    plan12.transform.Translate(vec3(0, 0, -(float)((sizeY / 2) + 11 * sizeY)));


    voiture.loadObject("../models/Porsche_911_GT2.obj", 1);
    voiture.loadTexture("../textures/textJeepB.png");
    voiture.transform.Translate(vec3(0, 0.62, -8));

    voiture.add(camera);

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
    sphereSky.transform.Scale(vec3(160, 160, 160));
    sphereSky.transform.Rotation(vec3(0, 1, 0), radians(-21.0));


    Object racine;

    racine.add(&planInfini);
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
            roadInfini(planInfini);

            if(!isFirstFrame){
                voiture.updatePhysics(deltaTime);
                if(collisionDetection(voiture, planInfini)){
                    resolveCollision(voiture, planInfini);
                }
            }

            desseleration(voiture);
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

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
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
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

bool collisionDetection(Object &voiture, Terrain &terrain){
    // for (size_t i = 0; i < terrain.getChilds().size(); i++)
    // {
    //     Terrain *currentPlan = dynamic_cast<Terrain *>(terrain.getChilds()[i]);
    //     if (voiture.transform.t.z > currentPlan->transform.t.z - currentPlan->getsizeY() / 2 && voiture.transform.t.z < currentPlan->transform.t.z + currentPlan->getsizeY() / 2)
    //     {
    //         if (voiture.transform.t.x > currentPlan->transform.t.x - currentPlan->getsizeX() / 2 && voiture.transform.t.x < currentPlan->transform.t.x + currentPlan->getsizeX() / 2)
    //         {
    //             return true;
    //         }
    //     }
    // }
    // return false;
    // std::cout<<"test"<<std::endl;
    glm::vec3 voiturePos = voiture.transform.getPosition();
    glm::vec3 sizeVoit = voiture.getSize();

    // Coordonnées et dimensions du terrain
    glm::vec3 terrainPos = terrain.transform.getPosition();
    glm::vec3 sizeTerrain = terrain.getSize();

    // Vérification de la collision en prenant en compte les dimensions
    bool collisionX = (voiturePos.x + sizeVoit.x / 2 >= terrainPos.x - sizeTerrain.x / 2) &&
                      (voiturePos.x - sizeVoit.x / 2 <= terrainPos.x + sizeTerrain.x / 2);
    
    // if(voiture.transform.t.x - sizeVoit.x/2 <= 16 || voiture.transform.t.x - sizeVoit.x/2 >= 0){ // car le plan est de taille 16
    //     return true;
    // }
    // return false;
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

