#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertices_position_modelspace;
layout(location = 1) in vec2 TexCoord;
layout(location = 2) in vec3 Normal_modelspace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform int useHeightMap;
uniform float scale;
uniform int isPBR;



out vec2 TexCoordout;
out float height;
out vec3 worldPos;
out vec3 Normal;
flat out int useHeightMapout;
flat out int isPBRout;

void main(){
        vec3 vertexPosition = vertices_position_modelspace;
        TexCoordout=TexCoord;
        useHeightMapout = useHeightMap ;
        isPBRout = isPBR;
        gl_Position = proj*view*model*(vec4(vertexPosition,1.0));
        worldPos = vertexPosition;
        height = vertexPosition.y;
        Normal = Normal_modelspace;

}