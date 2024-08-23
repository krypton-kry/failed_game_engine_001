#version 330 core

layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in vec4 color;

out vec2 TexCoord;
out vec4 FColor;

uniform mat4 mvp;

void main()
{
    gl_Position = mvp * vec4(pos, 0.0, 1.0);
    TexCoord = texCoord;
    FColor = color;
}
