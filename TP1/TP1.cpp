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
#include <TP1/Terrain.hpp>

void processInput(GLFWwindow *window, Terrain &plan);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
glm::vec3 camera_position   = glm::vec3(0.0f, 0.0f,  3.0f);
glm::vec3 camera_target = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 camera_up    = glm::vec3(0.0f, 1.0f,  0.0f);

glm::vec3 cameraPosOrbit = glm::vec3(0.0f, 0.0f, 2.0f);
glm::vec3 cameraTargetOrbit = glm::vec3(0., 0., 0.);

// timing
float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;

// rotation
float angle = 0.;
float zoom = 1.;


bool libre = true;
bool orbital = false;


/***** Control souris *****/

double lastX = 400, lastY = 300;
double yaw = -90.0f, pitch = 0.0f;
bool firstMouse = true;

/*******************************************************************************/

GLuint vertexbuffer;
GLuint elementbuffer;

// fonction pour centrer la caméra 
void centrerCamera(std::vector<glm::vec3> vertices, glm::vec3& camera_position, glm::vec3& camera_target) {
    // on trouve le centre de la surface
    glm::vec3 center(0.0f);
    for (const auto& vertex : vertices) {
        center += vertex;
    }
    center /= static_cast<float>(vertices.size());

    // puis on ajuste la position et la cible de la caméra
    camera_position = center + glm::vec3(0.0f, 2.0f, 2.0f);
    camera_target = glm::normalize(center - camera_position);
}

// Object cube;
Object voiture;

bool collisionDetection(Object &voiture, Terrain &terrain) {
    vec3 cubePosition = voiture.transform.getPosition();
    vec3 terrainPosition = terrain.transform.getPosition();

    // calcule des dimensions de la boîte de collision du cube et du terrain
    vec3 cubeSize = voiture.getSize(); // Taille de la boîte de collision du cube
    vec3 terrainSize = terrain.getSize(); // Taille de la boîte de collision du terrain

    // calcule des limites du cube et du terrain en prenant en compte les offsets
    float cubeMinX = cubePosition.x - cubeSize.x / 2.0f;
    float cubeMaxX = cubePosition.x + cubeSize.x / 2.0f;
    float cubeMinY = cubePosition.y - cubeSize.y / 2.0f;
    float cubeMaxY = cubePosition.y + cubeSize.y / 2.0f;
    float cubeMinZ = cubePosition.z - cubeSize.z / 2.0f;
    float cubeMaxZ = cubePosition.z + cubeSize.z / 2.0f;

    float terrainMinX = terrainPosition.x;
    float terrainMaxX = terrainPosition.x + terrainSize.x;
    float terrainMinY = terrainPosition.y;
    float terrainMaxY = terrainPosition.y + terrainSize.y;
    float terrainMinZ = terrainPosition.z;
    float terrainMaxZ = terrainPosition.z + terrainSize.z;

    // on vérifie la collision sur chaque axe
    bool collisionX = cubeMaxX >= terrainMinX && cubeMinX <= terrainMaxX;
    bool collisionY = cubeMaxY >= terrainMinY && cubeMinY <= terrainMaxY;
    bool collisionZ = cubeMaxZ >= terrainMinZ && cubeMinZ <= terrainMaxZ;

    return collisionX && collisionY && collisionZ;
}


// Fonction d'interpolation linéaire
float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

