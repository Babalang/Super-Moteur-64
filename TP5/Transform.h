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
            glm::mat3 combined_m = this->m * t.m; // Combine les rotations
            glm::vec3 combined_t = this->t + (this->m * t.t); // Combine les translations sans appliquer l'échelle
            float combined_s = this->s * t.s; // Combine les échelles
            return Transform(combined_m, combined_t, combined_s);
        }
        
             
        Transform inverse() {
            glm::mat3x3 invM = glm::inverse(this->m);
            float invS = (s != 0) ? 1.0f / this->s : 1.0f;
            return Transform(invM, -invM * t * invS, invS);
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
            glm::mat3 rotationMatrix = cleanMatrix(glm::mat3(glm::rotate(glm::mat4(1.0f), angleRadians, normalizedAxe)));
            return Transform(rotationMatrix, glm::vec3(0.0f), 1.0f);
        }


        Transform translation(glm::vec3 axe, float distance) {
            glm::vec3 translationVector = axe * distance;
            return Transform(glm::mat3(1.0f), translationVector, 1.0f);
        }

        Transform scale(float size) {
            return Transform(glm::mat3x3(1.0), glm::vec3(0.0f), size);
        }

        glm::mat3 cleanMatrix(glm::mat3 matrix) {
            for (int i = 0; i < 3; ++i) {
                for (int j = 0; j < 3; ++j) {
                    if (glm::abs(matrix[i][j]) < 1e-6) {
                        matrix[i][j] = 0.0f;
                    }
                }
            }
            return matrix;
        }

        glm::mat4 toMat4() const {
            glm::mat4 mat = glm::mat4(1.0f);
            mat = glm::translate(mat, t);
            mat *= glm::mat4(m) * glm::scale(glm::mat4(1.0f), glm::vec3(s));
            return mat;
        }
};

#endif // TRANSFORM_H
