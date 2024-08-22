#version 330 core
layout (location = 0) in vec2 inPosition;
out vec2 texturePos;

uniform vec2 resolution; // Resolution of window/display

uniform vec2 position; // Position of character based on above resolution.
                       // The coordinate is the leftmost pixel at the baseline of character.
                       // Text may go below the baseline (e.g. the bottom of g and j).

uniform vec2 size; // Size of character based on above resolution.

void main() {
    // Adjust inPosition (0,0 => 1,1 square) to the size and position provided by uniform values.
    vec2 pos = (inPosition * size / resolution);
    pos = pos + (position / resolution);
    
    // Move -1,-1 and scale by 2x since opengl viewport is 2x2 (-1,-1 => 1,1).
    gl_Position = vec4(pos * 2.0 - vec2(1.0, 1.0), 0.0, 1.0);
    
    // The inPosition with y flipped is used for texture position in fragment shader.
    texturePos = vec2(inPosition.x, (inPosition.y - 1.0) * -1.0);
}
