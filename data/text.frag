#version 330 core
in vec2 texpos;
out vec4 color;

uniform sampler2D tex;

void main(void) {
  gl_FragColor = vec4(1.0f, 1.0f, 1.0f, texture2D(tex, texpos).a) * vec4(1.0f, 1.0f, 1.0f, 1.0f);
}

