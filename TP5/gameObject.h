
#include <TP5/Plane.h>
#include <TP5/Transform.h>

class GameObject{
    public:
        // Construction de l'objet dans la scène : 
        GameObject *parent;
        std::vector<GameObject*> enfant;
        std::vector<GameObject*> objetsOBJ;
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
        }

        void draw(const glm::vec3 cameraPosition, float deltaTime){
            GLuint scaleUniformID = glGetUniformLocation(programID, "scale");
            if (isMoving) {
                if (speed != glm::vec3(0.0, 0.0, 0.0))
                    PhysicMove(deltaTime);
        
                if (hasMesh || M) {
                    if (hasLOD) {
                        updateLOD(cameraPosition);
                    }
                    this->mesh.draw();
                }
                if (hasPlan) {
                    glUniform1f(scaleUniformID, this->transform.s);
        
                    if (this->plan.hasHeightMap) {
                        this->plan.drawHM();
                    } else {
                        this->plan.draw();
                    }
                }
                for (int i = 0; i < this->enfant.size(); i++) {
                    this->enfant[i]->draw(cameraPosition, deltaTime);
                }
            }
        }
        

        void setLODMeshes(const char * pathToObj = "../mesh/suzanne.off", bool isPBR = false, const char* pathToText = "../texture/2k_moon.jpg") {
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


        // void avancer(float deltaTime, vitesse = this->speed){ 
        //     glm::vec2 intersect = this->parent->plan.intersection(this->globalTransform.t, glm::vec3(0.0,-1.0,0.0), this->parent->transform.s);
        //     float height = this->parent->plan.getHeightAtUV(intersect, this->parent->transform.s);
        //     std::cout<<this->height2parent<<std::endl;
        //     this->setGlobalTransform(Transform(this->globalTransform.m,glm::vec3(this->globalTransform.t[0], this->height2parent + height, this->globalTransform.t[2]+(vitesse*deltaTime)),this->globalTransform.s));
        // }

        // void reculer(float deltaTime, vitesse = this->speed){ 
        //     glm::vec2 intersect = this->parent->plan.intersection(this->globalTransform.t, glm::vec3(0.0,-1.0,0.0), this->parent->transform.s);
        //     float height = this->parent->plan.getHeightAtUV(intersect, this->parent->transform.s);
        //     this->setGlobalTransform(Transform(this->globalTransform.m,glm::vec3(this->globalTransform.t[0],  this->height2parent + height, this->globalTransform.t[2]-(vitesse*deltaTime)),this->globalTransform.s));
        // }

        // void droite(float deltaTime, vitesse = this->speed){ 
        //     glm::vec2 intersect = this->parent->plan.intersection(this->globalTransform.t, glm::vec3(0.0,-1.0,0.0), this->parent->transform.s);
        //     float height = this->parent->plan.getHeightAtUV(intersect, this->parent->transform.s);
        //     this->setGlobalTransform(Transform(this->globalTransform.m,glm::vec3(this->globalTransform.t[0]-(vitesse*deltaTime),  this->height2parent + height, this->globalTransform.t[2]),this->globalTransform.s));
        // }

        // void gauche(float deltaTime, vitesse = this->speed){
        //     glm::vec2 intersect = this->parent->plan.intersection(this->globalTransform.t, glm::vec3(0.0,-1.0,0.0), this->parent->transform.s);
        //     float height = this->parent->plan.getHeightAtUV(intersect, this->parent->transform.s);
        //     this->setGlobalTransform(Transform(this->globalTransform.m,glm::vec3(this->globalTransform.t[0]+(vitesse*deltaTime),  this->height2parent + height, this->globalTransform.t[2]),this->globalTransform.s));
        // }

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
                for(int i=0;i<e->mesh.indexed_vertices.size();i++){
                    // std::cout<<e->mesh.vertices[i][0]<<" "<<e->mesh.vertices[i][1]<<" "<<e->mesh.vertices[i][2]<<std::endl;
                }
            }
            this->objetsOBJ.push_back(e);
            this->enfant.push_back(e);
        }

        void setTransform(Transform t){
            this->transform = t;
            this->mesh.setVerticesEspace(this->globalTransform.combine_with(this->transform));
        }
        void setEspace(Transform t){
            this->globalTransform=t;
            this->mesh.setVerticesEspace(this->globalTransform.combine_with(this->transform));
            for(auto& i :this->enfant){
                    i->setEspace(i->globalTransform.combine_with(this->globalTransform));
            }
        }
};
