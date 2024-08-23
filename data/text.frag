#version 330 core

in vec2 TexCoord;
in vec4 FColor;
out vec4 FragColor;

uniform sampler2D text;

void main()
{
  vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoord).r);
  FragColor = FColor * sampled;
}