float terrainHeightAtCubePosition(Terrain& terrain, const vec3& cubePosition) {

    glm::vec3 size = terrain.getSize();

    // on récupère les dimensions du terrain
    int terrainWidth = size[0];
    int terrainDepth = size[2];

    vector<glm::vec3> terrainVertices = terrain.getVertices();

    // Calcule des coordonnées X et Z du cube par rapport au terrain
    float terrainX = cubePosition.x - terrain.transform.getPosition().x;
    float terrainZ = cubePosition.z - terrain.transform.getPosition().z;

    // on convertit les coordonnées du terrain en indices de grille
    int gridX = static_cast<int>((terrainX / terrainWidth) * (terrainWidth - 1));
    int gridZ = static_cast<int>((terrainZ / terrainDepth) * (terrainDepth - 1));

    // on s'assure que les indices de la grille sont dans les limites du terrain
    gridX = clamp(gridX, 0, terrainWidth - 1);
    gridZ = clamp(gridZ, 0, terrainDepth - 1);

    // on calcule les coordonnées interpolées pour trouver la hauteur du terrain
    float xCoord = fmod(terrainX, terrainWidth / (terrainWidth - 1)) / (terrainWidth / (terrainWidth - 1));
    float zCoord = fmod(terrainZ, terrainDepth / (terrainDepth - 1)) / (terrainDepth / (terrainDepth - 1));

    // Interpolation bilinéaire pour obtenir la hauteur du terrain
    float height00 = terrainVertices[gridZ * terrainWidth + gridX].y;
    float height10 = terrainVertices[gridZ * terrainWidth + (gridX + 1)].y;
    float height01 = terrainVertices[(gridZ + 1) * terrainWidth + gridX].y;
    float height11 = terrainVertices[(gridZ + 1) * terrainWidth + (gridX + 1)].y;

    float height = lerp(
        lerp(height00, height10, xCoord),
        lerp(height01, height11, xCoord),
        zCoord
    );

    return height;
}

// Saut sans rebond
// void resolveCollision(Object &cube, Terrain &terrain) {
//     vec3 cubePosition = cube.transform.getPosition();
//     vec3 terrainPosition = terrain.transform.getPosition();
//     float terrainHeight = terrainHeightAtCubePosition(terrain, cubePosition); 
//     cubePosition.y = terrainPosition.y + terrainHeight; 
//     cube.transform.setPosition(cubePosition + glm::vec3(0.0f, 0.5f, 0.0f));

//     cube.setVelocity(vec3(0.0f));
// }

void resolveCollision(Object &voiture, Terrain &terrain, float dampingFactor) {
    vec3 voiturePosition = voiture.transform.getPosition();
    vec3 terrainPosition = terrain.transform.getPosition();
    float terrainHeight = terrainHeightAtCubePosition(terrain, voiturePosition); 
    voiturePosition.y = terrainPosition.y + terrainHeight; // Placez le voiture sur le terrain
    // vec3 voitureSize = voiture.getSize();
    voiture.transform.setPosition(voiturePosition + glm::vec3(0.0f, 0.62, 0.0f));

    // on calcule la direction de la collision en comparant les positions de l'objet et du terrain
    vec3 collisionDirection = normalize(terrainPosition - voiturePosition);

    vec3 velocity = voiture.getVelocity();
    velocity.y = -velocity.y; 
    voiture.setVelocity(velocity);

    // on calcule le facteur de déplacement en fonction de la magnitude de la vitesse et du temps écoulé
    float displacementFactor = length(velocity) * deltaTime;

    // on applique un facteur d'amortissement à la vitesse du voiture pour réduire progressivement la magnitude de la vitesse
    velocity *= (1.0f - dampingFactor); 
    voiture.setVelocity(velocity);

    voiture.transform.setPosition(voiturePosition + glm::vec3(0., 0.62, 0.));

    // on déplace le voiture dans la direction opposée pour simuler le rebond
   // voiture.transform.setPosition(voiturePosition +  collisionDirection * displacementFactor + glm::vec3(0.00018f, 0.62, 0.00018f));
}


// Saut avec rebond
// void resolveCollision(Object &cube, Terrain &terrain, float dampingFactor) {
//     vec3 cubePosition = cube.transform.getPosition();
//     vec3 terrainPosition = terrain.transform.getPosition();
//     float terrainHeight = terrainHeightAtCubePosition(terrain, cubePosition); 
//     cubePosition.y = terrainPosition.y + terrainHeight; // Placez le cube sur le terrain
//     cube.transform.setPosition(cubePosition + glm::vec3(0.0f, 0.5f, 0.0f));

