#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "gameObject.h"

struct CAMERA_MODE{
    enum{
        CLASSIC,
        ORBITAL,
        FIRST_PERSON,
    };
};

// struct FrustumPlane {
//     glm::vec4 equation;
// };

// struct Frustum {
//     std::vector<FrustumPlane> planes=std::vector<FrustumPlane>(6);
// };

// // Normalise un plan (pour que a^2 + b^2 + c^2 = 1)
// inline glm::vec4 normalizePlane(const glm::vec4& plane) {
//     float length = glm::length(glm::vec3(plane));
//     return plane / length;
// }

// // Extrait les plans du frustum à partir de la matrice VP
// inline Frustum extractFrustumPlanes(const glm::mat4& VP) {
//     Frustum frustum;

//     // Left
//     frustum.planes[0].equation = normalizePlane(glm::vec4(
//         VP[0][3] + VP[0][0],
//         VP[1][3] + VP[1][0],
//         VP[2][3] + VP[2][0],
//         VP[3][3] + VP[3][0]
//     ));
//     // Right
//     frustum.planes[1].equation = normalizePlane(glm::vec4(
//         VP[0][3] - VP[0][0],
//         VP[1][3] - VP[1][0],
//         VP[2][3] - VP[2][0],
//         VP[3][3] - VP[3][0]
//     ));
//     // Bottom
//     frustum.planes[2].equation = normalizePlane(glm::vec4(
//         VP[0][3] + VP[0][1],
//         VP[1][3] + VP[1][1],
//         VP[2][3] + VP[2][1],
//         VP[3][3] + VP[3][1]
//     ));
//     // Top
//     frustum.planes[3].equation = normalizePlane(glm::vec4(
//         VP[0][3] - VP[0][1],
//         VP[1][3] - VP[1][1],
//         VP[2][3] - VP[2][1],
//         VP[3][3] - VP[3][1]
//     ));
//     // Near
//     frustum.planes[4].equation = normalizePlane(glm::vec4(
//         VP[0][3] + VP[0][2],
//         VP[1][3] + VP[1][2],
//         VP[2][3] + VP[2][2],
//         VP[3][3] + VP[3][2]
//     ));
//     // Far
//     frustum.planes[5].equation = normalizePlane(glm::vec4(
//         VP[0][3] - VP[0][2],
//         VP[1][3] - VP[1][2],
//         VP[2][3] - VP[2][2],
//         VP[3][3] - VP[3][2]
//     ));

//     return frustum;
// }

class Camera : public GameObject {
    public:
        glm::mat4 viewMatrix;
        glm::mat4 projectionMatrix;
        glm::mat4 vp;
        // Frustum frustum;
        bool hasMesh = false;

        float fov;
        float aspectRatio;
        float nearPlane;
        float farPlane;

        glm::vec3 targetPosition;
        int mode = CAMERA_MODE::ORBITAL;

        float sphericRadius = 0.5f;

        float orbitalRadius = 10.0f;
        float theta = 0.0f;     // angle horizontal en radians
        float phi = glm::radians(75.0f); // angle vertical (évite de passer par les pôles)


        Camera(float fov = 45.0f, float aspectRatio = 4.0f / 3.0f, float nearPlane = 0.1f, float farPlane = 500.0f)
            : fov(fov), aspectRatio(aspectRatio), nearPlane(nearPlane), farPlane(farPlane) {
            updateProjectionMatrix();
            nom = "Camera";
        }

        void updateProjectionMatrix() {
            projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
        }

        void lookAt(GameObject* target) {
            if (this->parent) {

                glm::vec3 position = globalTransform.t;
                glm::vec3 forward = glm::normalize(targetPosition - position);
                glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
                if(glm::dot(forward, up)<0.1f && mode != CAMERA_MODE::FIRST_PERSON && mode != CAMERA_MODE::ORBITAL){
                    up = glm::vec3(0.0f, 0.0f, 1.0f);
                }
                viewMatrix = glm::lookAt(position, targetPosition, up);
                glm::vec3 right = glm::normalize(glm::cross(up, forward));
                glm::vec3 correctedUp = glm::cross(forward, right);

                globalTransform.m = glm::mat3(right, correctedUp, forward);
                
            }
        }

