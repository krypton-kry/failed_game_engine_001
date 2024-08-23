u32 moe_compile_shader(moe_arena* arena, moe_string vertex_path, moe_string fragment_path)
{
  moe_string vertex_src = moe_os_read_file(arena, vertex_path);	
  moe_string fragment_src = moe_os_read_file(arena, fragment_path);	

  u32 vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_src.str, 0);
  glCompileShader(vertex_shader);

  u32 success;
  char info[512];
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
  if(!success)
  {
    glGetShaderInfoLog(vertex_shader, 512, 0, info);
    log_warn("Vertex Shader Commpilation : %s", info);
  }

  u32 fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_src.str, 0);
  glCompileShader(fragment_shader);
  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);

  if(!success)
  {
    glGetShaderInfoLog(fragment_shader, 512, 0, info);
    log_warn("Fragment Shader Commpilation : %s", info);
  }

  u32 program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);
  glGetProgramiv(program, GL_LINK_STATUS, &success);

  if(!success)
  {
    glGetProgramInfoLog(program, 512, 0, info);
    log_warn("Linking : %s", info);
  }

  arena_dealloc(arena, vertex_src.size);
  arena_dealloc(arena, fragment_src.size);

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  log_verbose("Shader %s and %s compilation successful", vertex_path.str, fragment_path.str);
  return program;
}

void moe_clear_screen(vec4 color){
  glClearColor(color.r, color.g, color.b, color.a);
  glClear(GL_COLOR_BUFFER_BIT);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

u32 moe_create_texture_from_image(moe_string path){
  moe_image img = moe_load_image(path);
  if(!img.data){
    log_warn("Loading image %s to create texture failed", path.str);
    return 0;
  }

  u32 texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width, img.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  stbi_image_free(img.data);

  return texture;
}

void moe_create_renderer(i32 cap){

  u32 vao, vbo;
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);

  glBindVertexArray(vao);
  glEnable(GL_BLEND);  
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(moe_vertex) * cap, 0, GL_DYNAMIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(moe_vertex), (void*)0);

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(moe_vertex), (void*)offsetof(moe_vertex, tex_coord));

  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(moe_vertex), (void*)offsetof(moe_vertex, color));

  moe_renderer* ptr = arena_alloc(&ctx.arena, sizeof(moe_renderer));
  *ptr = (moe_renderer){
    .vao = vao,
    .vbo = vbo,
    .shader = 0,
    .texture = 0,
    .vertices = arena_alloc(&ctx.arena, sizeof(moe_vertex) * cap),
    .vertex_count = 0,
    .vertex_capacity = cap,
  };
  draw_frame.renderer = ptr;
}

// FIXME(krypton): is setting mvp here really necessary?
void moe_render_flush(moe_renderer* re){
  if(re->vertex_count == 0) return;

  glUseProgram(re->shader);
  glBindTexture(GL_TEXTURE_2D, draw_frame.renderer->texture);

  glBindBuffer(GL_ARRAY_BUFFER, draw_frame.renderer->vbo);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(moe_vertex) * draw_frame.renderer->vertex_count, draw_frame.renderer->vertices);

  glBindVertexArray(draw_frame.renderer->vao);
  glDrawArrays(GL_TRIANGLES, 0, draw_frame.renderer->vertex_count);

  //glUniformMatrix4fv(glGetUniformLocation(re->shader, "mvp"), 1, GL_FALSE, draw_frame.projection.raw[0]);

  draw_frame.renderer->vertex_count = 0;
}

void moe_push_vertex(moe_renderer* re, moe_vertex vertex){
  if( re->vertex_count == re->vertex_capacity){
    moe_render_flush(re);
  }
  re->vertices[re->vertex_count++] = vertex;
}

void moe_texture(moe_renderer* re, u32 texture){
  if(re->texture != texture){
    moe_render_flush(re);
    re->texture = texture;
  }
}

void moe_set_matrix_uniform(moe_string name, u32 shader, mat4x4 m){

  draw_frame.renderer->shader = shader;
  u32 loc = glGetUniformLocation(shader, name.str);
  glUniformMatrix4fv(loc, 1, GL_FALSE, &m.raw[0][0]);

}

void moe_set_viewport(u32 x, u32 y, u32 width, u32 height){
  glViewport(x, y, width, height);
}

u32 moe_create_font_texture(u8* pixels){
  u32 font_texture = 0;
  // Create OpenGL texture with the font pack pixel data.
  // Only uses one color channel since font data is a monochrome alpha mask. 
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glGenTextures(1, &font_texture);
  glBindTexture(GL_TEXTURE_2D, font_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RED, GL_UNSIGNED_BYTE, pixels);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

  return font_texture;
}
