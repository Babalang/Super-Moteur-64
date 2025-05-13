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
#define GLFW_INCLUDE_NONE
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
// #include "../cegui-0-8-7/cegui/include/CEGUI/CEGUI.h"
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
bool toggleInputTab = false;
bool toggleInputI = false;
bool toggleInputK = false;
bool toggleInputL = false;
bool toggleInputJ = false;
bool toggleInputQ = false;
bool toggleInputZ = false;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;
int nbFrames = 0;

// Scène 
Scene scene;
Camera camera(45.0f, float(SCR_WIDTH)/float(SCR_HEIGHT), 0.1f, 1000.0f);
int niveau=0;
bool carte=false;
GLuint programID;

// Fonction pour afficher le compteur de FPS
double affiche(GLFWwindow *window,double lastTime){
    double currentTime = glfwGetTime();
    nbFrames++;
    if ( currentTime - lastTime >= 1.0 ){ 
        double fps = double(nbFrames);
        std::string title = "Super Mario - FPS : " + std::to_string(fps);
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
    float cameraZoomSpeed = 0.1f; // Vitesse de zoom de la caméra


    // Récupérer les vecteurs de direction de la caméra
    glm::vec3 cameraPosition = scene.camera.globalTransform.t; // Position actuelle de la caméra
    glm::mat3 cameraRotation = scene.camera.globalTransform.m; // Rotation actuelle de la caméra
    glm::vec3 cameraTarget = -cameraRotation[2];               // Axe Z local (direction vers l'avant)
    glm::vec3 cameraRight = cameraRotation[0];                 // Axe X local (direction droite)
    glm::vec3 cameraUp = cameraRotation[1];                    // Axe Y local (direction vers le haut)


    if (scene.camera.mode == CAMERA_MODE::ORBITAL) {
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            scene.camera.phi = glm::clamp(scene.camera.phi + cameraSpeed, glm::radians(-89.0f), glm::radians(89.0f)); // Reculer
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            scene.camera.phi = glm::clamp(scene.camera.phi - cameraSpeed, glm::radians(-89.0f), glm::radians(89.0f)); // Reculer
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            scene.camera.theta -= cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            scene.camera.theta += cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS){
            scene.camera.orbitalRadius = glm::max(10.0f, scene.camera.orbitalRadius - cameraZoomSpeed);
            std::cout<<"Camera distance : "<<scene.camera.orbitalRadius<<std::endl;

        }
        if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS){
            scene.camera.orbitalRadius = glm::min(100.0f, scene.camera.orbitalRadius + cameraZoomSpeed);
            std::cout<<"Camera distance : "<<scene.camera.orbitalRadius<<std::endl;
        }
    }
        if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
            if(!toggleInputI){
                scene.camera.parent->axe += glm::vec3(0.0f,0.0,1.0f);
                toggleInputI = true;
            }
        } 
        if(glfwGetKey(window, GLFW_KEY_W) == GLFW_RELEASE && toggleInputI == true){
            toggleInputI = false;
            scene.camera.parent->axe -= glm::vec3(0.0f,0.0,1.0f);
        }
        if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
            if(!toggleInputK){
                scene.camera.parent->axe -= glm::vec3(0.0f,0.0,1.0f);
                toggleInputK = true;
            }
        }
        if(glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE && toggleInputK == true){
            toggleInputK = false;
            scene.camera.parent->axe += glm::vec3(0.0f,0.0,1.0f);
        }
        if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
            if(!toggleInputL){
                scene.camera.parent->axe += glm::vec3(-1.0f,0.0,0.0f);
                toggleInputL = true;
            }
        }
        if(glfwGetKey(window, GLFW_KEY_D) == GLFW_RELEASE && toggleInputL == true){
            toggleInputL = false;
            scene.camera.parent->axe -= glm::vec3(-1.0f,0.0,0.0f);
        }
        if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
            if(!toggleInputJ){
                scene.camera.parent->axe += glm::vec3(1.0f,0.0,0.0f);
                toggleInputJ = true;
            }
        }
        if(glfwGetKey(window, GLFW_KEY_A) == GLFW_RELEASE && toggleInputJ == true){
            toggleInputJ = false;
            scene.camera.parent->axe -= glm::vec3(1.0f,0.0,0.0f);
        }
        if(glfwGetKey(window, GLFW_KEY_SPACE) && toggleInputSpace == false){
            toggleInputSpace = true;
            if(scene.camera.parent->speed[1] <= glm::vec3(0.0)[1]) scene.camera.parent->speed = glm::vec3(0.0f,10.0f,0.0f);
        }
        if(glfwGetKey(window, GLFW_KEY_TAB) && toggleInputTab == false){
            toggleInputTab = true;
            carte=!carte;
        }

    if(glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && toggleInputC == false){
        toggleInputC = true;
        if(scene.camera.mode == CAMERA_MODE::CLASSIC){
            scene.camera.mode = CAMERA_MODE::ORBITAL;
            std::cout<<"Mode camera : orbital"<<std::endl;
        } else if(scene.camera.mode == CAMERA_MODE::ORBITAL){
            scene.camera.mode = CAMERA_MODE::FIRST_PERSON;
            std::cout<<"Mode camera : up"<<std::endl;
        } else if(scene.camera.mode == CAMERA_MODE::FIRST_PERSON){
            scene.camera.mode = CAMERA_MODE::CLASSIC;
            std::cout<<"Mode camera : classic"<<std::endl;
        }
    }
    if(glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE && toggleInputC == true){
        toggleInputC = false;
    }
        if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE && toggleInputSpace == true){
        toggleInputSpace = false;
    }
    if(glfwGetKey(window, GLFW_KEY_TAB) == GLFW_RELEASE && toggleInputTab == true){
        toggleInputTab = false;
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

GameObject GOchateau;GameObject GOmariometal;GameObject light, GOPeach;
void sceneNiveau1(Scene *scene){
    GLuint programID=scene->root.programID;

    
    GOchateau.programID=programID;
    GOchateau.lireOBJ("../meshes/chateau.obj");
    GOchateau.rajouterOBJ();
    GOchateau.setLocalTransform(Transform(glm::mat3x3(1.0),glm::vec3(0.0,0.0,0.0),10.0));
    
    
    GOmariometal.programID=programID;
    GOmariometal.lireOBJ("../meshes/Mario.obj");
    std::cout<<"Chargement de l'objet"<<std::endl;
    GOmariometal.rajouterOBJ();
    GOmariometal.isGround = true;
    Transform scale = Transform().scale(0.020f);
    Transform roll = Transform().rotation(glm::vec3(1.0f, 0.0f, 0.0f), -90.0f); // Roll (90° autour de Z)
    Transform yaw = Transform().rotation(glm::vec3(0.0f, 1.0f, 0.0f), 180.0f); // Yaw (180° autour de Y)
    Transform combined = yaw.combine_with(roll); // Combinaison des transformations

    GOmariometal.setLocalTransform(Transform(glm::mat3x3(1.0),glm::vec3(0.0,1.0,0.0),1.0).combine_with(scale).combine_with(roll));
    GOmariometal.initialTransform = GOmariometal.transform;

    
    // Parent = translation uniquement
    Transform translation =Transform(glm::mat3(1.0f), glm::vec3(20.0f, 10.0f, -50.5f), 1.0f);
    GOmariometal.setGlobalTransform(translation);
    std::cout<<GOmariometal.globalTransform.t[0]<<" "<<GOmariometal.globalTransform.t[1]<<" "<<GOmariometal.globalTransform.t[2]<<std::endl;

    GOPeach.programID=programID;
    GOPeach.lireOBJ("../meshes/Peach.obj");
    GOPeach.rajouterOBJ();
    Transform tPeach=Transform(glm::mat3x3(1.0),glm::vec3(-3.0,5.0,20.0),0.02);
    GOPeach.setLocalTransform(tPeach.combine_with(tPeach.rotation(glm::vec3(0.0f, 1.0f, 0.0f), 180.0f)));
    
    // Affichage de la lumière :
    std::cout<<"Chargement de la lumière"<<std::endl;
    
    light.programID=programID;
    light.setLODMeshes("../meshes/sphere.off",false, "../textures/s2.ppm");
    light.setLocalTransform(Transform().scale(5.0f));
    light.setGlobalTransform(Transform(glm::mat3x3(1.0),glm::vec3(50.0,50.0,50.0),1.0));
    light.lightIntensity = 100000.0f;
    light.isLight = true;
    light.lightColor = glm::vec3(1.0f,1.0f,0.8f)* light.lightIntensity;

    // Ajout de la caméra :
    std::cout<<"Chargement de la Caméra"<<std::endl;
    GOmariometal.addChild(&camera);
    scene->camera = camera;
    scene->camera.lookAt(&GOmariometal);
    scene->root.addChild(&GOchateau);
    GOchateau.addChild(&GOmariometal);
    GOchateau.addChild(&GOPeach);
    scene->lights.push_back(&light);
    GOchateau.map=true;
    GOmariometal.collisions.push_back(&GOchateau);
    GOmariometal.collisions.push_back(&GOPeach);
    GOPeach.collisions.push_back(&GOchateau);
    GOmariometal.nom="mario";
    GOPeach.nom="peach";
    GOchateau.nom="chateau";
    GOPeach.mettreAuSol(&GOchateau);
    GOmariometal.mettreAuSol(&GOchateau);
    GOmariometal.pv=3;
}

GameObject GOBobombBattlefieldDS,GOBattanKing,light2,GOGoomba1,GOMetalMario2;
void sceneNiveau2(Scene *scene){
    GLuint programID=scene->root.programID;
    
    //Affichage de la Map :
    GOBobombBattlefieldDS.programID=programID;
    GOBobombBattlefieldDS.lireOBJ("../meshes/SM64DS_Model.obj");
    GOBobombBattlefieldDS.rajouterOBJ();
    GOBobombBattlefieldDS.setLocalTransform(Transform(glm::mat3x3(1.0),glm::vec3(0.0,0.0,0.0),10.0));
    GOBobombBattlefieldDS.setGlobalTransform(Transform(glm::mat3x3(1.0),glm::vec3(0.0,0.0,0.0),10.0));
    
    //Affichage de l'objet :
    GOMetalMario2.programID=programID;
    GOMetalMario2.lireOBJ("../meshes/Mario.obj");
    GOMetalMario2.rajouterOBJ();
    Transform scale      = Transform().scale(0.1f);
    Transform translate  = Transform().translation(glm::vec3(0.0f, 1.0f, 0.0f), 5.0f);
    Transform rotateX    = Transform().rotation(glm::vec3(1.0f, 0.0f, 0.0f), -90.0f);
    Transform rotateY    = Transform().rotation(glm::vec3(0.0f, 1.0f, 0.0f), 180.0f);
    Transform finalTransform = translate
        .combine_with(rotateY)
        .combine_with(rotateX)
        .combine_with(scale);
    GOMetalMario2.setLocalTransform(finalTransform);

    light2.programID=programID;
    light2.setLODMeshes("../meshes/sphere.off",false, "../textures/s2.ppm");
    light2.setLocalTransform(Transform().scale(5.0f));
    light2.setGlobalTransform(Transform(glm::mat3x3(1.0),glm::vec3(50.0,50.0,50.0),1.0));
    light2.lightIntensity = 100000.0f;
    light2.isLight = true;
    light2.lightColor = glm::vec3(1.0f,1.0f,0.8f)* light2.lightIntensity;

    GOMetalMario2.addChild(&camera);
    camera.setGlobalTransform(camera.globalTransform.combine_with(Transform(glm::mat3x3(1.0),glm::vec3(0.0,51.0,-1.0),1.0)));
    scene->camera = camera;
    scene->camera.lookAt(&GOMetalMario2);

    GOGoomba1.programID=programID;
    GOGoomba1.lireOBJ("../meshes/kuribo_model.obj");
    GOGoomba1.rajouterOBJ();
    Transform tGoomba1=Transform(glm::mat3x3(1.0),glm::vec3(0.0,5.0,-3.0),1.0);
    GOGoomba1.setLocalTransform(tGoomba1);
    GOGoomba1.setGlobalTransform(tGoomba1);

    GOBattanKing.programID=programID;
    GOBattanKing.lireOBJ("../meshes/battan_king.obj");
    Transform tBattanKing=Transform(glm::mat3x3(1.0),glm::vec3(0.0,5.0,-3.0),0.1);
    GOBattanKing.rajouterOBJ();
    GOBattanKing.setLocalTransform(tBattanKing);
    GOBattanKing.setGlobalTransform(tBattanKing);
    
    scene->root.addChild(&GOBobombBattlefieldDS);
    GOBobombBattlefieldDS.addChild(&GOMetalMario2);
    GOBobombBattlefieldDS.addChild(&GOGoomba1);
    GOBobombBattlefieldDS.addChild(&GOBattanKing);
    scene->lights.push_back(&light2);
}

GameObject GOkoopa1,GObowserStadium,GOMetalMario3,GOkoopa2,GOBowser,light3,GOstar;
void sceneNiveau3(Scene *scene){
    GLuint programID=scene->root.programID;
    
    //Affichage de la Map :
    GObowserStadium.programID=programID;
    GObowserStadium.lireOBJ("../meshes/Bowser_Stadium.obj");
    GObowserStadium.rajouterOBJ();
    GObowserStadium.setLocalTransform(Transform(glm::mat3x3(1.0),glm::vec3(0.0,-0.0,0.0),50.0));
    
    //Affichage de l'objet :
    GOMetalMario3.programID=programID;
    GOMetalMario3.lireOBJ("../meshes/Mario.obj");
    GOMetalMario3.rajouterOBJ();
    GOMetalMario3.isGround = true;
    Transform scale      = Transform().scale(0.04f);
    Transform translate  = Transform().translation(glm::vec3(-10.0f, 10.0f, 0.0f), 1.0f);
    Transform rotateX    = Transform().rotation(glm::vec3(1.0f, 0.0f, 0.0f), -90.0f);
    Transform rotateY    = Transform().rotation(glm::vec3(0.0f, 1.0f, 0.0f), 90.0f);
    Transform combined = rotateY.combine_with(rotateX);
    GOMetalMario3.setLocalTransform(Transform(glm::mat3x3(1.0),glm::vec3(0.0,1.0,0.0),1.0).combine_with(scale).combine_with(rotateX));
    GOMetalMario3.initialTransform = GOMetalMario3.transform;
    Transform translation =Transform(glm::mat3(1.0f), glm::vec3(0.0f, 120.0f, -50.0f), 1.0f);
    GOMetalMario3.setGlobalTransform(translation);

    light3.programID=programID;
    light3.setLODMeshes("../meshes/sphere.off",false, "../textures/s2.ppm");
    light3.setLocalTransform(Transform().scale(5.0f));
    light3.setGlobalTransform(Transform(glm::mat3x3(1.0),glm::vec3(50.0,50.0,50.0),1.0));
    light3.lightIntensity = 100000.0f;
    light3.isLight = true;
    light3.lightColor = glm::vec3(1.0f,1.0f,0.8f)* light3.lightIntensity;


    GameObject a,b;
    a.programID=programID;b.programID=programID;
    a.lireOBJ("../meshes/Koopa.obj");b.lireOBJ("../meshes/Shell.obj");

    GOkoopa1.programID=programID;
    GOkoopa1.lireOBJ("../meshes/Koopa.obj");
    GOkoopa1.rajouterOBJ();
    Transform tKoopa1=Transform(glm::mat3x3(1.0),glm::vec3(-30.0,120.0,30.0),1.0);
    tKoopa1=tKoopa1.combine_with(Transform().rotation(glm::vec3(0.0f, 1.0f, 0.0f), -90.0f));
    GOkoopa1.setGlobalTransform(tKoopa1);
    GOkoopa1.nom="koopa1";
    GOkoopa1.koopaOBJ=a.objetsOBJ;GOkoopa1.shellOBJ=b.objetsOBJ;

    GOkoopa2.programID=programID;
    GOkoopa2.lireOBJ("../meshes/Koopa.obj");
    Transform tKoopa2=Transform(glm::mat3x3(1.0),glm::vec3(30.0,120.0,30.0),1.0);
    tKoopa2=tKoopa2.combine_with(Transform().rotation(glm::vec3(0.0f, 1.0f, 0.0f), -90.0f));
    GOkoopa2.rajouterOBJ();
    GOkoopa2.setGlobalTransform(tKoopa2);
    GOkoopa2.nom="koopa2";
    GOkoopa2.koopaOBJ=a.objetsOBJ;GOkoopa2.shellOBJ=b.objetsOBJ;

    GOBowser.programID=programID;
    GOBowser.lireOBJ("../meshes/koopa_model.obj");
    Transform tkoopa_model=Transform(glm::mat3x3(1.0),glm::vec3(0.0,120.0,50.0),0.3);
    tkoopa_model=tkoopa_model.combine_with(Transform().rotation(glm::vec3(0.0f, 1.0f, 0.0f), 180.0f));
    GOBowser.rajouterOBJ();
    GOBowser.setGlobalTransform(tkoopa_model);
    GOBowser.nom="Bowser";
    
    scene->root.addChild(&GObowserStadium);
    GObowserStadium.addChild(&GOMetalMario3);
    GObowserStadium.addChild(&GOkoopa1);
    GObowserStadium.addChild(&GOkoopa2);
    GObowserStadium.addChild(&GOBowser);
    scene->lights.push_back(&light2);
    GOMetalMario3.collisions.push_back(&GObowserStadium);
    GOMetalMario3.collisions.push_back(&GOkoopa1);
    GOMetalMario3.collisions.push_back(&GOkoopa2);
    GOMetalMario3.collisions.push_back(&GOBowser);
    GOkoopa1.collisions.push_back(&GObowserStadium);
    GOkoopa2.collisions.push_back(&GObowserStadium);
    GOBowser.collisions.push_back(&GObowserStadium);
    GOkoopa1.collisions.push_back(&GOMetalMario3);
    GOkoopa2.collisions.push_back(&GOMetalMario3);
    GOBowser.collisions.push_back(&GOMetalMario3);
    GOkoopa1.collisions.push_back(&GOBowser);
    GOkoopa2.collisions.push_back(&GOBowser);
    GOBowser.collisions.push_back(&GOkoopa1);
    GOkoopa1.collisions.push_back(&GOkoopa2);
    GOkoopa2.collisions.push_back(&GOkoopa1);
    GOBowser.collisions.push_back(&GOkoopa2);
    GOMetalMario3.nom="mario";
    GOkoopa1.nom="koopa";
    GOkoopa2.nom="koopa";
    GOBowser.nom="Bowser";
    GObowserStadium.nom="bowser stadium";
    GOMetalMario3.mettreAuSol(&GObowserStadium);
    GOkoopa1.mettreAuSol(&GObowserStadium);
    GOkoopa2.mettreAuSol(&GObowserStadium);
    GOBowser.mettreAuSol(&GObowserStadium);
    GOMetalMario3.addChild(&camera);
    scene->camera = camera;
    scene->camera.lookAt(&GOMetalMario3);
    GOBowser.creerIA();
    GOBowser.boiteEnglobante.setVerticesEspace(GOBowser.globalTransform);
    GOkoopa2.creerIA();
    GOkoopa2.boiteEnglobante.setVerticesEspace(GOkoopa2.globalTransform);
    GOkoopa1.creerIA();
    GOkoopa1.boiteEnglobante.setVerticesEspace(GOkoopa1.globalTransform);
    GOMetalMario3.pv=3;
    GOBowser.pv=3;
    GOkoopa1.pv=1;
    GOkoopa2.pv=1;
    GObowserStadium.stars.push_back(&GOstar);
}

Camera Cswitch(45.0f, float(SCR_WIDTH)/float(SCR_HEIGHT), 0.1f, 1000.0f);
void changerNiveau(){
    if(scene.niveau!=niveau){
        scene.root.enfant.clear();
        scene.lights.clear();
        GOmariometal.axe=glm::vec3(0.0);
        GOMetalMario2.axe=glm::vec3(0.0);
        GOMetalMario3.axe=glm::vec3(0.0);
        camera.setGlobalTransform(Transform());
        camera.speed=glm::vec3(0.0);
        camera.axe=glm::vec3(0.0);
        niveau=scene.niveau;
        if(scene.niveau==1){
            scene.textureSkybox("../textures/ciel.jpg","../textures/ciel.jpg","../textures/ciel.jpg","../textures/ciel.jpg","../textures/ciel.jpg","../textures/ciel.jpg");
            sceneNiveau1(&scene);
        } else if(scene.niveau==2){
            scene.textureSkybox("../textures/2k_moon.jpg","../textures/2k_moon.jpg","../textures/2k_moon.jpg","../textures/2k_moon.jpg","../textures/2k_moon.jpg","../textures/2k_moon.jpg");
            sceneNiveau2(&scene);
        } else if(scene.niveau==3){
            // camera=Cswitch;
            scene.textureSkybox("../textures/univers.jpg","../textures/univers.jpg","../textures/univers.jpg","../textures/univers.jpg","../textures/univers.jpg","../textures/univers.jpg");
            sceneNiveau3(&scene);
        }
    }if(scene.reset){
        scene.reset=false;
        if(niveau==1){
            GOmariometal.setGlobalTransform(GOmariometal.transformSol);
            GOmariometal.auSol=true;
        } else if(niveau==2){
            sceneNiveau2(&scene);
        } else if(niveau==3){
            GOMetalMario3.setGlobalTransform(GOMetalMario3.transformSol);
            GOkoopa1.setGlobalTransform(GOkoopa1.transformSol);
            GOkoopa2.setGlobalTransform(GOkoopa2.transformSol);
            GOBowser.setGlobalTransform(GOBowser.transformSol);
        }
    }
}

std::vector<glm::vec2> verticesCarte{glm::vec2{-0.5f,0.5f},glm::vec2{-0.5f,-0.5f},glm::vec2{0.5f,-0.5f},glm::vec2{0.5f,-0.5f},glm::vec2{0.5f,0.5f},glm::vec2{-0.5f,0.5f}};
std::vector<glm::vec2> uvCarte{glm::vec2{0.0f,1.0f},glm::vec2{0.0f,0.0f},glm::vec2{1.0f,0.0f},glm::vec2{0.0f,0.1f},glm::vec2{1.0f,0.0f},glm::vec2{1.0f,1.0f}};
std::vector<unsigned short> indicesCarte{0,1,2,3,4,5};
GLuint hudVBuffer, hudIBuffer, hudUVBuffer, textureCarte;
void afficherCarte(){
    glGenBuffers(1, &hudVBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, hudVBuffer);
    glBufferData(GL_ARRAY_BUFFER, verticesCarte.size()*sizeof(glm::vec2), &verticesCarte[0], GL_STATIC_DRAW);
    glGenBuffers(1, &hudIBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, hudIBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesCarte), &indicesCarte[0], GL_STATIC_DRAW);
    glGenBuffers(1, &hudUVBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, hudUVBuffer);
    glBufferData(GL_ARRAY_BUFFER, uvCarte.size()*sizeof(glm::vec2), &uvCarte[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, hudVBuffer);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(4);
    glBindBuffer(GL_ARRAY_BUFFER, hudUVBuffer);
    glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, hudIBuffer);
    glUniform1i(glGetUniformLocation(programID, "isHUD"), GL_TRUE);
    glUniform4f(glGetUniformLocation(programID, "hudColor"), 0.0f, 0.0f, 0.0f, 1.0f);
    glDisable(GL_DEPTH_TEST);
    // sendTexture(textureCarte);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)0);
    glEnable(GL_DEPTH_TEST);
}
void sendTexture(GLuint text){
    GLuint Text2DUniformID = glGetUniformLocation(programID, "albedoMap");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,text);
    glUniform1i(Text2DUniformID,0);
}

