
#include <TP5/Plane.h>
#include <TP5/Transform.h>

class GameObject{
    public:
        // Construction de l'objet dans la scène : 
        GameObject *parent;
        std::vector<GameObject*> enfant;
        std::vector<GameObject> objetsOBJ;
        std::vector<GameObject*> objetsOBJtmp;
        // Placement de l'objet
        Mesh mesh;
        Transform transform;
        Transform globalTransform;
        bool hasMesh = false;
        GLuint programID;
        // Si l'objet est un plan
        Plane plan;
        bool hasPlan = false;
        // Distance au parent
        float height2parent = 0.0f;
        // Gestion LOD 
        bool hasLOD = false;
        Mesh highMesh;
        Mesh medMesh;
        Mesh lowMesh;
        float lodHighDistance = 2.0f;
        float lodMediumDistance = 5.0f;
        // Physique de l'objet
        glm::vec3 speed = glm::vec3(0.0f);
        float poids = 1.0f;
        glm::vec3 Force = glm::vec3(0.0,poids * -9.83,0.0);
        glm::vec3 acceleration =Force/poids;
        std::vector<MTL> mtls;
        std::string nom;
        bool M=false;

        // light
        int index = 0;
        bool isLight = false;
        float lightIntensity = 1.0f;
        glm::vec3 lightColor = glm::vec3(1.0f,1.0f,1.0f);
        
        bool isMoving = true;

        GameObject() : parent(nullptr), programID(0) {}

        void setChild(const std::vector<GameObject*> enfants){
            this->enfant.clear();
            for(auto& i : enfants){
                this->enfant.push_back(i);
            }
        }
        void addChild(GameObject* child){
            child->programID = this->programID;
            if(child->hasMesh){
                if(child->hasLOD){
                    child->highMesh.programID=this->programID;
                    child->medMesh.programID=this->programID;
                    child->lowMesh.programID=this->programID;
                } else {
                    child->mesh.programID=this->programID;
                }
            }
            if(child->hasPlan){
                child->plan.programID=this->programID;
            }
            child->setGlobalTransform(this->globalTransform);
            this->enfant.push_back(child);
            child->parent = this;
        }

        void setParent(GameObject *Parent){this->parent = Parent; Parent->addChild(this);}

        void setMesh(const Mesh& newMesh) {
            this->mesh = newMesh;
            this->hasMesh = true;
        }

        void setPlan(Plane p){this->plan = p ; this->hasPlan = true;}

        void setGlobalTransform(Transform t){
            this->globalTransform=t;
            if(hasMesh) {
                Transform tmp = this->globalTransform.combine_with(this->transform);
                if(hasLOD){
                    this->highMesh.setVerticesEspace(tmp);
                    this->medMesh.setVerticesEspace(tmp);
                    this->lowMesh.setVerticesEspace(tmp);
                } else {
                    this->mesh.setVerticesEspace(tmp);
                }
            }
            if(hasPlan) this->plan.setVerticesEspace(this->globalTransform.combine_with(this->transform));
            for(auto& i :this->enfant){
                i->setGlobalTransform(this->globalTransform);
            }
        }

        void setLocalTransform(Transform t){
            this->transform = t;
            if(hasMesh) {
                Transform tmp = this->globalTransform.combine_with(this->transform);
                this->highMesh.setVerticesEspace(tmp);
                this->medMesh.setVerticesEspace(tmp);
                this->lowMesh.setVerticesEspace(tmp);
            }
            if(hasPlan) this->plan.setVerticesEspace(this->globalTransform.combine_with(this->transform));
            for(auto& i :this->objetsOBJ){
                i.setLocalTransform(this->transform);
            }
        }