//     // on calcule la direction de la collision en comparant les positions de l'objet et du terrain
//     vec3 collisionDirection = normalize(terrainPosition - cubePosition);

//     vec3 velocity = cube.getVelocity();
//     velocity.y = -velocity.y; 
//     cube.setVelocity(velocity);

//     // on calcule le facteur de déplacement en fonction de la magnitude de la vitesse et du temps écoulé
//     float displacementFactor = length(velocity) * deltaTime;

//     // on applique un facteur d'amortissement à la vitesse du cube pour réduire progressivement la magnitude de la vitesse
//     velocity *= (1.0f - dampingFactor); 
//     cube.setVelocity(velocity);

//     // on déplace le cube dans la direction opposée pour simuler le rebond
//     cube.transform.setPosition(cubePosition +  collisionDirection * displacementFactor + glm::vec3(0.00018f, 0.5f, 0.00018f));
// }

// void resolveCollision(Object &cube, Terrain &terrain, float dampingFactor) {
//     vec3 cubePosition = cube.transform.getPosition();
//     vec3 terrainPosition = terrain.transform.getPosition();
//     float terrainHeight = terrainHeightAtCubePosition(terrain, cubePosition); 
//     float cubeHeight = cubePosition.y - terrainPosition.y;
    
//     // Vérifier si une collision est détectée
//     if (cubeHeight < terrainHeight) {
//         cubePosition.y = terrainPosition.y + terrainHeight; // Placez le cube sur le terrain
//         cube.transform.setPosition(cubePosition);

//         // on calcule la direction de la collision en comparant les positions de l'objet et du terrain
//         vec3 collisionDirection = normalize(terrainPosition - cubePosition);

//         vec3 velocity = cube.getVelocity();
//         velocity.y = -velocity.y; 
//         cube.setVelocity(velocity);

//         // on calcule le facteur de déplacement en fonction de la magnitude de la vitesse et du temps écoulé
//         float displacementFactor = length(velocity) * deltaTime;

//         // on applique un facteur d'amortissement à la vitesse du cube pour réduire progressivement la magnitude de la vitesse
//         velocity *= (1.0f - dampingFactor); 
//         cube.setVelocity(velocity);

