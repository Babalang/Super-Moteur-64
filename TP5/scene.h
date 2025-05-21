#ifndef SCENE_H
#define SCENE_H

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

#include <TP5/camera.h>
#include <TP5/audio.h>

class Scene{
    public : 
    GameObject root;
    Camera camera;
    GLuint programID;
    int niveau=1;
    bool reset=false;
    Mesh skyboxplafond;
    Mesh skyboxsol;
    Mesh skyboxmur1;
    Mesh skyboxmur2;
    Mesh skyboxmur3;
    Mesh skyboxmur4;
    bool niveau2=false;
    bool niveau3=false;
    int nbEtoiles=0;
    int nbPV=3;
    bool nouvelleEtoile=false;
    bool nouveauPV=false;
    bool nouveauNiveau=false;
    std::vector<GameObject*> lights;
        Scene(){}

    void animation(float deltaTime){

        camera.animation(deltaTime);
        this->camera.parent->Move(deltaTime);

    }

    void draw(float elapsedTime){
        root.nom="root";
        GameObject* Map = this->root.enfant[0];
        GameObject* Obj = Map->enfant[Map->objetsOBJ.size()];
        for(auto& i : this->lights){ 
            i->draw(camera.globalTransform.t, elapsedTime);
        }
        this->animation(elapsedTime);
        this->sautIA(Obj);
        this->root.testIA(Obj,elapsedTime);
        this->drawSkybox();
        this->root.draw(camera.globalTransform.t, elapsedTime);
        this->testChangementNiveau(Map,Obj);
    }

    void testChangementNiveau(GameObject* map, GameObject* obj){
        if(obj->pv!=this->nbPV){
            this->nbPV=obj->pv;
            this->nouveauPV=true;
        }
        if(obj->pv==0 || obj->basEspace[1]<=-30.0f || obj->collisionChateau=="eau"){
            this->reset=true;
            if(obj->collisionChateau=="eau"){
                Audio::playAudioOnce("../audios/UI/Drowning.wav",glm::vec3(0.0f));
            } else if(obj->basEspace[1]<=30.0f){
                Audio::playAudioOnce("../audios/UI/Wahhh.wav",glm::vec3(0.0f));
            } else {
                Audio::playAudioOnce("../audios/UI/Ooph.wav",glm::vec3(0.0f));
            }
        }
        if(niveau==1){
            float minX=std::numeric_limits<float>::max(),minY=std::numeric_limits<float>::max(),minZ=std::numeric_limits<float>::max(),maxX=0.0,maxY=0.0,maxZ=0.0;
            for(int i=0;i<map->objetsOBJ.size();i++){
                if(map->objetsOBJ[i].nom=="9"){
                    for(int j=0;j<map->objetsOBJ[i].mesh.vertices_Espace.size();j++){
                        if(map->objetsOBJ[i].mesh.vertices_Espace[j][0]<minX) minX=map->objetsOBJ[i].mesh.vertices_Espace[j][0];
                        if(map->objetsOBJ[i].mesh.vertices_Espace[j][1]<minY) minY=map->objetsOBJ[i].mesh.vertices_Espace[j][1];
                        if(map->objetsOBJ[i].mesh.vertices_Espace[j][2]<minZ) minZ=map->objetsOBJ[i].mesh.vertices_Espace[j][2];
                        if(map->objetsOBJ[i].mesh.vertices_Espace[j][0]>maxX) maxX=map->objetsOBJ[i].mesh.vertices_Espace[j][0];
                        if(map->objetsOBJ[i].mesh.vertices_Espace[j][1]>maxY) maxY=map->objetsOBJ[i].mesh.vertices_Espace[j][1];
                        if(map->objetsOBJ[i].mesh.vertices_Espace[j][2]>maxZ) maxZ=map->objetsOBJ[i].mesh.vertices_Espace[j][2];
                    }
                }
            }
            if(obj->centreEspace[0]>=minX && obj->centreEspace[0]<=maxX && obj->centreEspace[2]>=minZ && obj->centreEspace[2]<=maxZ){
                Audio::playAudioOnce("../audios/UI/Here we go.wav",glm::vec3(0.0f));
                if(niveau2){
                    niveau=3;
                }else if(niveau3){
                    niveau=2;
                }else{
                    niveau=2;
                }
            }
        }
        if(niveau==2){
            // for(int i=0;i<obj->collisions.size();i++){
            //     if(obj->collisions[i]->nom=="star"){
            //         Audio::playAudioOnce("../audios/UI/Continued.wav",glm::vec3(0.0));
            //         camera.lookAt(obj->collisions[i]);
            //     }
            // }
            if(obj->nbCollision>0){
                if(obj->collisions[obj->nbCollision]->nom=="star"){
                    Audio::playAudioOnce("../audios/UI/Completion.wav",glm::vec3(0.0f));
                    niveau=1;
                    this->niveau2=true;
                    nbEtoiles++;nouvelleEtoile=true;
                }
            }
        }
        if(niveau==3){
            // for(int i=0;i<obj->collisions.size();i++){
            //     if(obj->collisions[i]->nom=="star"){
            //         Audio::playAudioOnce("../audios/UI/Continued.wav",glm::vec3(0.0));
            //         camera.lookAt(obj->collisions[i]);
            //     }
            // }
            if(obj->nbCollision>0){
                if(obj->collisions[obj->nbCollision]->nom=="star"){
                    Audio::playAudioOnce("../audios/UI/Completion.wav",glm::vec3(0.0f));
                    niveau=1;
                    this->niveau3=true;
                    nbEtoiles++;nouvelleEtoile=true;
                }
            }
        }
    }

