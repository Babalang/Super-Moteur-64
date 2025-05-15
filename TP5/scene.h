// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

#include <TP5/camera.h>

class Scene{
    public : 
    GameObject root;
    Camera camera;
    GLuint programID;
    int niveau=3;
    bool reset=false;
    Mesh skyboxplafond;
    Mesh skyboxsol;
    Mesh skyboxmur1;
    Mesh skyboxmur2;
    Mesh skyboxmur3;
    Mesh skyboxmur4;
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
        if(obj->pv=0 || obj->basEspace[1]<=-30.0f || obj->collisionChateau=="eau"){
            this->reset=true;
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
                niveau=3;
            }
        }
        if(niveau==3){
            for(int i=0;i<obj->collisions.size();i++){
                if(obj->collisions[i]->nom=="star"){
                    camera.lookAt(obj->collisions[i]);
                }
            }
            if(obj->nbCollision>0){
                if(obj->collisions[obj->nbCollision]->nom=="star"){
                    niveau=1;
                }
            }
        }
    }

    void creerSkybox(){
        this->skyboxplafond.vertices_Espace={
            glm::vec3(-300.0f, 300.0f, -300.0f),
            glm::vec3(300.0f, 300.0f, -300.0f),
            glm::vec3(300.0f, 300.0f, 300.0f),
            glm::vec3(-300.0f, 300.0f, 300.0f)
        };
        this->skyboxplafond.texCoords={
            glm::vec2(0.0f, 1.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(0.0f, 0.0f)
        };
        this->skyboxplafond.indices={1,0,3,3,2,1};
        this->skyboxplafond.triangles={{1,0,3},{3,2,1}};
        this->skyboxplafond.programID=this->root.programID;
        
        this->skyboxsol.vertices_Espace={
            glm::vec3(-300.0f, -300.0f, -300.0f),
            glm::vec3(300.0f, -300.0f, -300.0f),
            glm::vec3(300.0f, -300.0f, 300.0f),
            glm::vec3(-300.0f, -300.0f, 300.0f)
        };
        this->skyboxsol.texCoords={
            glm::vec2(0.0f, 1.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(0.0f, 0.0f)
        };
        this->skyboxsol.indices={1,0,3,3,2,1};
        this->skyboxsol.triangles={{1,0,3},{3,2,1}};
        this->skyboxsol.programID=this->root.programID;
        
        this->skyboxmur1.vertices_Espace={
            glm::vec3(-300.0f, -300.0f, -300.0f),
            glm::vec3(300.0f, -300.0f, -300.0f),
            glm::vec3(-300.0f, 300.0f, -300.0f),
            glm::vec3(300.0f, 300.0f, -300.0f)
        };
        this->skyboxmur1.texCoords={
            glm::vec2(0.0f, 1.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(0.0f, 0.0f),
            glm::vec2(1.0f, 0.0f)
        };
        this->skyboxmur1.indices={1,0,2,2,3,1};
        this->skyboxmur1.triangles={{1,0,2},{2,3,1}};
        this->skyboxmur1.programID=this->root.programID;
        
        this->skyboxmur2.vertices_Espace={
            glm::vec3(300.0f, -300.0f, -300.0f),
            glm::vec3(300.0f, -300.0f, 300.0f),
            glm::vec3(300.0f, 300.0f, -300.0f),
            glm::vec3(300.0f, 300.0f, 300.0f)
        };
        this->skyboxmur2.texCoords={
            glm::vec2(0.0f, 1.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(0.0f, 0.0f),
            glm::vec2(1.0f, 0.0f)
        };
        this->skyboxmur2.indices={1,0,2,2,3,1};
        this->skyboxmur2.triangles={{1,0,2},{2,3,1}};
        this->skyboxmur2.programID=this->root.programID;
        
        this->skyboxmur3.vertices_Espace={
            glm::vec3(300.0f, -300.0f, 300.0f),
            glm::vec3(-300.0f, -300.0f, 300.0f),
            glm::vec3(-300.0f, 300.0f, 300.0f),
            glm::vec3(300.0f, 300.0f, 300.0f)
        };
        this->skyboxmur3.texCoords={
            glm::vec2(0.0f, 1.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(0.0f, 0.0f)
        };
        this->skyboxmur3.indices={1,0,3,3,2,1};
        this->skyboxmur3.triangles={{1,0,3},{3,2,1}};
        this->skyboxmur3.programID=this->root.programID;
        
        this->skyboxmur4.vertices_Espace={
            glm::vec3(-300.0f, -300.0f, 300.0f),
            glm::vec3(-300.0f, -300.0f, -300.0f),
            glm::vec3(-300.0f, 300.0f, -300.0f),
            glm::vec3(-300.0f, 300.0f, 300.0f)
        };
        this->skyboxmur4.texCoords={
            glm::vec2(0.0f, 1.0f),
            glm::vec2(1.0f, 1.0f),
            glm::vec2(1.0f, 0.0f),
            glm::vec2(0.0f, 0.0f)
        };
        this->skyboxmur4.indices={1,0,3,3,2,1};
        this->skyboxmur4.triangles={{1,0,3},{3,2,1}};
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
        if(obj->nbCollision>0){
            obj->globalTransform.t[1]+=0.1;
            obj->setGlobalTransform(obj->globalTransform);
            if(obj->collisions[obj->nbCollision]->nom=="shell" && obj->collisions[obj->nbCollision]->pv==0){
                obj->collisions[obj->nbCollision]->pv=1;
                obj->collisions[obj->nbCollision]->nbCollision=-1;
            }else{
                obj->collisions[obj->nbCollision]->pv-=1;
            }
            this->camera.parent->speed = glm::vec3(0.0f,10.0f,0.0f);
        }
    }
};
