
#include <TP5/Plane.h>
#include <TP5/Transform.h>

class GameObject{
    public:
        // Construction de l'objet dans la scène : 
        GameObject *parent;
        std::vector<GameObject*> enfant;
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
                child->highMesh.programID=this->programID;
                child->medMesh.programID=this->programID;
                child->lowMesh.programID=this->programID;
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
                this->highMesh.setVerticesEspace(tmp);
                this->medMesh.setVerticesEspace(tmp);
                this->lowMesh.setVerticesEspace(tmp);
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
            if(isMoving){
                GLuint PBRboolUniformID = glGetUniformLocation(programID, "isPBR");
                if(speed != glm::vec3(0.0,0.0,0.0)) PhysicMove(deltaTime);
                if(hasMesh){
                    glUniform1i(glGetUniformLocation(programID, "useHeightMap"), 0);
                    if(this->mesh.isPBR == true){
                        glUniform1i(PBRboolUniformID,1);
                    } else {glUniform1i(PBRboolUniformID,0);}
                    updateLOD(cameraPosition);
                    this->mesh.draw();
                    
                }
                if(hasPlan){
                    glUniform1i(PBRboolUniformID,0);
                    glUniform1f(glGetUniformLocation(programID, "scale"), this->transform.s);
                    if(this->plan.hasHeightMap){
                        this->plan.drawHM();
                    } else {
                        glUniform1i(glGetUniformLocation(programID, "useHeightMap"), 0);
                        this->plan.draw();
                    }
                }
                for(int i=0;i<this->enfant.size();i++){
                    this->enfant[i]->draw(cameraPosition,deltaTime);
                }
            }
        }

        void setLODMeshes(const char * pathToObj = "../mesh/suzanne.off", bool isPBR = false, const char* pathToText = "../texture/2k_moon.jpg") {
            if(isPBR){
                std::string basePath(pathToText);
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
            }
            hasMesh = true;

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
};