    void creerSkybox(){
        this->skyboxplafond.indexed_vertices.push_back(glm::vec3(300.0f, 300.0f, -300.0f));
        this->skyboxplafond.indexed_vertices.push_back(glm::vec3(-300.0f, 300.0f, -300.0f));
        this->skyboxplafond.indexed_vertices.push_back(glm::vec3(-300.0f, 300.0f, 300.0f));
        this->skyboxplafond.indexed_vertices.push_back(glm::vec3(-300.0f, 300.0f, 300.0f));
        this->skyboxplafond.indexed_vertices.push_back(glm::vec3(300.0f, 300.0f, 300.0f));
        this->skyboxplafond.indexed_vertices.push_back(glm::vec3(300.0f, 300.0f, -300.0f));
        this->skyboxplafond.vertices_Espace.push_back(glm::vec3(300.0f, 300.0f, -300.0f));
        this->skyboxplafond.vertices_Espace.push_back(glm::vec3(-300.0f, 300.0f, -300.0f));
        this->skyboxplafond.vertices_Espace.push_back(glm::vec3(-300.0f, 300.0f, 300.0f));
        this->skyboxplafond.vertices_Espace.push_back(glm::vec3(-300.0f, 300.0f, 300.0f));
        this->skyboxplafond.vertices_Espace.push_back(glm::vec3(300.0f, 300.0f, 300.0f));
        this->skyboxplafond.vertices_Espace.push_back(glm::vec3(300.0f, 300.0f, -300.0f));
        this->skyboxplafond.normal.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        this->skyboxplafond.normal.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        this->skyboxplafond.normal.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        this->skyboxplafond.normal.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        this->skyboxplafond.normal.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        this->skyboxplafond.normal.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        this->skyboxplafond.texCoords.push_back(glm::vec2(1.0f, 1.0f));
        this->skyboxplafond.texCoords.push_back(glm::vec2(0.0f, 1.0f));
        this->skyboxplafond.texCoords.push_back(glm::vec2(0.0f, 0.0f));
        this->skyboxplafond.texCoords.push_back(glm::vec2(0.0f, 0.0f));
        this->skyboxplafond.texCoords.push_back(glm::vec2(1.0f, 0.0f));
        this->skyboxplafond.texCoords.push_back(glm::vec2(1.0f, 1.0f));
        this->skyboxplafond.indices.push_back(0);
        this->skyboxplafond.indices.push_back(1);
        this->skyboxplafond.indices.push_back(2);
        this->skyboxplafond.indices.push_back(3);
        this->skyboxplafond.indices.push_back(4);
        this->skyboxplafond.indices.push_back(5);
        std::vector<unsigned short> a={0,1,2},b={3,4,5};
        this->skyboxplafond.triangles.push_back(a);
        this->skyboxplafond.triangles.push_back(b);
        this->skyboxplafond.programID=this->root.programID;
        
        this->skyboxsol.vertices_Espace.push_back(glm::vec3(300.0f, -300.0f, -300.0f));
        this->skyboxsol.vertices_Espace.push_back(glm::vec3(-300.0f, -300.0f, -300.0f));
        this->skyboxsol.vertices_Espace.push_back(glm::vec3(-300.0f, -300.0f, 300.0f));
        this->skyboxsol.vertices_Espace.push_back(glm::vec3(-300.0f, -300.0f, 300.0f));
        this->skyboxsol.vertices_Espace.push_back(glm::vec3(300.0f, -300.0f, 300.0f));
        this->skyboxsol.vertices_Espace.push_back(glm::vec3(300.0f, -300.0f, -300.0f));
        this->skyboxsol.texCoords={
            glm::vec2(1.0f, 1.0f),
            glm::vec2(0.0f, 1.0f),
            glm::vec2(0.0f, 0.0f),
            glm::vec2(0.0f, 0.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(1.0f, 1.0f)
        };

        this->skyboxsol.normal.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        this->skyboxsol.normal.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        this->skyboxsol.normal.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        this->skyboxsol.normal.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        this->skyboxsol.normal.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        this->skyboxsol.normal.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        this->skyboxsol.indices={0,1,2,3,4,5};
        this->skyboxsol.triangles={{0,1,2},{3,4,5}};
        this->skyboxsol.programID=this->root.programID;
        
        this->skyboxmur1.vertices_Espace={
            glm::vec3(300.0f, -300.0f, -300.0f),
            glm::vec3(-300.0f, -300.0f, -300.0f),
            glm::vec3(-300.0f, 300.0f, -300.0f),
            glm::vec3(-300.0f, 300.0f, -300.0f),
            glm::vec3(300.0f, 300.0f, -300.0f),
            glm::vec3(300.0f, -300.0f, -300.0f)
        };
        this->skyboxmur1.texCoords={
            glm::vec2(1.0f, 1.0f),
            glm::vec2(0.0f, 1.0f),
            glm::vec2(0.0f, 0.0f),
            glm::vec2(0.0f, 0.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(1.0f, 1.0f)
        };

        this->skyboxmur1.normal.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        this->skyboxmur1.normal.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        this->skyboxmur1.normal.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        this->skyboxmur1.normal.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        this->skyboxmur1.normal.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        this->skyboxmur1.normal.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        this->skyboxmur1.indices={0,1,2,3,4,5};
        this->skyboxmur1.triangles={{0,1,2},{3,4,5}};
        this->skyboxmur1.programID=this->root.programID;
        
        this->skyboxmur2.vertices_Espace={
            glm::vec3(300.0f, -300.0f, 300.0f),
            glm::vec3(300.0f, -300.0f, -300.0f),
            glm::vec3(300.0f, 300.0f, -300.0f),
            glm::vec3(300.0f, 300.0f, -300.0f),
            glm::vec3(300.0f, 300.0f, 300.0f),
            glm::vec3(300.0f, -300.0f, 300.0f)
        };
        this->skyboxmur2.texCoords={
            glm::vec2(1.0f, 1.0f),
            glm::vec2(0.0f, 1.0f),
            glm::vec2(0.0f, 0.0f),
            glm::vec2(0.0f, 0.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(1.0f, 1.0f)
        };

        this->skyboxmur2.normal.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        this->skyboxmur2.normal.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        this->skyboxmur2.normal.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        this->skyboxmur2.normal.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        this->skyboxmur2.normal.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        this->skyboxmur2.normal.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        this->skyboxmur2.indices={0,1,2,3,4,5};
        this->skyboxmur2.triangles={{0,1,2},{3,4,5}};
        this->skyboxmur2.programID=this->root.programID;
        
        this->skyboxmur3.vertices_Espace={
            glm::vec3(-300.0f, -300.0f, 300.0f),
            glm::vec3(300.0f, -300.0f, 300.0f),
            glm::vec3(300.0f, 300.0f, 300.0f),
            glm::vec3(300.0f, 300.0f, 300.0f),
            glm::vec3(-300.0f, 300.0f, 300.0f),
            glm::vec3(-300.0f, -300.0f, 300.0f)
        };
        this->skyboxmur3.texCoords={
            glm::vec2(1.0f, 1.0f),
            glm::vec2(0.0f, 1.0f),
            glm::vec2(0.0f, 0.0f),
            glm::vec2(0.0f, 0.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(1.0f, 1.0f)
        };

        this->skyboxmur3.normal.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        this->skyboxmur3.normal.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        this->skyboxmur3.normal.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        this->skyboxmur3.normal.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        this->skyboxmur3.normal.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        this->skyboxmur3.normal.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        this->skyboxmur3.indices={0,1,2,3,4,5};
        this->skyboxmur3.triangles={{0,1,2},{3,4,5}};
        this->skyboxmur3.programID=this->root.programID;
        
        this->skyboxmur4.vertices_Espace={
            glm::vec3(-300.0f, -300.0f, -300.0f),
            glm::vec3(-300.0f, -300.0f, 300.0f),
            glm::vec3(-300.0f, 300.0f, 300.0f),
            glm::vec3(-300.0f, 300.0f, 300.0f),
            glm::vec3(-300.0f, 300.0f, -300.0f),
            glm::vec3(-300.0f, -300.0f, -300.0f)
        };
        this->skyboxmur4.texCoords={
            glm::vec2(1.0f, 1.0f),
            glm::vec2(0.0f, 1.0f),
            glm::vec2(0.0f, 0.0f),
            glm::vec2(0.0f, 0.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(1.0f, 1.0f)
        };
        this->skyboxmur4.normal.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        this->skyboxmur4.normal.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        this->skyboxmur4.normal.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        this->skyboxmur4.normal.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        this->skyboxmur4.normal.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        this->skyboxmur4.normal.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        this->skyboxmur4.indices={0,1,2,3,4,5};
        this->skyboxmur4.triangles={{0,1,2},{3,4,5}};
        this->skyboxmur4.programID=this->root.programID;
    }

    void textureSkybox(std::string f1,std::string f2,std::string f3,std::string f4,std::string f5,std::string f6){
        this->skyboxplafond.filename=f1;
        this->skyboxplafond.loadTexture();
        this->skyboxsol.filename=f2;
        this->skyboxsol.loadTexture();
        this->skyboxmur1.filename=f3;
        this->skyboxmur1.loadTexture();
        this->skyboxmur2.filename=f4;
        this->skyboxmur2.loadTexture();
        this->skyboxmur3.filename=f5;
        this->skyboxmur3.loadTexture();
        this->skyboxmur4.filename=f6;
        this->skyboxmur4.loadTexture();
    }

    void drawSkybox(){
        this->skyboxplafond.draw();
        this->skyboxsol.draw();
        this->skyboxmur1.draw();
        this->skyboxmur2.draw();
        this->skyboxmur3.draw();
        this->skyboxmur4.draw();
    }

    void sautIA(GameObject* obj){
        if(obj->nbCollision>0 && difftime(time(NULL),obj->timerSautMario)>0){
            obj->globalTransform.t[1]+=0.1;
            obj->setGlobalTransform(obj->globalTransform);
            if(obj->collisions[obj->nbCollision]->nom=="shell" && obj->collisions[obj->nbCollision]->pv==0){
                Audio::playAudioOnce("../audios/UI/move shell.wav",glm::vec3(0.0f));
                obj->collisions[obj->nbCollision]->pv=1;
                obj->collisions[obj->nbCollision]->nbCollision=-1;
            }else{
                if(obj->collisions[obj->nbCollision]->nom=="koopa" || obj->collisions[obj->nbCollision]->nom=="shell"){
                    obj->collisions[obj->nbCollision]->pv-=1;
                    Audio::playAudioOnce("../audios/UI/hit koopa.wav",glm::vec3(0.0f));
                }else if(obj->collisions[obj->nbCollision]->nom=="goomba"){
                    obj->collisions[obj->nbCollision]->pv-=1;
                    Audio::playAudioOnce("../audios/UI/hit koopa.wav",glm::vec3(0.0f));
                }else if(obj->collisions[obj->nbCollision]->nom=="battan" && obj->collisions[obj->nbCollision]->rotationner && !obj->collisions[obj->nbCollision]->pvBattan){
                    obj->collisions[obj->nbCollision]->pv-=1;
                    obj->collisions[obj->nbCollision]->pvBattan=true;
                    obj->collisions[obj->nbCollision]->timerIA=time(nullptr)-4;
                }
            }
            this->camera.parent->speed = glm::vec3(0.0f,10.0f,0.0f);
            obj->timerSautMario=time(nullptr);
        }
    }
};
#endif
