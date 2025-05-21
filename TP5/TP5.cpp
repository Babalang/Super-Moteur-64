// Include standard headers
#include <GL/glew.h>
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <unistd.h>
#include <random>
#include <fstream>
#include <string>
#include <regex>

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
#include <TP5/audio.h>
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
bool toggleInputSHIFT = false;
bool activeToogle=false;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;
int nbFrames = 0;
bool StartOftheGame = true;


// Scène 
Scene scene;
Camera camera(45.0f, float(SCR_WIDTH)/float(SCR_HEIGHT), 0.1f, 1000.0f);
int niveau=0;
bool carte=false;
GLuint programID;
bool changementDuNiveau=true;
float vitesse=1.0;

//Animations
std::vector<std::string> animations = {
    "../animations/mario/Idle.dae",
    "../animations/mario/Walking.dae",
    "../animations/mario/Running.dae",
    "../animations/mario/Jump.dae"
};

// Création d'un générateur et d'une distribution
std::random_device rd;
std::mt19937 gen(rd()); // moteur Mersenne Twister
std::uniform_int_distribution<int> dist(0, 3);

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
    glm::mat3 cameraRotation = scene.camera.transform.m; // Rotation actuelle de la caméra
    glm::vec3 cameraTarget = -cameraRotation[2];               // Axe Z local (direction vers l'avant)
    glm::vec3 cameraRight = cameraRotation[0];                 // Axe X local (direction droite)
    glm::vec3 cameraUp = cameraRotation[1];  
    glm::vec3 cameraFront = glm::normalize(cameraPosition - scene.camera.targetPosition);                  // Axe Y local (direction vers le haut)

    if(activeToogle){if(!changementDuNiveau){
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
            scene.camera.orbitalRadius = glm::min(1000.0f, scene.camera.orbitalRadius + cameraZoomSpeed);
            std::cout<<"Camera distance : "<<scene.camera.orbitalRadius<<std::endl;
        }
    }
        if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
            if((!glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)  && (scene.camera.parent->isGround)){scene.camera.parent->setAnimation("../animations/mario/Walking.dae");}
            if(!toggleInputI && !changementDuNiveau){
                toggleInputI = true;
            }else{
                glm::vec3 tmp = glm::normalize(scene.camera.parent->globalTransform.t - scene.camera.globalTransform.t)*vitesse;
                Audio::playAudioOnce("../audios/UI/snd_se_common_Step_Grass.wav",glm::vec3(0.0f));
                tmp.y = 0.0f ;
                scene.camera.parent->frontAxe = tmp ;
                changementDuNiveau=false;
            }
        } 
        if(glfwGetKey(window, GLFW_KEY_W) == GLFW_RELEASE && toggleInputI == true){
            toggleInputI = false;
            scene.camera.parent->frontAxe = glm::vec3(0.0f);
        }
        if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
            if((!glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) && (scene.camera.parent->isGround)){scene.camera.parent->setAnimation("../animations/mario/Walking.dae");}
            if(!toggleInputK && !changementDuNiveau){
                toggleInputK = true;
            }else{
                glm::vec3 tmp = -glm::normalize(scene.camera.parent->globalTransform.t - scene.camera.globalTransform.t)*vitesse;
                Audio::playAudioOnce("../audios/UI/snd_se_common_Step_Grass.wav",glm::vec3(0.0f));
                tmp.y = 0.0f ;
                scene.camera.parent->frontAxe = tmp ;
                changementDuNiveau=false;
            }
        }
        if(glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE && toggleInputK == true){
            toggleInputK = false;
            scene.camera.parent->frontAxe = glm::vec3(0.0f);
        }
        if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
            if((!glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) && (scene.camera.parent->isGround)){scene.camera.parent->setAnimation("../animations/mario/Walking.dae");}
            if(!toggleInputL && !changementDuNiveau){
                toggleInputL = true;
            }else{
                scene.camera.parent->rightAxe = glm::normalize(glm::cross((scene.camera.parent->globalTransform.t - scene.camera.globalTransform.t), scene.camera.parent->upAxe))*vitesse;
                Audio::playAudioOnce("../audios/UI/snd_se_common_Step_Grass.wav",glm::vec3(0.0f));
                changementDuNiveau=false;
            }
        }
        if(glfwGetKey(window, GLFW_KEY_D) == GLFW_RELEASE && toggleInputL == true){
            toggleInputL = false;
            scene.camera.parent->rightAxe = glm::vec3(0.0f,0.0,0.0f);
        }
        if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
            if((!glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) && (scene.camera.parent->isGround)){scene.camera.parent->setAnimation("../animations/mario/Walking.dae");}
            if(!toggleInputJ && !changementDuNiveau){
                toggleInputJ = true;
            }else{
                scene.camera.parent->rightAxe = -glm::normalize(glm::cross((scene.camera.parent->globalTransform.t - scene.camera.globalTransform.t), scene.camera.parent->upAxe))*vitesse;
                Audio::playAudioOnce("../audios/UI/snd_se_common_Step_Grass.wav",glm::vec3(0.0f));
                changementDuNiveau=false;
            }
        }
        if(glfwGetKey(window, GLFW_KEY_A) == GLFW_RELEASE && toggleInputJ == true){
            toggleInputJ = false;
            scene.camera.parent->rightAxe = glm::vec3(0.0f,0.0,0.0f);
        }
        if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){
            if(toggleInputSpace == false){
                toggleInputSpace = true;
                if(scene.camera.parent->isGround){
                    if(scene.camera.parent->speed[1] <= glm::vec3(0.0)[1]) scene.camera.parent->speed = glm::vec3(0.0f,10.0f,0.0f);
                    scene.camera.parent->isGround=false;
                    int r = dist(gen);
                    Audio::stopAudio("../audios/UI/snd_se_common_Step_Grass.wav");
                    if(r==0){
                        Audio::playAudio("../audios/Sauts/jump.wav",glm::vec3(0.0f));
                    } else if (r==1) {
                        Audio::playAudio("../audios/Sauts/jump_2.wav",glm::vec3(0.0f));
                    } else {
                        Audio::playAudio("../audios/Sauts/Yahoo.wav",glm::vec3(0.0f));
                    }
                }
            }
            scene.camera.parent->setAnimation("../animations/mario/Jump.dae");
        }
        if(glfwGetKey(window, GLFW_KEY_TAB) && toggleInputTab == false){
            toggleInputTab = true;
            carte=!carte;
            Audio::playAudio("../audios/menu.wav", glm::vec3(0.0f));
        }
        if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){
            if((scene.camera.parent->isGround)){scene.camera.parent->setAnimation("../animations/mario/Running.dae");}            }
            if(toggleInputSHIFT==false){
            toggleInputSHIFT = true;
            vitesse=3.0;
        }
        if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE && toggleInputSHIFT == true){
            toggleInputSHIFT = false;
            vitesse=1.0;
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
        carte=!carte;
    }
    }else changementDuNiveau=false;}
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void sendTexture(GLuint text){
    GLuint Text2DUniformID = glGetUniformLocation(programID, "hudTexture");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,text);
    glUniform1i(Text2DUniformID,0);
}
GLuint loadTextureHUD(std::string filename){
    GLuint texture;
    int width, height, numComponents;
    stbi_set_flip_vertically_on_load(true);
    unsigned char * data = stbi_load (filename.c_str(),&width,&height,&numComponents,0);
    if(data == NULL){
        std::cout<<"Erreur de chargement de la texture : "<<filename<<std::endl;
        return -1;
    }
    glGenTextures (1, &texture);
    glBindTexture (GL_TEXTURE_2D, texture);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D (GL_TEXTURE_2D,0,(numComponents == 1 ? GL_RED : numComponents == 3 ? GL_RGB : GL_RGBA),width,height,0,(numComponents == 1 ? GL_RED : numComponents == 3 ? GL_RGB : GL_RGBA),GL_UNSIGNED_BYTE,data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    glBindTexture (GL_TEXTURE_2D, 0);    
    return texture;                                                                      
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
    GOmariometal.loadAnimations(animations);

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
    scene->camera.orbitalRadius *= (scene->camera.parent->transform.s/0.02f);
    Audio::switchBackgroundMusic("../audios/SoundTrack/Super Mario 64 - Inside Peachs Castle Music.wav");
}

GameObject GOBobombBattlefieldDS,GOBattanKing,light2,GOGoomba1,GOGoomba2,GOGoomba3,GOGoomba4,GOGoomba5,GOGoomba6,GOGoomba7,GOGoomba8,GOMetalMario2,GOstar,GOcanon,GOchainchomp;
Camera cameraNiv2(45.0f, float(SCR_WIDTH)/float(SCR_HEIGHT), 0.1f, 1000.0f);
void sceneNiveau2(Scene *scene){
    GLuint programID=scene->root.programID;
    
    //Affichage de la Map :
    GOBobombBattlefieldDS.programID=programID;
    GOBobombBattlefieldDS.lireOBJ("../meshes/SM64DS_Model.obj");
    GOBobombBattlefieldDS.rajouterOBJ();
    GOBobombBattlefieldDS.setLocalTransform(Transform(glm::mat3x3(1.0),glm::vec3(0.0,0.0,0.0),20.0));
    
    //Affichage de l'objet :
    // GOMetalMario2.programID=programID;
    // GOMetalMario2.lireOBJ("../meshes/Mario.obj");
    // GOMetalMario2.rajouterOBJ();
    // Transform scale      = Transform().scale(0.1f);
    // Transform translate  = Transform().translation(glm::vec3(0.0f, 1.0f, 0.0f), 5.0f);
    // Transform rotateX    = Transform().rotation(glm::vec3(1.0f, 0.0f, 0.0f), -90.0f);
    // Transform rotateY    = Transform().rotation(glm::vec3(0.0f, 1.0f, 0.0f), 180.0f);
    // Transform finalTransform = translate
    //     .combine_with(rotateY)
    //     .combine_with(rotateX)
    //     .combine_with(scale);
    // GOMetalMario2.setLocalTransform(finalTransform);

    GOMetalMario2.programID=programID;
    GOMetalMario2.lireOBJ("../meshes/Mario.obj");
    std::cout<<"Chargement de l'objet"<<std::endl;
    GOMetalMario2.rajouterOBJ();
    GOMetalMario2.isGround = true;
    Transform scale = Transform().scale(0.020f);
    Transform roll = Transform().rotation(glm::vec3(1.0f, 0.0f, 0.0f), -90.0f);
    Transform yaw = Transform().rotation(glm::vec3(0.0f, 1.0f, 0.0f), 180.0f);
    Transform combined = yaw.combine_with(roll);
    GOMetalMario2.setLocalTransform(Transform(glm::mat3x3(1.0),glm::vec3(0.0,1.0,0.0),1.0).combine_with(scale).combine_with(roll));
    GOMetalMario2.initialTransform = GOMetalMario2.transform;
    Transform translation =Transform(glm::mat3(1.0f), glm::vec3(-132.0,10.0,126.0), 1.0f);//-132.0,10.0,126.0 50.0,90.0,-100.0
    GOMetalMario2.setGlobalTransform(translation);
    GOMetalMario2.nom="mario";
    GOMetalMario2.loadAnimations(animations);

    light2.programID=programID;
    light2.setLODMeshes("../meshes/sphere.off",false, "../textures/s2.ppm");
    light2.setLocalTransform(Transform().scale(5.0f));
    light2.setGlobalTransform(Transform(glm::mat3x3(1.0),glm::vec3(275.0,275.0,275.0),1.0));
    light2.lightIntensity = 100000.0f;
    light2.isLight = true;
    light2.lightColor = glm::vec3(1.0f,1.0f,0.8f)* light2.lightIntensity;

    // GOGoomba1.programID=programID;
    // GOGoomba1.lireOBJ("../meshes/kuribo_model.obj");
    // GOGoomba1.rajouterOBJ();
    // Transform tGoomba1=Transform(glm::mat3x3(1.0),glm::vec3(0.0,5.0,-3.0),1.0);
    // GOGoomba1.setLocalTransform(tGoomba1);
    // GOGoomba1.setGlobalTransform(tGoomba1);

    GOGoomba1.programID=programID;
    GOGoomba1.lireOBJ("../meshes/kuribo_model.obj");
    Transform tgoomba=Transform(glm::mat3x3(1.0),glm::vec3(-90.0,25.0,-11.0),0.1);
    // tkoopa_model=tkoopa_model.combine_with(Transform().rotation(glm::vec3(0.0f, 1.0f, 0.0f), 180.0f));
    GOGoomba1.rajouterOBJ();
    GOGoomba1.setGlobalTransform(tgoomba);
    GOGoomba1.nom="goomba";

    GOGoomba2.programID=programID;
    GOGoomba2.lireOBJ("../meshes/kuribo_model.obj");
    tgoomba=Transform(glm::mat3x3(1.0),glm::vec3(-38.0,10.0,110.0),0.1);
    // tkoopa_model=tkoopa_model.combine_with(Transform().rotation(glm::vec3(0.0f, 1.0f, 0.0f), 180.0f));
    GOGoomba2.rajouterOBJ();
    GOGoomba2.setGlobalTransform(tgoomba);
    GOGoomba2.nom="goomba";

    GOGoomba3.programID=programID;
    GOGoomba3.lireOBJ("../meshes/kuribo_model.obj");
    tgoomba=Transform(glm::mat3x3(1.0),glm::vec3(26.0,20.0,105.0),0.1);
    // tkoopa_model=tkoopa_model.combine_with(Transform().rotation(glm::vec3(0.0f, 1.0f, 0.0f), 180.0f));
    GOGoomba3.rajouterOBJ();
    GOGoomba3.setGlobalTransform(tgoomba);
    GOGoomba3.nom="goomba";

    GOGoomba4.programID=programID;
    GOGoomba4.lireOBJ("../meshes/kuribo_model.obj");
    tgoomba=Transform(glm::mat3x3(1.0),glm::vec3(-42.0,5.0,70.5),0.1);
    // tkoopa_model=tkoopa_model.combine_with(Transform().rotation(glm::vec3(0.0f, 1.0f, 0.0f), 180.0f));
    GOGoomba4.rajouterOBJ();
    GOGoomba4.setGlobalTransform(tgoomba);
    GOGoomba4.nom="goomba";

    GOGoomba5.programID=programID;
    GOGoomba5.lireOBJ("../meshes/kuribo_model.obj");
    tgoomba=Transform(glm::mat3x3(1.0),glm::vec3(-28.0,5.0,68.0),0.1);
    // tkoopa_model=tkoopa_model.combine_with(Transform().rotation(glm::vec3(0.0f, 1.0f, 0.0f), 180.0f));
    GOGoomba5.rajouterOBJ();
    GOGoomba5.setGlobalTransform(tgoomba);
    GOGoomba5.nom="goomba";

    GOGoomba6.programID=programID;
    GOGoomba6.lireOBJ("../meshes/kuribo_model.obj");
    tgoomba=Transform(glm::mat3x3(1.0),glm::vec3(-48.0,5.0,-74.0),0.1);
    // tkoopa_model=tkoopa_model.combine_with(Transform().rotation(glm::vec3(0.0f, 1.0f, 0.0f), 180.0f));
    GOGoomba6.rajouterOBJ();
    GOGoomba6.setGlobalTransform(tgoomba);
    GOGoomba6.nom="goomba";

    GOGoomba7.programID=programID;
    GOGoomba7.lireOBJ("../meshes/kuribo_model.obj");
    tgoomba=Transform(glm::mat3x3(1.0),glm::vec3(-98.0,25.0,-28.0),0.1);
    // tkoopa_model=tkoopa_model.combine_with(Transform().rotation(glm::vec3(0.0f, 1.0f, 0.0f), 180.0f));
    GOGoomba7.rajouterOBJ();
    GOGoomba7.setGlobalTransform(tgoomba);
    GOGoomba7.nom="goomba";

    GOGoomba8.programID=programID;
    GOGoomba8.lireOBJ("../meshes/kuribo_model.obj");
    tgoomba=Transform(glm::mat3x3(1.0),glm::vec3(-110.0,20.0,56.0),0.1);
    // tkoopa_model=tkoopa_model.combine_with(Transform().rotation(glm::vec3(0.0f, 1.0f, 0.0f), 180.0f));
    GOGoomba8.rajouterOBJ();
    GOGoomba8.setGlobalTransform(tgoomba);
    GOGoomba8.nom="goomba";

    GOBattanKing.programID=programID;
    GOBattanKing.lireOBJ("../meshes/battan_king.obj");
    Transform tBattanKing=Transform(glm::mat3x3(1.0),glm::vec3(37.0,90.0,-90.0),0.15);
    GOBattanKing.rajouterOBJ();
    GOBattanKing.setGlobalTransform(tBattanKing);
    GOBattanKing.nom="battan";
    // GOBattanKing.battanPV3=loadTextureHUD()

    GOcanon.programID=programID;
    GOcanon.lireOBJ("../meshes/houdai.obj");
    // GOcanon.setLocalTransform(Transform().rotation(glm::vec3(1.0,0.0,0.0),90));
    Transform thoudai=Transform(glm::mat3x3(1.0),glm::vec3(-111.3,2.0,109.5),0.13);
    thoudai=thoudai.combine_with(Transform().rotation(glm::vec3(1.0,0.0,0.0),90));
    GOcanon.rajouterOBJ();
    GOcanon.setGlobalTransform(thoudai);
    GOcanon.nom="houdai";

    GOchainchomp.programID=programID;
    GOchainchomp.lireOBJ("../meshes/ChainChomp.obj");
    // GOcanon.setLocalTransform(Transform().rotation(glm::vec3(1.0,0.0,0.0),90));
    Transform tchainchomp=Transform(glm::mat3x3(1.0),glm::vec3(13.0,40.0,36.0),0.2);
    tchainchomp=tchainchomp.combine_with(Transform().rotation(glm::vec3(0.0,1.0,0.0),-90));
    GOchainchomp.rajouterOBJ();
    GOchainchomp.setGlobalTransform(tchainchomp);
    GOchainchomp.nom="chainchomp";
    
    scene->root.addChild(&GOBobombBattlefieldDS);
    GOBobombBattlefieldDS.addChild(&GOMetalMario2);
    GOBobombBattlefieldDS.addChild(&GOGoomba1);
    GOBobombBattlefieldDS.addChild(&GOBattanKing);
    GOBobombBattlefieldDS.addChild(&GOGoomba2);
    GOBobombBattlefieldDS.addChild(&GOGoomba3);
    GOBobombBattlefieldDS.addChild(&GOGoomba4);
    GOBobombBattlefieldDS.addChild(&GOGoomba5);
    GOBobombBattlefieldDS.addChild(&GOGoomba6);
    GOBobombBattlefieldDS.addChild(&GOGoomba7);
    GOBobombBattlefieldDS.addChild(&GOGoomba8);
    GOBobombBattlefieldDS.addChild(&GOcanon);
    GOBobombBattlefieldDS.addChild(&GOchainchomp);
    // GOBobombBattlefieldDS.addChild(&GObridge);
    scene->lights.push_back(&light2);
    Audio::switchBackgroundMusic("../audios/SoundTrack/Super Mario 64 - Main Theme Music - Bob-Omb Battlefield.wav");
    GOMetalMario2.collisions.push_back(&GOBobombBattlefieldDS);
    GOMetalMario2.collisions.push_back(&GOGoomba1);
    GOMetalMario2.collisions.push_back(&GOGoomba8);
    GOMetalMario2.collisions.push_back(&GOGoomba7);
    GOMetalMario2.collisions.push_back(&GOGoomba6);
    GOMetalMario2.collisions.push_back(&GOGoomba5);
    GOMetalMario2.collisions.push_back(&GOGoomba4);
    GOMetalMario2.collisions.push_back(&GOGoomba3);
    GOMetalMario2.collisions.push_back(&GOGoomba2);
    GOMetalMario2.collisions.push_back(&GOBattanKing);
    GOMetalMario2.collisions.push_back(&GOcanon);
    GOMetalMario2.collisions.push_back(&GOchainchomp);
    // GOMetalMario2.collisions.push_back(&GObridge);
    GOGoomba1.collisions.push_back(&GOBobombBattlefieldDS);
    GOBattanKing.collisions.push_back(&GOBobombBattlefieldDS);
    GOGoomba1.collisions.push_back(&GOMetalMario2);
    GOBattanKing.collisions.push_back(&GOMetalMario2);
    GOGoomba2.collisions.push_back(&GOBobombBattlefieldDS);
    GOGoomba2.collisions.push_back(&GOMetalMario2);
    GOGoomba3.collisions.push_back(&GOBobombBattlefieldDS);
    GOGoomba3.collisions.push_back(&GOMetalMario2);
    GOGoomba4.collisions.push_back(&GOBobombBattlefieldDS);
    GOGoomba4.collisions.push_back(&GOMetalMario2);
    GOGoomba5.collisions.push_back(&GOBobombBattlefieldDS);
    GOGoomba5.collisions.push_back(&GOMetalMario2);
    GOGoomba6.collisions.push_back(&GOBobombBattlefieldDS);
    GOGoomba6.collisions.push_back(&GOMetalMario2);
    GOGoomba7.collisions.push_back(&GOBobombBattlefieldDS);
    GOGoomba7.collisions.push_back(&GOMetalMario2);
    GOGoomba8.collisions.push_back(&GOBobombBattlefieldDS);
    GOGoomba8.collisions.push_back(&GOMetalMario2);
    GOchainchomp.collisions.push_back(&GOMetalMario2);
    GOchainchomp.collisions.push_back(&GOBobombBattlefieldDS);
    GOMetalMario2.nom="mario";
    GOBobombBattlefieldDS.nom="bobombbattlefield";
    GOBobombBattlefieldDS.map=true;
    GOMetalMario2.mettreAuSol(&GOBobombBattlefieldDS);
    GOGoomba1.mettreAuSol(&GOBobombBattlefieldDS);
    GOBattanKing.mettreAuSol(&GOBobombBattlefieldDS);
    GOGoomba2.mettreAuSol(&GOBobombBattlefieldDS);
    GOGoomba3.mettreAuSol(&GOBobombBattlefieldDS);
    GOGoomba4.mettreAuSol(&GOBobombBattlefieldDS);
    GOGoomba5.mettreAuSol(&GOBobombBattlefieldDS);
    GOGoomba6.mettreAuSol(&GOBobombBattlefieldDS);
    GOGoomba7.mettreAuSol(&GOBobombBattlefieldDS);
    GOGoomba8.mettreAuSol(&GOBobombBattlefieldDS);
    GOchainchomp.mettreAuSol(&GOBobombBattlefieldDS);
    GOMetalMario2.addChild(&cameraNiv2);
    scene->camera = cameraNiv2;
    scene->camera.lookAt(&GOMetalMario2);
    GOGoomba1.creerIA();
    GOGoomba1.boiteEnglobante.setVerticesEspace(GOGoomba1.globalTransform);
    GOBattanKing.creerIA();
    GOBattanKing.boiteEnglobante.setVerticesEspace(GOBattanKing.globalTransform);
    GOGoomba2.creerIA();
    GOGoomba2.boiteEnglobante.setVerticesEspace(GOGoomba1.globalTransform);
    GOGoomba3.creerIA();
    GOGoomba3.boiteEnglobante.setVerticesEspace(GOGoomba1.globalTransform);
    GOGoomba4.creerIA();
    GOGoomba4.boiteEnglobante.setVerticesEspace(GOGoomba1.globalTransform);
    GOGoomba5.creerIA();
    GOGoomba5.boiteEnglobante.setVerticesEspace(GOGoomba1.globalTransform);
    GOGoomba6.creerIA();
    GOGoomba6.boiteEnglobante.setVerticesEspace(GOGoomba1.globalTransform);
    GOGoomba7.creerIA();
    GOGoomba7.boiteEnglobante.setVerticesEspace(GOGoomba1.globalTransform);
    GOGoomba8.creerIA();
    GOGoomba8.boiteEnglobante.setVerticesEspace(GOGoomba1.globalTransform);
    GOMetalMario2.pv=3;
    GOBattanKing.pv=3;
    GOGoomba1.pv=1;
    GOGoomba2.pv=1;
    GOGoomba3.pv=1;
    GOGoomba4.pv=1;
    GOGoomba5.pv=1;
    GOGoomba6.pv=1;
    GOGoomba7.pv=1;
    GOGoomba8.pv=1;
    GOMetalMario2.pv=10;
    GOBobombBattlefieldDS.stars.push_back(&GOstar);
    // scene->camera.orbitalRadius *= (scene->camera.parent->transform.s/0.02f);z
    // scene->camera.orbitalRadius *= (scene->camera.parent->transform.s/0.02f);
}

GameObject GOkoopa1,GObowserStadium,GOMetalMario3,GOkoopa2,GOBowser,light3,GOBowserFireBall;
Camera Cswitch(45.0f, float(SCR_WIDTH)/float(SCR_HEIGHT), 0.1f, 1000.0f);
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
    GOMetalMario3.loadAnimations(animations);

    light3.programID=programID;
    light3.setLODMeshes("../meshes/sphere.off",false, "../textures/s2.ppm");
    light3.setLocalTransform(Transform().scale(5.0f));
    light3.setGlobalTransform(Transform(glm::mat3x3(1.0),glm::vec3(275.0,275.0,275.0),1.0));
    light3.lightIntensity = 100000.0f;
    light3.isLight = true;
    light3.lightColor = glm::vec3(1.0f,1.0f,0.8f)* light3.lightIntensity;


    GameObject a,b;
    a.programID=programID;b.programID=programID;
    a.lireOBJ("../meshes/Koopa.obj");b.lireOBJ("../meshes/Shell.obj");

    GOkoopa1.programID=programID;
    GOkoopa1.lireOBJ("../meshes/Koopa.obj");
    GOkoopa1.rajouterOBJ();
    Transform tKoopa1=Transform(glm::mat3x3(1.0),glm::vec3(-30.0,120.0,30.0),0.8);
    tKoopa1=tKoopa1.combine_with(Transform().rotation(glm::vec3(0.0f, 1.0f, 0.0f), -90.0f));
    GOkoopa1.setGlobalTransform(tKoopa1);
    GOkoopa1.nom="koopa1";
    GOkoopa1.koopaOBJ=a.objetsOBJ;GOkoopa1.shellOBJ=b.objetsOBJ;

    GOkoopa2.programID=programID;
    GOkoopa2.lireOBJ("../meshes/Koopa.obj");
    Transform tKoopa2=Transform(glm::mat3x3(1.0),glm::vec3(30.0,120.0,30.0),0.8);
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
    GOBowser.bowserFireBall.push_back(GOBowserFireBall);
    GOBowser.bowserFireBall[0].programID=programID;
    GOBowser.bowserFireBall[0].mesh.programID=programID;
    GOBowser.bowserFireBall[0].nom="boule de feu";
    GOBowser.bowserFireBall[0].setLODMeshes("../meshes/sphere.off",false, "../textures/s2.ppm");
    GOBowser.bowserFireBall[0].setMesh(GOBowser.bowserFireBall[0].highMesh);
    GOBowser.bowserFireBall[0].calculerBoiteEnglobante();
    GOBowser.bowserFireBall[0].collisions.push_back(&GOMetalMario3);
    GOBowser.bowserFireBall[0].collisions.push_back(&GObowserStadium);
    GOBowser.bowserFireBall[0].setGlobalTransform(Transform(glm::mat3x3(1.0),glm::vec3(0.0),4.0));
    
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
    GOMetalMario3.addChild(&Cswitch);
    scene->camera = Cswitch;
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
    std::cout<<"mario position : "<<GOMetalMario3.centreEspace[0]<<" "<<GOMetalMario3.centreEspace[1]<<" "<<GOMetalMario3.centreEspace[2]<<std::endl;
    scene->camera.orbitalRadius *= (scene->camera.parent->transform.s/0.02f);
    Audio::switchBackgroundMusic("../audios/SoundTrack/Super Mario 64 Soundtrack - Bowser's Theme.wav");
}

