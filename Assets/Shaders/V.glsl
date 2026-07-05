#version 330 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec2 aUV;
layout (location = 2) in vec4 aColor;

out vec2 vUV;
out vec4 vColor;

uniform mat4 uProjection;

void main()
{
    gl_Position = uProjection * vec4(aPosition, 1.0);
    vUV         = aUV;
    vColor      = aColor;
}