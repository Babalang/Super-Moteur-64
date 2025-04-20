// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

#include <TP5/camera.h>


class Scene{
    public : 
    GameObject root;
    Camera camera;
    GLuint programID;
    std::vector<GameObject> objetsOBJ;
    std::vector<GameObject*> lights;
        Scene(){}

    void animation(float deltaTime){

        GameObject* Map = this->root.enfant[0];
        GameObject* Obj = Map->enfant[0];    

        if(camera.mode == CAMERA_MODE::CLASSIC){
            camera.setGlobalTransform(Obj->globalTransform.combine_with(Transform(camera.transform.m,glm::vec3(0.0,1.0,-1.0),1.0)));
        }

    }

    void draw(float elapsedTime){
        for(auto& i : this->lights){
            i->draw(camera.globalTransform.t, elapsedTime);
        }
        this->animation(elapsedTime);
        this->root.draw(camera.globalTransform.t, elapsedTime);
    }

    void rajouterOBJ(GameObject *go){
        for(int i=0;i<objetsOBJ.size();i++){
            objetsOBJ[i].mesh.programID=programID;
            objetsOBJ[i].mesh.creerTextureOBJ(objetsOBJ[i].mesh.mtl.texture);
            go->addEnfantOBJ(&objetsOBJ[i]);
        }
    }
    bool lireOBJ(const char* filename, GameObject *go){
        printf("Loading OBJ file %s...\n", filename);
        GameObject *GOA; GameObject goa;goa.M=true;
        goa.setTransform(Transform(glm::mat3x3(1.0),glm::vec3(0.0,0.0,0.0),1.0));
        goa.setEspace(Transform(glm::mat3x3(1.0),glm::vec3(0.0,0.0,0.0),1.0));
        unsigned short nb=0;
        std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
        std::vector<glm::vec3> temp_vertices;
        std::vector<glm::vec2> temp_uvs;
        std::vector<glm::vec3> temp_normals;
        bool Factuel=false;
        int nbTriangles=0;
        FILE * file = fopen(filename, "r");
        if( file == NULL ){
            printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
            getchar();
            return false;
        }
        while( 1 ){
            char lineHeader[128];
            // read the first word of the line
            int res = fscanf(file, "%s", lineHeader);
            if (res == EOF){
                if(Factuel){
                    for( unsigned int i=nb; i<vertexIndices.size(); i++ ){
                        unsigned int vertexIndex = vertexIndices[i];
                        unsigned int uvIndex = uvIndices[i];
                        unsigned int normalIndex = normalIndices[i];
                        glm::vec3 vertex = temp_vertices[ vertexIndex-1 ];
                        glm::vec2 uv = temp_uvs[ uvIndex-1 ];
                        glm::vec3 normal = temp_normals[ normalIndex-1 ];
                        goa.mesh.indexed_vertices.push_back(vertex);
                        goa.mesh.texCoords     .push_back(uv);
                        goa.mesh.normal .push_back(normal);
                    }
                    for( unsigned short i=nb; i<vertexIndices.size(); i+=3 ){
                        unsigned short a=(unsigned short)(vertexIndices[i])-nb-1;
                        unsigned short b=(unsigned short)(vertexIndices[i+1])-nb-1;
                        unsigned short c=(unsigned short)(vertexIndices[i+2])-nb-1;
                        std::vector<unsigned short> ind{a,b,c};
                        goa.mesh.triangles.push_back(ind);
                        nbTriangles++;
                        goa.mesh.indices.push_back(a);
                        goa.mesh.indices.push_back(b);
                        goa.mesh.indices.push_back(c);
                    }
                    objetsOBJ.push_back(goa);
                    nb=vertexIndices.size();
                }
                this->rajouterOBJ(go);
                break; // EOF = End Of File. Quit the loop.
            }
            else if(Factuel && strcmp(lineHeader, "f")!=0){
                Factuel=false;
                for( unsigned int i=nb; i<vertexIndices.size(); i++ ){
                    unsigned int vertexIndex = vertexIndices[i];
                    unsigned int uvIndex = uvIndices[i];
                    unsigned int normalIndex = normalIndices[i];
                    glm::vec3 vertex = temp_vertices[ vertexIndex-1 ];
                    glm::vec2 uv = temp_uvs[ uvIndex-1 ];
                    glm::vec3 normal = temp_normals[ normalIndex-1 ];
                    goa.mesh.indexed_vertices.push_back(vertex);
                    goa.mesh.texCoords     .push_back(uv);
                    goa.mesh.normal .push_back(normal);
                }
                for( unsigned short i=nb; i<vertexIndices.size(); i+=3 ){
                    unsigned short a=(unsigned short)(vertexIndices[i])-nb-1;
                    unsigned short b=(unsigned short)(vertexIndices[i+1])-nb-1;
                    unsigned short c=(unsigned short)(vertexIndices[i+2])-nb-1;
                    std::vector<unsigned short> ind{a,b,c};
                    goa.mesh.triangles.push_back(ind);
                    nbTriangles++;
                    goa.mesh.indices.push_back(a);
                    goa.mesh.indices.push_back(b);
                    goa.mesh.indices.push_back(c);
                }
                objetsOBJ.push_back(goa);
                nb=vertexIndices.size();
                goa.enfant.clear();
                goa.mesh.indexed_vertices.clear();goa.mesh.indices.clear();goa.mesh.triangles.clear();goa.mesh.normal.clear();goa.mesh.texCoords.clear();
            }
            // else : parse lineHeader
            else if ( strcmp( lineHeader, "mtllib" ) == 0 ){
                char mtllib[100];
                fscanf(file, "%s\n", mtllib );
                std::string s="../meshes/";
                s+=mtllib;
                go->lireMTL(s.c_str());
            }else if ( strcmp( lineHeader, "o" ) == 0 ){
                char nom[250];
                fscanf(file, "%s\n", nom );
                goa.nom=std::string(nom);
            }else if ( strcmp( lineHeader, "usemtl" ) == 0 ){
                char usemtl[250];
                fscanf(file, "%s\n", usemtl );
                go->trouverMTL(usemtl, &goa);
            }else if ( strcmp( lineHeader, "v" ) == 0 ){
                glm::vec3 vertex;
                fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
                temp_vertices.push_back(vertex);
            }else if ( strcmp( lineHeader, "s" ) == 0 ){
                char s[250];
                fscanf(file, "%s\n", s );
            }else if ( strcmp( lineHeader, "vt" ) == 0 ){
                glm::vec2 uv;
                fscanf(file, "%f %f\n", &uv.x, &uv.y );
                uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
                temp_uvs.push_back(uv);
            }else if ( strcmp( lineHeader, "vn" ) == 0 ){
                glm::vec3 normal;
                fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
                temp_normals.push_back(normal);
            }else if ( strcmp( lineHeader, "f" ) == 0 ){
                Factuel=true;
                std::string vertex1, vertex2, vertex3;
                unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
                int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2] );
                if (matches == 1){
                    matches = fscanf(file, "%d//%d %d//%d %d//%d\n", &vertexIndex[0], &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2] );
                    uvIndex[0]=1;
                    uvIndex[1]=1;
                    uvIndex[2]=1;
                }
                else if (matches != 9){
                    printf("File can't be read by our simple parser :-( Try exporting with other options\n");
                    fclose(file);
                    return false;
                }
                vertexIndices.push_back(vertexIndex[0]);
                vertexIndices.push_back(vertexIndex[1]);
                vertexIndices.push_back(vertexIndex[2]);
                uvIndices    .push_back(uvIndex[0]);
                uvIndices    .push_back(uvIndex[1]);
                uvIndices    .push_back(uvIndex[2]);
                normalIndices.push_back(normalIndex[0]);
                normalIndices.push_back(normalIndex[1]);
                normalIndices.push_back(normalIndex[2]);
            }else{
                // Probably a comment, eat up the rest of the line
                char stupidBuffer[10000];
                fgets(stupidBuffer, 1000, file);
            }

        }
        // For each vertex of each triangle
        
        fclose(file);
        std::cout<<"fini de charger le fichier OBJ !"<<std::endl;
        return true;
    }

};
