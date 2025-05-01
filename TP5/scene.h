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
        for(auto& i : this->lights){ 
            i->draw(camera.globalTransform.t, elapsedTime);
        }
        this->animation(elapsedTime);
        this->root.draw(camera.globalTransform.t, elapsedTime);
    }
};
