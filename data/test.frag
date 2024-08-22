#version 330 core

in vec2 texturePos;
out vec4 outColor;

uniform sampler2D fontTexture; // Texture from stb_truetype
uniform vec4 charPosition; // Position and size of character in texture

void main() {
    // Get size of texture in pixels and scale charPosition for texture(...) function
    ivec2 texSize = textureSize(fontTexture, 0);
    vec2 texPos = texturePos * ((charPosition.zw - charPosition.xy) / texSize.x) + charPosition.xy / texSize.y;

    // Get color/alpha mask of fragment from texture.
    float col = texture(fontTexture, texPos).r;

    // Generate a color pattern for text for demo.
    vec3 textColor = vec3(texPos.x, texturePos.y, texPos.y);

    // Set color using mask from front texture as alpha channel.
    outColor = vec4(textColor, col);
}