        void draw(const glm::vec3 cameraPosition, float deltaTime) {
            GLuint scaleUniformID = glGetUniformLocation(programID, "scale");
            if (isMoving) {
                if (speed != glm::vec3(0.0, 0.0, 0.0))
                    PhysicMove(deltaTime);
        
                if (hasMesh || M) {
                    if (hasLOD) {
                        updateLOD(cameraPosition);
                    }
                    this->mesh.draw();
                    //std::cout << "Dessin de l'objet : " << nom << std::endl;
                } else if (hasPlan) {
                    glUniform1f(scaleUniformID, this->transform.s);
        
                    if (this->plan.hasHeightMap) {
                        this->plan.drawHM();
                    } else {
                        this->plan.draw();
                    }
                }
                if(isLight){
                    std::string lightPositionName = "LightPositions[" + std::to_string(this->index) + "]";
                    GLuint LightID = glGetUniformLocation(programID, lightPositionName.c_str());
                    glUniform3f(LightID, this->globalTransform.t[0], this->globalTransform.t[1], this->globalTransform.t[2]);
                    std::string lightColorName = "lightColors[" + std::to_string(this->index) + "]";
                    GLuint LightColorUniformID = glGetUniformLocation(programID, lightColorName.c_str());
                    glUniform3f(LightColorUniformID, this->lightColor[0], this->lightColor[1], this->lightColor[2]);
                }
                for (int i = 0; i < this->enfant.size(); i++) {
                    //std::cout << "Dessin de l'enfant : " << enfant[i]->nom << std::endl;
                    this->enfant[i]->draw(cameraPosition, deltaTime);
                }
                for (int i = 0; i < this->objetsOBJ.size(); i++) {
                    //std::cout << "Dessin de l'enfant : " << enfant[i]->nom << std::endl;
                    this->objetsOBJ[i].draw(cameraPosition, deltaTime);
                }
            }
        }
        

        void setLODMeshes(const char * pathToObj = "../meshes/suzanne.off", bool isPBR = false, const char* pathToText = "../textures/2k_moon.jpg") {
            std::string basePath(pathToText);
            if(isPBR){
                Mesh highRes(pathToObj, "", isPBR);
                Mesh mediumRes(pathToObj, "", isPBR);
                Mesh lowRes(pathToObj, "", isPBR);
                highRes.loadPBR((basePath + "albedo.png").c_str(),(basePath + "normal.png").c_str(),(basePath + "roughness.png").c_str(),(basePath + "metallic.png").c_str(),(basePath + "ao.png").c_str());
                mediumRes.loadPBR((basePath + "albedo.png").c_str(),(basePath + "normal.png").c_str(),(basePath + "roughness.png").c_str(),(basePath + "metallic.png").c_str(),(basePath + "ao.png").c_str());
                lowRes.loadPBR((basePath + "albedo.png").c_str(),(basePath + "normal.png").c_str(),(basePath + "roughness.png").c_str(),(basePath + "metallic.png").c_str(),(basePath + "ao.png").c_str());
                mediumRes.simplify(15);
                lowRes.simplify(5);
                this->highMesh = highRes;
                this->medMesh = mediumRes;
                this->lowMesh = lowRes;
                this->highMesh.programID = this->programID;
                this->medMesh.programID = this->programID;
                this->lowMesh.programID = this->programID;
                highRes.compute_Normals();
                mediumRes.compute_Normals();
                lowRes.compute_Normals();
            } else{
                Mesh highRes(pathToObj, pathToText,isPBR);
                Mesh mediumRes(pathToObj, pathToText, isPBR);
                Mesh lowRes(pathToObj, pathToText,isPBR);
                mediumRes.simplify(15);
                lowRes.simplify(5);
                this->highMesh = highRes;
                this->medMesh = mediumRes;
                this->lowMesh = lowRes;
                this->highMesh.programID = this->programID;
                this->medMesh.programID = this->programID;
                this->lowMesh.programID = this->programID;
                highRes.compute_Normals();
                mediumRes.compute_Normals();
                lowRes.compute_Normals();
            }
            hasMesh = true;
            hasLOD = true;


        }

        void updateLOD(const glm::vec3& cameraPosition) {
            float distance = glm::length(globalTransform.t - cameraPosition);
        
            if (distance < lodHighDistance) {
                setMesh(highMesh);
            } else if (distance < lodMediumDistance) {
                setMesh(medMesh);
            } else {
                setMesh(lowMesh);
            }
        }

