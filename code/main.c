static void moe_init_engine() {
  arena_init(&ctx.arena);
  moe_os_create_window(800, 600, str_lit("MoeGame"));
  moe_create_renderer(1024);

  SHAPE_SHADER = moe_compile_shader(&ctx.arena, str_lit("rect.vert"), str_lit("rect.frag"));
  IMAGE_SHADER = moe_compile_shader(&ctx.arena, str_lit("image.vert"), str_lit("image.frag"));
}

// TODO (krypton) : 
// input handling
// text rendering
// audio

int main(void)
{
  moe_init_engine();  

  u32 tex = moe_create_texture_from_image(str_lit("player.png"));

  f64 last_time = moe_os_time();
  f64 seconds_counter = 0.0;
  i32 frame_count = 0;
  
  for (;;)
  {
    //log_info("%f %f", ctx.input.mouse.pos.x, ctx.input.mouse.pos.y);
    f64 now = moe_os_time();
    f64 delta_t = now - last_time;
    last_time = now;

    moe_os_handle_messages();
    moe_render_update();

    moe_draw_image(tex, V2(0.13, .16));
    moe_draw_rect(V2(0.13, 0.616), COLOR_RED);
    moe_draw_rect(V2(0.73, 0.216), COLOR_RED);
    moe_draw_rect(V2(0.63, 0.126), COLOR_RED);
    moe_draw_rect(V2(0.43, 0.16), COLOR_RED);
    moe_draw_rect(V2(0.13, 0.146), COLOR_RED);
    moe_draw_rect(V2(0.13, 0.16), COLOR_RED);

    moe_os_swap_buffers(&ctx);
    
    if(is_down(KEY_ESCAPE)){
      moe_os_destroy_window(&ctx.window);
    }

    seconds_counter += delta_t;
    frame_count += 1;
    if (seconds_counter > 1.0) {
      //log_info("fps: %i", frame_count);
      seconds_counter = 0.0;
      frame_count = 0;
    }
  }

  arena_free(&ctx.arena);
  return 0;
}