//         // on déplace le cube dans la direction opposée pour simuler le rebond
//         cube.transform.Translate(collisionDirection * displacementFactor);
//     }
// }

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
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(1024, 768, "TP1 - GLFW", NULL, NULL);
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
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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
    //  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, 1024 / 2, 768 / 2);

    // Dark blue background
    glClearColor(0.8f, 0.8f, 0.8f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Cull triangles which normal is not towards the camera
    // glEnable(GL_CULL_FACE);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders("vertex_shader.glsl", "fragment_shader.glsl");
    // GLuint programID2 = LoadShaders("vertexCube.glsl", "fragmentCube.glsl");

    /*****************TODO***********************/
    // Get a handle for our "Model View Projection" matrices uniforms

    /****************************************/
    // std::vector<std::vector<unsigned short>> triangles;

    glm::mat4 Model  = glm::mat4(1.f);
    glm::mat4 View =  glm::mat4(1.f);
    glm::mat4 Projection = glm::mat4(1.f);

    // Chargement du fichier de maillage
    //std::string filename("chair.off");
    // loadOFF(filename, indexed_vertices, indices, triangles );

    // Get a handle for our "LightPosition" uniform
    glUseProgram(programID);
    GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

    // For speed computation
    double lastTime = glfwGetTime();
    int nbFrames = 0;

     //soleil.loadObject("./sphere.off");
     //soleil.loadTexture("../textures/sun.jpg");

    // cube.loadObject("./cube.off");
    // cube.loadTexture("../textures/snowrocks.png");
    // // cube.loadTexture("../textures/sun.jpg");
    // // cube.transform.Scale(vec3(0.3, 0.3, 0.3));
    // cube.transform.Translate(vec3(8., 0.5, 8.));

    // soleil.transform.Scale(vec3(0.3, 0.3, 0.3));
    // soleil.transform.Translate(vec3(2, 2, 0));

    voiture.loadObjectCustom("./Porsche_911_GT2.obj",0);
    voiture.loadTexture("../textures/or.jpg");
    voiture.transform.Translate(vec3(8, 0.62, 8));

    // Terrain plan;
    Terrain plan(64, 64, 16, 16, 0, 0, true, "../textures/grass.png",0);
    plan.loadTexture("../textures/grass.png");


    // plan.add(cube);
    plan.add(voiture);

    // centrer la caméra sur le plan
    centrerCamera(plan.getVertices(), camera_position, camera_target);
    centrerCamera(plan.getVertices(), cameraPosOrbit, cameraTargetOrbit); // have to try to fix this...


    // cube.setVelocity(glm::vec3(1.f));
    // cube.setAcceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    // cube.setMass(1.0f);
    // cube.setForce(glm::vec3(0.0f, 0.0f, 0.0f));

    voiture.setVelocity(glm::vec3(0));
    voiture.setAcceleration(glm::vec3(0.0f, -0.0f, 0.0f));
    voiture.setMass(1.0f);
    voiture.setForce(glm::vec3(0.0f, 0.0f, 0.0f));
    voiture.setk(0.3);
    bool isFirstFrame = true;

    do
    {
        // Measure speed
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window,plan);

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use our shader
        glUseProgram(programID);

        /*****************TODO***********************/
        // Model matrix : an identity matrix (model will be at the origin) then change

        // View matrix : camera/view transformation lookat() utiliser cameraFree_position cameraFree_target cameraFree_up
        if (libre)
        {
            View = glm::lookAt(camera_position, camera_position + camera_target, camera_up);
            // glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        }

        if (orbital)
        {
            View = glm::lookAt(cameraPosOrbit, cameraTargetOrbit, camera_up);
        }

        // Projection matrix : 45 Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
        Projection = glm::perspective(glm::radians(45.f), 4.0f / 3.0f, 0.1f, 100.0f);
        
        // Send our transformation to the currently bound shader,
        // in the "Model View Projection" to the shader uniforms
        glUniformMatrix4fv(glGetUniformLocation(programID, "View"), 1, GL_FALSE, &View[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(programID, "Projection"), 1, GL_FALSE, &Projection[0][0]);

        if(!isFirstFrame){
            voiture.updatePhysics2(deltaTime);
        }
        // voiture.updatePhysics(deltaTime);

        // on vérifie si il y a collision entre le cube et le terrain
        if (!isFirstFrame && collisionDetection(voiture, plan)) {
            // // std::cout<<plan.getSize()[0]<<" "<<plan.getSize()[1]<<" "<<plan.getSize()[2]<<std::endl;
            // std::cout<<plan.transform.getPosition()[0]<<" "<<plan.transform.getPosition()[1]<<" "<<plan.transform.getPosition()[2]<<std::endl;
            // std::cout<<cube.transform.getPosition()[0]<<" "<<cube.transform.getPosition()[1]<<" "<<cube.transform.getPosition()[2]<<std::endl;
            resolveCollision(voiture, plan, 0.5);
            // resolveCollision(voiture, plan);
        }
        // voiture.update();
        voiture.draw2(programID);

        plan.update();
        plan.draw(programID);

        isFirstFrame = false;

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
void processInput(GLFWwindow *window, Terrain &plan)
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
        if(libre){
            camera_target = glm::normalize(front);
        }
        if(orbital){
            cameraTargetOrbit = glm::normalize(front);
        }
    } else {
        firstMouse = true;
    }
    /******************************/

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if(glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS){
        libre = false; orbital = true;
    }

    if(glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS){
        libre = true; orbital = false;
    }

    float cameraSpeed = 2.5 * deltaTime;

    if(libre){
        //Camera zoom in and out
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera_position += cameraSpeed * camera_target;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera_position -= cameraSpeed * camera_target;

        // camera move left and right
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera_position -= glm::normalize(glm::cross(camera_target, camera_up)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            camera_position += glm::normalize(glm::cross(camera_target, camera_up)) * cameraSpeed;

        // move up and down
        if (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
            camera_position += camera_up * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            camera_position -= camera_up * cameraSpeed;
    }

    if(orbital){
        //Camera zoom in and out
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cameraPosOrbit += cameraSpeed * cameraTargetOrbit;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cameraPosOrbit -= cameraSpeed * cameraTargetOrbit;
            
        if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            cameraPosOrbit += glm::normalize(camera_up) * cameraSpeed;
        if(glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
            cameraPosOrbit -= glm::normalize(camera_up) * cameraSpeed;
        if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cameraPosOrbit += glm::normalize(glm::cross(cameraTargetOrbit - cameraPosOrbit, camera_up))*cameraSpeed;
        if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cameraPosOrbit -= glm::normalize(glm::cross(cameraTargetOrbit - cameraPosOrbit, camera_up))*cameraSpeed;
    }

    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        // voiture.transform.Translate(vec3(0.2, 0, 0));
       voiture.transform.Rotation(vec3(0.,1.,0.),2);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        // voiture.transform.Translate(vec3(-0.2, 0, 0));
        voiture.transform.Rotation(vec3(0.,1.,0.),-2);
    }

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        voiture.applyForce(vec3(0, 0, -5));
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        voiture.applyForce(vec3(0, 0, 5));
    }

    // controls pour translater le cube sur l'axe y (haut et bas)
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
    {
        voiture.transform.Translate(vec3(0, 0.2, 0));
    }
    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
    {
        voiture.transform.Translate(vec3(0, -0.2, 0));
    }


    // if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    // {
    //     cube.setVelocity(glm::vec3(1.f));
    //     // Calculer le vecteur de direction à 45 degrés vers le haut par rapport à la direction de vue
    //     glm::vec3 direction = glm::normalize(camera_target + camera_up);

    //     // Mettre à jour la position de l'objet en fonction de la vitesse et du temps écoulé
    //     float deltaTimeCurr = deltaTime;
    //     glm::vec3 newPosition = cube.transform.getPosition() + cube.getVelocity() * direction * deltaTimeCurr;
    //     cube.transform.setPosition(newPosition);
    // }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        // vecteur de direction à 45 degrés vers le haut par rapport à la direction de vue
        glm::vec3 direction = glm::normalize(camera_target + camera_up);

        voiture.setSpecialVelocity(direction * 2.0f); // Vitesse pour le saut
    }


    // reset the position 
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
    {
        // vecteur de direction à 45 degrés vers le haut par rapport à la direction de vue
        voiture.transform.setPosition(glm::vec3(8, 0.61, 8));
        voiture.setVelocity(glm::vec3(1.f));
        voiture.setAcceleration(glm::vec3(0.0f, -9.81f, 0.0f));
        voiture.setMass(1.0f);
        voiture.setForce(glm::vec3(0.0f, 0.0f, 0.0f));

    }

    

    // if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    // {
    //     cube.setVelocity(glm::vec3(1.f));
    //     // Calculer le vecteur de direction à 45 degrés vers le haut par rapport à la direction de vue
    //     glm::vec3 direction = glm::normalize(camera_target + camera_up);
    //     float velocityMagnitude = glm::length(cube.getVelocity());
    //     glm::vec3 newVelocity = cube.getVelocity() + direction * velocityMagnitude;

    //     cube.transform.setPosition(cube.transform.getPosition() + newVelocity * deltaTime);

    //     // Mettre à jour la vélocité du cube
    //     cube.setVelocity(newVelocity);
    // }


    // if (glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS){
    //     resolution++;
    //     setTesselatedSquare(indices, indexed_vertices);
    //     glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    //     glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

    //     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
    //     glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);
    // }
    // if(glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS){
    //     if(resolution>0){ resolution--;}
    //     setTesselatedSquare(indices, indexed_vertices);
    //     glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    //     glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

    //     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
    //     glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);
    // }

    // update camera target based on the new position
    camera_target = glm::normalize(camera_target);
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