void empecherMouvement(){
    toggleInputC = false;
    toggleInput1 = false;
    toggleInput2 = false;
    toggleInput3 = false;
    toggleInput5 = false;
    toggleInput8 = false;
    toggleInputSpace = false;
    toggleInputTab = false;
    toggleInputI = false;
    toggleInputK = false;
    toggleInputL = false;
    toggleInputJ = false;
    toggleInputQ = false;
    toggleInputZ = false;
    activeToogle=false;
    changementDuNiveau=true;
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
    empecherMouvement();
}

void changerNiveau(){
    if(niveau==3){
        for(int i=0;i<GOMetalMario3.collisions.size();i++){
            if(GOMetalMario3.collisions[i]->nom=="star"){
                Audio::playAudioOnce("../audios/UI/Continued.wav",glm::vec3(0.0));
                scene.camera.lookAt(GOMetalMario3.collisions[i]);
            }
        }
    }
    if(scene.niveau!=niveau){
        activeToogle=false;
        clearNiveaux();
        scene.root.enfant.clear();
        scene.lights.clear();
        camera.setGlobalTransform(Transform());
        scene.nouveauNiveau=true;
        // camera.speed=glm::vec3(0.0);
        // camera.axe=glm::vec3(0.0);
        niveau=scene.niveau;
        if(scene.niveau==1){
            scene.textureSkybox("../textures/ciel.jpg","../textures/ciel.jpg","../textures/ciel.jpg","../textures/ciel.jpg","../textures/ciel.jpg","../textures/ciel.jpg");
            sceneNiveau1(&scene);
            GOmariometal.axe=glm::vec3(0.0);
            GOMetalMario2.axe=glm::vec3(0.0);
            GOMetalMario3.axe=glm::vec3(0.0);
            GOmariometal.changementDuNiveau=true;
            GOMetalMario2.changementDuNiveau=true;
            GOMetalMario3.changementDuNiveau=true;
        } else if(scene.niveau==2){
            scene.textureSkybox("../textures/ciel.jpg","../textures/ciel.jpg","../textures/ciel.jpg","../textures/ciel.jpg","../textures/ciel.jpg","../textures/ciel.jpg");
            sceneNiveau2(&scene);
            GOmariometal.axe=glm::vec3(0.0);
            GOMetalMario2.axe=glm::vec3(0.0);
            GOMetalMario3.axe=glm::vec3(0.0);
            GOmariometal.changementDuNiveau=true;
            GOMetalMario2.changementDuNiveau=true;
            GOMetalMario3.changementDuNiveau=true;
        } else if(scene.niveau==3){
            // camera=Cswitch;
            scene.textureSkybox("../textures/univers.jpg","../textures/univers.jpg","../textures/univers.jpg","../textures/univers.jpg","../textures/univers.jpg","../textures/univers.jpg");
            sceneNiveau3(&scene);
            GOmariometal.axe=glm::vec3(0.0);
            GOMetalMario2.axe=glm::vec3(0.0);
            GOMetalMario3.axe=glm::vec3(0.0);
            GOmariometal.changementDuNiveau=true;
            GOMetalMario2.changementDuNiveau=true;
            GOMetalMario3.changementDuNiveau=true;
        }
        toggleInputC = false;
        toggleInput1 = false;
        toggleInput2 = false;
        toggleInput3 = false;
        toggleInput5 = false;
        toggleInput8 = false;
        toggleInputSpace = false;
        toggleInputTab = false;
        toggleInputI = false;
        toggleInputK = false;
        toggleInputL = false;
        toggleInputJ = false;
        toggleInputQ = false;
        toggleInputZ = false;
        activeToogle=true;
    }if(scene.reset){
        scene.nouveauPV=true;
        scene.reset=false;
        if(niveau==1){
            GOmariometal.setGlobalTransform(GOmariometal.transformSol);
            GOmariometal.auSol=true;
        } else if(niveau==2){
            GOGoomba1.setGlobalTransform(GOGoomba1.transformSol);
            GOGoomba2.setGlobalTransform(GOGoomba2.transformSol);
            GOGoomba3.setGlobalTransform(GOGoomba3.transformSol);
            GOGoomba4.setGlobalTransform(GOGoomba4.transformSol);
            GOGoomba5.setGlobalTransform(GOGoomba5.transformSol);
            GOGoomba6.setGlobalTransform(GOGoomba6.transformSol);
            GOGoomba7.setGlobalTransform(GOGoomba7.transformSol);
            GOGoomba8.setGlobalTransform(GOGoomba8.transformSol);
            GOBattanKing.setGlobalTransform(GOBattanKing.transformSol);
            GOMetalMario2.setGlobalTransform(GOMetalMario2.transformSol);
            GOGoomba1.pv=1;
            GOGoomba2.pv=1;
            GOGoomba3.pv=1;
            GOGoomba4.pv=1;
            GOGoomba5.pv=1;
            GOGoomba6.pv=1;
            GOGoomba7.pv=1;
            GOGoomba8.pv=1;
            GOBattanKing.pv=3;
            GOMetalMario2.pv=3;
        } else if(niveau==3){
            GOMetalMario3.setGlobalTransform(GOMetalMario3.transformSol);
            GOkoopa1.clearGameObject();
            GOkoopa1.nom="koopa";
            GOkoopa1.isIA=true;
            GOkoopa1.lireOBJ("../meshes/Koopa.obj");
            GOkoopa1.rajouterOBJ();
            GOkoopa1.setGlobalTransform(GOkoopa1.transformSol);
            GOkoopa1.creerIA();
            GOkoopa1.carapaceRespawn=false;
            GOkoopa1.nbCollision=-1;
            GOkoopa2.clearGameObject();
            GOkoopa2.nom="koopa";
            GOkoopa2.isIA=true;
            GOkoopa2.lireOBJ("../meshes/Koopa.obj");
            GOkoopa2.rajouterOBJ();
            GOkoopa2.setGlobalTransform(GOkoopa2.transformSol);
            GOkoopa2.creerIA();
            GOkoopa2.carapaceRespawn=false;
            GOkoopa2.nbCollision=-1;
            GOBowser.setGlobalTransform(GOBowser.transformSol);
            GOMetalMario3.pv=3;
            GOkoopa1.pv=1;
            GOkoopa2.pv=1;
            GOBowser.pv=3;
        }
    }
}