        void Move(float deltaTime, glm::vec3 axe, float vitesse = 1.0f){
            float height = 0.0f;
            if(this->parent->hasPlan){
                glm::vec2 intersect = this->parent->plan.intersection(this->globalTransform.t, glm::vec3(0.0,-1.0,0.0), this->parent->transform.s);
                height = this->parent->plan.getHeightAtUV(intersect, this->parent->transform.s);
            }
            Transform Mov = Transform(this->globalTransform.m, this->globalTransform.t + axe*(vitesse*deltaTime), this->globalTransform.s);
            if(Mov.t[1]<this->height2parent+height){
                Mov.t[1] = this->height2parent+height;
            } else if(speed == glm::vec3(0.0)) speed = glm::vec3(0.0,-1.0,0.0);
            this->setGlobalTransform(Mov);
            isMoving = true;
        }

        void PhysicMove(float deltaTime){
            float height=0.0f;
            if(this->parent && this->parent->hasPlan){
                glm::vec2 intersect = this->parent->plan.intersection(this->globalTransform.t, glm::vec3(0.0,-1.0,0.0), this->parent->transform.s);
                height = this->parent->plan.getHeightAtUV(intersect, this->parent->transform.s);
            }
            Transform Mov = Transform(this->globalTransform.m, this->globalTransform.t + (speed * deltaTime), this->globalTransform.s);
            speed = speed + (acceleration*deltaTime);
            if(Mov.t[1]<this->height2parent+height){
                Mov.t[1] = this->height2parent+height;
                speed = glm::vec3(0.0f);
            }
            this->setGlobalTransform(Mov);
            std::cout<<this->globalTransform.t[0]<<" "<<this->globalTransform.t[1]<<" "<<this->globalTransform.t[2]<<std::endl;
            isMoving = true;
        }


        void trouverMTL(const char* mat, GameObject *go){
            // std::cout<<"taille des mtls : "<<this->mtls.size()<<std::endl;
            // std::cout<<"mat : "<<mat<<std::endl;
            for(int i=0;i<mtls.size();i++){
                // std::cout<<"nom : "<<this->mtls[i].nom<<std::endl;
                if(this->mtls[i].nom==mat){
                    // std::cout<<"trouve !"<<std::endl;
                    go->mesh.mtl=mtls[i];
                }
            }
        }
        bool lireMTL(const char* fichier){
            printf("Loading MTL file %s...\n", fichier);
            FILE * file = fopen(fichier, "r");
            if( file == NULL ){
                getchar();
                return false;
            }
            MTL mtl;
            bool debut=false;
            while( 1 ){
                char lineHeader[128];
                int res = fscanf(file, "%s", lineHeader);
                if (res == EOF){
                    this->mtls.push_back(mtl);
                    break;
                }
                if ( strcmp( lineHeader, "newmtl" ) == 0 ){
                    if(debut) this->mtls.push_back(mtl);
                    else debut=true;
                    char nom[250];
                    fscanf(file, "%s\n", nom );
                    mtl.nom=std::string(nom);
                }else if ( strcmp( lineHeader, "Ns" ) == 0 ){
                    float ns;
                    fscanf(file, "%f\n", &ns );
                    mtl.ns=ns;
                }else if ( strcmp( lineHeader, "Ka" ) == 0 ){
                    glm::vec3 Ka;
                    fscanf(file, "%f %f %f\n", &Ka.x, &Ka.y, &Ka.z );
                    mtl.ka=Ka;
                }else if ( strcmp( lineHeader, "Kd" ) == 0 ){
                    glm::vec3 Kd;
                    fscanf(file, "%f %f %f\n", &Kd.x, &Kd.y, &Kd.z );
                    mtl.kd=Kd;
                }else if ( strcmp( lineHeader, "Ks" ) == 0 ){
                    glm::vec3 Ks;
                    fscanf(file, "%f %f %f\n", &Ks.x, &Ks.y, &Ks.z );
                    mtl.ks=Ks;
                }else if ( strcmp( lineHeader, "Ke" ) == 0 ){
                    glm::vec3 Ke;
                    fscanf(file, "%f %f %f\n", &Ke.x, &Ke.y, &Ke.z );
                    mtl.ke=Ke;
                }else if ( strcmp( lineHeader, "Ni" ) == 0 ){
                    float ni;
                    fscanf(file, "%f\n", &ni );
                    mtl.ns=ni;
                }else if ( strcmp( lineHeader, "d" ) == 0 ){
                    float d;
                    fscanf(file, "%f\n", &d );
                    mtl.d=d;
                }else if ( strcmp( lineHeader, "illum" ) == 0 ){
                    int illum;
                    fscanf(file, "%d\n", &illum );
                    mtl.illum=illum;
                }else if( strcmp( lineHeader, "map_Kd" ) == 0 ){
                    char map_Kd[250];
                    fscanf(file, "%s\n", map_Kd );
                    std::string chemin="../textures/";
                    chemin+=map_Kd;
                    char* cstr = new char[chemin.size() + 1];
                    std::copy(chemin.c_str(), chemin.c_str() + chemin.size() + 1, cstr);
                    mtl.texture=cstr;
                }else{
                    // Probably a comment, eat up the rest of the line
                    char stupidBuffer[1000];
                    fgets(stupidBuffer, 1000, file);
                }
            }
            std::cout<<"fini de charger le fichier MTL !"<<std::endl;
            return true;
        }
        void addEnfantOBJ(GameObject* e){
            e->programID=this->programID;
            if(e->M){
                e->mesh.programID=programID;
            }
            this->enfant.push_back(e);
            e->hasMesh = true;
            e->parent=this;
            e->setGlobalTransform(this->globalTransform);
            e->setLocalTransform(e->transform);
        }
        void addEnfantOBJ2(GameObject* e){
            e->programID=this->programID;
            if(e->M){
                e->mesh.programID=programID;
            }
            this->objetsOBJtmp.push_back(e);
            e->hasMesh = true;
            e->parent=this;
            e->setGlobalTransform(this->globalTransform);
            e->setLocalTransform(e->transform);
        }

