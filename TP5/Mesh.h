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
#include <TP5/stb_image.h>
#include<unordered_map>
#include "Transform.h"

class Mesh{

    public :
        std::vector<unsigned short> indices;
        std::vector<std::vector<unsigned short> > triangles;
        std::vector<glm::vec3> indexed_vertices;
        std::vector<glm::vec3> vertices_Espace;
        std::vector<glm::vec2> texCoords;
        std::vector<glm::vec3> normal;
        std::string filename;
        GLuint programID;
        GLuint textureID;
        GLuint vertexbuffer;
        GLuint elementbuffer;
        GLuint Text2DUVBufferID;
        GLuint normalbuffer;
        // Si l'objet est PBR
        bool isPBR = false;
        GLuint Text2DalbedoID;
        GLuint Text2DnormalID;
        GLuint Text2DroughnessID;
        GLuint Text2DmetallicID;
        GLuint Text2DaoID;

        Mesh():filename(""){}

        Mesh(const char* path, const char* texture, bool isPBR = false):filename(texture), isPBR(isPBR){
            bool test = loadOFF(path,indexed_vertices, this->indices, this->triangles);
            for(int i=0;i<this->indexed_vertices.size();i++){
                this->texCoords.push_back(glm::vec2(0.0,0.0));
            }
            loadTexture();
            generateUVs();
            if(test){
                std::cout<<"Calcul des normales"<<std::endl;
                compute_Normals();
            }
        }

        Mesh(const char* texture, bool isPBR = false):filename(texture), isPBR(isPBR){
            loadTexture();
        }



        bool setMesh(const char* path) {
            bool test = loadOFF(path, indexed_vertices, this->indices, this->triangles );
            if(test){
                std::cout<<"Calcul des normales"<<std::endl;
                compute_Normals();
            }
            return test;
        }

        void compute_Normals(){
            normal.resize(indexed_vertices.size());
            for(int i=0;i<indexed_vertices.size();i++){
                normal[i] = glm::vec3(0.0,0.0,0.0);
            }
            for(size_t i=0; i<triangles.size();i++){
                for(size_t y = 0;y<3;y++){
                    if(normal[triangles[i][y]]==vec3(0.,0.,0.)){
                        normal[triangles[i][y]]= -glm::cross(indexed_vertices[triangles[i][abs(y-1)%3]]-indexed_vertices[triangles[i][y]], indexed_vertices[triangles[i][(y+1)%3]]-indexed_vertices[triangles[i][y]] );
         //               std::cout<<normal[triangles[i][y]][0]<<" "<<normal[triangles[i][y]][1]<<" "<<normal[triangles[i][y]][2]<<std::endl;
                    }
                }
            }
        }

        void loadPBR(const char* pathToAlbedo, const char* pathToNormal, const char* pathToRoughness, const char* pathToMetallic, const char* pathToAo){
            loadTexture(this->Text2DalbedoID,pathToAlbedo);
            loadTexture(this->Text2DnormalID,pathToNormal);
            loadTexture(this->Text2DroughnessID,pathToRoughness);
            loadTexture(this->Text2DmetallicID,pathToMetallic);
            loadTexture(this->Text2DaoID,pathToAo);
        }

        std::vector<glm::vec3> giveVertices(){
            return this->indexed_vertices;
        }

        std::vector<std::vector<unsigned short> > giveTriangles(){
            return this->triangles;
        }

        std::vector<unsigned short> getIndices(){
            return this->indices;
        }

        std::vector<glm::vec2> getTexCoords(){
            return this->texCoords;
        }

        void loadTexture(){
            int width, height, numComponents;
            unsigned char * data = stbi_load (this->filename.c_str(),&width,&height,&numComponents,0);
            glGenTextures (1, &textureID);
            glBindTexture (GL_TEXTURE_2D, textureID);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexImage2D (GL_TEXTURE_2D,0,(numComponents == 1 ? GL_RED : numComponents == 3 ? GL_RGB : GL_RGBA),width,height,0,(numComponents == 1 ? GL_RED : numComponents == 3 ? GL_RGB : GL_RGBA),GL_UNSIGNED_BYTE,data);
            glGenerateMipmap(GL_TEXTURE_2D);
            stbi_image_free(data);
            glBindTexture (GL_TEXTURE_2D, 0);                                                                          
        }

