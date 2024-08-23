moe_font moe_load_font_data(moe_string font){

  // Load TTF file into memory.
  u8* ttf_data = moe_os_read_binary_file(&ctx.arena, font);
  if( !ttf_data ) ERROR_EXIT("Font %s not found!", font.str);

  // Pack TTF into pixel data using stb_truetype.
  stbtt_pack_context pack_context;
  stbtt_packedchar *char_data = (stbtt_packedchar*)arena_alloc(&ctx.arena, 126 * sizeof(stbtt_packedchar));
  u8* pixels = (u8*)arena_alloc(&ctx.arena, TEXTURE_WIDTH * TEXTURE_HEIGHT * sizeof(char));
  stbtt_PackBegin(&pack_context, pixels, TEXTURE_WIDTH, TEXTURE_HEIGHT, TEXTURE_WIDTH, 1, NULL);

  // Pack unicode codepoints 0 to 125 into the texture and character data. If a different starting
  // point than 0 is picked then lookups in charData array must be offset by that number.
  // With 0-125 the uppercase A will be at charData[65].
  // With 32-125 the uppercase A will be at charData[65-32].
  // The TEXTURE_FONT_SIZE must be a value which can accomodate the generated data inside the texture size
  // else everything will be zero
  stbtt_PackSetOversampling(&pack_context, 1, 1); // reduce padding
  stbtt_PackFontRange(&pack_context, (u8*)ttf_data, 0, TEXTURE_FONT_SIZE, 0, 125, char_data);
  stbtt_PackEnd(&pack_context);

  u32 font_texture = moe_create_font_texture(pixels);
  // The data is in Pixel format ie., in relation to the actual pixels
  return (moe_font){
    .char_data = char_data,
    .texture = font_texture
  };
}

//FIXME(krypton) : small text doesnt offset properly
//TODO(krypton) : turn str into moe_string and remove strlen
void moe_render_text(moe_font *font, char* str, vec2 pos, f32 size)
{
  f32 scale = size / TEXTURE_FONT_SIZE;
  for(int i = 0; i < strlen(str); i++){
    stbtt_packedchar ch = font->char_data[str[i]];

    //(x1, y1) and (x2, y2) draw the rectangle on which the texture is drawn
    f32 x1 = pos.x;
    f32 y1 = pos.y;
    f32 size[2] = {(ch.xoff2 - ch.xoff) * scale, (ch.yoff2 - ch.yoff) * scale};

    f32 x2 = x1 + size[0];
    f32 y2 = y1 + size[1];


    // in the texture this is the lefttop and rightbottom of the char
    vec4 uv = V4(ch.x0, ch.y0, ch.x1, ch.y1);

    moe_texture(draw_frame.renderer, font->texture);
    moe_set_matrix_uniform(str_lit("mvp"), TEXT_SHADER, m4_mul(draw_frame.projection, draw_frame.view));

    f32 u1 = uv.x / TEXTURE_WIDTH;
    f32 v1 = uv.y / TEXTURE_HEIGHT;
    f32 u2 = uv.z / TEXTURE_WIDTH;
    f32 v2 = uv.w / TEXTURE_HEIGHT;

    moe_push_vertex(draw_frame.renderer, (moe_vertex){.pos=V2(x1, y1), .tex_coord=V2(u1, v1), .color=V4(1, 1, 1, 1)});
    moe_push_vertex(draw_frame.renderer, (moe_vertex){.pos=V2(x2, y2), .tex_coord=V2(u2, v2), .color=V4(1, 1, 1, 1)});
    moe_push_vertex(draw_frame.renderer, (moe_vertex){.pos=V2(x1, y2), .tex_coord=V2(u1, v2), .color=V4(1, 1, 1, 1)});
    moe_push_vertex(draw_frame.renderer, (moe_vertex){.pos=V2(x1, y1), .tex_coord=V2(u1, v1), .color=V4(1, 1, 1, 1)});
    moe_push_vertex(draw_frame.renderer, (moe_vertex){.pos=V2(x2, y1), .tex_coord=V2(u2, v1), .color=V4(1, 1, 1, 1)});
    moe_push_vertex(draw_frame.renderer, (moe_vertex){.pos=V2(x2, y2), .tex_coord=V2(u2, v2), .color=V4(1, 1, 1, 1)});

    pos.x += ch.xadvance * scale;
  }
}