std::vector<glm::vec2> verticesCarte{glm::vec2{-0.5f,0.5f},glm::vec2{-0.5f,-0.5f},glm::vec2{0.5f,-0.5f},glm::vec2{0.5f,-0.5f},glm::vec2{0.5f,0.5f},glm::vec2{-0.5f,0.5f}};
std::vector<glm::vec2> uvCarte{glm::vec2{0.0f,0.0f},glm::vec2{0.0f,1.0f},glm::vec2{1.0f,1.0f},glm::vec2{1.0f,1.0f},glm::vec2{1.0f,0.0f},glm::vec2{0.0f,0.0f}};
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
    glDisable(GL_DEPTH_TEST);
    sendTexture(textureCarte);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)0);
    glEnable(GL_DEPTH_TEST);
}
std::vector<glm::vec2> verticesPV{glm::vec2{-0.9f,0.9f},glm::vec2{-0.9f,0.7f},glm::vec2{-0.6f,0.7f},glm::vec2{-0.6f,0.7f},glm::vec2{-0.6f,0.9f},glm::vec2{-0.9f,0.9f}};
std::vector<glm::vec2> uvPV{glm::vec2{0.0f,0.0f},glm::vec2{0.0f,1.0f},glm::vec2{1.0f,1.0f},glm::vec2{1.0f,1.0f},glm::vec2{1.0f,0.0f},glm::vec2{0.0f,0.0f}};
std::vector<unsigned short> indicesPV{0,1,2,3,4,5};
GLuint PVVBuffer, PVIBuffer, PVUVBuffer, texturePV;
void afficherPV(){
    glGenBuffers(1, &PVVBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, PVVBuffer);
    glBufferData(GL_ARRAY_BUFFER, verticesPV.size()*sizeof(glm::vec2), &verticesPV[0], GL_STATIC_DRAW);
    glGenBuffers(1, &PVIBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, PVIBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesPV), &indicesPV[0], GL_STATIC_DRAW);
    glGenBuffers(1, &PVUVBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, PVUVBuffer);
    glBufferData(GL_ARRAY_BUFFER, uvPV.size()*sizeof(glm::vec2), &uvPV[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, PVVBuffer);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(4);
    glBindBuffer(GL_ARRAY_BUFFER, PVUVBuffer);
    glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, PVIBuffer);
    glDisable(GL_DEPTH_TEST);
    sendTexture(texturePV);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)0);
    glEnable(GL_DEPTH_TEST);
}
std::vector<glm::vec2> verticesStar{glm::vec2{0.6f,0.9f},glm::vec2{0.6f,0.7f},glm::vec2{0.9f,0.7f},glm::vec2{0.9f,0.7f},glm::vec2{0.9f,0.9f},glm::vec2{0.6f,0.9f}};
std::vector<glm::vec2> uvStar{glm::vec2{0.0f,0.0f},glm::vec2{0.0f,1.0f},glm::vec2{1.0f,1.0f},glm::vec2{1.0f,1.0f},glm::vec2{1.0f,0.0f},glm::vec2{0.0f,0.0f}};
std::vector<unsigned short> indicesStar{0,1,2,3,4,5};
GLuint StarVBuffer, StarIBuffer, StarUVBuffer, textureStar;
void afficherStar(){
    glGenBuffers(1, &StarVBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, StarVBuffer);
    glBufferData(GL_ARRAY_BUFFER, verticesStar.size()*sizeof(glm::vec2), &verticesStar[0], GL_STATIC_DRAW);
    glGenBuffers(1, &StarIBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, StarIBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesStar), &indicesStar[0], GL_STATIC_DRAW);
    glGenBuffers(1, &StarUVBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, StarUVBuffer);
    glBufferData(GL_ARRAY_BUFFER, uvStar.size()*sizeof(glm::vec2), &uvStar[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, StarVBuffer);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(4);
    glBindBuffer(GL_ARRAY_BUFFER, StarUVBuffer);
    glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, StarIBuffer);
    glDisable(GL_DEPTH_TEST);
    sendTexture(textureStar);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)0);
    glEnable(GL_DEPTH_TEST);
}
GLuint VBuffer, IBuffer, UVBuffer;
void afficherFond(std::vector<glm::vec2> vertices,std::vector<glm::vec2> uv,std::vector<unsigned short> indices){
    glGenBuffers(1, &VBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, VBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(glm::vec2), &vertices[0], GL_STATIC_DRAW);
    glGenBuffers(1, &IBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices[0], GL_STATIC_DRAW);
    glGenBuffers(1, &UVBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, UVBuffer);
    glBufferData(GL_ARRAY_BUFFER, uv.size()*sizeof(glm::vec2), &uv[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, VBuffer);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(4);
    glBindBuffer(GL_ARRAY_BUFFER, UVBuffer);
    glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBuffer);
    glDisable(GL_DEPTH_TEST);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)0);
    glEnable(GL_DEPTH_TEST);
}
GLuint Text2DTextureID;
GLuint Text2DVertexBufferID;
GLuint Text2DUVBufferID;
GLuint Text2DShaderID;
GLuint Text2DUniformID;
void initText2Da(const char * texturePath){
	GLuint texture;
    int width, height, numComponents;
    unsigned char * data = stbi_load (texturePath,&width,&height,&numComponents,0);
    if(data == NULL){
        std::cout<<"Erreur de chargement de la texture : "<<texturePath<<std::endl;
        return ;
    }
    glGenTextures (1, &Text2DTextureID);
    glBindTexture (GL_TEXTURE_2D, Text2DTextureID);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D (GL_TEXTURE_2D,0,(numComponents == 1 ? GL_RED : numComponents == 3 ? GL_RGB : GL_RGBA),width,height,0,(numComponents == 1 ? GL_RED : numComponents == 3 ? GL_RGB : GL_RGBA),GL_UNSIGNED_BYTE,data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    glBindTexture (GL_TEXTURE_2D, 0);    
	glGenBuffers(1, &Text2DVertexBufferID);
	glGenBuffers(1, &Text2DUVBufferID);
	Text2DUniformID = glGetUniformLocation( programID, "text" );
}

void printText2Da(const char * text, int x, int y, int size){
	unsigned int length = strlen(text);
	std::vector<glm::vec2> vertices;
	std::vector<glm::vec2> UVs;
	for ( unsigned int i=0 ; i<length ; i++ ){
		glm::vec2 vertex_up_left    = glm::vec2( x+i*size     , y+size );
		glm::vec2 vertex_up_right   = glm::vec2( x+i*size+size, y+size );
		glm::vec2 vertex_down_right = glm::vec2( x+i*size+size, y      );
		glm::vec2 vertex_down_left  = glm::vec2( x+i*size     , y      );
		vertices.push_back(vertex_up_left   );
		vertices.push_back(vertex_down_left );
		vertices.push_back(vertex_up_right  );
		vertices.push_back(vertex_down_right);
		vertices.push_back(vertex_up_right);
		vertices.push_back(vertex_down_left);
		char character = text[i];
		float uv_x = (character%16)/16.0f;
		float uv_y = (character/16)/16.0f;
		glm::vec2 uv_up_left    = glm::vec2( uv_x           , uv_y );
		glm::vec2 uv_up_right   = glm::vec2( uv_x+1.0f/16.0f, uv_y );
		glm::vec2 uv_down_right = glm::vec2( uv_x+1.0f/16.0f, (uv_y + 1.0f/16.0f) );
		glm::vec2 uv_down_left  = glm::vec2( uv_x           , (uv_y + 1.0f/16.0f) );
		UVs.push_back(uv_up_left   );
		UVs.push_back(uv_down_left );
		UVs.push_back(uv_up_right  );
		UVs.push_back(uv_down_right);
		UVs.push_back(uv_up_right);
		UVs.push_back(uv_down_left);
	}
	glBindBuffer(GL_ARRAY_BUFFER, Text2DVertexBufferID);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec2), &vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, Text2DUVBufferID);
	glBufferData(GL_ARRAY_BUFFER, UVs.size() * sizeof(glm::vec2), &UVs[0], GL_STATIC_DRAW);
	glUseProgram(programID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Text2DTextureID);
	glUniform1i(Text2DUniformID, 0);
	glEnableVertexAttribArray(5);
	glBindBuffer(GL_ARRAY_BUFFER, Text2DVertexBufferID);
	glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, 0, (void*)0 );
	glEnableVertexAttribArray(6);
	glBindBuffer(GL_ARRAY_BUFFER, Text2DUVBufferID);
	glVertexAttribPointer(6, 2, GL_FLOAT, GL_FALSE, 0, (void*)0 );
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDrawArrays(GL_TRIANGLES, 0, vertices.size() );
	glDisable(GL_BLEND);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, StarIBuffer);
    // glDisable(GL_DEPTH_TEST);
    // glDrawElements(GL_TRIANGLES, vertices.size(), GL_UNSIGNED_SHORT, (void*)0);
    // glEnable(GL_DEPTH_TEST);
	glDisableVertexAttribArray(5);
	glDisableVertexAttribArray(6);
}