        void loadTexture(GLuint& id, const char* path){
            int width, height, numComponents;
            unsigned char * data = stbi_load (path,&width,&height,&numComponents,0);
            glGenTextures (1, &id);
            glBindTexture (GL_TEXTURE_2D, id);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexImage2D (GL_TEXTURE_2D,0,(numComponents == 1 ? GL_RED : numComponents == 3 ? GL_RGB : GL_RGBA),width,height,0,(numComponents == 1 ? GL_RED : numComponents == 3 ? GL_RGB : GL_RGBA),GL_UNSIGNED_BYTE,data);
            glGenerateMipmap(GL_TEXTURE_2D);
            stbi_image_free(data);
            glBindTexture (GL_TEXTURE_2D, 0);                                                                          
        }

        void sendTexture(){
            if(!isPBR){
                GLuint Text2DUniformID = glGetUniformLocation(this->programID, "albedoMap");
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D,textureID);
                glUniform1i(Text2DUniformID,0);
            } else {
                GLuint Text2DalbedoUniformID = glGetUniformLocation(this->programID, "albedoMap");
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D,Text2DalbedoID);
                glUniform1i(Text2DalbedoUniformID,0);

                GLuint Text2DnormalUniformID = glGetUniformLocation(this->programID, "normalMap");
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D,Text2DnormalID);
                glUniform1i(Text2DnormalUniformID,1);

                GLuint Text2DroughnessUniformID = glGetUniformLocation(this->programID, "roughnessMap");
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D,Text2DroughnessID);
                glUniform1i(Text2DroughnessUniformID,2);

                GLuint Text2DmetallicUniformID = glGetUniformLocation(this->programID, "metallicMap");
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D,Text2DmetallicID);
                glUniform1i(Text2DmetallicUniformID,3);

                GLuint Text2DaoUniformID = glGetUniformLocation(this->programID, "aoMap");
                glActiveTexture(GL_TEXTURE4);
                glBindTexture(GL_TEXTURE_2D,Text2DaoID);
                glUniform1i(Text2DaoUniformID,4);
            }
        }

        void draw(){
            glGenBuffers(1,&this->vertexbuffer);
            glBindBuffer(GL_ARRAY_BUFFER,this->vertexbuffer);
            glBufferData(GL_ARRAY_BUFFER,this->vertices_Espace.size()*sizeof(glm::vec3),&this->vertices_Espace[0],GL_STATIC_DRAW);

            glGenBuffers(1,&this->Text2DUVBufferID);
            glBindBuffer(GL_ARRAY_BUFFER,this->Text2DUVBufferID);
            glBufferData(GL_ARRAY_BUFFER,this->texCoords.size()*sizeof(glm::vec2),&this->texCoords[0],GL_STATIC_DRAW);

            glGenBuffers(1,&this->elementbuffer);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->elementbuffer);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(unsigned short), &this->indices[0], GL_STATIC_DRAW);
            
            glGenBuffers(1,&this->normalbuffer);
            glBindBuffer(GL_ARRAY_BUFFER,this->normalbuffer);
            glBufferData(GL_ARRAY_BUFFER,this->normal.size()*sizeof(glm::vec3),&this->normal[0],GL_STATIC_DRAW);

            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, this->vertexbuffer);
            glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void*)0);

            glEnableVertexAttribArray(1);
            glBindBuffer(GL_ARRAY_BUFFER, this->Text2DUVBufferID);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->elementbuffer);

            glEnableVertexAttribArray(2);
            glBindBuffer(GL_ARRAY_BUFFER, this->normalbuffer);
            glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,0,(void*)0);

            sendTexture();
            glDrawElements(GL_TRIANGLES,this->indices.size(),GL_UNSIGNED_SHORT,(void*)0);
            glDisableVertexAttribArray(1);
            glDisableVertexAttribArray(0);
            glDeleteBuffers(1, &this->vertexbuffer);
            glDeleteBuffers(1, &this->Text2DUVBufferID);
            glDeleteBuffers(1, &this->elementbuffer);
            glDeleteBuffers(1,&this->normalbuffer);
            glBindTexture (GL_TEXTURE_2D, 0);
        }


        void setVerticesEspace(Transform e){
            this->vertices_Espace.clear();
            for(int i=0;i<this->indexed_vertices.size();i++){
                this->vertices_Espace.push_back(e.applyToPoint(this->indexed_vertices[i]));
            }
        }

        void generateUVs() {
            this->texCoords.clear();
            for (auto& vertex : this->indexed_vertices) {
                float r = glm::length(vertex);
                float theta = atan2(vertex.z, vertex.x);
                float phi = acos(glm::clamp(vertex.y / r, -1.0f, 1.0f));
                float u = theta / (2.0f * M_PI);
                float v = phi / M_PI;
                if (u < 0.0f) {
                    u += 1.0f;
                }
                this->texCoords.push_back(glm::vec2(u, v));
            }
        }


        struct GridData {
            glm::vec3 position;
            int cpt;
        };
        struct Grid {
            std::vector<GridData> cells;
        
            glm::vec3 minPos, maxPos;
            int resolution;
        
            int getCellX(glm::vec3 pos) { return resolution * (pos[0] - minPos[0]) / (maxPos[0] - minPos[0]); }
            int getCellY(glm::vec3 pos) { return resolution * (pos[1] - minPos[1]) / (maxPos[1] - minPos[1]); }
            int getCellZ(glm::vec3 pos) { return resolution * (pos[2] - minPos[2]) / (maxPos[2] - minPos[2]); }
        
            int getIndex(int x, int y, int z) { return x * resolution * resolution + y * resolution + z; }
            int getIndex(glm::vec3 pos) { return getCellX(pos) * resolution * resolution + getCellY(pos) * resolution + getCellZ(pos); }
        };
        
        
        void simplify(unsigned int resolution) {
            glm::vec3 C = glm::vec3(indexed_vertices[0]);
            glm::vec3 Cmax = C;
            for(size_t i=1;i<indexed_vertices.size();i++){
                if(C[0]>=indexed_vertices[i][0]){C[0] = indexed_vertices[i][0];}
                if(C[1]>=indexed_vertices[i][1]){C[1] = indexed_vertices[i][1];}
                if(C[2]>=indexed_vertices[i][2]){C[2] = indexed_vertices[i][2];}
                if(Cmax[0]<indexed_vertices[i][0]){Cmax[0] = indexed_vertices[i][0];}
                if(Cmax[1]<indexed_vertices[i][1]){Cmax[1] = indexed_vertices[i][1];}
                if(Cmax[2]<indexed_vertices[i][2]){Cmax[2] = indexed_vertices[i][2];}
            }
            Grid grille;
            grille.resolution = resolution;
            grille.minPos = C-glm::vec3(0.01,0.01,0.01);
            grille.maxPos = Cmax + glm::vec3(0.01,0.01,0.01);
            grille.cells.clear();
            grille.cells.resize(grille.getIndex(grille.maxPos));
        
            for(size_t i=0;i<indexed_vertices.size();i++){
                grille.cells[grille.getIndex(indexed_vertices[i])].position += indexed_vertices[i];
                grille.cells[grille.getIndex(indexed_vertices[i])].cpt ++;
            }
            for(size_t i=0;i<triangles.size();i++){
                glm::vec3 a = indexed_vertices[triangles[i][0]];
                glm::vec3 b = indexed_vertices[triangles[i][1]];
                glm::vec3 c = indexed_vertices[triangles[i][2]];
                if(grille.getIndex(a) == grille.getIndex(b) || grille.getIndex(b) == grille.getIndex(c) ||grille.getIndex(a) == grille.getIndex(c)){
                    triangles.erase(triangles.begin()+i);
                    i--;
                }
                else{
                    triangles[i][0] = grille.getIndex(a);
                    triangles[i][1] = grille.getIndex(b);
                    triangles[i][2] = grille.getIndex(c);
                    
                    }
            }
            indexed_vertices.resize(grille.cells.size());
            for(size_t i=0;i<grille.cells.size();i++){
                grille.cells[i].position /= grille.cells[i].cpt;
                indexed_vertices[i] = grille.cells[i].position;
            }
            indices.clear();
            for(int i=0; i<triangles.size();i++){
                indices.push_back(triangles[i][0]);
                indices.push_back(triangles[i][1]);
                indices.push_back(triangles[i][2]);
            }
            generateUVs();
        }
        
};