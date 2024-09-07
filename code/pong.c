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
moe_bool is_colliding(vec2 pos1, vec2 size1, vec2 pos2 , vec2 size2){
  moe_bool collisonx = pos1.x + size1.x >= pos2.x && pos2.x + size2.x >= pos1.x;
  moe_bool collisony = pos1.y + size1.y >= pos2.y && pos2.y + size2.y >= pos1.y;

  return collisonx && collisony;
}

int main(void)
{
  moe_init_engine();

  moe_font font = moe_load_font_data(str_lit("Silkscreen.ttf"));
  u32 circle = moe_create_texture_from_image(str_lit("circle.png"));

  f64 last_time = moe_os_time();
  f64 seconds_counter = 0.0;
  i32 frame_count = 0;

  char fps_str[10]; 
  char enemy_score_str[20]; 
  char player_score_str[20]; 

  vec2 player_pos = V2(20, ctx.window.height/2);
  vec2 enemy_pos = V2(ctx.window.width - 40, ctx.window.height/2);
  vec2 ball_pos = V2(ctx.window.width/2, ctx.window.height/2);
  vec2 paddle_size = V2(20, 105);
  vec2 ball_size = V2(20, 20);
  
  vec2 velocity = V2(300, 0);
  i32 player_score = 0;
  i32 enemy_score = 0;

  //FIXME(krypton) : disable resize
  //Nah it isnt
  //FIXME(krypton): quad centering
  for (;;)
  {
    moe_set_viewport(0, 0, ctx.window.width, ctx.window.height);
    f64 now = moe_os_time();
    f64 delta_t = now - last_time;
    last_time = now;

    moe_os_handle_messages();
    moe_render_update();
  
    if(is_down(KEY_ESCAPE)){
      moe_os_destroy_window(&ctx.window);
    }

    snprintf(player_score_str, 20, "%d", player_score);
    snprintf(enemy_score_str, 20, "%d", enemy_score);
    // render
    {
      moe_render_text(&font, fps_str, V2(0,0), 30);

      //scores
      moe_render_text(&font, player_score_str, V2(200, 20), 50);
      moe_render_text(&font, enemy_score_str, V2(ctx.window.width - 200, 20), 50);
      
      //central line
      for (int i = 0; i < 10; i++){
        moe_draw_rect(V2(2, 25), V2(ctx.window.width/2, 100 + i * 40), COLOR_GREY);
      }
      
      if(is_down(KEY_UP)){
        player_pos.y -= 400 * delta_t;
      }

      if(is_down(KEY_DOWN)){
        player_pos.y += 400 * delta_t;
      }
      
      if(player_pos.y > 525) {
        player_pos.y = 525;
      }
      if(player_pos.y < 0){
        player_pos.y = 0;
      }
      //players
      moe_draw_rect(paddle_size, player_pos, COLOR_WHITE);
      moe_draw_rect(paddle_size, enemy_pos, COLOR_WHITE);

      //ball
      moe_draw_image(circle, ball_size, ball_pos);
    } 

    // update
    {
      ball_pos.x += (velocity.x * delta_t);
      ball_pos.y += (velocity.y * delta_t);

      // colission
      if(is_colliding(enemy_pos, paddle_size, ball_pos, ball_size)){
        f32 res_y = (enemy_pos.y + (paddle_size.y / 2)) - ball_pos.y;
        f32 nres_y = res_y / (paddle_size.y/2);
        f32 angle = nres_y * 1.31;
        
        velocity.x = -velocity.x;
        velocity.y = -velocity.y + (50 * -sin(angle));
      }

      if(is_colliding(player_pos, paddle_size, ball_pos, ball_size)){
        f32 res_y = (player_pos.y + (paddle_size.y / 2)) - ball_pos.y;
        f32 nres_y = res_y / (paddle_size.y/2);
        f32 angle = nres_y * 1.31;
        
        velocity.x = -velocity.x;
        velocity.y = -velocity.y + (50 * -sin(angle));
      }

      //ai
      {
        enemy_pos.y = ball_pos.y - (paddle_size.y / 2);
        if(enemy_pos.y < 0) {
          enemy_pos.y += paddle_size.y/2;
        }
      }

      // add score if out of bound
      // TODO(krypton) : y axis
      if(ball_pos.x > ctx.window.width){
        player_score += 1; 
        ball_pos.x = ctx.window.width / 2;
        ball_pos.y = ctx.window.height / 2;
      } else if(ball_pos.x < 0){
        enemy_score +=1;
        ball_pos.x = ctx.window.width / 2;
        ball_pos.y = ctx.window.height / 2;
      }


      if(ball_pos.y > ctx.window.height){
        velocity.y -= velocity.y;
      } else if(ball_pos.y < 0){
        velocity.y -= velocity.y;
      }
    }

    moe_os_swap_buffers(&ctx);

    seconds_counter += delta_t;
    frame_count += 1;
    if (seconds_counter > 1.0) {
      //log_verbose("fps: %i", frame_count);
      sprintf(fps_str, "FPS : %d", frame_count);
      seconds_counter = 0.0;
      frame_count = 0;
    }
  }

  arena_free(&ctx.arena);
  return 0;
}