        void rajouterOBJ() {
            for (int i = 0; i < objetsOBJ.size(); i++) {
                objetsOBJ[i].programID = this->programID;
                objetsOBJ[i].mesh.programID = programID;
                objetsOBJ[i].hasMesh = true;
                objetsOBJ[i].M = true;
                objetsOBJ[i].mesh.creerTextureOBJ(objetsOBJ[i].mesh.mtl.texture);
                this->addEnfantOBJ(&objetsOBJ[i]);
                // this->addEnfantOBJ2(&objetsOBJ[i]);
            }
            std::cout << "Nombre d'objets ajoutés : " << objetsOBJ.size() << std::endl;
        }
        bool lireOBJ(const char* filename){
            this->M = false; this->hasMesh = false; this->hasPlan = false;
            printf("Loading OBJ file %s...\n", filename);
            GameObject goa;goa.M=true;goa.programID=this->programID;
            goa.setLocalTransform(Transform(glm::mat3x3(1.0),glm::vec3(0.0,0.0,0.0),1.0));
            goa.setGlobalTransform(Transform(glm::mat3x3(1.0),glm::vec3(0.0,0.0,0.0),1.0));
            unsigned short nb=0;
            std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
            std::vector<glm::vec3> temp_vertices;
            std::vector<glm::vec2> temp_uvs;
            std::vector<glm::vec3> temp_normals;
            std::vector<unsigned short> nbTrianglesActu;
            bool Factuel=false;
            int nbTriangles=0;
            nbTrianglesActu.push_back(nbTriangles);
            std::string nomOBJ="";
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
                        for( unsigned short i=nb; i<vertexIndices.size(); i++ ){
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
                        for( unsigned short i=0; i<goa.mesh.indexed_vertices.size(); i+=3 ){
                            unsigned short a=(unsigned short)(i+2);
                            unsigned short b=(unsigned short)(i+1);
                            unsigned short c=(unsigned short)(i);
                            std::vector<unsigned short> ind{a,b,c};
                            goa.mesh.triangles.push_back(ind);
                            nbTriangles++;
                            goa.mesh.indices.push_back(a);
                            goa.mesh.indices.push_back(b);
                            goa.mesh.indices.push_back(c);
                        }
                        // this->objetsOBJ.push_back(goa);
                        objetsOBJ.push_back(goa);
                        // std::cout<<"nombre de traingles : "<<goa.mesh.triangles.size()<<std::endl;
                        // std::cout<<"nombre de vertices : "<<goa.mesh.indexed_vertices.size()<<std::endl;
                        nb=vertexIndices.size();
                    }
                    this->rajouterOBJ();
                    break; // EOF = End Of File. Quit the loop.
                }
                else if(Factuel && strcmp(lineHeader, "f")!=0){
                    Factuel=false;
                    unsigned short nba=0;
                    for( unsigned short i=nb; i<vertexIndices.size(); i++ ){
                        unsigned int vertexIndex = vertexIndices[i];
                        // std::cout<<"i : "<<i<<std::endl;
                        unsigned int uvIndex = uvIndices[i];
                        unsigned int normalIndex = normalIndices[i];
                        glm::vec3 vertex = temp_vertices[ vertexIndex-1 ];
                        glm::vec2 uv = temp_uvs[ uvIndex-1 ];
                        glm::vec3 normal = temp_normals[ normalIndex-1 ];
                        goa.mesh.indexed_vertices.push_back(vertex);
                        goa.mesh.texCoords     .push_back(uv);
                        goa.mesh.normal .push_back(normal);
                        // if(vertexIndices[i]>temp_vertices.size()){
                        //     std::cout<<"iiiiiiiiiiiiiiiiiiiiiiiiiiiii"<<std::endl;
                        // }
                        // std::cout<<"vertex : "<<vertexIndices[i]<<std::endl;
                        // std::cout<<"vertices : "<<vertex.x<<" "<<vertex.y<<" "<<vertex.z<<std::endl;
                        nba++;
                    }
                    for( unsigned short i=0; i<goa.mesh.indexed_vertices.size(); i+=3 ){
                        unsigned short a,b,c;
                        if(nomOBJ=="Metal_Mario.mtl" || nomOBJ=="Mario64.mtl" || nomOBJ=="Mario64_Cap.mtl"){
                            a=i;
                            b=i+1;
                            c=i+2;
                        }else{
                            a=(unsigned short)(vertexIndices[i+nb])-nb-1;
                            b=(unsigned short)(vertexIndices[i+1+nb])-nb-1;
                            c=(unsigned short)(vertexIndices[i+2+nb])-nb-1;
                        }
                        // std::cout<<"a : "<<a<<" b : "<<b<<" c : "<<c<<std::endl;
                        std::vector<unsigned short> ind{a,b,c};
                        goa.mesh.triangles.push_back(ind);
                        nbTriangles++;
                        goa.mesh.indices.push_back(a);
                        goa.mesh.indices.push_back(b);
                        goa.mesh.indices.push_back(c);
                    }
                    objetsOBJ.push_back(goa);
                    nb+=nba;
                    // std::cout<<"nbVertices : "<<goa.mesh.indexed_vertices.size()<<std::endl;
                    // std::cout<<"nb : "<<nb<<std::endl;
                    nbTrianglesActu.push_back(nbTriangles*3);
                    goa.enfant.clear();
                    goa.mesh.indexed_vertices.clear();
                    goa.mesh.indices.clear();
                    goa.mesh.triangles.clear();
                    goa.mesh.normal.clear();
                    goa.mesh.texCoords.clear();
                }
                // else : parse lineHeader
                else if ( strcmp( lineHeader, "mtllib" ) == 0 ){
                    char mtllib[100];
                    fscanf(file, "%s\n", mtllib );
                    std::string s="../meshes/";
                    s+=mtllib;
                    nomOBJ+=mtllib;
                    this->lireMTL(s.c_str());
                }else if ( strcmp( lineHeader, "o" ) == 0 ){
                    char nom[250];
                    fscanf(file, "%s\n", nom );
                    goa.nom=std::string(nom);
                }else if ( strcmp( lineHeader, "usemtl" ) == 0 ){
                    char usemtl[250];
                    fscanf(file, "%s\n", usemtl );
                    this->trouverMTL(usemtl, &goa);
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
