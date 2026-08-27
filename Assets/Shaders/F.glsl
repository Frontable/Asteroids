#version 330 core

in vec2 vUV;
in vec4 vColor;

out vec4 FragColor;

uniform sampler2D uTexture;
uniform bool      uUseTexture;

void main()
{
    if (uUseTexture)
    {
        vec4 texColor = texture(uTexture, vUV) * vColor;

        // Discard fully transparent fragments
        // so they don't write to the depth buffer
        if (texColor.a < 0.01)
            discard;

        FragColor = texColor;
    }
    else
    {
        FragColor = vColor;
    }
}