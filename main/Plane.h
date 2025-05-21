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
#include <main/Mesh.h>

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
        const char * heightmap = "../textures/heighMap.png";
        bool hasHeightMap = false;

        Plane():Mesh(),size(0){}

        Plane(int size):Mesh(""),size(size){
            loadHeightMaps(programID);
            generatePlan(size);
            isPBR = false;
        }

        Plane(int size, const char* texture):size(size),Mesh(""){
            generatePlan(size);
            this->filename = texture;
            loadTexture();
            hasHeightMap = false;
            isPBR = false;
        }

        void generatePlan(int size){
            indexed_vertices.clear(); indices.clear(); triangles.clear(); texCoords.clear();
            indexed_vertices.resize(size*size);texCoords.resize(size*size); normal.resize(size*size);
            int v = 0;
            for (int i=0;i<size;i+=1){
                for(int j=0;j<size;j+=1){
                    glm::vec3 vertex = glm::vec3(i/float(size-1)-0.5,0,j/float(size-1)-0.5);
                    indexed_vertices[v] = vertex;
                    texCoords[v] = glm::vec2(1.0-float(i)/(size-1),1.0-float(j)/(size-1));
                    getHeightAtUV(texCoords[v], 1.0f);
                    indexed_vertices[v][1] = getHeightAtUV(texCoords[v], 1.0f);
                    normal[v] = glm::vec3(0.0,1.0,0.0);
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
            Text2DGrass = Loadtexture("../textures/grass.png");
            Text2DRock = Loadtexture("../textures/rock.png");
            Text2DSnow = Loadtexture("../textures/snowrocks.png");
            if(Text2DHMID == 0) {
                std::cerr << "Failed to load height map texture." << std::endl;
            } else {
                hasHeightMap = true;
            }
        }

        void drawHM() {
            GLuint PBRboolUniformID = glGetUniformLocation(programID, "isPBR");
            GLuint useHeightMapUniformID = glGetUniformLocation(programID, "useHeightMap");
            GLuint scaleUniformID = glGetUniformLocation(programID, "scale");
            HM2DUniformID = glGetUniformLocation( programID, "HeightMapSampler");
            Text2DGrassUniform = glGetUniformLocation( programID, "albedoMap");
            Text2DRockUniform = glGetUniformLocation( programID, "normalMap");
            Text2DSnowUniform = glGetUniformLocation( programID, "roughnessMap");
            GrassHeightID = glGetUniformLocation( programID, "GrassHeight");
            RockHeightID = glGetUniformLocation( programID, "RockHeight");
            bool heightMapAvailable = (Text2DHMID != 0);
            glUniform1i(glGetUniformLocation(programID, "useHeightMap"), heightMapAvailable==true ? 1 : 0);
            glUniform1i(glGetUniformLocation(programID, "isPBR"), isPBR==true ? 1 : 0);
            glUniform1f(GrassHeightID, GrassHeight);
            glUniform1f(RockHeightID, RockHeight);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, Text2DGrass);
            glUniform1i(Text2DGrassUniform, 0);
            
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, Text2DRock);
            glUniform1i(Text2DRockUniform, 1);
            
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, Text2DSnow);
            glUniform1i(Text2DSnowUniform, 2);
    
            glGenBuffers(1, &this->vertexbuffer);
            glBindBuffer(GL_ARRAY_BUFFER, this->vertexbuffer);
            glBufferData(GL_ARRAY_BUFFER, this->vertices_Espace.size() * sizeof(glm::vec3), &this->vertices_Espace[0], GL_STATIC_DRAW);

            glGenBuffers(1, &this->Text2DUVBufferID);
            glBindBuffer(GL_ARRAY_BUFFER, this->Text2DUVBufferID);
            glBufferData(GL_ARRAY_BUFFER, this->texCoords.size() * sizeof(glm::vec2), &this->texCoords[0], GL_STATIC_DRAW);

            glGenBuffers(1, &this->elementbuffer);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->elementbuffer);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(unsigned short), &this->indices[0], GL_STATIC_DRAW);

            glGenBuffers(1,&this->normalbuffer);
            glBindBuffer(GL_ARRAY_BUFFER,this->normalbuffer);
            glBufferData(GL_ARRAY_BUFFER,this->normal.size()*sizeof(glm::vec3),&this->normal[0],GL_STATIC_DRAW);

            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, this->vertexbuffer);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
            
            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER, this->Text2DUVBufferID);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
            
            glEnableVertexAttribArray(2);
            glBindBuffer(GL_ARRAY_BUFFER, this->normalbuffer);
            glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,0,(void*)0);

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

        // Fonction pour appliquer un filtre de moyenne mobile sur la heightmap
        float applySmoothingFilter(int x, int y, int textureWidth, int textureHeight, const std::vector<unsigned char>& textureData) {
            float sum = 0.0f;
            int count = 0;

            // Appliquer un filtre de 3x3 (pixel voisin immédiat)
            for (int dy = -4; dy <= 4; ++dy) {
                for (int dx = -4; dx <= 4; ++dx) {
                    int nx = x + dx;
                    int ny = y + dy;

                    // Gérer les bords de l'image
                    if (nx >= 0 && ny >= 0 && nx < textureWidth && ny < textureHeight) {
                        int index = ny * textureWidth + nx;
                        sum += (textureData[index] / 255.0f);  // Valeur normalisée de [0,1]
                        ++count;
                    }
                }
            }

            // Retourner la moyenne des voisins
            return sum / count;
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
                float smoothedHeight = applySmoothingFilter(x, y, textureWidth, textureHeight, textureData);
                return ((smoothedHeight - 0.5) * scale);  // Retourner la hauteur lissée
            } 
            return 0.0f;
        }
        
};