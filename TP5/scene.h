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
    int niveau=1;
    std::vector<GameObject*> lights;
        Scene(){}

    void animation(float deltaTime){

        GameObject* Map = this->root.enfant[0];
        GameObject* Obj = Map->enfant[Map->objetsOBJ.size()];

        if(camera.mode == CAMERA_MODE::CLASSIC){
            camera.setGlobalTransform(Obj->globalTransform.combine_with(Transform(camera.transform.m,glm::vec3(0.0,Obj->transform.t[1]*8+Map->transform.s,-Map->transform.s*2),1.0)));
        }

    }

    void draw(float elapsedTime){
        root.nom="root";
        GameObject* Map = this->root.enfant[0];
        GameObject* Obj = Map->enfant[Map->objetsOBJ.size()];
        for(auto& i : this->lights){ 
            i->draw(camera.globalTransform.t, elapsedTime);
        }
        this->animation(elapsedTime);
        this->root.testIA(Obj);
        this->root.draw(camera.globalTransform.t, elapsedTime);
        this->testChangementNiveau(Map,Obj);
    }

    void testChangementNiveau(GameObject* map, GameObject* obj){
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
    }
};
