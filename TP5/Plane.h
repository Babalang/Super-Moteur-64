// Include standard headers
#include <GL/glew.h>
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <unistd.h>

// Include GLEW

// Include GLFW
#include <GLFW/glfw3.h>

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
#include <TP5/Mesh.h>

#include "Transform.h"

class Plane : public Mesh{

    public :
        int size;
        GLuint Text2DHMID;
        GLuint Text2DGrass;
        GLuint Text2DRock;
        GLuint Text2DSnow;
        GLuint HM2DUniformID;
        GLuint Text2DGrassUniform;
        GLuint Text2DRockUniform;
        GLuint Text2DSnowUniform;
        GLuint GrassHeightID;
        GLuint RockHeightID;
        float GrassHeight = 0.3f;
        float RockHeight = 0.6f;
        const char * heightmap = "../texture/heighMap.png";
        bool hasHeightMap = false;

        Plane():Mesh(),size(0){}

        Plane(int size):Mesh(""),size(size){
            generatePlan(size);
            loadHeightMaps(programID);
            hasHeightMap = true;
        }

        Plane(int size, const char* texture):size(size),Mesh(""){
            generatePlan(size);
            this->filename = texture;
            loadTexture();
            hasHeightMap = false;
        }

        void generatePlan(int size){
            indexed_vertices.clear(); indices.clear(); triangles.clear(); texCoords.clear();
            indexed_vertices.resize(size*size);texCoords.resize(size*size);
            int v = 0;
            for (int i=0;i<size;i+=1){
                for(int j=0;j<size;j+=1){
                    glm::vec3 vertex = glm::vec3(i/float(size-1)-0.5,0,j/float(size-1)-0.5);
                    indexed_vertices[v] = vertex;
                    texCoords[v] = glm::vec2(1.0-float(i)/(size-1),1.0-float(j)/(size-1));
                    v++;
                }
            }
            for (int i=0;i<size-1;i++){
                for(int j=0;j<size-1;j++){
                    std::vector< unsigned short > tri1(3);
                    tri1[0]=i*size+j;
                    tri1[1]=i*size+j+1;
                    tri1[2]=(i+1)*size+j;
                    triangles.push_back(tri1);
                    indices.push_back(tri1[0]);indices.push_back(tri1[1]);indices.push_back(tri1[2]);
                    std::vector< unsigned short > tri2(3);
                    tri2[0]=(i+1)*size+j+1;
                    tri2[1]=(i+1)*size+j;
                    tri2[2]=i*size+j+1;
                    triangles.push_back(tri2);
                    indices.push_back(tri2[0]);indices.push_back(tri2[1]);indices.push_back(tri2[2]);
                }
            }
        }

        GLuint Loadtexture(const char* path){
            GLuint textureID;
            int width, height, numComponents;
            unsigned char * data = stbi_load (path,&width,&height,&numComponents,0);
            if(!data){
                std::cout<<"Pas de texture"<<std::endl;
            }
            glGenTextures (1, &textureID);
            glBindTexture (GL_TEXTURE_2D, textureID);
            glTexImage2D (GL_TEXTURE_2D,0,(numComponents == 1 ? GL_RED : numComponents == 3 ? GL_RGB : GL_RGBA),width,height,0,(numComponents == 1 ? GL_RED : numComponents == 3 ? GL_RGB : GL_RGBA),GL_UNSIGNED_BYTE,data);
            glGenerateMipmap(GL_TEXTURE_2D);
            stbi_image_free(data);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glBindTexture (GL_TEXTURE_2D, 0); // unbind the texture
            std::cout<<"Texture loaded"<<std::endl;
            return textureID;
        }


        void loadHeightMaps(GLuint programID){
            Text2DHMID = Loadtexture(heightmap);
            Text2DGrass = Loadtexture("../texture/grass.png");
            Text2DRock = Loadtexture("../texture/rock.png");
            Text2DSnow = Loadtexture("../texture/snowrocks.png");
        }

