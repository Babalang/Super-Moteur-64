#ifndef ANIMATEDMODEL_H
#define ANIMATEDMODEL_H
// Include standard headers
#include <GL/glew.h>
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <unistd.h>

// Include GLFW
#include <GLFW/glfw3.h>
#include <glm/gtx/string_cast.hpp>

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
#include <TP5/Mesh.h>
#include<unordered_map>
#include "Transform.h"

#include <assimp/scene.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <map>
#include <string>
#include <array>
#include <future>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

class AnimatedModel {
    private:
    std::shared_ptr<Assimp::Importer> importer;
    std::map<std::string, const aiScene*> animScenes;
    std::map<std::string, std::shared_ptr<Assimp::Importer>> animationImporters;

    bool processAnimation(const std::string& path, const aiAnimation* animation) {
        try {
            float ticksPerSecond = animation->mTicksPerSecond != 0 ? animation->mTicksPerSecond : 25.0f;
            float durationInSeconds = animation->mDuration / ticksPerSecond;
            const float targetFPS = 24.0f;
            std::vector<std::vector<glm::mat4>> bakedFrames;
            size_t numFrames = static_cast<size_t>(durationInSeconds * targetFPS);
            bakedFrames.reserve(numFrames);
            std::cout << "Baking animation: " << path << " (" << numFrames << " frames)" << std::endl;            
            if (!baseScene || !baseScene->mRootNode) {
                std::cerr << "❌ Invalid base scene or root node" << std::endl;
                return false;
            }
            std::vector<aiNodeAnim*> channelsCopy;
            for (unsigned int i = 0; i < animation->mNumChannels; i++) {
                channelsCopy.push_back(animation->mChannels[i]);
            }
            for (float t = 0.0f; t < durationInSeconds; t += 1.0f / targetFPS) {
                float animationTime = fmod(t * ticksPerSecond, animation->mDuration);
                std::map<std::string, glm::mat4> boneTransforms;
                try {
                    ReadNodeHierarchy(animationTime, baseScene->mRootNode, glm::mat4(1.0f), boneTransforms, animation);
                    std::vector<glm::mat4> finalBones(boneOffsets.size(), glm::mat4(1.0f));
                    for (const auto& pair : boneNameToIndex) {
                        const std::string& boneName = pair.first;
                        int boneIndex = pair.second;
                        if (boneIndex >= 0 && boneIndex < boneOffsets.size()) {
                            if (boneTransforms.count(boneName) > 0) {
                                finalBones[boneIndex] = boneTransforms[boneName] * boneOffsets[boneIndex];
                            }
                        }
                    }
                    bakedFrames.push_back(std::move(finalBones));
                }
                catch (const std::exception& e) {
                    std::cerr << " !! Error processing frame at time " << t << ": " << e.what() << std::endl;
                    continue;
                }
            }
            if (!bakedFrames.empty()) {
                bakedAnimations[path] = std::move(bakedFrames);
                return true;
            }
            return false;
        }
        catch (const std::exception& e) {
            std::cerr << " !! Error baking animation: " << e.what() << std::endl;
            return false;
        }
    }

    public:
        const aiScene* baseScene = nullptr;
        std::vector<Mesh> baseMeshes;
        std::map<std::string, int> boneNameToIndex;
        std::vector<glm::mat4> boneOffsets;
        std::vector<glm::mat4> finalBoneMatrices;
        std::map<std::string, std::vector<const aiAnimation*>> animations;
        std::map<std::string, std::vector<std::vector<glm::mat4>>> bakedAnimations;
        glm::mat4 globalInverseTransform;
        float scale;

        AnimatedModel() : importer(std::make_shared<Assimp::Importer>()) {
            importer->SetPropertyInteger(AI_CONFIG_IMPORT_COLLADA_IGNORE_UP_DIRECTION, 1);
        }

    
        glm::mat4 aiMat4ToGlm(const aiMatrix4x4& from) {
            glm::mat4 to;
            to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
            to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
            to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
            to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
            return to;
        }

