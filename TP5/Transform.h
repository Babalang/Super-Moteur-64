#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

class Transform{
    public :
        float s;
        glm::mat3x3 m;
        glm::vec3 t;

        Transform() : m(glm::mat3(1.0)), t(glm::vec3(0.0)), s(1.0) {}
        Transform(glm::mat3 m, glm::vec3 t, float s = 1.0f) : s(s), m(m), t(t) {}

        glm::vec3 applyToPoint(glm::vec3 p){
            return (s*(m*p))+t;
        }

        glm::vec3 applyToVector(glm::vec3 p){
            return s*(m*p);
        }

        glm::vec3 applyToVersor(glm::vec3 p){
            return glm::normalize(m*p);
        }

        Transform combine_with(Transform t) const {
            auto combined_m = this->m * t.m;        
            auto combined_t = this->t + (this->m * (t.t * this->s));        
            auto combined_s = this->s * t.s;        
            return Transform(combined_m, combined_t, combined_s);
        }        
        Transform inverse(){
            glm::mat3x3 invM = glm::inverse(this->m);
            float invS = (s != 0) ? 1.0f/this->s : 1.0f;
            return Transform(invM, -invM * (t*invS), invS);
        }
        Transform interpolate_with(Transform t, float k){
            float interpS = (1-k) *this->s +k * t.s;
            glm::mat3x3 interpM = glm::mix(this->m,t.m,k);
            glm::vec3 interpT = (1-k)*this->t+k*t.t;
            return Transform(interpM,interpT,interpS);
        }

        Transform rotation(glm::vec3 axe, float angle) {
            glm::vec3 normalizedAxe = glm::normalize(axe);
            float angleRadians = glm::radians(angle);
            glm::mat3 rotationMatrix = glm::mat3(glm::rotate(glm::mat4(1.0f), angleRadians, normalizedAxe));
            return Transform(rotationMatrix, glm::vec3(0.0f), 1.0f);
        }

        Transform translation(glm::vec3 axe, float distance) {
            glm::vec3 translationVector = axe * distance;
            return Transform(glm::mat3(1.0f), translationVector, 1.0f);
        }

        Transform scale(float size) {
            return Transform(glm::mat3x3(1.0), glm::vec3(0.0f), size);
        }
};

#endif // TRANSFORM_H