void clearNiveaux(){
    scene.root.enfant.clear();
    scene.lights.clear();
    GOBattanKing.clearGameObject();
    GOBowser.clearGameObject();
    GOBobombBattlefieldDS.clearGameObject();
    GOGoomba1.clearGameObject();
    GOkoopa1.clearGameObject();
    GOkoopa2.clearGameObject();
    GOMetalMario2.clearGameObject();
    GOMetalMario3.clearGameObject();
    GObowserStadium.clearGameObject();
    GOmariometal.clearGameObject();
    GOPeach.clearGameObject();
    GOchateau.clearGameObject();
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

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    GLuint programID = LoadShaders( "vertex_shader.glsl", "fragment_shader.glsl" );
        
    scene.root.programID=programID;
    scene.programID=programID;
    scene.creerSkybox();
    std::cout<<"Niveau crée"<<std::endl;




    GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

    // For speed computation
    double lastTime = glfwGetTime();
    int nbFrames = 0;

    do{
        changerNiveau();
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
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
        GLuint CamPosUniformID = glGetUniformLocation(programID,"camPos");
        glUniform3f(CamPosUniformID,camera.globalTransform.t[0],camera.globalTransform.t[1],camera.globalTransform.t[2]);
        int cpt = 0;
        for(auto& i : scene.lights){
            GLuint lightPosLoc = glGetUniformLocation(programID, "lightPositions[0]");
            glUniform3f(lightPosLoc, i->globalTransform.t[0], i->globalTransform.t[1], i->globalTransform.t[2]);

            GLuint LightColorUniformID = glGetUniformLocation(programID,"lightColors[0]");
            glUniform3f(LightColorUniformID, i->lightColor[0], i->lightColor[1], i->lightColor[2]);

        }

        glUniform1i(glGetUniformLocation(programID, "isHUD"), GL_FALSE);
        scene.draw(deltaTime);
        if(carte){
            afficherCarte();
        }


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
