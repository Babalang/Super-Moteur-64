// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>

#include <TP5/camera.h>

class Scene{
    public : 
    GameObject root;
    Camera camera;
    GLuint programID;
        Scene(){}

    void animation(float deltaTime){

        GameObject* Map = this->root.enfant[0];
        GameObject* Obj = Map->enfant[0];    

        if(camera.mode == CAMERA_MODE::CLASSIC){
            camera.setGlobalTransform(Obj->globalTransform.combine_with(Transform(camera.transform.m,glm::vec3(0.0,1.0,-1.0),1.0)));
        }

    }

    void draw(float elapsedTime){
        this->animation(elapsedTime);
        this->root.draw(camera.globalTransform.t, elapsedTime);
    }

};
