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

class Line {
    private:
    public:
        glm::vec3 m_origin , m_direction;
        Line() {}
        Line( glm::vec3  const & o , glm::vec3  const & d )  {
            m_origin = o;
            m_direction = d; m_direction=glm::normalize(m_direction);
        }
        glm::vec3  & origin() { return m_origin; }
        glm::vec3  const & origin() const { return m_origin; }
        glm::vec3  & direction() { return m_direction; }
        glm::vec3  const & direction() const { return m_direction; }
        glm::vec3  project( glm::vec3  const & p ) const {
            glm::vec3  result;
            return result;
        }
        float squareDistance( glm::vec3  const & p ) const {
            float result;
            return result;
        }
        float distance( glm::vec3  const & p ) const {
            return sqrt( squareDistance(p) );
        }
    };

class Ray : public Line {
    public:
        Ray() : Line() {}
        Ray( glm::vec3  const & o , glm::vec3  const & d ) : Line(o,d) {}
};

struct RayTriangleIntersection{
    bool intersectionExists;
    float t;
    float w0,w1,w2;
    unsigned int tIndex;
    glm::vec3 intersection;
    glm::vec3 normal;
};

struct MTL{
    std::string nom;
    float ns;
    glm::vec3 ka;
    glm::vec3 kd;
    glm::vec3 ks;
    glm::vec3 ke;
    float ni;
    float d;
    int illum;
    char* texture;
    std::string albedo;
    std::string ao;
    std::string roughness;
    std::string normal;
    std::string metallic;
    bool pbr=false;
};


class Mesh{

    public :
        std::vector<unsigned short> indices;
        std::vector<std::vector<unsigned short> > triangles;
        std::vector<glm::vec3> indexed_vertices;
        std::vector<glm::vec3> vertices_Espace;
        std::vector<glm::vec2> texCoords;
        std::vector<glm::vec3> normal;
        std::vector<float> areas;
        std::string filename;
        GLuint programID;
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
        std::vector<GLuint> texturesID;
        int scale=1;
        MTL mtl;

        Mesh():filename(""){}

        Mesh(const char* path, const char* texture, bool isPBR = false):filename(texture), isPBR(isPBR){
            bool test = loadOFF(path,indexed_vertices, this->indices, this->triangles);
            for(int i=0;i<this->indexed_vertices.size();i++){
                this->texCoords.push_back(glm::vec2(0.0,0.0));
            }
            if(filename != ""){
                loadTexture();
            }
            generateUVs();
            if(test){
                // std::cout<<"Calcul des normales"<<std::endl;
                compute_Normals();
            }
        }

        Mesh(const char* texture, bool isPBR = false):filename(texture), isPBR(isPBR){
            loadTexture();
        }



