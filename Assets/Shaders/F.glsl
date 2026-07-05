#version 330 core

in vec2 vUV;
in vec4 vColor;

out vec4 FragColor;

uniform sampler2D uTexture;
uniform bool      uUseTexture;

void main()
{
    if (uUseTexture)
        FragColor = texture(uTexture, vUV) * vColor;
    else
        FragColor = vColor;
}