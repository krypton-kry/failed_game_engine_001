static void moe_init_engine() {
  arena_init(&ctx.arena);
  moe_os_create_window(800, 600, str_lit("MoeGame"));
  moe_create_renderer(1024);

  SHAPE_SHADER = moe_compile_shader(&ctx.arena, str_lit("rect.vert"), str_lit("rect.frag"));
  IMAGE_SHADER = moe_compile_shader(&ctx.arena, str_lit("image.vert"), str_lit("image.frag"));
}

// TODO (krypton) :
// Handle Multiple keys
// text rendering
// audio
// Do cleanup at end instead of just destoying the window?

#define TEXTURE_WIDTH 1024
#define TEXTURE_HEIGHT 1024
#define TEXTURE_FONT_SIZE 200.0f

GLuint fontTexture;
// Load a TTF file into a texture and return the character data.
stbtt_packedchar* LoadFont(const char* filename) {
  // Load TTF file into memory.
  u8* ttfData = moe_os_read_binary_file(&ctx.arena, str_lit(filename));
  // Pack TTF into pixel data using stb_truetype.
  stbtt_pack_context packContext;
  stbtt_packedchar *charData = (stbtt_packedchar*)arena_alloc(&ctx.arena, 126 * sizeof(stbtt_packedchar));
  unsigned char* pixels = (unsigned char*)arena_alloc(&ctx.arena, TEXTURE_WIDTH * TEXTURE_HEIGHT * sizeof(char));
  stbtt_PackBegin(&packContext, pixels, TEXTURE_WIDTH, TEXTURE_HEIGHT, TEXTURE_WIDTH, 1, NULL);

  // Pack unicode codepoints 0 to 125 into the texture and character data. If a different starting
  // point than 0 is picked then lookups in charData array must be offset by that number.
  // With 0-125 the uppercase A will be at charData[65].
  // With 32-125 the uppercase A will be at charData[65-32].
  // The TEXTURE_FONT_SIZE must be a value which can accomodate the generated data inside the texture size
  // else everything will be zero
  stbtt_PackFontRange(&packContext, (unsigned char*)ttfData, 0, TEXTURE_FONT_SIZE, 0, 125, charData);
  stbtt_PackEnd(&packContext);

  //stbi_write_png("Pixel.png", TEXTURE_WIDTH, TEXTURE_HEIGHT, 1, pixels, 0);
  // Create OpenGL texture with the font pack pixel data.
  // Only uses one color channel since font data is a monochrome alpha mask.
  glGenTextures(1, &fontTexture);
  glBindTexture(GL_TEXTURE_2D, fontTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, pixels);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // The data is in Pixel format ie., in relation to the actual pixels
  return charData;
}

// Calculate pixel width of a string.
f32 TextWidth(const char* text, f32 height, stbtt_packedchar *charData) {
  f32 w = 0.0f;

  for (int i = 0; i < strlen(text); i++) {
    stbtt_packedchar c = charData[text[i]];
    w += c.xadvance;
  }
  // Scale width by rendered height to texture generated height ratio for final size.
  return w * (height / TEXTURE_FONT_SIZE);
}

static void push_vertex(f32 a, f32 b, f32 c, f32 d){
  moe_push_vertex(draw_frame.renderer, (moe_vertex){.pos=V2(a,b), .tex_coord=V2(c,d), .color=COLOR_RED});
}

static void render_text(stbtt_packedchar* char_data, char* str, u32 tex, vec2 pos)
{
  // aligns to the left
  //pos.x -= TextWidth(str, 200, char_data) / 2.0f;
  f32 scale = 0.25f;
  for(int i = 0; i < strlen(str); i++){
    stbtt_packedchar ch = char_data[str[i]];

    //(x1, y1) and (x2, y2) draw the rectangle on which the texture is drawn
    f32 x1 = pos.x;
    f32 y1 = pos.y;
    f32 size[2] = {(ch.xoff2 - ch.xoff) * scale, (ch.yoff2 - ch.yoff) * scale};

    f32 x2 = x1 + size[0];
    f32 y2 = y1 + size[1];


    // in the texture this is the lefttop and rightbottom of the char
    vec4 uv = V4(ch.x0, ch.y0, ch.x1, ch.y1);

    moe_texture(draw_frame.renderer, tex);
    moe_set_matrix_uniform(str_lit("mvp"), IMAGE_SHADER, m4_mul(draw_frame.projection, draw_frame.view));

    f32 u1 = uv.x / TEXTURE_WIDTH;
    f32 v1 = uv.y / TEXTURE_HEIGHT;
    f32 u2 = uv.z / TEXTURE_WIDTH;
    f32 v2 = uv.w / TEXTURE_HEIGHT;

    push_vertex(x1, y1, u1, v1);
    push_vertex(x2, y2, u2, v2);
    push_vertex(x1, y2, u1, v2);

    push_vertex(x1, y1, u1, v1);
    push_vertex(x2, y1, u2, v1);
    push_vertex(x2, y2, u2, v2);
  
    // FIXME(krypton) : WTF is this 17?
    pos.x += ch.xadvance * scale;
  }
}

int main(void)
{
  moe_init_engine();
  u32 tex = moe_create_texture_from_image(str_lit("player.png"));
  //u32 font = moe_create_texture_from_image(str_lit("Pixel.png"));

  f64 last_time = moe_os_time();
  f64 seconds_counter = 0.0;
  i32 frame_count = 0;

  f32 speed = 1.5f;
  vec2 player_pos = V2(0,0);
  char str[10] = {0};
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  stbtt_packedchar *comic = LoadFont("Jet.ttf");
  for (;;)
  {
    f64 now = moe_os_time();
    f64 delta_t = now - last_time;
    last_time = now;

    moe_os_handle_messages();
    moe_render_update();

    //FIXME : Diagonal Movement is seperate?
    vec2 input = V2(0,0);
    if(is_down('W')){
      input.y -= 100.0f;
    } else if(is_down('A')){
      input.x -= 100.0f;
    } else if(is_down('S')){
      input.y += 100.0f;
    } else if(is_down('D')){
      input.x += 100.0f;
    }

    player_pos = v2_add(player_pos, v2_mulf(input, speed * delta_t));

    // render
    {
      moe_draw_image(tex, V2(0.13 * 100, 0.16 * 100), player_pos);
      //moe_draw_rect(V2(200, 200), V2(200, 100), COLOR_WHITE);
    }        

    // render piece of tex
    {
      render_text(comic, str, fontTexture, V2(0, 0));
    }

    moe_os_swap_buffers(&ctx);
    if(is_down(KEY_ESCAPE)){
      moe_os_destroy_window(&ctx.window);
    }

    seconds_counter += delta_t;
    frame_count += 1;
    if (seconds_counter > 1.0) {
      //log_verbose("fps: %i", frame_count);
      sprintf(str, "FPS : %d", frame_count);
      seconds_counter = 0.0;
      frame_count = 0;
    }
  }

  arena_free(&ctx.arena);
  return 0;
}
