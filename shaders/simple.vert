#version 410

layout (location = 0) in vec4 Position;
layout (location = 1) in vec4 Normal;
layout (location = 2) in vec2 TexCoord;

out vec2 vTexCoord;
out vec3 vNormal;
out vec4 vPosition;

uniform mat4 ProjectionViewModel;

// Need this matrix to transform the position
uniform mat4 ModelMatrix;

// Need this matrix to transform the normal
uniform mat3 NormalMatrix;

void main()
{
    vNormal = NormalMatrix * Normal.xyz;
    vTexCoord = TexCoord;
    vPosition = ModelMatrix * Position;
    gl_Position = ProjectionViewModel * Position;
}