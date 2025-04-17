// Include standard headers
#include <GL/glew.h>
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <unistd.h>

// Include GLEW

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

using namespace glm;

#include <common/shader.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
#include <common/texture.hpp>
#include <common/text2D.hpp>

#include "scene.h"
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
// #include "stb_image.h"
#include <TP5/stb_image.h>
#endif

void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

//Gestion input 
bool toggleInputC = false;
bool toggleInput1 = false;
bool toggleInput2 = false;
bool toggleInput3 = false;
bool toggleInput5 = false;
bool toggleInput8 = false;
bool toggleInputSpace = false;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;
int nbFrames = 0;

// Scène 
Scene scene;

// Fonction pour afficher le compteur de FPS
double affiche(GLFWwindow *window,double lastTime){
    double currentTime = glfwGetTime();
    nbFrames++;
    if ( currentTime - lastTime >= 1.0 ){ 
        double fps = double(nbFrames);
        std::string title = "TP3 - FPS : " + std::to_string(fps);
        glfwSetWindowTitle(window, title.c_str());
        nbFrames = 0;
        lastTime += 1.0;
    }
    return lastTime;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);


    float cameraSpeed = 5.f * deltaTime; // Ajuster la vitesse en fonction du temps écoulé

    // Récupérer les vecteurs de direction de la caméra
    glm::vec3 cameraPosition = scene.camera.globalTransform.t; // Position actuelle de la caméra
    glm::mat3 cameraRotation = scene.camera.globalTransform.m; // Rotation actuelle de la caméra
    glm::vec3 cameraTarget = -cameraRotation[2];               // Axe Z local (direction vers l'avant)
    glm::vec3 cameraRight = cameraRotation[0];                 // Axe X local (direction droite)
    glm::vec3 cameraUp = cameraRotation[1];                    // Axe Y local (direction vers le haut)

    if (scene.camera.mode == CAMERA_MODE::ORBITAL) {
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            cameraPosition += cameraSpeed * cameraTarget; // Avancer
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            cameraPosition -= cameraSpeed * cameraTarget; // Reculer
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cameraPosition += cameraRight * cameraSpeed;  // Déplacer à gauche
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cameraPosition -= cameraRight * cameraSpeed;  // Déplacer à droite
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cameraPosition += cameraUp * cameraSpeed;     // Monter
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cameraPosition -= cameraUp * cameraSpeed;     // Descendre
        scene.camera.globalTransform.t = cameraPosition;
    }
        if(glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS){
            scene.camera.parent->Move(deltaTime,glm::vec3(0.0f,0.0,1.0f));
        }
        if(glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS){
            scene.camera.parent->Move(deltaTime,glm::vec3(0.0f,0.0,-1.0f));
        }
        if(glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS){
            scene.camera.parent->Move(deltaTime,glm::vec3(-1.0f,0.0,0.0f));
        }
        if(glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS){
            scene.camera.parent->Move(deltaTime,glm::vec3(1.0f,0.0,0.0f));
        }
        if(glfwGetKey(window, GLFW_KEY_Q)){
            scene.camera.parent->Move(deltaTime,glm::vec3(0.0f,1.0,0.0f));
        }
        if(glfwGetKey(window, GLFW_KEY_Z)){
            scene.camera.parent->Move(deltaTime,glm::vec3(0.0f,-1.0,0.0f));
        }
        if(glfwGetKey(window, GLFW_KEY_SPACE) && toggleInputSpace == false){
            toggleInputSpace = true;
            if(scene.camera.parent->speed[1] <= glm::vec3(0.0)[1]) scene.camera.parent->speed = glm::vec3(0.0f,4.0f,0.0f);
        }

    if(glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && toggleInputC == false){
        toggleInputC = true;
        if(scene.camera.mode == CAMERA_MODE::CLASSIC){
            scene.camera.mode = CAMERA_MODE::ORBITAL;
            std::cout<<"Mode camera : orbital"<<std::endl;
        } else{
            scene.camera.mode = CAMERA_MODE::CLASSIC;
            std::cout<<"Mode camera : classique"<<std::endl;
        }
    }
    if(glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE && toggleInputC == true){
        toggleInputC = false;
    }
        if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE && toggleInputSpace == true){
        toggleInputSpace = false;
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main( void )
{
    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
        return -1;
    }
    if (!glewIsSupported("GL_ARB_texture_non_power_of_two")) {
        std::cerr << "Your hardware does not support Non-Power-Of-Two textures." << std::endl;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow( SCR_WIDTH, SCR_HEIGHT, "TP3 - GLFW", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
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
    glfwSetCursorPos(window, SCR_WIDTH/2, SCR_HEIGHT/2);

    // Dark blue background
    glClearColor(0.8f, 0.8f, 0.8f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Cull triangles which normal is not towards the camera
    //glEnable(GL_CULL_FACE);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    GLuint programID = LoadShaders( "vertex_shader.glsl", "fragment_shader.glsl" );
        
    scene.root.programID=programID;
    
    // Affichage de la Map :
    std::cout<<"Chargement de la Map"<<std::endl;
    GameObject Map;
    Plane map(16);
    Map.setPlan(map);
    scene.root.addChild(&Map);
    Map.setGlobalTransform(Transform(glm::mat3x3(1.0),glm::vec3(0.0,0.0,0.0),1.0));
    Map.setLocalTransform(Transform().scale(5));
    
    // Affichage de l'objet :
    GameObject Obj;
    std::cout<<"Chargement de l'objet"<<std::endl;
    Obj.setLODMeshes("../mesh/sphere.off",true);
    Map.addChild(&Obj);
    Obj.setLocalTransform(Transform().scale(0.1));
    Obj.setGlobalTransform(Obj.globalTransform.combine_with(Transform().translation(glm::vec3(0.0,1.0,0.0),0.1)));  
    Obj.height2parent = 0.1f;   


    // Ajout de la caméra :
    std::cout<<"Chargement de la Caméra"<<std::endl;
    Camera camera(45.0f, float(SCR_WIDTH)/float(SCR_HEIGHT), 0.1f, 100.0f);
    Obj.addChild(&camera);
    camera.setGlobalTransform(camera.globalTransform.combine_with(Transform(glm::mat3x3(1.0),glm::vec3(0.0,1.0,-1.0),1.0)));
    scene.camera = camera;
    scene.camera.lookAt(&Obj);


    GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");


    // For speed computation
    double lastTime = glfwGetTime();
    int nbFrames = 0;

    do{
        lastTime = affiche(window,lastTime);
        // Measure speed
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);


        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use our shader
        glUseProgram(programID);


        // Model matrix : an identity matrix (model will be at the origin) then change
        glm::mat4 Model = glm::mat4(1.0f);
        GLint uniModel = glGetUniformLocation(programID,"model");
        glUniformMatrix4fv(uniModel,1,GL_FALSE,&Model[0][0]);
        scene.camera.lookAt(scene.camera.parent);
        scene.camera.updateProjectionMatrix();

        // View matrix : camera/view transformation lookat() utiliser camera_position camera_target camera_up
        glm::mat4 View = scene.camera.viewMatrix;
        GLint uniView = glGetUniformLocation(programID,"view");
        glUniformMatrix4fv(uniView,1,GL_FALSE,&View[0][0]);

        // Projection matrix : 45 Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
        glm::mat4 Projection = scene.camera.projectionMatrix;
        GLint uniProjection = glGetUniformLocation(programID,"proj");
        glUniformMatrix4fv(uniProjection,1,GL_FALSE,&Projection[0][0]);

        scene.draw(deltaTime);
        


        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0 );

    glDeleteProgram(programID);
    glDeleteVertexArrays(1, &VertexArrayID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}