        aiVector3D CalcInterpolatedValueFromKey(float animationTime, const int numKeys, const aiVectorKey* const vectorKey) const
        {
            aiVector3D ret;
            if (numKeys == 1)
            {
                ret = vectorKey[0].mValue;
                return ret;
            }
            unsigned int keyIndex = FindKeyIndex(animationTime, numKeys, vectorKey);
            unsigned int nextKeyIndex = keyIndex + 1;
            assert(nextKeyIndex < numKeys);
            float deltaTime = vectorKey[nextKeyIndex].mTime - vectorKey[keyIndex].mTime;
            float factor = (animationTime - (float)vectorKey[keyIndex].mTime) / deltaTime;
            assert(factor >= 0.0f && factor <= 1.0f);
            const aiVector3D& startValue = vectorKey[keyIndex].mValue;
            const aiVector3D& endValue = vectorKey[nextKeyIndex].mValue;
            ret.x = startValue.x + (endValue.x - startValue.x) * factor;
            ret.y = startValue.y + (endValue.y - startValue.y) * factor;
            ret.z = startValue.z + (endValue.z - startValue.z) * factor;
            return ret;
        }

        aiQuaternion CalcInterpolatedValueFromKey(float animationTime, const int numKeys, const aiQuatKey* const quatKey) const
        {
            aiQuaternion ret;
            if (numKeys == 1)
            {
                ret = quatKey[0].mValue;
                return ret;
            }
            unsigned int keyIndex = FindKeyIndex(animationTime, numKeys, quatKey);
            unsigned int nextKeyIndex = keyIndex + 1;
            assert(nextKeyIndex < numKeys);
            float deltaTime = quatKey[nextKeyIndex].mTime - quatKey[keyIndex].mTime;
            float factor = (animationTime - (float)quatKey[keyIndex].mTime) / deltaTime;
            assert(factor >= 0.0f && factor <= 1.0f);
            const aiQuaternion& startValue = quatKey[keyIndex].mValue;
            const aiQuaternion& endValue = quatKey[nextKeyIndex].mValue;
            aiQuaternion::Interpolate(ret, startValue, endValue, factor);
            ret = ret.Normalize();
            return ret;
        }

        unsigned int FindKeyIndex(const float animationTime, const int numKeys, const aiVectorKey* const vectorKey) const
        {
            assert(numKeys > 0);
            for (int i = 0; i < numKeys - 1; i++)
                if (animationTime < (float)vectorKey[i + 1].mTime)
                    return i;
            return numKeys - 1;
        }

        unsigned int FindKeyIndex(const float animationTime, const int numKeys, const aiQuatKey* const quatKey) const
        {
            assert(numKeys > 0);
            for (int i = 0; i < numKeys - 1; i++)
                if (animationTime < (float)quatKey[i + 1].mTime)
                    return i;
            return numKeys - 1;
        }


        const aiNodeAnim* FindNodeAnim(const aiAnimation* animation, const std::string& nodeName) {
            for (unsigned int i = 0; i < animation->mNumChannels; i++) {
                const aiNodeAnim* nodeAnim = animation->mChannels[i];
                if (std::string(nodeAnim->mNodeName.C_Str()) == nodeName) {
                    return nodeAnim;
                }
            }
            return nullptr;
        }
    
