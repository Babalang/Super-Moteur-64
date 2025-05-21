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

// #include <assimp/scene.h>       // pour aiScene, aiNode, aiAnimation...
// #include <glm/glm.hpp>          // glm pour les maths (vec3, mat4, quat)
// #include <glm/gtc/matrix_transform.hpp>
// #include <glm/gtc/quaternion.hpp>
// #include <glm/gtx/quaternion.hpp>
// #include <map>
// #include <string>
// #include <array>
// #include <future>

// #include <assimp/Importer.hpp>
// #include <assimp/postprocess.h>

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
        // std::vector<std::array<int,4>> BoneIDs;
        // std::vector<std::array<float,4>> Weights;
        // std::map<std::string, glm::mat4> boneOffsets;
        // std::map<int, std::string> boneIDToName;

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
            normal.clear();
            normal.resize(indexed_vertices.size());
            for(int i=0;i<indexed_vertices.size();i++){
                normal[i] = glm::vec3(0.0,0.0,0.0);
            }
            for(size_t i=0; i<triangles.size();i++){
                for(size_t y = 0;y<3;y++){
                    if(normal[triangles[i][y]]==vec3(0.,0.,0.)){
                        normal[triangles[i][y]]= glm::cross(indexed_vertices[triangles[i][abs(y-1)%3]]-indexed_vertices[triangles[i][y]], indexed_vertices[triangles[i][(y+1)%3]]-indexed_vertices[triangles[i][y]] );
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

        void draw(bool test = true){
            if(this->programID == 0){
                std::cout<<"Erreur de chargement du shader !"<<std::endl;
                return;
            }
            glUseProgram(this->programID);
            glGenBuffers(1,&this->vertexbuffer);
            glBindBuffer(GL_ARRAY_BUFFER,this->vertexbuffer);
            if(test) {glBufferData(GL_ARRAY_BUFFER,this->vertices_Espace.size()*sizeof(glm::vec3),&this->vertices_Espace[0],GL_STATIC_DRAW);}
            else {glBufferData(GL_ARRAY_BUFFER,this->indexed_vertices.size()*sizeof(glm::vec3),&this->indexed_vertices[0],GL_STATIC_DRAW);}

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
                if(s=="../textures/koopa_all.png" || s=="../textures/wood.png" || s=="../textures/wall_grass_b.png" || s=="../textures/wall_grass_a.png" || s=="../textures/tree_shadow.png" || s=="../textures/tonnel_grass.png" || s=="../textures/tonnel.png" || s=="../textures/start_grass.png" || s=="../textures/start.png" || s=="../textures/soil.png" || s=="../textures/slope_soil.png" || s=="../textures/slope_grass.png" || s=="../textures/slope.png" || s=="../textures/roller.png" || s=="../textures/rock_cannon.png" || s=="../textures/rock_b.png" || s=="../textures/rock_a_grass.png" || s=="../textures/rock_a.png" || s=="../textures/grass2.png" || s=="../textures/flower_leaf.png" || s=="../textures/flower.png" || s=="../textures/fence_thorn.png" || s=="../textures/fence.png" || s=="../textures/way.png" || s=="../textures/kuribo_cmp4.png" || s=="../textures/battan_king_face.png" || s=="../textures/battan_king_back_1.png" || s=="../textures/ookan.png" || s=="../textures/houdai_cmp4.png" || s=="../textures/kinopio.png"){
                    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
                    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
                }else{
                    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                }
                glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
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

        bool triangleIntersection(const glm::vec3& A1, const glm::vec3& B1, const glm::vec3& C1,const glm::vec3& A2, const glm::vec3& B2, const glm::vec3& C2){
            glm::vec3 normal1 = glm::normalize(glm::cross(B1 - A1, C1 - A1));
            glm::vec3 normal2 = glm::normalize(glm::cross(B2 - A2, C2 - A2));
            float d1 = glm::dot(normal1, A1);
            bool allOutside1 = (glm::dot(normal1, A2) - d1 > 0) &&
                    (glm::dot(normal1, B2) - d1 > 0) &&
                    (glm::dot(normal1, C2) - d1 > 0);
            float d2 = glm::dot(normal2, A2);
            bool allOutside2 = (glm::dot(normal2, A1) - d2 > 0) &&
                    (glm::dot(normal2, B1) - d2 > 0) &&
                    (glm::dot(normal2, C1) - d2 > 0);
            return !(allOutside1 || allOutside2);
            }

            int trianglePointIntersection(const glm::vec3& A1, const glm::vec3& B1, const glm::vec3& C1, glm::vec3 A2){
                glm::vec3 normal1 = glm::normalize(glm::cross(B1 - A1, C1 - A1));  
                float d1 = glm::dot(normal1,A2 - A1);
                const float epsilon = 1e-5f;
                if(std::abs(d1)>epsilon) return 1;
                return -1;
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

            bool isPointInsideMesh(const glm::vec3& point) {
                glm::vec3 direction = glm::normalize(glm::vec3(1.0f, 0.373f, 0.179f));
                int intersectionCount = 0;

                for (size_t i = 0; i < this->indices.size(); i += 3) {
                    glm::vec3 v0 = this->vertices_Espace[this->indices[i]];
                    glm::vec3 v1 = this->vertices_Espace[this->indices[i + 1]];
                    glm::vec3 v2 = this->vertices_Espace[this->indices[i + 2]];

                    float t, u, v;
                    if (rayIntersectsTriangle(point, direction, v0, v1, v2, t, u, v)) {
                        if (t >= 0.0f)
                            intersectionCount++;
                    }
                }
                return (intersectionCount % 2) == 1;
            }



        bool rayIntersectsTriangle(const glm::vec3& origin,const glm::vec3& dir,const glm::vec3& v0,const glm::vec3& v1,const glm::vec3& v2,float& t, float& u, float& v) {
            const float EPSILON = 1e-6f;
            glm::vec3 edge1 = v1 - v0;
            glm::vec3 edge2 = v2 - v0;
            glm::vec3 h = glm::cross(dir, edge2);
            float a = glm::dot(edge1, h);

            if (fabs(a) < EPSILON)
                return false; // Rayon parallèle au triangle

            float f = 1.0f / a;
            glm::vec3 s = origin - v0;
            u = f * glm::dot(s, h);

            if (u < 0.0f || u > 1.0f)
                return false;

            glm::vec3 q = glm::cross(s, edge1);
            v = f * glm::dot(dir, q);

            if (v < 0.0f || u + v > 1.0f)
                return false;

            t = f * glm::dot(edge2, q);
            return t > EPSILON;
        }

        

};

// class AnimatedModel {
//     public:
//         const aiScene* baseScene = nullptr;
//         std::vector<Mesh> baseMeshes;
//         std::map<std::string, int> boneNameToIndex;
//         std::vector<glm::mat4> boneOffsets;
//         std::vector<glm::mat4> finalBoneMatrices;
//         std::map<std::string, std::vector<const aiAnimation*>> animations;
//         std::map<std::string, std::vector<std::vector<glm::mat4>>> bakedAnimations;
//         glm::mat4 globalInverseTransform;
//         float scale;

    
//     glm::mat4 aiMat4ToGlm(const aiMatrix4x4& from) {
//         glm::mat4 to;
//         to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
//         to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
//         to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
//         to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
//         return to;
//     }


//         aiVector3D CalcInterpolatedValueFromKey(float animationTime, const int numKeys, const aiVectorKey* const vectorKey) const
//         {
//             aiVector3D ret;
//             if (numKeys == 1)
//             {
//                 ret = vectorKey[0].mValue;
//                 return ret;
//             }

//             unsigned int keyIndex = FindKeyIndex(animationTime, numKeys, vectorKey);
//             unsigned int nextKeyIndex = keyIndex + 1;

//             assert(nextKeyIndex < numKeys);

//             float deltaTime = vectorKey[nextKeyIndex].mTime - vectorKey[keyIndex].mTime;
//             float factor = (animationTime - (float)vectorKey[keyIndex].mTime) / deltaTime;

//             assert(factor >= 0.0f && factor <= 1.0f);

//             const aiVector3D& startValue = vectorKey[keyIndex].mValue;
//             const aiVector3D& endValue = vectorKey[nextKeyIndex].mValue;

//             ret.x = startValue.x + (endValue.x - startValue.x) * factor;
//             ret.y = startValue.y + (endValue.y - startValue.y) * factor;
//             ret.z = startValue.z + (endValue.z - startValue.z) * factor;

//             return ret;
//         }

//         aiQuaternion CalcInterpolatedValueFromKey(float animationTime, const int numKeys, const aiQuatKey* const quatKey) const
//         {
//             aiQuaternion ret;
//             if (numKeys == 1)
//             {
//                 ret = quatKey[0].mValue;
//                 return ret;
//             }

//             unsigned int keyIndex = FindKeyIndex(animationTime, numKeys, quatKey);
//             unsigned int nextKeyIndex = keyIndex + 1;

//             assert(nextKeyIndex < numKeys);

//             float deltaTime = quatKey[nextKeyIndex].mTime - quatKey[keyIndex].mTime;
//             float factor = (animationTime - (float)quatKey[keyIndex].mTime) / deltaTime;

//             assert(factor >= 0.0f && factor <= 1.0f);

//             const aiQuaternion& startValue = quatKey[keyIndex].mValue;
//             const aiQuaternion& endValue = quatKey[nextKeyIndex].mValue;
//             aiQuaternion::Interpolate(ret, startValue, endValue, factor);
//             ret = ret.Normalize();

//             return ret;
//         }

//         unsigned int FindKeyIndex(const float animationTime, const int numKeys, const aiVectorKey* const vectorKey) const
//         {
//             assert(numKeys > 0);
//             for (int i = 0; i < numKeys - 1; i++)
//                 if (animationTime < (float)vectorKey[i + 1].mTime)
//                     return i;
//             return numKeys - 1;
//         }

//         unsigned int FindKeyIndex(const float animationTime, const int numKeys, const aiQuatKey* const quatKey) const
//         {
//             assert(numKeys > 0);
//             for (int i = 0; i < numKeys - 1; i++)
//                 if (animationTime < (float)quatKey[i + 1].mTime)
//                     return i;
//             return numKeys - 1;
//         }


//         const aiNodeAnim* FindNodeAnim(const aiAnimation* animation, const std::string& nodeName) {
//             for (unsigned int i = 0; i < animation->mNumChannels; i++) {
//                 const aiNodeAnim* nodeAnim = animation->mChannels[i];
//                 if (std::string(nodeAnim->mNodeName.C_Str()) == nodeName) {
//                     return nodeAnim;
//                 }
//             }
//             return nullptr;
//         }
    
//         Mesh processMesh(aiMesh* mesh) {
//             Mesh result;
//             for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
//                 aiVector3D pos = mesh->mVertices[i];
//                 result.indexed_vertices.push_back(glm::vec3(pos.x, pos.y, pos.z));
    
//                 if (mesh->HasNormals()) {
//                     aiVector3D n = mesh->mNormals[i];
//                     result.normal.push_back(glm::vec3(n.x, n.y, n.z));
//                 } else {
//                     result.normal.push_back(glm::vec3(0.0f));
//                 }
    
//                 if (mesh->mTextureCoords[0]) {
//                     aiVector3D uv = mesh->mTextureCoords[0][i];
//                     result.texCoords.push_back(glm::vec2(uv.x, uv.y));
//                 } else {
//                     result.texCoords.push_back(glm::vec2(0.0f));
//                 }
//             }
//             for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
//                 aiFace face = mesh->mFaces[i];
//                 for (unsigned int j = 0; j < face.mNumIndices; j++) {
//                     result.indices.push_back(static_cast<unsigned short>(face.mIndices[j]));
//                 }
//             }
//             result.BoneIDs.resize(result.indexed_vertices.size(), {-1, -1, -1, -1});
//             result.Weights.resize(result.indexed_vertices.size(), {0.f, 0.f, 0.f, 0.f});
//             return result;
//         }
    
//     void ReadNodeHierarchy(float animationTime, const aiNode* node, const glm::mat4& parentTransform, std::map<std::string, glm::mat4>& boneTransforms) {
//         std::string nodeName(node->mName.data);
//         const aiAnimation* animation = baseScene->mAnimations[0];
//         glm::mat4 nodeTransform;

//         const aiNodeAnim* nodeAnim = FindNodeAnim(animation, nodeName);
//         if (node == baseScene->mRootNode || nodeName.find("Hips") != std::string::npos) {
//             // Pour le root node et les hanches : garder seulement rotation et scale
//             if (nodeAnim) {
//                 const aiVector3D& scaling = CalcInterpolatedValueFromKey(animationTime, nodeAnim->mNumScalingKeys, nodeAnim->mScalingKeys);
//                 const aiQuaternion& rotationQ = CalcInterpolatedValueFromKey(animationTime, nodeAnim->mNumRotationKeys, nodeAnim->mRotationKeys);
                
//                 glm::mat4 scalingM = glm::scale(glm::mat4(1.0f), glm::vec3(scaling.x, scaling.y, scaling.z));
//                 glm::mat4 rotationM = glm::toMat4(glm::quat(rotationQ.w, rotationQ.x, rotationQ.y, rotationQ.z));
                
//                 if (node == baseScene->mRootNode) {
//                     scalingM = glm::scale(glm::mat4(1.0f), glm::vec3(this->scale)); // Scale global voulu
//                 }
                
//                 nodeTransform = rotationM * scalingM; // Pas de translation
//             } else {
//                 if (node == baseScene->mRootNode) {
//                     nodeTransform = glm::scale(glm::mat4(1.0f), glm::vec3(this->scale));
//                 } else {
//                     glm::mat4 transform = aiMat4ToGlm(node->mTransformation);
//                     transform[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); // Annule la translation
//                     nodeTransform = transform;
//                 }
//             }
//         }
//         else {
//             if (nodeAnim) {
//                 const aiVector3D& scaling = CalcInterpolatedValueFromKey(animationTime, nodeAnim->mNumScalingKeys, nodeAnim->mScalingKeys);
//                 const aiQuaternion& rotationQ = CalcInterpolatedValueFromKey(animationTime, nodeAnim->mNumRotationKeys, nodeAnim->mRotationKeys);
//                 const aiVector3D& translation = CalcInterpolatedValueFromKey(animationTime, nodeAnim->mNumPositionKeys, nodeAnim->mPositionKeys);

//                 glm::mat4 scalingM = glm::scale(glm::mat4(1.0f), glm::vec3(scaling.x, scaling.y, scaling.z));
//                 glm::mat4 rotationM = glm::toMat4(glm::quat(rotationQ.w, rotationQ.x, rotationQ.y, rotationQ.z));
//                 glm::mat4 translationM = glm::translate(glm::mat4(1.0f), glm::vec3(translation.x, translation.y, translation.z));

//                 nodeTransform = translationM * rotationM * scalingM;
//             } else {
//                 nodeTransform = aiMat4ToGlm(node->mTransformation);
//             }
//         }
//         glm::mat4 globalTransform = parentTransform * nodeTransform;
//         boneTransforms[nodeName] = globalTransform;

//         for (unsigned int i = 0; i < node->mNumChildren; i++) {
//             ReadNodeHierarchy(animationTime, node->mChildren[i], globalTransform, boneTransforms);
//         }
//     }
//         void loadAnimatedModel(const std::string& path, float scale = 1.0f) {
//             this->scale = scale;
//             Assimp::Importer importer;
//             baseScene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
//             std::cout << "Nombre de meshes : " << baseScene->mNumMeshes << std::endl;
//             std::cout << "Nombre d’animations : " << baseScene->mNumAnimations << std::endl;
//             for (unsigned int i = 0; i < baseScene->mNumMeshes; i++) {
//                 std::cout << "Mesh " << i << " contient " << baseScene->mMeshes[i]->mNumVertices << " vertices" << std::endl;
//             }
//             if (!baseScene || !baseScene->mRootNode) {
//                 std::cerr << "Scene or root node is null!" << std::endl;
//                 return;
//             }
//             if (baseScene->mNumAnimations == 0 || !baseScene->mAnimations[0]) {
//                 std::cerr << "No animation found in model." << std::endl;
//                 return;
//             }
//             this->globalInverseTransform = glm::inverse(aiMat4ToGlm(baseScene->mRootNode->mTransformation));
//             if (!baseScene->HasAnimations()) {
//                 std::cerr << "⚠️ Aucune animation dans le fichier DAE !" << std::endl;
//             } else {
//                 std::cout << "Root node name: " << baseScene->mRootNode->mName.C_Str() << std::endl;
//                 std::cout << "Root node children: " << baseScene->mRootNode->mNumChildren << std::endl;
//                 std::cout << "Nombre d'animations : " << baseScene->mNumAnimations << std::endl;
//                 std::cout << "Frames (duration): " << baseScene->mAnimations[0]->mDuration << std::endl;
//                 std::cout << "Ticks per second: " << baseScene->mAnimations[0]->mTicksPerSecond << std::endl;
//                 PrintNodeHierarchy(baseScene->mRootNode);  
//             }
//             for (unsigned int i = 0; i < baseScene->mNumMeshes; i++) {
//                 aiMesh* mesh = baseScene->mMeshes[i];
//                 Mesh processedMesh = processMesh(mesh);
//                 for (unsigned int b = 0; b < mesh->mNumBones; b++) {
//                     std::string boneName = mesh->mBones[b]->mName.C_Str();
//                     if (boneNameToIndex.find(boneName) == boneNameToIndex.end()) {
//                         int newIndex = (int)boneNameToIndex.size();
//                         boneNameToIndex[boneName] = newIndex;
//                         glm::mat4 offset = aiMat4ToGlm(mesh->mBones[b]->mOffsetMatrix);
//                         boneOffsets.push_back(offset);
//                     }
//                     int boneIndexGlobal = boneNameToIndex[boneName];
//                     float test =0.0f;
//                     for (unsigned int w = 0; w < mesh->mBones[b]->mNumWeights; w++) {
//                         unsigned int vertexID = mesh->mBones[b]->mWeights[w].mVertexId;
//                         float weight = mesh->mBones[b]->mWeights[w].mWeight;
//                         auto& boneIDs = processedMesh.BoneIDs[vertexID];
//                         auto& weights = processedMesh.Weights[vertexID];
//                         for (int k = 0; k < 4; ++k) {
//                             if (boneIDs[k] == -1) {
//                                 boneIDs[k] = boneIndexGlobal;
//                                 weights[k] = weight;
//                                 test+=weight;
//                                 break;
//                             }
//                         }
                        
//                     }
//                     for (unsigned int v = 0; v < processedMesh.Weights.size(); v++) {
//                         float sumWeights = 0.0f;
//                         for (int k = 0; k < 4; ++k) {
//                             sumWeights += processedMesh.Weights[v][k];
//                         }
//                     }
//                 }
//                 baseMeshes.push_back(processedMesh);
//             }
//             finalBoneMatrices.resize(boneOffsets.size(), glm::mat4(1.0f));
//             if (baseScene->mNumAnimations > 0) {
//                 for (unsigned int i = 0; i < baseScene->mNumAnimations; i++) {
//                     animations[path].push_back(baseScene->mAnimations[i]);
//                 }
//             }
//             const float targetFPS = 24.0f;
//             for (unsigned int i = 0; i < baseScene->mNumAnimations; i++) {
//                 const aiAnimation* animation = baseScene->mAnimations[i];
//                 float ticksPerSecond = animation->mTicksPerSecond != 0 ? animation->mTicksPerSecond : 25.0f;
//                 float durationInSeconds = animation->mDuration / ticksPerSecond;
//                 std::vector<std::vector<glm::mat4>> bakedFrames;
//                 for (float t = 0.0f; t < durationInSeconds; t += 1.0f / targetFPS) {
//                     float animationTime = fmod(t * ticksPerSecond, animation->mDuration);
//                     std::map<std::string, glm::mat4> boneOffsetsMap;
//                     for (const auto& [name, index] : boneNameToIndex) {
//                         boneOffsetsMap[name] = boneOffsets[index];
//                     }
//                     std::map<std::string, glm::mat4> boneTransforms;
//                     if (baseScene->mRootNode == nullptr) {
//                         std::cerr << "Warning: child node " << i << " is null" << std::endl;
//                         continue;
//                     }
//                     ReadNodeHierarchy(animationTime, baseScene->mRootNode, glm::mat4(1.0f),boneTransforms);
                    
//                     std::vector<glm::mat4> finalBones(boneOffsets.size(), glm::mat4(1.0f));
//                     for (auto& [boneName, index] : boneNameToIndex) {
//                         if (boneTransforms.find(boneName) != boneTransforms.end()) {
//                             finalBones[index] = boneTransforms[boneName] * boneOffsets[index];
//                         } else {
//                             finalBones[index] = glm::mat4(1.0f);
//                         }
//                     }
//                     bakedFrames.push_back(finalBones);
//                 }
                
//                 bakedAnimations[path] = bakedFrames;
//             }
//         }
// void addAnimation(const std::string& path) {
//     // Garder l'importeur en vie pendant toute l'exécution de la fonction
//     static Assimp::Importer importer;
//     const aiScene* animScene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    
//     if (!animScene || !animScene->HasAnimations()) {
//         std::cerr << "❌ No animation found in file: " << path << std::endl;
//         return;
//     }

//     if (!baseScene || !baseScene->mRootNode) {
//         std::cerr << "❌ Base model scene is invalid!" << std::endl;
//         return;
//     }

//     try {
//         // Traiter une seule animation à la fois
//         const aiAnimation* animation = animScene->mAnimations[0];
        
//         // S'assurer que l'animation est valide
//         if (!animation || animation->mDuration <= 0) {
//             std::cerr << "❌ Invalid animation data" << std::endl;
//             return;
//         }
        
//         // Stocker l'animation
//         animations[path].push_back(animation);
            
//         float ticksPerSecond = animation->mTicksPerSecond != 0 ? animation->mTicksPerSecond : 25.0f;
//         float durationInSeconds = animation->mDuration / ticksPerSecond;
//         const float targetFPS = 24.0f;
            
//         std::vector<std::vector<glm::mat4>> bakedFrames;
//         size_t numFrames = static_cast<size_t>(durationInSeconds * targetFPS);
//         bakedFrames.reserve(numFrames);
        
//         // Bake frames
//         for (float t = 0.0f; t < durationInSeconds; t += 1.0f / targetFPS) {
//             float animationTime = fmod(t * ticksPerSecond, animation->mDuration);
//             std::map<std::string, glm::mat4> boneTransforms;
                
//             // S'assurer que le root node existe toujours
//             if (!baseScene->mRootNode) {
//                 throw std::runtime_error("Root node became invalid");
//             }
                
//             ReadNodeHierarchy(animationTime, baseScene->mRootNode, glm::mat4(1.0f), boneTransforms);
                
//             std::vector<glm::mat4> finalBones(boneOffsets.size(), glm::mat4(1.0f));
                
//             // Appliquer les transformations seulement pour les os existants
//             for (const auto& bone : boneNameToIndex) {
//                 if (boneTransforms.count(bone.first) > 0) {
//                     finalBones[bone.second] = boneTransforms[bone.first] * boneOffsets[bone.second];
//                 }
//             }
                
//             bakedFrames.push_back(std::move(finalBones));
//         }
            
//         bakedAnimations[path] = std::move(bakedFrames);
        
//         std::cout << "✅ Added animation from: " << path << std::endl;
//     }
//     catch (const std::exception& e) {
//         std::cerr << "❌ Error while processing animation: " << e.what() << std::endl;
//     }
// }
//         void UpdateAnimation(float timeInSeconds, size_t animIndex, const std::string& animPath) {
//             if (!baseScene || !baseScene->mRootNode) {
//                 std::cerr << "Scene or root node is null!" << std::endl;
//                 return;
//             }
//             if (baseScene->mNumAnimations == 0 || !baseScene->mAnimations[0]) {
//                 std::cerr << "No animation found in model." << std::endl;
//                 return;
//             }
//             if (animIndex >= animations[animPath].size()) return;
//             const aiAnimation* animation = animations[animPath][animIndex];
//             float ticksPerSecond = animation->mTicksPerSecond != 0 ? animation->mTicksPerSecond : 25.0f;
//             float timeInTicks = timeInSeconds * ticksPerSecond;
//             float animationTime = fmod(timeInTicks, animation->mDuration);
//             std::map<std::string, glm::mat4> boneOffsetsMap;
//             for (const auto& [name, index] : boneNameToIndex) {
//                 boneOffsetsMap[name] = boneOffsets[index];
//             }
//             std::map<std::string, glm::mat4> boneTransforms;
//             ReadNodeHierarchy(animationTime, baseScene->mRootNode, glm::mat4(1.0f), boneTransforms);
//             for (auto& [boneName, index] : boneNameToIndex) {
//                 if (boneTransforms.find(boneName) != boneTransforms.end()) {
//                     finalBoneMatrices[index] = boneTransforms[boneName] * boneOffsets[index];
//                 } else {
//                     std::cout << "Bone non trouvé dans boneTransforms : " << boneName << std::endl;
//                     finalBoneMatrices[index] = glm::mat4(1.0f);
//                 }
//             }

//         }
//         void DrawAnimatedModel(GLuint shaderProgram, int frameIndex, float animTime, const std::string& animPath, const Transform modelMatrix) {
//             if (bakedAnimations.find(animPath) == bakedAnimations.end()) return;
//             const auto& frames = bakedAnimations[animPath];
//             int frameCount = (int)frames.size();
//             if (frameCount == 0) return;
//             frameIndex = frameIndex % frameCount;
//             finalBoneMatrices = frames[frameIndex];
//             GLint boneLoc = glGetUniformLocation(shaderProgram, "u_BoneMatrices");
//             if (boneLoc == -1) {
//                 std::cerr << "Uniform 'u_BoneMatrices' not found in shader!" << std::endl;
//             }
//             const auto& boneMats = bakedAnimations[animPath][frameIndex];
//             glUniformMatrix4fv(boneLoc, (GLsizei)boneMats.size(), GL_FALSE, glm::value_ptr(boneMats[0]));
//             GLint isAnimatedLoc = glGetUniformLocation(shaderProgram, "isAnimated");
//             glUniform1i(isAnimatedLoc, 1);
//             glm::mat4 model = modelMatrix.toMat4();
//             for (auto& mesh : baseMeshes) {
//                 GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
//                 glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
//                 GLuint boneIDVBO, weightsVBO;       
//                 glGenBuffers(1, &boneIDVBO);
//                 glBindBuffer(GL_ARRAY_BUFFER, boneIDVBO);
//                 glBufferData(GL_ARRAY_BUFFER, mesh.BoneIDs.size() * sizeof(glm::ivec4), &mesh.BoneIDs[0], GL_STATIC_DRAW);
//                 glEnableVertexAttribArray(7);
//                 glVertexAttribIPointer(7, 4, GL_INT, sizeof(glm::ivec4), (void*)0);
//                 glGenBuffers(1, &weightsVBO);
//                 glBindBuffer(GL_ARRAY_BUFFER, weightsVBO);
//                 glBufferData(GL_ARRAY_BUFFER, mesh.Weights.size() * sizeof(glm::vec4), &mesh.Weights[0], GL_STATIC_DRAW);
//                 glEnableVertexAttribArray(8);
//                 glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
//                 mesh.draw(false);
//                 glDisableVertexAttribArray(7);
//                 glDisableVertexAttribArray(8);
//                 glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
//                 glDeleteBuffers(1, &boneIDVBO);
//                 glDeleteBuffers(1, &weightsVBO);
//             }
//             glUniform1i(isAnimatedLoc, 0);
//         }

//         void PrintNodeHierarchy(const aiNode* node, int depth = 0) {
//             for (int i = 0; i < depth; ++i) std::cout << "  ";
//             std::cout << node->mName.C_Str() << std::endl;
//             for (unsigned int i = 0; i < node->mNumChildren; ++i) {
//                 PrintNodeHierarchy(node->mChildren[i], depth + 1);
//             }
//         }
    
// };