        void drawHM() {
            HM2DUniformID = glGetUniformLocation( programID, "HeightMapSampler");
            Text2DGrassUniform = glGetUniformLocation( programID, "Text2DGrass");
            Text2DRockUniform = glGetUniformLocation( programID, "Text2DRock");
            Text2DSnowUniform = glGetUniformLocation( programID, "Text2DSnow");
            GrassHeightID = glGetUniformLocation( programID, "GrassHeight");
            RockHeightID = glGetUniformLocation( programID, "RockHeight");
            glUseProgram(this->programID);
            bool heightMapAvailable = (Text2DHMID != 0);
            glUniform1i(glGetUniformLocation(programID, "useHeightMap"), heightMapAvailable==true ? 1 : 0);
            glUniform1f(GrassHeightID, GrassHeight);
            glUniform1f(RockHeightID, RockHeight);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, Text2DHMID);
            glUniform1i(HM2DUniformID, 0);
    
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, Text2DGrass);
            glUniform1i(Text2DGrassUniform, 1);
    
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, Text2DRock);
            glUniform1i(Text2DRockUniform, 2);
    
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, Text2DSnow);
            glUniform1i(Text2DSnowUniform, 3);

            glGenBuffers(1, &this->vertexbuffer);
            glBindBuffer(GL_ARRAY_BUFFER, this->vertexbuffer);
            glBufferData(GL_ARRAY_BUFFER, this->vertices_Espace.size() * sizeof(glm::vec3), &this->vertices_Espace[0], GL_STATIC_DRAW);

            glGenBuffers(1, &this->Text2DUVBufferID);
            glBindBuffer(GL_ARRAY_BUFFER, this->Text2DUVBufferID);
            glBufferData(GL_ARRAY_BUFFER, this->texCoords.size() * sizeof(glm::vec2), &this->texCoords[0], GL_STATIC_DRAW);

            glGenBuffers(1, &this->elementbuffer);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->elementbuffer);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(unsigned short), &this->indices[0], GL_STATIC_DRAW);

            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, this->vertexbuffer);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER, this->Text2DUVBufferID);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

            glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_SHORT, (void*)0);

            glDisableVertexAttribArray(1);
            glDisableVertexAttribArray(0);

            glDeleteBuffers(1, &this->vertexbuffer);
            glDeleteBuffers(1, &this->Text2DUVBufferID);
            glDeleteBuffers(1, &this->elementbuffer);
            glBindTexture (GL_TEXTURE_2D, 0);

        }

        glm::vec2 intersection(const glm::vec3& objectCenter, const glm::vec3& direction, float scale) {
            glm::vec3 planeNormal = glm::vec3(0.0f, 1.0f, 0.0f);
            glm::vec3 planePoint = glm::vec3(0.0f, 0.0f, 0.0f);
            float denominator = glm::dot(planeNormal, direction);
            if (fabs(denominator) < 1e-6) {
                std::cerr << "No intersection: the ray is parallel to the plane." << std::endl;
                return glm::vec2(-1.0f, -1.0f);
            }
            float t = glm::dot(planeNormal, planePoint - objectCenter) / denominator;
            glm::vec3 intersectionPoint = objectCenter + t * direction;
            glm::vec2 uv = calculUV(intersectionPoint, scale);
            if (uv.x < 0.0f || uv.x > 1.0f || uv.y < 0.0f || uv.y > 1.0f) {
                std::cerr << "Intersection UV hors des limites : (" << uv.x << ", " << uv.y << ")" << std::endl;
                return glm::vec2(-1.0f, -1.0f);
            }

            return uv;
        }

        glm::vec2 calculUV(glm::vec3 point, float scale) {
            float u = 1-(point.x / scale + 0.5f);
            float v = 1-(point.z / scale + 0.5f);
            return glm::vec2(u, v);
        }

        float getHeightAtUV(const glm::vec2& uv, float scale) {
            if(hasHeightMap){
                int textureWidth = 512;
                int textureHeight = 512;
                glBindTexture(GL_TEXTURE_2D, Text2DHMID);
                std::vector<unsigned char> textureData(textureWidth * textureHeight);
                glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, textureData.data());
                glBindTexture(GL_TEXTURE_2D, 0);
                int x = static_cast<int>(uv.x * textureWidth) % textureWidth;
                int y = static_cast<int>(uv.y * textureHeight) % textureHeight;
                int index = y * textureWidth + x;
                return ((textureData[index] / 255.0f)-0.5)*scale;
            } 
            return 0.0f;
        }
        
};