        void animation(float deltaTime) {
            if(this->mode == CAMERA_MODE::CLASSIC){
                this->setGlobalTransform(this->parent->globalTransform.combine_with(Transform(this->globalTransform.m,glm::vec3(0.0,this->parent->globalTransform.t[1]*3.0f,0.0),1.0)));
                targetPosition = this->parent->globalTransform.t + glm::vec3(0.0f, 10.0, 0.0f);
            }
            else if(this->mode == CAMERA_MODE::FIRST_PERSON){
                glm::vec3 parentFront = glm::vec3(this->parent->transform.m[1].x, -1.0f, this->parent->transform.m[1].z)*1.0f;
                this->setGlobalTransform(Transform(this->parent->transform.m,this->parent->globalTransform.t + glm::vec3(-0.5,-2.0f,-0.5f)*parentFront,1.0));
                targetPosition = this->globalTransform.t - parentFront - glm::vec3(0.0,1.2f,0.0f);
            }
            else if (this->mode == CAMERA_MODE::ORBITAL) {
                this->targetPosition = lerp(this->targetPosition, arrondirAuDixieme(this->parent->globalTransform.t) + glm::vec3(0.0f, 3.0f, 0.0f), 0.2f);
                float adjustedPhi = phi + glm::radians(0.0f);
                float adjustedTheta = theta + glm::radians(-90.0f);
                adjustedPhi = glm::clamp(adjustedPhi, glm::radians(1.0f), glm::radians(90.0f));
                float x = targetPosition.x + orbitalRadius * sin(adjustedPhi) * cos(adjustedTheta);
                float y = targetPosition.y + orbitalRadius * cos(adjustedPhi);
                float z = targetPosition.z + orbitalRadius * sin(adjustedPhi) * sin(adjustedTheta);
                glm::vec3 cameraPosition = glm::vec3(x, y, z);
                float lerpFactor = 0.2f;
                float distance = glm::length(cameraPosition - this->globalTransform.t);
                if (distance > 0.1f) {
                    cameraPosition = lerp(this->globalTransform.t, cameraPosition, lerpFactor);
                }
                this->setGlobalTransform(Transform(this->transform.m, cameraPosition, 1.0));
                handleCollision(deltaTime);
            }
            // this->vp=this->projectionMatrix*this->viewMatrix;
            // this->frustum = extractFrustumPlanes(this->vp);
        }

        glm::vec3 lerp(const glm::vec3& start, const glm::vec3& end, float t) {
            return start + t * (end - start);
        }
        glm::vec3 arrondirAuDixieme(glm::vec3 valeur) {
            return glm::vec3(std::round(valeur[0] * 20.0f),std::round(valeur[1]*20.0f), std::round(valeur[2]*20.0f))/ 20.0f;
        }

        bool handleCollision(float deltaTime){
            int index = 0;
            for(auto& i: this->parent->collisions){
                if(i->map){
                    for(int j=0;j<i->objetsOBJ.size();j++){
                        glm::vec3 ret;
                        if(i->objetsOBJ[j].mesh.isPointInsideMesh(this->globalTransform.t)){
                            float axeCollision = glm::dot((i->objetsOBJ[j].globalTransform.t-this->globalTransform.t), glm::cross(this->frontAxe,this->upAxe));
                            std::cout<<axeCollision<<std::endl;
                            if(axeCollision>0){
                                theta += 5.0f*deltaTime;
                            } else{
                                theta-=5.0f*deltaTime;
                            }   
                        }
                    }
                }
                else {
                    bool test = i->boiteEnglobante.isPointInsideMesh(this->globalTransform.t);
                    if(test){
                        float axeCollision = glm::dot((i->globalTransform.t-this->globalTransform.t), glm::cross(this->frontAxe,this->upAxe));
                        if(axeCollision>0){
                            theta += 5.0f*deltaTime;
                        } else{
                            theta-=5.0f*deltaTime;
                        }
    
                    }
                }
                index++;
            }
            return false;
        }
};

#endif