void cleanupText2Da(){
	glDeleteBuffers(1, &Text2DVertexBufferID);
	glDeleteBuffers(1, &Text2DUVBufferID);
	glDeleteTextures(1, &Text2DTextureID);
	// glDeleteProgram(Text2DShaderID);
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

    Audio::init();


    GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

    // For speed computation
    double lastTime = glfwGetTime();
    int nbFrames = 0;

    std::cout<<"iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"<<std::endl;
    texturePV=loadTextureHUD("../textures/mario3.png");
    textureCarte=loadTextureHUD("../textures/star0.png");
    textureStar=loadTextureHUD("../textures/star0.png");
    initText2Da("../textures/white.png");
    scene.nbPV=3;
    GOMetalMario3.pv=3;

    do{
        changerNiveau();
        if(scene.nouvelleEtoile){
            if(scene.nbEtoiles==1){
                textureStar=loadTextureHUD("../textures/star1.png");
            }if(scene.nbEtoiles==2){
                textureStar=loadTextureHUD("../textures/star2.png");
            }if(scene.nbEtoiles==3){
                textureStar=loadTextureHUD("../textures/star3.png");
            }
            scene.nouvelleEtoile=false;
        }if(scene.nouveauPV){
            if(scene.nbPV==1){
                texturePV=loadTextureHUD("../textures/mario1.png");
            }if(scene.nbPV==2){
                texturePV=loadTextureHUD("../textures/mario2.png");
            }if(scene.nbPV==3){
                texturePV=loadTextureHUD("../textures/mario3.png");
            }
            scene.nouveauPV=false;
        }if(scene.nouveauNiveau){
            if(scene.niveau==1){
                textureCarte=loadTextureHUD("../textures/5EDC83BD_c.png");
            }if(scene.niveau==2){
                textureCarte=loadTextureHUD("../textures/5EDC83BD_c.png");
            }if(scene.niveau==3){
                textureCarte=loadTextureHUD("../textures/5EDC83BD_c.png");
            }
            scene.nouveauNiveau=false;
        }
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
        
        glUniform1i(glGetUniformLocation(programID, "isText"), GL_FALSE);
        glUniform1i(glGetUniformLocation(programID, "isHUD"), GL_FALSE);
        scene.draw(deltaTime);
        glUniform1i(glGetUniformLocation(programID, "isHUD"), GL_TRUE);
        // glUniform4f(glGetUniformLocation(programID, "hudColor"), 0.0f, 0.0f, 0.0f, 1.0f);
        glUniform1i(glGetUniformLocation(programID, "isFond"), GL_TRUE);
        afficherFond(verticesPV,uvPV,indicesPV);
        afficherFond(verticesStar,uvStar,indicesStar);
        glUniform1i(glGetUniformLocation(programID, "isFond"), GL_FALSE);
        afficherPV();
        afficherStar();
        if(carte){
            glUniform1i(glGetUniformLocation(programID, "isFond"), GL_TRUE);
            afficherFond(verticesCarte,uvCarte,indicesCarte);
            glUniform1i(glGetUniformLocation(programID, "isFond"), GL_FALSE);
            afficherCarte();
        }
        // glUniform1i(glGetUniformLocation(programID, "isHUD"), GL_FALSE);
        // glUniform1i(glGetUniformLocation(programID, "isText"), GL_TRUE);
        // printText2Da("caca",100,100,30);

        Audio::update();
        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0 );

    glDeleteProgram(programID);
    glDeleteVertexArrays(1, &VertexArrayID);
    Audio::cleanup();
    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}
