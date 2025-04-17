#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "gameObject.h"

struct CAMERA_MODE{
    enum{
        CLASSIC,
        ORBITAL
    };
};

class Camera : public GameObject {
    public:
        glm::mat4 viewMatrix;
        glm::mat4 projectionMatrix;
        bool hasMesh = false;

        float fov;
        float aspectRatio;
        float nearPlane;
        float farPlane;
        int mode = CAMERA_MODE::CLASSIC;

        float orbitalRadius = 3.0f;

        Camera(float fov = 45.0f, float aspectRatio = 4.0f / 3.0f, float nearPlane = 0.1f, float farPlane = 100.0f)
            : fov(fov), aspectRatio(aspectRatio), nearPlane(nearPlane), farPlane(farPlane) {
            updateProjectionMatrix();
        }

        void updateProjectionMatrix() {
            projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
        }

        void lookAt(GameObject* target) {
            if (target) {
                glm::vec3 position = globalTransform.t;
                glm::vec3 targetPosition = target->globalTransform.t;
                glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
                viewMatrix = glm::lookAt(position, targetPosition, up);
                glm::vec3 forward = glm::normalize(targetPosition - position);
                glm::vec3 right = glm::normalize(glm::cross(up, forward));
                glm::vec3 correctedUp = glm::cross(forward, right);

                globalTransform.m = glm::mat3(right, correctedUp, -forward);
                
            }
        }
};

#endif