        Mesh processMesh(aiMesh* mesh) {
            Mesh result;
            for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
                aiVector3D pos = mesh->mVertices[i];
                result.indexed_vertices.push_back(glm::vec3(pos.x, pos.y, pos.z));
                if (mesh->HasNormals()) {
                    aiVector3D n = mesh->mNormals[i];
                    result.normal.push_back(glm::vec3(n.x, n.y, n.z));
                } else {
                    result.normal.push_back(glm::vec3(0.0f));
                }
                if (mesh->mTextureCoords[0]) {
                    aiVector3D uv = mesh->mTextureCoords[0][i];
                    result.texCoords.push_back(glm::vec2(uv.x, uv.y));
                } else {
                    result.texCoords.push_back(glm::vec2(0.0f));
                }
            }
            for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
                aiFace face = mesh->mFaces[i];
                for (unsigned int j = 0; j < face.mNumIndices; j++) {
                    result.indices.push_back(static_cast<unsigned short>(face.mIndices[j]));
                }
            }
            result.BoneIDs.resize(result.indexed_vertices.size(), {-1, -1, -1, -1});
            result.Weights.resize(result.indexed_vertices.size(), {0.f, 0.f, 0.f, 0.f});
            return result;
        }
    
        void ReadNodeHierarchy(float animationTime, const aiNode* node, 
            const glm::mat4& parentTransform, 
            std::map<std::string, glm::mat4>& boneTransforms, 
            const aiAnimation* currentAnimation = nullptr) {
            try {
                if (!node) {
                    std::cerr << "Node is null!" << std::endl;
                    return;
                }

                std::string nodeName(node->mName.data);
                const aiAnimation* animation = currentAnimation ? currentAnimation : 
                (baseScene && baseScene->mNumAnimations > 0 ? baseScene->mAnimations[0] : nullptr);
                if (!animation) {
                    std::cerr << "No valid animation found for node: " << nodeName << std::endl;
                    return;
                }

                glm::mat4 nodeTransform;

                const aiNodeAnim* nodeAnim = FindNodeAnim(animation, nodeName);

                if (node == baseScene->mRootNode || nodeName.find("Hips") != std::string::npos) {
                    if (nodeAnim && nodeAnim->mNumRotationKeys > 0 && nodeAnim->mNumScalingKeys > 0) {
                        const aiVector3D& scaling = CalcInterpolatedValueFromKey(
                            animationTime, 
                            nodeAnim->mNumScalingKeys, 
                            nodeAnim->mScalingKeys
                        );
                        const aiQuaternion& rotationQ = CalcInterpolatedValueFromKey(
                            animationTime, 
                            nodeAnim->mNumRotationKeys, 
                            nodeAnim->mRotationKeys
                        );
                        glm::mat4 scalingM = glm::scale(glm::mat4(1.0f), 
                            glm::vec3(scaling.x, scaling.y, scaling.z));
                        glm::mat4 rotationM = glm::toMat4(
                            glm::quat(rotationQ.w, rotationQ.x, rotationQ.y, rotationQ.z));
                        if (node == baseScene->mRootNode) {
                            scalingM = glm::scale(glm::mat4(1.0f), glm::vec3(this->scale));
                        }
                        nodeTransform = rotationM * scalingM;
                    } else {
                        nodeTransform = node == baseScene->mRootNode ? 
                            glm::scale(glm::mat4(1.0f), glm::vec3(this->scale)) : 
                            aiMat4ToGlm(node->mTransformation);
                    }
                } else {
                    if (nodeAnim) {
                        if (nodeAnim->mNumScalingKeys > 0 && 
                            nodeAnim->mNumRotationKeys > 0 && 
                            nodeAnim->mNumPositionKeys > 0) {
                            const aiVector3D& scaling = CalcInterpolatedValueFromKey(
                                animationTime, 
                                nodeAnim->mNumScalingKeys, 
                                nodeAnim->mScalingKeys
                            );
                            const aiQuaternion& rotationQ = CalcInterpolatedValueFromKey(
                                animationTime, 
                                nodeAnim->mNumRotationKeys, 
                                nodeAnim->mRotationKeys
                            );
                            const aiVector3D& translation = CalcInterpolatedValueFromKey(
                                animationTime, 
                                nodeAnim->mNumPositionKeys, 
                                nodeAnim->mPositionKeys
                            );
                            nodeTransform = glm::translate(glm::mat4(1.0f), 
                                glm::vec3(translation.x, translation.y, translation.z)) *
                                glm::toMat4(glm::quat(rotationQ.w, rotationQ.x, rotationQ.y, rotationQ.z)) *
                                glm::scale(glm::mat4(1.0f), 
                                    glm::vec3(scaling.x, scaling.y, scaling.z));
                        } else {
                            nodeTransform = aiMat4ToGlm(node->mTransformation);
                        }
                    } else {
                        nodeTransform = aiMat4ToGlm(node->mTransformation);
                    }
                }
                glm::mat4 globalTransform = parentTransform * nodeTransform;
                boneTransforms[nodeName] = globalTransform;
                for (unsigned int i = 0; i < node->mNumChildren; i++) {
                    if (node->mChildren[i]) {
                        ReadNodeHierarchy(animationTime, node->mChildren[i], 
                            globalTransform, boneTransforms, currentAnimation);
                    }
                }
            }
            catch (const std::exception& e) {
                std::cerr << "Error in ReadNodeHierarchy: " << e.what() << std::endl;
            }
        }

        void loadAnimatedModel(const std::string& path, float scale = 1.0f) {
            this->scale = scale;
            unsigned int flags = 
                aiProcess_Triangulate | 
                aiProcess_GenNormals | 
                aiProcess_JoinIdenticalVertices |
                aiProcess_SortByPType;
            baseScene = importer->ReadFile(path,flags);
            std::cout << "Nombre de meshes : " << baseScene->mNumMeshes << std::endl;
            std::cout << "Nombre d’animations : " << baseScene->mNumAnimations << std::endl;
            for (unsigned int i = 0; i < baseScene->mNumMeshes; i++) {
                std::cout << "Mesh " << i << " contient " << baseScene->mMeshes[i]->mNumVertices << " vertices" << std::endl;
            }
            if (!baseScene || !baseScene->mRootNode) {
                std::cerr << "Scene or root node is null!" << std::endl;
                return;
            }
            animScenes[path] = baseScene;
            if (baseScene->mNumAnimations == 0 || !baseScene->mAnimations[0]) {
                std::cerr << "No animation found in model." << std::endl;
                return;
            }
            this->globalInverseTransform = glm::inverse(aiMat4ToGlm(baseScene->mRootNode->mTransformation));
            if (!baseScene->HasAnimations()) {
                std::cerr << " !! Aucune animation dans le fichier DAE !" << std::endl;
            } else {
                std::cout << "Root node name: " << baseScene->mRootNode->mName.C_Str() << std::endl;
                std::cout << "Root node children: " << baseScene->mRootNode->mNumChildren << std::endl;
                std::cout << "Nombre d'animations : " << baseScene->mNumAnimations << std::endl;
                std::cout << "Frames (duration): " << baseScene->mAnimations[0]->mDuration << std::endl;
                std::cout << "Ticks per second: " << baseScene->mAnimations[0]->mTicksPerSecond << std::endl;
                //PrintNodeHierarchy(baseScene->mRootNode);  
            }
            for (unsigned int i = 0; i < baseScene->mNumMeshes; i++) {
                aiMesh* mesh = baseScene->mMeshes[i];
                Mesh processedMesh = processMesh(mesh);
                for (unsigned int b = 0; b < mesh->mNumBones; b++) {
                    std::string boneName = mesh->mBones[b]->mName.C_Str();
                    if (boneNameToIndex.find(boneName) == boneNameToIndex.end()) {
                        int newIndex = (int)boneNameToIndex.size();
                        boneNameToIndex[boneName] = newIndex;
                        glm::mat4 offset = aiMat4ToGlm(mesh->mBones[b]->mOffsetMatrix);
                        boneOffsets.push_back(offset);
                    }
                    int boneIndexGlobal = boneNameToIndex[boneName];
                    float test =0.0f;
                    for (unsigned int w = 0; w < mesh->mBones[b]->mNumWeights; w++) {
                        unsigned int vertexID = mesh->mBones[b]->mWeights[w].mVertexId;
                        float weight = mesh->mBones[b]->mWeights[w].mWeight;
                        auto& boneIDs = processedMesh.BoneIDs[vertexID];
                        auto& weights = processedMesh.Weights[vertexID];
                        for (int k = 0; k < 4; ++k) {
                            if (boneIDs[k] == -1) {
                                boneIDs[k] = boneIndexGlobal;
                                weights[k] = weight;
                                test+=weight;
                                break;
                            }
                        } 
                    }
                    for (unsigned int v = 0; v < processedMesh.Weights.size(); v++) {
                        float sumWeights = 0.0f;
                        for (int k = 0; k < 4; ++k) {
                            sumWeights += processedMesh.Weights[v][k];
                        }
                    }
                }
                baseMeshes.push_back(processedMesh);
            }
            finalBoneMatrices.resize(boneOffsets.size(), glm::mat4(1.0f));
            if (baseScene->mNumAnimations > 0) {
                for (unsigned int i = 0; i < baseScene->mNumAnimations; i++) {
                    animations[path].push_back(baseScene->mAnimations[i]);
                }
            }
            const float targetFPS = 24.0f;
            for (unsigned int i = 0; i < baseScene->mNumAnimations; i++) {
                const aiAnimation* animation = baseScene->mAnimations[i];
                float ticksPerSecond = animation->mTicksPerSecond != 0 ? animation->mTicksPerSecond : 25.0f;
                float durationInSeconds = animation->mDuration / ticksPerSecond;
                std::vector<std::vector<glm::mat4>> bakedFrames;
                for (float t = 0.0f; t < durationInSeconds; t += 1.0f / targetFPS) {
                    float animationTime = fmod(t * ticksPerSecond, animation->mDuration);
                    std::map<std::string, glm::mat4> boneOffsetsMap;
                    for (const auto& [name, index] : boneNameToIndex) {
                        boneOffsetsMap[name] = boneOffsets[index];
                    }
                    std::map<std::string, glm::mat4> boneTransforms;
                    if (baseScene->mRootNode == nullptr) {
                        std::cerr << "Warning: child node " << i << " is null" << std::endl;
                        continue;
                    }
                    ReadNodeHierarchy(animationTime, baseScene->mRootNode, glm::mat4(1.0f),boneTransforms);
                    std::vector<glm::mat4> finalBones(boneOffsets.size(), glm::mat4(1.0f));
                    for (auto& [boneName, index] : boneNameToIndex) {
                        if (boneTransforms.find(boneName) != boneTransforms.end()) {
                            finalBones[index] = boneTransforms[boneName] * boneOffsets[index];
                        } else {
                            finalBones[index] = glm::mat4(1.0f);
                        }
                    }
                    bakedFrames.push_back(finalBones);
                }
                
                bakedAnimations[path] = bakedFrames;
            }
        }

        bool addAnimation(const std::string& path) {
            try {
                unsigned int flags = 
                    aiProcess_Triangulate |
                    aiProcess_ValidateDataStructure |
                    aiProcess_JoinIdenticalVertices;
                
                auto animImporter = std::make_shared<Assimp::Importer>();
                animImporter->SetPropertyInteger(AI_CONFIG_IMPORT_COLLADA_IGNORE_UP_DIRECTION, 1);
                
                const aiScene* animScene = animImporter->ReadFile(path, flags);
                
                if (!animScene || !animScene->HasAnimations()) {
                    std::cerr << " !! No animation found in: " << path << std::endl;
                    return false;
                }        
                animationImporters[path] = animImporter;
                animScenes[path] = animScene;                
                const aiAnimation* animation = animScene->mAnimations[0];
                float ticksPerSecond = animation->mTicksPerSecond != 0 ? animation->mTicksPerSecond : 25.0f;
                float durationInSeconds = animation->mDuration / ticksPerSecond;
                const float targetFPS = 24.0f;
                std::vector<std::vector<glm::mat4>> bakedFrames;
                size_t numFrames = static_cast<size_t>(durationInSeconds * targetFPS);
                bakedFrames.reserve(numFrames);
                std::cout << "Baking animation: " << path << std::endl;
                std::cout << "Duration: " << durationInSeconds << " seconds" << std::endl;
                std::cout << "Frames: " << numFrames << std::endl;
                for (float t = 0.0f; t < durationInSeconds; t += 1.0f / targetFPS) {
                    float animationTime = fmod(t * ticksPerSecond, animation->mDuration);
                    std::map<std::string, glm::mat4> boneTransforms;
                    ReadNodeHierarchy(animationTime, baseScene->mRootNode, glm::mat4(1.0f), boneTransforms, animation);
                    std::vector<glm::mat4> finalBones(boneOffsets.size(), glm::mat4(1.0f));
                    for (const auto& pair : boneNameToIndex) {
                        if (boneTransforms.count(pair.first)) {
                            finalBones[pair.second] = boneTransforms[pair.first] * boneOffsets[pair.second];
                        }
                    }
                    bakedFrames.push_back(finalBones);
                }
                if (!bakedFrames.empty()) {
                    bakedAnimations[path] = std::move(bakedFrames);
                    animations[path].push_back(animation);
                    std::cout << "Animation ajoutée: " << path << std::endl;
                    return true;
                }
        
                return false;
            }
            catch (const std::exception& e) {
                std::cerr << " !! Error loading animation: " << e.what() << std::endl;
                return false;
            }
        }
        
        void DrawAnimatedModel(GLuint shaderProgram, int frameIndex, float animTime, const std::string& animPath, const Transform modelMatrix) {
            if (bakedAnimations.find(animPath) == bakedAnimations.end()) {
                std::cerr << " !! Animation not found: " << animPath << std::endl;
                return;
            } 
            const auto& frames = bakedAnimations[animPath];
            if (frames.empty()) {
                std::cerr << " !! No frames in animation: " << animPath << std::endl;
                return;
            }
            frameIndex = frameIndex % frames.size();
            int frameCount = (int)frames.size();
            if (frameCount == 0) return;
            frameIndex = frameIndex % frameCount;
            finalBoneMatrices = frames[frameIndex];
            GLint boneLoc = glGetUniformLocation(shaderProgram, "u_BoneMatrices");
            if (boneLoc == -1) {
                std::cerr << "Uniform 'u_BoneMatrices' not found in shader!" << std::endl;
            }
            const auto& boneMats = bakedAnimations[animPath][frameIndex];
            glUniformMatrix4fv(boneLoc, (GLsizei)boneMats.size(), GL_FALSE, glm::value_ptr(boneMats[0]));
            GLint isAnimatedLoc = glGetUniformLocation(shaderProgram, "isAnimated");
            glUniform1i(isAnimatedLoc, 1);
            glm::mat4 model = modelMatrix.toMat4();
            for (auto& mesh : baseMeshes) {
                GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                GLuint boneIDVBO, weightsVBO;       
                glGenBuffers(1, &boneIDVBO);
                glBindBuffer(GL_ARRAY_BUFFER, boneIDVBO);
                glBufferData(GL_ARRAY_BUFFER, mesh.BoneIDs.size() * sizeof(glm::ivec4), &mesh.BoneIDs[0], GL_STATIC_DRAW);
                glEnableVertexAttribArray(7);
                glVertexAttribIPointer(7, 4, GL_INT, sizeof(glm::ivec4), (void*)0);
                glGenBuffers(1, &weightsVBO);
                glBindBuffer(GL_ARRAY_BUFFER, weightsVBO);
                glBufferData(GL_ARRAY_BUFFER, mesh.Weights.size() * sizeof(glm::vec4), &mesh.Weights[0], GL_STATIC_DRAW);
                glEnableVertexAttribArray(8);
                glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
                mesh.draw(false);
                glDisableVertexAttribArray(7);
                glDisableVertexAttribArray(8);
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
                glDeleteBuffers(1, &boneIDVBO);
                glDeleteBuffers(1, &weightsVBO);
            }
            glUniform1i(isAnimatedLoc, 0);
        }

        void PrintNodeHierarchy(const aiNode* node, int depth = 0) {
            for (int i = 0; i < depth; ++i) std::cout << "  ";
            std::cout << node->mName.C_Str() << std::endl;
            for (unsigned int i = 0; i < node->mNumChildren; ++i) {
                PrintNodeHierarchy(node->mChildren[i], depth + 1);
            }
        }
    
};
#endif