static void moe_init_engine() {
  arena_init(&ctx.arena);
  moe_os_create_window(800, 600, str_lit("MoeGame"));
  moe_create_renderer(1024);

  SHAPE_SHADER = moe_compile_shader(&ctx.arena, str_lit("rect.vert"), str_lit("rect.frag"));
  IMAGE_SHADER = moe_compile_shader(&ctx.arena, str_lit("image.vert"), str_lit("image.frag"));
  TEXT_SHADER = moe_compile_shader(&ctx.arena, str_lit("text.vert"), str_lit("text.frag"));
}

// TODO (krypton) :
// audio
// Do cleanup at end instead of just destoying the window?

int main(void)
{
  moe_init_engine();
  
  u32 tex = moe_create_texture_from_image(str_lit("player.png"));
  moe_font font = moe_load_font_data(str_lit("Jet.ttf"));

  f64 last_time = moe_os_time();
  f64 seconds_counter = 0.0;
  i32 frame_count = 0;

  f32 speed = 1.5f;
  vec2 player_pos = V2(0,0);
  char str[10]; 

  for (;;)
  {
    f64 now = moe_os_time();
    f64 delta_t = now - last_time;
    last_time = now;

    moe_os_handle_messages();
    moe_render_update();

    // movement
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
    
    if(is_down(KEY_ESCAPE)){
      moe_os_destroy_window(&ctx.window);
    }

    player_pos = v2_add(player_pos, v2_mulf(input, speed * delta_t));

    // render
    {
      moe_draw_image(tex, V2(0.13 * 100, 0.16 * 100), player_pos);
      moe_draw_rect(V2(200, 200), V2(200, 100), COLOR_WHITE);
      moe_render_text(&font, str, V2(0,0), 30);
    }        

    moe_os_swap_buffers(&ctx);

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
