void moe_render_update(){
  moe_clear_screen(hex_to_rgba(0x121414ff));
  moe_reset_frame(&draw_frame);
  moe_render_flush(draw_frame.renderer);
}

void moe_reset_frame(moe_frame *frame){
  f32 aspect = ctx.window.width / ctx.window.height;
  frame->projection = m4_make_orthographic_projection(-aspect, aspect, -1, 1, -1, 10);
  frame->view = m4_scalar(1.0);
}

void moe_draw_quad(u32 shader, vec2 size, vec2 pos, vec4 color){
  
  moe_set_matrix_uniform(str_lit("mvp"), shader, draw_frame.projection);
  moe_push_vertex(draw_frame.renderer, (moe_vertex){.pos=V2(size.x + pos.x, size.y + pos.y), .tex_coord=V2(1, 1), .color=color}); // top right
  moe_push_vertex(draw_frame.renderer, (moe_vertex){.pos=V2(size.x + pos.x, 0 + pos.y), .tex_coord=V2(1, 0), .color=color}); // bottom right
  moe_push_vertex(draw_frame.renderer, (moe_vertex){.pos=V2(0 + pos.x, size.y + pos.y), .tex_coord=V2(0, 1), .color=color}); // top left

  moe_push_vertex(draw_frame.renderer, (moe_vertex){.pos=V2(size.x + pos.x, 0 + pos.y), .tex_coord=V2(1, 0), .color=color}); // bottom right
  moe_push_vertex(draw_frame.renderer, (moe_vertex){.pos=V2(0 + pos.x, 0 + pos.y), .tex_coord=V2(0, 0), .color=color}); // bottom left
  moe_push_vertex(draw_frame.renderer, (moe_vertex){.pos=V2(0 + pos.x, size.y + pos.y), .tex_coord=V2(0, 1), .color=color}); // top left
}

void moe_draw_rect(vec2 size, vec2 pos, vec4 color){
  moe_draw_quad(SHAPE_SHADER, size, pos, color);
}

void moe_draw_image(u32 texture, vec2 size, vec2 pos){
  moe_texture(draw_frame.renderer, texture);
  moe_draw_quad(IMAGE_SHADER, size, pos, COLOR_WHITE); 
  moe_texture(draw_frame.renderer, 0);
}

