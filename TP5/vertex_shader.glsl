#version 330 core
#define MAX_BONES 100

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertices_position_modelspace;
layout(location = 1) in vec2 TexCoord;
layout(location = 2) in vec3 Normal_modelspace;
layout(location = 3) in vec2 pos;
layout(location = 4) in vec2 corduv;
layout(location = 5) in vec2 posText;
layout(location = 6) in vec2 corduvText;
layout(location = 7) in ivec4 a_boneIDs;
layout(location = 8) in vec4 a_weights;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform bool isHUD;
uniform bool isText;
uniform bool isAnimated;

uniform mat4 u_BoneMatrices[MAX_BONES];

out vec2 TexCoordout;
out float height;
out vec3 worldPos;
out vec3 Normal;

void main(){
        if(isHUD){
                gl_Position = vec4(pos, 0.0, 1.0);
                TexCoordout = corduv;
        }else if(isText){
                vec2 vertexPosition_homoneneousspace = posText - vec2(400,300);
                vertexPosition_homoneneousspace /= vec2(400,300); 
                gl_Position =  vec4(vertexPosition_homoneneousspace,0,1);
                TexCoordout = corduvText;
        }
        else if (isAnimated) {
                vec4 skinnedPos = vec4(0.0);
                mat3 skinnedNormalMatrix = mat3(0.0);
                for (int i = 0; i < 4; i++) {
                int boneID = a_boneIDs[i];
                float weight = a_weights[i];
                        if (boneID >= 0) {
                                mat4 boneMatrix = u_BoneMatrices[boneID];
                                skinnedPos += weight * (boneMatrix * vec4(vertices_position_modelspace, 1.0));
                                skinnedNormalMatrix += weight * mat3(boneMatrix); // On prend la partie rotation
                        }
                }
                gl_Position = proj * view * model * skinnedPos;
                
                Normal = normalize(skinnedNormalMatrix * Normal_modelspace);
                TexCoordout = TexCoord;
                worldPos = skinnedPos.xyz;
                height = skinnedPos.y;
        } 
        else{
                vec3 vertexPosition = vertices_position_modelspace;
                TexCoordout=TexCoord;
                gl_Position = proj*view*model*(vec4(vertexPosition,1.0));
                worldPos = vertexPosition;
                height = vertexPosition.y;
                Normal = Normal_modelspace;
        }
}