        bool setMesh(const char* path) {
            bool test = loadOFF(path, indexed_vertices, this->indices, this->triangles );
            if(test){
                // std::cout<<"Calcul des normales"<<std::endl;
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
                        normal[triangles[i][y]]= glm::cross(indexed_vertices[triangles[i][abs(y-1)%3]]-indexed_vertices[triangles[i][y]], indexed_vertices[triangles[i][(y+1)%3]]-indexed_vertices[triangles[i][y]] );
         //               std::cout<<normal[triangles[i][y]][0]<<" "<<normal[triangles[i][y]][1]<<" "<<normal[triangles[i][y]][2]<<std::endl;
                    }
                }
            }
        }

        void loadPBR(const char* pathToAlbedo, const char* pathToNormal, const char* pathToRoughness, const char* pathToMetallic, const char* pathToAo){
            this->isPBR=true;
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
            if(data == NULL){
                std::cout<<"Erreur de chargement de la texture : "<<this->filename<<std::endl;
                return;
            }
            glGenTextures (1, &Text2DalbedoID);
            glBindTexture (GL_TEXTURE_2D, Text2DalbedoID);
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
            if(data == NULL){
                std::cout<<"Erreur de chargement de la texture : "<<path<<std::endl;
                return;
            }
            glGenTextures (1, &id);
            glBindTexture (GL_TEXTURE_2D, id);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            // std::cout<<"help me"<<std::endl;
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            GLenum format = GL_RGB;
            if(numComponents == 2){
                format = GL_R16F;
                glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_FLOAT, data);
            } 
            else{
                if (numComponents == 1)
                format = GL_RED;
            else if (numComponents == 3)
                format = GL_RGB;
            else if (numComponents == 4)
                format = GL_RGBA;
            else {
                std::cerr << "Unsupported number of components: " << numComponents << std::endl;
                return;
            }
            glTexImage2D (GL_TEXTURE_2D,0,format,width,height,0,format,GL_UNSIGNED_BYTE,data);
        }
                        glGenerateMipmap(GL_TEXTURE_2D);
            stbi_image_free(data);
            glBindTexture (GL_TEXTURE_2D, 0);                                                                          
        }

        void sendTexture(){
            GLuint PBRboolUniformID = glGetUniformLocation(programID, "isPBR");
            GLuint useHeightMapUniformID = glGetUniformLocation(programID, "useHeightMap");
            if(!isPBR){
                glUniform1i(useHeightMapUniformID, 0);
                glUniform1i(PBRboolUniformID, 0);
                GLuint Text2DUniformID = glGetUniformLocation(this->programID, "albedoMap");
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D,Text2DalbedoID);
                glUniform1i(Text2DUniformID,0);
            } else {
                glUniform1i(useHeightMapUniformID, 0);
                glUniform1i(PBRboolUniformID, 1);
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
            if(this->programID == 0){
                std::cout<<"Erreur de chargement du shader !"<<std::endl;
                return;
            }
            glUseProgram(this->programID);
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

        void creerTextureOBJ(const char* f){
            if(mtl.pbr){
                // std::cout<<"pbr"<<std::endl;
                loadPBR(mtl.albedo.c_str(),mtl.normal.c_str(),mtl.roughness.c_str(),mtl.metallic.c_str(),mtl.ao.c_str());
            }else{
                int width, height, numComponents;
                stbi_set_flip_vertically_on_load(true);
                unsigned char * data = stbi_load (f,
                                                &width,
                                                &height,
                                                &numComponents, 
                                                STBI_rgb_alpha);
                                                // 0);
                if(data == NULL){
                    std::cout<<"Erreur de chargement de la texture : "<<f<<std::endl;
                    return;
                }
                glGenTextures (1, &this->Text2DalbedoID);
                glBindTexture (GL_TEXTURE_2D, this->Text2DalbedoID);
                std::string s(f);
                if(s=="../textures/koopa_all.png" || s=="../textures/kuribo_cmp4.png" || s=="../textures/battan_king_face.png" || s=="../textures/battan_king_back_1.png" || s=="../textures/ookan.png"){
                    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
                    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
                }else{
                    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                }
                glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                // glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                // glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                // glTexImage2D (GL_TEXTURE_2D,
                //             0,
                //             (numComponents == 1 ? GL_RED : numComponents == 3 ? GL_RGB : GL_RGBA), 
                //             width,
                //             height,
                //             0,
                //             (numComponents == 1 ? GL_RED : numComponents == 3 ? GL_RGB : GL_RGBA), 
                //             GL_UNSIGNED_BYTE,
                //             data);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
                stbi_image_free(data);
                glBindTexture (GL_TEXTURE_2D, 0); 
            }
        }
        
        void updateAreaAndNormal() {
            for(int i=0;i<this->triangles.size();i++){
                glm::vec3 nNotNormalized = glm::cross( this->indexed_vertices[this->triangles[i][1]] - this->indexed_vertices[this->triangles[i][0]] , this->indexed_vertices[this->triangles[i][2]] - this->indexed_vertices[this->triangles[i][0]] );
                float norm = nNotNormalized.length();
                this->normal.push_back(nNotNormalized / norm);
                this->areas.push_back(norm / 2.f);
            }
        }
        glm::vec3 projectOnSupportPlane( glm::vec3 const & p,int t) {
            glm::vec3 result;glm::vec3 m_normal=this->normal[t];
            glm::vec3 proj=p-this->vertices_Espace[this->triangles[t][0]];
            float d=glm::dot(proj,m_normal);
            result=p-d*m_normal;
            return result;
        }
        float squareDistanceToSupportPlane( glm::vec3 const & p,int t) {
            float result;
            glm::vec3 proj=projectOnSupportPlane(p,t);
            result=(proj[0]-p[0])*(proj[0]-p[0])*(proj[1]-p[1])*(proj[1]-p[1])*(proj[2]-p[2])*(proj[2]-p[2]);
            return result;
        }
        float distanceToSupportPlane( glm::vec3 const & p,int t ) { return sqrt( squareDistanceToSupportPlane(p,t) ); }
        bool isParallelTo( Line const & L,int t ) {
            bool result=false;glm::vec3 m_normal=this->normal[t];
            if(glm::dot(m_normal,L.direction())==0)result=true;
            return result;
        }
        glm::vec3 getIntersectionPointWithSupportPlane( Line const & L,int t ) {
            glm::vec3 result;glm::vec3 m_normal=this->normal[t];glm::vec3 m_c[3]{this->vertices_Espace[this->triangles[t][0]],this->vertices_Espace[this->triangles[t][1]],this->vertices_Espace[this->triangles[t][2]]};
            if(isParallelTo(L,t)==false){
                float denominator =glm::dot(L.direction(),m_normal);
                float t = ((glm::dot(m_c[1]-m_c[0],m_normal)-(glm::dot(L.origin(),m_normal)))/denominator);
                result=L.origin()+t*L.direction();
            }
            return result;
        }
        void computeBarycentricCoordinates( glm::vec3 const & p , float & u0 , float & u1 , float & u2, int t ) {
            glm::vec3 m_normal=this->normal[t];glm::vec3 m_c[3]{this->vertices_Espace[this->triangles[t][0]],this->vertices_Espace[this->triangles[t][1]],this->vertices_Espace[this->triangles[t][2]]};
            glm::vec3 v0 = m_c[1] - m_c[0];
            glm::vec3 v1 = m_c[2] - m_c[0];
            glm::vec3 v2 = p - m_c[0];
            float d00 = glm::dot(v0, v0);
            float d01 = glm::dot(v0, v1);
            float d11 = glm::dot(v1, v1);
            float d20 = glm::dot(v2, v0);
            float d21 = glm::dot(v2, v1);
            float denom = d00 * d11 - d01 * d01;
            u1 = (d11 * d20 - d01 * d21) / denom;
            u2 = (d00 * d21 - d01 * d20) / denom;
            u0 = 1.0f - u1 - u2;
        }

        RayTriangleIntersection getIntersection( Ray const & ray, int t ) {
            glm::vec3 m_normal=this->normal[t];
            glm::vec3 m_c[3]{this->vertices_Espace[this->triangles[t][0]],this->vertices_Espace[this->triangles[t][1]],this->vertices_Espace[this->triangles[t][2]]};
            // std::cout<<m_c[0][0]<<" "<<m_c[0][1]<<" "<<m_c[0][2]<<std::endl;
            // std::cout<<m_c[1][0]<<" "<<m_c[1][1]<<" "<<m_c[1][2]<<std::endl;
            // std::cout<<m_c[2][0]<<" "<<m_c[2][1]<<" "<<m_c[2][2]<<std::endl;
            RayTriangleIntersection result;
            bool parallel=isParallelTo(ray,t);
            if(parallel){
                result.t=FLT_MAX;
                result.intersectionExists=false;
                return result;
            }
            float denominator =glm::dot(ray.direction(),m_normal);
            float ta = ((glm::dot(m_c[0] - ray.origin(),m_normal))/denominator);

            glm::vec3 intersectionPoint = ray.origin()+(float)ta*ray.direction();
            if(t<=0){
                result.t = FLT_MAX;
                result.intersectionExists = false;
                return result;
            }
            float a,b,g;
            computeBarycentricCoordinates(intersectionPoint, a, b, g,t);
            if(a<0||b<0||g<0||a>1||b>1||g>1){
                result.t=FLT_MAX;
                result.intersectionExists = false;
                return result;
            }
            result.intersectionExists=true;
            result.intersection=intersectionPoint;
            result.t = ta;     
            result.w0=a;
            result.w1=b;
            result.w2=g;
            result.normal=m_normal;
            return result;
        }

        bool triangleIntersection(const glm::vec3& A1, const glm::vec3& B1, const glm::vec3& C1,
                        const glm::vec3& A2, const glm::vec3& B2, const glm::vec3& C2) {
            // Exemple simplifié : vérifier si les triangles se chevauchent
            // Vous pouvez utiliser un algorithme plus précis ici
            glm::vec3 normal1 = glm::normalize(glm::cross(B1 - A1, C1 - A1));
            glm::vec3 normal2 = glm::normalize(glm::cross(B2 - A2, C2 - A2));

            // Vérifier si les sommets du triangle 2 sont du même côté du plan du triangle 1
            float d1 = glm::dot(normal1, A1);
            bool allOutside1 = (glm::dot(normal1, A2) - d1 > 0) &&
                    (glm::dot(normal1, B2) - d1 > 0) &&
                    (glm::dot(normal1, C2) - d1 > 0);

            // Vérifier si les sommets du triangle 1 sont du même côté du plan du triangle 2
            float d2 = glm::dot(normal2, A2);
            bool allOutside2 = (glm::dot(normal2, A1) - d2 > 0) &&
                    (glm::dot(normal2, B1) - d2 > 0) &&
                    (glm::dot(normal2, C1) - d2 > 0);

            // Si les triangles ne sont pas complètement en dehors l'un de l'autre, ils s'intersectent
            return !(allOutside1 || allOutside2);
            }
            glm::vec3 getMin() const {
                glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());
                for (const auto& vertex : this->vertices_Espace) {
                    min.x = std::min(min.x, vertex.x);
                    min.y = std::min(min.y, vertex.y);
                    min.z = std::min(min.z, vertex.z);
                }
                return min;
            }
            
            glm::vec3 getMax() const {
                glm::vec3 max = glm::vec3(std::numeric_limits<float>::lowest());
                for (const auto& vertex : this->vertices_Espace) {
                    max.x = std::max(max.x, vertex.x);
                    max.y = std::max(max.y, vertex.y);
                    max.z = std::max(max.z, vertex.z);
                }
                return max;
            }

        bool collisionCheck(const Mesh& other, float tolerance = 0.0f) {
            for (int i = 0; i < this->triangles.size(); ++i) {
                for (int j = 0; j < other.triangles.size(); ++j) {
                    // On vérifie l'intersection de chaque paire de triangles
                    if (triangleIntersection(this->vertices_Espace[this->triangles[i][0]],
                                            this->vertices_Espace[this->triangles[i][1]],
                                            this->vertices_Espace[this->triangles[i][2]],
                                            other.vertices_Espace[other.triangles[j][0]],
                                            other.vertices_Espace[other.triangles[j][1]],
                                            other.vertices_Espace[other.triangles[j][2]])) {
                        return true;  // Si les triangles s'intersectent, collision détectée
                    }
                }
            }

            return false;  // Si aucune intersection n'est trouvée, il n'y a pas de collision
        }

};

