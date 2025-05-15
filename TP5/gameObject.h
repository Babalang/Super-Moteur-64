
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
        Transform initialTransform = Transform(glm::mat3(1.0f), glm::vec3(0.0f), 1.0f);
        glm::vec3 axe;
        glm::vec3 frontAxe;
        glm::vec3 rightAxe;
        glm::vec3 upAxe = glm::vec3(0.0f,1.0f,0.0f);
        std::vector<MTL> mtls;
        std::string nom;
        bool M=false;
        int tailleObjetOBJ=0;
        bool isBoiteEnglobante=true;
        glm::vec3 centre;
        glm::vec3 centreEspace;
        glm::vec3 bas;
        glm::vec3 basEspace;
        glm::vec3 rayonDepart;
        glm::mat3x3 rotationDepart;
        bool isGravite=false;
        bool aTerre=false;
        // Collision
        bool isCollision=false;
        Mesh boiteEnglobante;
        bool isGround = false;
        std::vector<GameObject*> collisions;
        bool map=false;
        bool auSol=true;
        int pv;
        std::string collisionChateau;
        Transform transformSol;
        int nbCollision=-1;
        std::vector<GameObject> koopaOBJ;
        std::vector<GameObject> shellOBJ;
        glm::vec3 directionCarapace;
        bool carapaceRespawn=false;
        std::vector<GameObject*> stars;
        bool changementDuNiveau=false;

        // light
        int index = 0;
        bool isLight = false;
        float lightIntensity = 1.0f;
        glm::vec3 lightColor = glm::vec3(1.0f,1.0f,1.0f);
        Ray visionIA;
        bool isIA=false;
        bool avancer=false;
        glm::vec3 positionAvance;
        
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
            child->setGlobalTransform(this->globalTransform.combine_with(child->globalTransform));
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
            Transform worldTransform = this->globalTransform.combine_with(this->transform);
            if(hasMesh) {
                if(hasLOD){
                    this->highMesh.setVerticesEspace(worldTransform);
                    this->medMesh.setVerticesEspace(worldTransform);
                    this->lowMesh.setVerticesEspace(worldTransform);
                } else {
                    this->mesh.setVerticesEspace(worldTransform);
                }
            }
            if(hasPlan) this->plan.setVerticesEspace(worldTransform);
            if(isBoiteEnglobante){
                this->boiteEnglobante.setVerticesEspace(worldTransform);
                this->centreEspace=worldTransform.applyToPoint(this->centre);
                this->basEspace=worldTransform.applyToPoint(this->bas);
            }
            int index = 0;
            for(auto& i :this->enfant){
                if(index <this->objetsOBJ.size()){
                    i->setGlobalTransform(this->globalTransform);
                } else {
                    Transform combinedTransform = this->globalTransform.combine_with(i->transform);
                    i->setGlobalTransform(combinedTransform);
                }
                index++;
            }
        }

        void setLocalTransform(Transform t){
            this->transform = t;
            if(hasMesh) {
                Transform tmp = this->globalTransform.combine_with(this->transform);
                if(hasLOD){
                    this->highMesh.setVerticesEspace(tmp);
                    this->medMesh.setVerticesEspace(tmp);
                    this->lowMesh.setVerticesEspace(tmp);
                } else {
                    this->mesh.setVerticesEspace(tmp);
                }
                if(isBoiteEnglobante){
                    this->boiteEnglobante.setVerticesEspace(tmp);
                    this->centreEspace=tmp.applyToPoint(this->centre);
                    this->basEspace=tmp.applyToPoint(this->bas);
                }
            }
            if(hasPlan) this->plan.setVerticesEspace(this->globalTransform.combine_with(this->transform));
            for(int i = 0; i < this->objetsOBJ.size(); i++){
                    Transform childWorld = this->transform;
                    this->enfant[i]->setLocalTransform(childWorld);
            }
                            
        }

        void draw(const glm::vec3 cameraPosition, float deltaTime) {
            GLuint scaleUniformID = glGetUniformLocation(programID, "scale");
            if (isMoving) {
                if (speed != glm::vec3(0.0, 0.0, 0.0)){
                    if(!isCollision)PhysicMove(deltaTime);
                }
                if (hasMesh || M) {
                    if (hasLOD) {
                        updateLOD(cameraPosition);
                    }
                    this->mesh.draw();
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
                    this->enfant[i]->draw(cameraPosition, deltaTime);
                    this->enfant[i]->isCollision=this->isCollision;
                }
                for (int i = 0; i < this->objetsOBJ.size(); i++) {
                    this->objetsOBJ[i].draw(cameraPosition, deltaTime);
                    this->objetsOBJ[i].isCollision=this->isCollision;
                }
            }
            if(avancer){
                this->moveToPosition(deltaTime);
            }
        }

        void testIA(GameObject *obj, float deltaTime){
            if(this->isIA){
                if(this->pv<=0){
                    if(this->nom=="koopa"){
                        this->avancer=false;
                        this->clearGameObject();
                        this->nom="shell";
                        this->isIA=true;
                        this->globalTransform=this->globalTransform.combine_with(Transform(glm::mat3x3(1.0),glm::vec3(0.0,0.0,0.0),4.0));
                        this->lireOBJ("../meshes/Shell.obj");
                        this->rajouterOBJ();
                        this->setGlobalTransform(this->globalTransform);
                        this->pv=1;
                    }else if(this->nom=="shell"){
                        this->bougeCarapace(deltaTime);
                        if(this->carapaceRespawn){
                            std::cout<<this->carapaceRespawn<<std::endl;
                            this->clearGameObject();
                            this->nom="koopa";
                            this->isIA=true;
                            this->globalTransform=this->globalTransform.combine_with(Transform(glm::mat3x3(1.0),glm::vec3(0.0,0.0,0.0),0.25));
                            this->lireOBJ("../meshes/Koopa.obj");
                            this->rajouterOBJ();
                            this->globalTransform.t[1]+=0.1;
                            this->setGlobalTransform(this->globalTransform);
                            this->pv=1;
                            this->creerIA();
                            this->carapaceRespawn=false;
                            this->nbCollision=-1;
                        }
                    }else if(this->nom=="Bowser"){
                        obj->collisions[0]->enfant.pop_back();
                        obj->collisions[0]->enfant.pop_back();
                        obj->collisions[0]->enfant.pop_back();
                        obj->collisions.pop_back();
                        obj->collisions.pop_back();
                        obj->collisions.pop_back();
                        obj->collisions[0]->stars[0]->programID=this->programID;
                        obj->collisions[0]->stars[0]->lireOBJ("../meshes/star.obj");
                        obj->collisions[0]->stars[0]->rajouterOBJ();
                        obj->collisions[0]->stars[0]->setGlobalTransform(Transform(glm::mat3x3(1.0),glm::vec3(0.0,120.0,0.0),1.0));
                        obj->collisions[0]->stars[0]->nom="star";
                        obj->collisions[0]->stars[0]->collisions.push_back(obj->collisions[0]);
                        obj->collisions[0]->stars[0]->collisions.push_back(obj);
                        obj->collisions[0]->addChild(obj->collisions[0]->stars[0]);
                        obj->collisions.push_back(obj->collisions[0]->stars[0]);
                    }
                }else{
                    if(this->nom!="shell"){
                        for(int i=0;i<this->boiteEnglobante.triangles.size();i++){
                            RayTriangleIntersection intersection = obj->boiteEnglobante.getIntersection(this->visionIA,i);
                            if(intersection.intersectionExists && intersection.t<500.0f && intersection.t>0.0f){
                                this->avancer=true;
                            }
                        }
                    }
                }
            }else{
                for(int i=0;i<this->enfant.size();i++){
                    this->enfant[i]->testIA(obj,deltaTime);
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

        void Move(float deltaTime, float vitesse = 5.0f){
            if(changementDuNiveau){
                this->axe=glm::vec3(0.0);
                this->frontAxe=glm::vec3(0.0);
                this->rightAxe=glm::vec3(0.0);
                this->speed=glm::vec3(0.0);
            }
            float height = 0.0f;
            if(this->parent && this->parent->hasPlan){
                glm::vec2 intersect = this->parent->plan.intersection(this->globalTransform.t, glm::vec3(0.0,-1.0,0.0), this->parent->transform.s);
                height = this->parent->plan.getHeightAtUV(intersect, this->parent->transform.s);
            }
            glm::vec3 NormalizedAxe = glm::length(frontAxe+rightAxe) > 0.0f ? glm::normalize(frontAxe+rightAxe) : frontAxe+rightAxe;
            axe = NormalizedAxe;
            glm::vec3 movement = NormalizedAxe * (vitesse * this->globalTransform.s * deltaTime);
            Transform Mov = Transform(this->globalTransform.m, this->globalTransform.t + movement, this->globalTransform.s);
            if (glm::length(axe) > 0.0f) {
                glm::vec3 projectedAxe = glm::normalize(glm::vec3(axe.x, 0.0f, axe.z));
                glm::vec3 reference = glm::vec3(0.0f, 0.0f, 1.0f);
                float dotProduct = glm::dot(projectedAxe, reference);
                float angleRadians = glm::acos(glm::clamp(dotProduct, -1.0f, 1.0f));
                float angleSignedRadians = glm::atan(projectedAxe.x, projectedAxe.z);
                float angleDegrees = glm::degrees(angleSignedRadians);
                Transform rotation = Transform();
                rotation = Transform().rotation(glm::vec3(0.0f, 1.0f, 0.0f), angleDegrees);
                Transform transformTmp = Transform(rotation.m*initialTransform.m, transform.t, transform.s);
                setLocalTransform(transformTmp);
            }
            Transform ancien=this->globalTransform;
            this->setGlobalTransform(Mov);
            int col=this->getCollision();
            if(col>=0){
                axe=glm::vec3(axe[0],1.0,axe[2]);
                glm::vec3 NormalizedAxe = glm::length(axe) > 0.0f ? glm::normalize(axe) : axe;
                glm::vec3 movement = axe * (vitesse * this->globalTransform.s * deltaTime);
                Transform Mov = Transform(this->globalTransform.m, this->globalTransform.t + movement, this->globalTransform.s);   
                this->setGlobalTransform(Mov);
                col=this->getCollision();
                if(col>=0){
                    this->setGlobalTransform(ancien);
                    isCollision=true;
                }
                isCollision=false;
                axe=glm::vec3(axe[0],0.0,axe[2]);
            }else{
                isMoving = true;
            }
            if(ancien.t[0]!=this->globalTransform.t[0] || ancien.t[1]!=this->globalTransform.t[1] || ancien.t[2]!=this->globalTransform.t[2]){
                this->auSol=false;
            }
            if(this->getCollision()==-1 && !this->auSol){
                this->PhysicMove(deltaTime);
            }
            if(changementDuNiveau){
                this->axe=glm::vec3(0.0);
                this->speed=glm::vec3(0.0);
                this->changementDuNiveau=false;
            }
        }
        

        void PhysicMove(float deltaTime){
            float height=-10.0f;
            this->isGravite=true;
            if(this->parent && this->parent->hasPlan){
                glm::vec2 intersect = this->parent->plan.intersection(this->globalTransform.t, glm::vec3(0.0,-1.0,0.0), this->parent->transform.s);
                height = this->parent->plan.getHeightAtUV(intersect, this->parent->transform.s);
            }
            Transform Mov = Transform(this->globalTransform.m, this->globalTransform.t + (speed * deltaTime), this->globalTransform.s);
            speed = speed + (acceleration*deltaTime);
            Transform ancien=this->globalTransform;
            this->setGlobalTransform(Mov);
            int col=this->getCollision();
            this->nbCollision=col;
            if(col>=0){
                this->setGlobalTransform(ancien);
                isCollision=true;
                this->auSol=true;
                speed = glm::vec3(0.0f);
            }else{
                isMoving = true;
                isCollision=false;
            }
        }

        int getCollision(){
            for(int i=0;i<this->collisions.size();i++){
                if(collisions[i]->map){
                    for(int j=0;j<collisions[i]->objetsOBJ.size();j++){
                        for(int k=0;k<collisions[i]->objetsOBJ[j].mesh.triangles.size();k++){
                            for(int l=0;l<collisions[i]->boiteEnglobante.triangles.size();l++){
                                if(trianglesIntersect(collisions[i]->objetsOBJ[j].mesh.vertices_Espace[collisions[i]->objetsOBJ[j].mesh.triangles[k][0]],collisions[i]->objetsOBJ[j].mesh.vertices_Espace[collisions[i]->objetsOBJ[j].mesh.triangles[k][1]],collisions[i]->objetsOBJ[j].mesh.vertices_Espace[collisions[i]->objetsOBJ[j].mesh.triangles[k][2]],this->boiteEnglobante.vertices_Espace[this->boiteEnglobante.triangles[l][0]],this->boiteEnglobante.vertices_Espace[this->boiteEnglobante.triangles[l][1]],this->boiteEnglobante.vertices_Espace[this->boiteEnglobante.triangles[l][2]])){
                                    this->collisionChateau=collisions[i]->objetsOBJ[j].nom;
                                    return i;
                                }
                            }
                        }
                    }
                }else{
                    for(int j=0;j<collisions[i]->boiteEnglobante.triangles.size();j++){
                        Ray ray1,ray2,ray3,ray4,ray5,ray6;
                        for(int k=0;k<this->boiteEnglobante.triangles.size();k++){
                            if(trianglesIntersect(collisions[i]->boiteEnglobante.vertices_Espace[collisions[i]->boiteEnglobante.triangles[j][0]],collisions[i]->boiteEnglobante.vertices_Espace[collisions[i]->boiteEnglobante.triangles[j][1]],collisions[i]->boiteEnglobante.vertices_Espace[collisions[i]->boiteEnglobante.triangles[j][2]],this->boiteEnglobante.vertices_Espace[this->boiteEnglobante.triangles[k][0]],this->boiteEnglobante.vertices_Espace[this->boiteEnglobante.triangles[k][1]],this->boiteEnglobante.vertices_Espace[this->boiteEnglobante.triangles[k][2]])){
                                return i;
                            }
                        }
                    }
                }
            }
            return -1;
        }


        void trouverMTL(const char* mat, GameObject *go){
            for(int i=0;i<mtls.size();i++){
                if(this->mtls[i].nom==mat){
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
                }else if( strcmp( lineHeader, "albedo" ) == 0 ){
                    mtl.pbr=true;
                    char albedo[250];
                    fscanf(file, "%s\n", albedo );
                    std::string chemin="../textures/";
                    chemin+=albedo;
                    char* cstr = new char[chemin.size() + 1];
                    std::copy(chemin.c_str(), chemin.c_str() + chemin.size() + 1, cstr);
                    mtl.albedo=chemin;
                }else if( strcmp( lineHeader, "ao" ) == 0 ){
                    char ao[250];
                    fscanf(file, "%s\n", ao );
                    std::string chemin="../textures/";
                    chemin+=ao;
                    char* cstr = new char[chemin.size() + 1];
                    std::copy(chemin.c_str(), chemin.c_str() + chemin.size() + 1, cstr);
                    mtl.ao=chemin;
                }else if( strcmp( lineHeader, "roughness" ) == 0 ){
                    char roughness[250];
                    fscanf(file, "%s\n", roughness );
                    std::string chemin="../textures/";
                    chemin+=roughness;
                    char* cstr = new char[chemin.size() + 1];
                    std::copy(chemin.c_str(), chemin.c_str() + chemin.size() + 1, cstr);
                    mtl.roughness=chemin;
                }else if( strcmp( lineHeader, "normal" ) == 0 ){
                    char normal[250];
                    fscanf(file, "%s\n", normal );
                    std::string chemin="../textures/";
                    chemin+=normal;
                    char* cstr = new char[chemin.size() + 1];
                    std::copy(chemin.c_str(), chemin.c_str() + chemin.size() + 1, cstr);
                    mtl.normal=chemin;
                }else if( strcmp( lineHeader, "metallic" ) == 0 ){
                    char metallic[250];
                    fscanf(file, "%s\n", metallic );
                    std::string chemin="../textures/";
                    chemin+=metallic;
                    char* cstr = new char[chemin.size() + 1];
                    std::copy(chemin.c_str(), chemin.c_str() + chemin.size() + 1, cstr);
                    mtl.metallic=chemin;
                }else{
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
            e->programID=this->programID;
            e->calculerBoiteEnglobante();
        }
        void addEnfantOBJ2(GameObject* e){
            e->programID=this->programID;
            e->boiteEnglobante.programID=this->programID;
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
            for (int i = tailleObjetOBJ; i < objetsOBJ.size(); i++) {
                objetsOBJ[i].programID = this->programID;
                objetsOBJ[i].mesh.programID = programID;
                objetsOBJ[i].hasMesh = true;
                objetsOBJ[i].M = true;
                objetsOBJ[i].mesh.creerTextureOBJ(objetsOBJ[i].mesh.mtl.texture);
                this->addEnfantOBJ(&objetsOBJ[i]);
            }
            tailleObjetOBJ=objetsOBJ.size();
        }
        void rajouterKoopaOBJ() {
            for (int i = 0; i < koopaOBJ.size(); i++) {
                koopaOBJ[i].programID = this->programID;
                koopaOBJ[i].mesh.programID = programID;
                koopaOBJ[i].hasMesh = true;
                koopaOBJ[i].M = true;
                koopaOBJ[i].mesh.creerTextureOBJ(koopaOBJ[i].mesh.mtl.texture);
                this->addEnfantOBJ(&koopaOBJ[i]);
            }
        }void rajouterShellOBJ() {
            for (int i = 0; i < shellOBJ.size(); i++) {
                shellOBJ[i].programID = this->programID;
                shellOBJ[i].mesh.programID = programID;
                shellOBJ[i].hasMesh = true;
                shellOBJ[i].M = true;
                shellOBJ[i].mesh.creerTextureOBJ(shellOBJ[i].mesh.mtl.texture);
                this->addEnfantOBJ(&shellOBJ[i]);
            }
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
            float minX=std::numeric_limits<float>::max(),minY=std::numeric_limits<float>::max(),minZ=std::numeric_limits<float>::max(),maxX=0.0,maxY=0.0,maxZ=0.0;
            FILE * file = fopen(filename, "r");
            if( file == NULL ){
                printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
                getchar();
                return false;
            }
            while( 1 ){
                long position = ftell(file);
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
                            unsigned short a,b,c;
                            if(nomOBJ=="Peaches_Castale.mtl"){
                                a=(unsigned short)(vertexIndices[i+nb])-nb-1;
                                b=(unsigned short)(vertexIndices[i+1+nb])-nb-1;
                                c=(unsigned short)(vertexIndices[i+2+nb])-nb-1;
                            }else{
                                a=i;
                                b=i+1;
                                c=i+2;
                            }
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
                    glm::vec3 centre = glm::vec3((minX+maxX)/2,(minY+maxY)/2,(minZ+maxZ)/2);
                    this->centre=centre;
                    std::cout<<"centre : "<<centre.x<<" "<<centre.y<<" "<<centre.z<<std::endl;
                    this->bas=glm::vec3(centre[0],minY,centre[2]);
                    std::cout<<"bas : "<<this->bas.x<<" "<<this->bas.y<<" "<<this->bas.z<<std::endl;
                    glm::vec3 p1=glm::vec3(minX,minY,minZ);
                    glm::vec3 p2=glm::vec3(maxX,minY,minZ);
                    glm::vec3 p3=glm::vec3(maxX,minY,maxZ);
                    glm::vec3 p4=glm::vec3(minX,minY,maxZ);
                    glm::vec3 p5=glm::vec3(minX,maxY,minZ);
                    glm::vec3 p6=glm::vec3(maxX,maxY,minZ);
                    glm::vec3 p7=glm::vec3(maxX,maxY,maxZ);
                    glm::vec3 p8=glm::vec3(minX,maxY,maxZ);
                    this->boiteEnglobante.indexed_vertices.push_back(p1);
                    this->boiteEnglobante.indexed_vertices.push_back(p2);
                    this->boiteEnglobante.indexed_vertices.push_back(p3);
                    this->boiteEnglobante.indexed_vertices.push_back(p4);
                    this->boiteEnglobante.indexed_vertices.push_back(p5);
                    this->boiteEnglobante.indexed_vertices.push_back(p6);
                    this->boiteEnglobante.indexed_vertices.push_back(p7);
                    this->boiteEnglobante.indexed_vertices.push_back(p8);
                    this->boiteEnglobante.triangles.push_back(std::vector<unsigned short>{0,1,2});
                    this->boiteEnglobante.triangles.push_back(std::vector<unsigned short>{2,3,0});
                    this->boiteEnglobante.triangles.push_back(std::vector<unsigned short>{0,1,5});
                    this->boiteEnglobante.triangles.push_back(std::vector<unsigned short>{5,4,0});
                    this->boiteEnglobante.triangles.push_back(std::vector<unsigned short>{1,2,6});
                    this->boiteEnglobante.triangles.push_back(std::vector<unsigned short>{6,5,1});
                    this->boiteEnglobante.triangles.push_back(std::vector<unsigned short>{2,3,7});
                    this->boiteEnglobante.triangles.push_back(std::vector<unsigned short>{7,6,2});
                    this->boiteEnglobante.triangles.push_back(std::vector<unsigned short>{3,0,4});
                    this->boiteEnglobante.triangles.push_back(std::vector<unsigned short>{4,7,3});
                    this->boiteEnglobante.triangles.push_back(std::vector<unsigned short>{4,5,6});
                    this->boiteEnglobante.triangles.push_back(std::vector<unsigned short>{6,7,4});
                    this->boiteEnglobante.indices.push_back(0);
                    this->boiteEnglobante.indices.push_back(1);
                    this->boiteEnglobante.indices.push_back(2);
                    this->boiteEnglobante.indices.push_back(2);
                    this->boiteEnglobante.indices.push_back(3);
                    this->boiteEnglobante.indices.push_back(0);
                    this->boiteEnglobante.indices.push_back(0);
                    this->boiteEnglobante.indices.push_back(1);
                    this->boiteEnglobante.indices.push_back(5);
                    this->boiteEnglobante.indices.push_back(5);
                    this->boiteEnglobante.indices.push_back(4);
                    this->boiteEnglobante.indices.push_back(0);
                    this->boiteEnglobante.indices.push_back(1);
                    this->boiteEnglobante.indices.push_back(2);
                    this->boiteEnglobante.indices.push_back(6);
                    this->boiteEnglobante.indices.push_back(6);
                    this->boiteEnglobante.indices.push_back(5);
                    this->boiteEnglobante.indices.push_back(1);
                    this->boiteEnglobante.indices.push_back(2);
                    this->boiteEnglobante.indices.push_back(3);
                    this->boiteEnglobante.indices.push_back(7);
                    this->boiteEnglobante.indices.push_back(7);
                    this->boiteEnglobante.indices.push_back(6);
                    this->boiteEnglobante.indices.push_back(2);
                    this->boiteEnglobante.indices.push_back(3);
                    this->boiteEnglobante.indices.push_back(0);
                    this->boiteEnglobante.indices.push_back(4);
                    this->boiteEnglobante.indices.push_back(4);
                    this->boiteEnglobante.indices.push_back(7);
                    this->boiteEnglobante.indices.push_back(3);
                    this->boiteEnglobante.indices.push_back(4);
                    this->boiteEnglobante.indices.push_back(5);
                    this->boiteEnglobante.indices.push_back(6);
                    this->boiteEnglobante.indices.push_back(6);
                    this->boiteEnglobante.indices.push_back(7);
                    this->boiteEnglobante.indices.push_back(4);
                    this->boiteEnglobante.compute_Normals();
                    this->isBoiteEnglobante=true;
                    this->boiteEnglobante.programID=this->programID;
                    this->boiteEnglobante.filename="../textures/2k_moon.jpg";
                    this->boiteEnglobante.loadTexture();
                    break; // EOF = End Of File. Quit the loop.
                }
                else if(Factuel && strcmp(lineHeader, "f")!=0){
                    Factuel=false;
                    unsigned short nba=0;
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
                        nba++;
                    }
                    for( unsigned short i=0; i<goa.mesh.indexed_vertices.size(); i+=3 ){
                        unsigned short a,b,c;
                        if(nomOBJ=="Peaches_Casatle.mtl"){
                            a=(unsigned short)(vertexIndices[i+nb])-nb-1;
                            b=(unsigned short)(vertexIndices[i+1+nb])-nb-1;
                            c=(unsigned short)(vertexIndices[i+2+nb])-nb-1;
                        }else{
                            a=i;
                            b=i+1;
                            c=i+2;
                        }
                        std::vector<unsigned short> ind{a,b,c};
                        goa.mesh.triangles.push_back(ind);
                        nbTriangles++;
                        goa.mesh.indices.push_back(a);
                        goa.mesh.indices.push_back(b);
                        goa.mesh.indices.push_back(c);

                    }
                    if(goa.hasMesh){
                        goa.calculerBoiteEnglobante();
                        goa.boiteEnglobante.programID=this->programID;
                        goa.boiteEnglobante.filename="../textures/2k_moon.jpg";
                        goa.boiteEnglobante.loadTexture();
                    }
                    objetsOBJ.push_back(goa);
                    nb+=nba;
                    nbTrianglesActu.push_back(nbTriangles*3);
                    goa.enfant.clear();
                    goa.mesh.indexed_vertices.clear();
                    goa.mesh.indices.clear();
                    goa.mesh.triangles.clear();
                    goa.mesh.normal.clear();
                    goa.mesh.texCoords.clear();
                    fseek(file, position, SEEK_SET);
                }
                // else : parse lineHeader
                else if ( strcmp( lineHeader, "mtllib" ) == 0 ){
                    char mtllib[100];
                    fscanf(file, "%s\n", mtllib );
                    std::string s="../meshes/";
                    s+=mtllib;
                    nomOBJ+=mtllib;
                    if(nomOBJ=="goomba.mtl" || nomOBJ=="PowerStar.mtl" || nomOBJ=="blakbobomb.mtl"){
                        temp_normals.push_back(glm::vec3(0.0,1.0,0.0));
                    }
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
                    if(vertex.x<minX) minX=vertex.x;
                    if(vertex.y<minY) minY=vertex.y;
                    if(vertex.z<minZ) minZ=vertex.z;
                    if(vertex.x>maxX) maxX=vertex.x;
                    if(vertex.y>maxY) maxY=vertex.y;
                    if(vertex.z>maxZ) maxZ=vertex.z;
                }else if ( strcmp( lineHeader, "s" ) == 0 ){
                    char s[250];
                    fscanf(file, "%s\n", s );
                }else if ( strcmp( lineHeader, "vt" ) == 0 ){
                    glm::vec2 uv;
                    fscanf(file, "%f %f\n", &uv.x, &uv.y);
                    temp_uvs.push_back(uv);
                }else if ( strcmp( lineHeader, "vn" ) == 0 ){
                    glm::vec3 normal;
                    fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
                    temp_normals.push_back(normal);
                }else if ( strcmp( lineHeader, "f" ) == 0 ){
                    Factuel=true;
                    std::string vertex1, vertex2, vertex3, vertex4;
                    unsigned int vertexIndex[4], uvIndex[4], normalIndex[4];
                    int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2], &vertexIndex[3], &uvIndex[3], &normalIndex[3] );
                    if (matches == 12){
                        vertexIndices.push_back(vertexIndex[2]);
                        vertexIndices.push_back(vertexIndex[3]);
                        vertexIndices.push_back(vertexIndex[0]);
                        uvIndices    .push_back(uvIndex[2]);
                        uvIndices    .push_back(uvIndex[3]);
                        uvIndices    .push_back(uvIndex[0]);
                        normalIndices.push_back(normalIndex[2]);
                        normalIndices.push_back(normalIndex[3]);
                        normalIndices.push_back(normalIndex[0]);
                    }else if (matches == 1){
                        fseek(file, position, SEEK_SET);
                        matches = fscanf(file, "f %d//%d %d//%d %d//%d\n", &vertexIndex[0], &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2] );
                        uvIndex[0]=1;
                        uvIndex[1]=1;
                        uvIndex[2]=1;
                    }else if(matches==2){
                        matches = fscanf(file, "%d/%d %d/%d\n", &vertexIndex[1], &uvIndex[1], &vertexIndex[2], &uvIndex[2] );
                        normalIndex[0]=1;
                        normalIndex[1]=1;
                        normalIndex[2]=1;
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

        void creerIA(){
            this->isIA=true;
            this->visionIA.m_origin = this->centreEspace;
            this->visionIA.m_direction = glm::normalize(this->collisions[1]->centreEspace-this->centreEspace);
            this->visionIA.m_direction[1]=0.0;
            this->rayonDepart=this->visionIA.m_direction;
            this->rotationDepart = this->globalTransform.m;
            std::cout<<"nouvelle IA cree !"<<std::endl;
            std::cout<<"direction : "<<this->visionIA.m_direction.x<<" "<<this->visionIA.m_direction.y<<" "<<this->visionIA.m_direction.z<<std::endl;
            std::cout<<"position : "<<this->visionIA.m_origin.x<<" "<<this->visionIA.m_origin.y<<" "<<this->visionIA.m_origin.z<<std::endl;
        }

        void calculerBoiteEnglobante() {
            if (!hasMesh) {
                std::cerr << "Aucun mesh disponible pour calculer la boîte englobante." << std::endl;
                return;
            }

            if (isGround) {
                std::cout << "Cet objet est marqué comme sol, pas de boîte englobante générée." << std::endl;
                return;
            }
        
            // Initialiser les valeurs min et max
            glm::vec3 min = glm::vec3(std::numeric_limits<float>::max());
            glm::vec3 max = glm::vec3(std::numeric_limits<float>::lowest());
        
            // Parcourir les vertices du mesh
            for (const auto& vertex : mesh.indexed_vertices) {
                if (vertex.x < min.x) min.x = vertex.x;
                if (vertex.y < min.y) min.y = vertex.y;
                if (vertex.z < min.z) min.z = vertex.z;
        
                if (vertex.x > max.x) max.x = vertex.x;
                if (vertex.y > max.y) max.y = vertex.y;
                if (vertex.z > max.z) max.z = vertex.z;
            }
        
            // Calculer le centre et les dimensions de la boîte englobante
            this->centre = (min + max) / 2.0f;
            this->bas = glm::vec3(centre.x, min.y, centre.z);
        
            // Créer les 8 points de la boîte englobante
            glm::vec3 p1 = glm::vec3(min.x, min.y, min.z);
            glm::vec3 p2 = glm::vec3(max.x, min.y, min.z);
            glm::vec3 p3 = glm::vec3(max.x, min.y, max.z);
            glm::vec3 p4 = glm::vec3(min.x, min.y, max.z);
            glm::vec3 p5 = glm::vec3(min.x, max.y, min.z);
            glm::vec3 p6 = glm::vec3(max.x, max.y, min.z);
            glm::vec3 p7 = glm::vec3(max.x, max.y, max.z);
            glm::vec3 p8 = glm::vec3(min.x, max.y, max.z);
        
            // Ajouter les points à la boîte englobante
            boiteEnglobante.indexed_vertices = {p1, p2, p3, p4, p5, p6, p7, p8};
        
            // Ajouter les triangles pour former la boîte
            boiteEnglobante.triangles = {
                {0, 1, 2}, {2, 3, 0}, // Face inférieure
                {4, 5, 6}, {6, 7, 4}, // Face supérieure
                {0, 1, 5}, {5, 4, 0}, // Face avant
                {1, 2, 6}, {6, 5, 1}, // Face droite
                {2, 3, 7}, {7, 6, 2}, // Face arrière
                {3, 0, 4}, {4, 7, 3}  // Face gauche
            };
        
            // Ajouter les indices pour le rendu
            boiteEnglobante.indices.clear();
            for (const auto& triangle : boiteEnglobante.triangles) {
                boiteEnglobante.indices.insert(boiteEnglobante.indices.end(), triangle.begin(), triangle.end());
            }
        
            // Calculer les normales pour le rendu
            boiteEnglobante.compute_Normals();

            boiteEnglobante.programID = this->programID;
            boiteEnglobante.filename = "../textures/2k_moon.jpg";
        
            // Marquer la boîte englobante comme active
            isBoiteEnglobante = true;
        }

        void moveToPosition(float deltaTime) {
            int nb=this->getCollision();
            this->speed=glm::vec3(0.0);
            if (nb==-1) {
                glm::vec3 direction=glm::normalize(this->collisions[1]->centreEspace-this->centreEspace);
                direction[1]=0.0;
                Transform rotateY;glm::vec3 newPosition;
                if(this->nom=="koopa"){
                    newPosition = this->globalTransform.t + direction * glm::vec3(3.0) * deltaTime*glm::vec3(1.0,0.0,1.0);
                    float dotProduct = glm::dot(glm::vec3(-1.0,0.0,0.0), direction);
                    float angle = glm::degrees(glm::acos(dotProduct));
                    rotateY= Transform().rotation(glm::vec3(0.0f, 1.0f, 0.0f), -angle);
                }if(this->nom=="Bowser"){
                    newPosition = this->globalTransform.t + direction * glm::vec3(2.0) * deltaTime*glm::vec3(1.0,0.0,1.0);
                    float dotProduct = glm::dot(glm::vec3(0.0,0.0,1.0), direction);
                    float angle = glm::degrees(glm::acos(dotProduct));
                    rotateY= Transform().rotation(glm::vec3(0.0f, 1.0f, 0.0f), angle);
                }
                Transform a=Transform(rotationDepart, this->transform.t, this->transform.s);
                Transform newTransform = Transform(glm::mat3x3(1.0), newPosition, this->globalTransform.s);
                Transform ancien=this->globalTransform;
                this->setGlobalTransform(newTransform.combine_with(rotateY));
                this->visionIA.m_direction = direction;
                if(this->getCollision()>=0){
                    this->setGlobalTransform(ancien);
                }
            } 
            else {
                this->nbCollision=nb;
            }
        }

        void gravite(GameObject* obj, float deltaTime){
            if(isGravite && aTerre==false){
                Ray ray1 = Ray(boiteEnglobante.vertices_Espace[0], glm::vec3(0.0,-1.0,0.0));
                Ray ray2 = Ray(boiteEnglobante.vertices_Espace[1], glm::vec3(0.0,-1.0,0.0));
                Ray ray3 = Ray(boiteEnglobante.vertices_Espace[2], glm::vec3(0.0,-1.0,0.0));
                Ray ray4 = Ray(boiteEnglobante.vertices_Espace[3], glm::vec3(0.0,-1.0,0.0));
                RayTriangleIntersection intersection1;
                RayTriangleIntersection intersection2;
                RayTriangleIntersection intersection3;
                RayTriangleIntersection intersection4;
                for(int i=0;i<obj->objetsOBJ.size();i++){
                    for(int j=0;j<obj->objetsOBJ[i].mesh.triangles.size();j++){
                        if(!intersection1.intersectionExists)intersection1=obj->objetsOBJ[i].mesh.getIntersection(ray1,j);
                        if(!intersection2.intersectionExists)intersection2=obj->objetsOBJ[i].mesh.getIntersection(ray2,j);
                        if(!intersection3.intersectionExists)intersection3=obj->objetsOBJ[i].mesh.getIntersection(ray3,j);
                        if(!intersection4.intersectionExists)intersection4=obj->objetsOBJ[i].mesh.getIntersection(ray4,j);
                        if(intersection1.intersectionExists && intersection1.t>0.01f && intersection2.intersectionExists && intersection2.t>0.01f && intersection3.intersectionExists && intersection3.t>0.01f && intersection4.intersectionExists && intersection4.t>0.01f){
                            glm::vec3 direction = glm::vec3(0.0,-1.0,0.0);
                            glm::vec3 newPosition = this->globalTransform.t + direction * glm::vec3(1.0) * deltaTime;
                            Transform newTransform = Transform(this->globalTransform.m, newPosition, this->globalTransform.s);
                            this->setGlobalTransform(newTransform);
                            break;
                        }else if(intersection1.intersectionExists && intersection1.t<=0.01f && intersection2.intersectionExists && intersection2.t<=0.01f && intersection3.intersectionExists && intersection3.t<=0.01f && intersection4.intersectionExists && intersection4.t<=0.01f){
                            this->aTerre=true;
                            break;
                        }
                    }
                }
            }else{
                for(int i=0;i<this->enfant.size();i++){
                    this->enfant[i]->gravite(obj,deltaTime);
                }
            }
        }

        bool isPointInTriangle(const glm::vec3& point, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2) {
            glm::vec3 u = v1 - v0;
            glm::vec3 v = v2 - v0;
            glm::vec3 w = point - v0;
            float uu = glm::dot(u, u);
            float uv = glm::dot(u, v);
            float vv = glm::dot(v, v);
            float wu = glm::dot(w, u);
            float wv = glm::dot(w, v);
            float denominator = uv * uv - uu * vv;
            float s = (uv * wv - vv * wu) / denominator;
            float t = (uv * wu - uu * wv) / denominator;
            return (s >= 0.0f && t >= 0.0f && (s + t) <= 1.0f);
        }
        
        bool edgeIntersectsTriangle(const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2) {
            glm::vec3 edgeDir = p2 - p1;
            glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
            float d = glm::dot(normal, v0);
            float t = (d - glm::dot(normal, p1)) / glm::dot(normal, edgeDir);
            if (t < 0.0f || t > 1.0f) {
                return false;
            }
            glm::vec3 intersectionPoint = p1 + t * edgeDir;
            return isPointInTriangle(intersectionPoint, v0, v1, v2);
        }
        
        bool trianglesIntersect(const glm::vec3& t1_v0, const glm::vec3& t1_v1, const glm::vec3& t1_v2,
                                const glm::vec3& t2_v0, const glm::vec3& t2_v1, const glm::vec3& t2_v2) {
            if (edgeIntersectsTriangle(t1_v0, t1_v1, t2_v0, t2_v1, t2_v2) ||
                edgeIntersectsTriangle(t1_v1, t1_v2, t2_v0, t2_v1, t2_v2) ||
                edgeIntersectsTriangle(t1_v2, t1_v0, t2_v0, t2_v1, t2_v2)) {
                return true;
            }
            if (edgeIntersectsTriangle(t2_v0, t2_v1, t1_v0, t1_v1, t1_v2) ||
                edgeIntersectsTriangle(t2_v1, t2_v2, t1_v0, t1_v1, t1_v2) ||
                edgeIntersectsTriangle(t2_v2, t2_v0, t1_v0, t1_v1, t1_v2)) {
                return true;
            }
            return false;
        }

        void clearGameObject(){
            this->enfant.clear();
            this->objetsOBJ.clear();
            mesh.indexed_vertices.clear();
            mesh.indices.clear();
            mesh.triangles.clear();
            mesh.normal.clear();
            mesh.texCoords.clear();
            hasMesh = false;
            hasPlan = false;
            height2parent = 0.0f;
            hasLOD = false;
            lodHighDistance = 2.0f;
            lodMediumDistance = 5.0f;
            speed = glm::vec3(0.0f);
            poids = 1.0f;
            Force = glm::vec3(0.0,poids * -9.83,0.0);
            acceleration =Force/poids;
            initialTransform = Transform(glm::mat3(1.0f), glm::vec3(0.0f), 1.0f);
            mtls.clear();
            nom="";
            M=false;
            tailleObjetOBJ=0;
            isBoiteEnglobante=false;
            isGravite=false;
            aTerre=false;
            isCollision=true;
            boiteEnglobante.indexed_vertices.clear();
            boiteEnglobante.indices.clear();
            boiteEnglobante.triangles.clear();
            boiteEnglobante.normal.clear();
            boiteEnglobante.texCoords.clear();
            isGround = false;
            // collisions.clear();
            map=false;
            auSol=false;
            index = 0;
            isLight = false;
            lightIntensity = 1.0f;
            lightColor = glm::vec3(1.0f,1.0f,1.0f);
            isIA=false;
            avancer=false;
            isMoving = true;
            nbCollision=-1;
        }

        void mettreAuSol(GameObject* map){
            while(this->getCollision()==-1){
                this->globalTransform.t.y-=0.1f;
                this->setGlobalTransform(this->globalTransform);
            }
            this->globalTransform.t.y+=0.1f;
            this->setGlobalTransform(this->globalTransform);
            this->transformSol=this->globalTransform;
        }

        void bougeCarapace(float deltaTime){
            glm::vec3 direction;
            if(this->nbCollision!=-1){
                direction=-this->directionCarapace;
                Ray ray;ray.m_origin=this->centreEspace;ray.m_direction=glm::vec3(0.0,-1.0,0.0);
                for(int i=0;i<this->collisions[0]->objetsOBJ.size();i++){
                    for(int j=0;j<this->collisions[0]->objetsOBJ[i].mesh.triangles.size();j++){
                        RayTriangleIntersection intersection=this->collisions[0]->objetsOBJ[i].mesh.getIntersection(ray,j);
                        if(intersection.intersectionExists && intersection.t<50.0f && intersection.t>0.0f){
                            this->carapaceRespawn=false;
                            break;
                        }else{
                            this->carapaceRespawn=true;
                        }
                    }
                    if(!this->carapaceRespawn)break;
                }
            }else{
                direction=this->collisions[2]->basEspace-this->basEspace;
                direction=glm::normalize(direction);
            }
            glm::vec3 newPosition = this->globalTransform.t + direction * glm::vec3(50.0) * deltaTime;
            Transform newTransform = Transform(this->globalTransform.m, newPosition, this->globalTransform.s);
            newTransform=newTransform.combine_with(Transform(this->globalTransform.m,glm::vec3(0.0),1.0).rotation(glm::vec3(0.0,1.0,0.0),10));
            Transform ancien=this->globalTransform;
            this->setGlobalTransform(newTransform);
            int a=this->getCollision();
            if(a>=0 && nbCollision==-1){
                this->setGlobalTransform(ancien);
                this->directionCarapace=direction;
                nbCollision=a;
                this->collisions[nbCollision]->pv-=1;
                std::cout<<this->collisions[nbCollision]->nom<<std::endl;
                std::cout<<this->collisions[nbCollision]->pv<<std::endl;
            }
        }
};
