void moe_handle_mouse_motion(f32 x, f32 y)
{
	ctx.input.mouse.pos.x = x;
	ctx.input.mouse.pos.y = y;
}

void moe_handle_window_resize(f32 width, f32 height)
{
	ctx.window.width = width;
	ctx.window.height = height;
	moe_set_viewport(0, 0, width, height);
}

void moe_handle_inputs(moe_input_key key, u8 is_down){
  log_verbose("Key Pressed : %d  is_down : %d", key, is_down);
  ctx.input.keys[key].is_down = is_down; 
}

u8 is_down(moe_input_key key){
  return ctx.input.keys[key].is_down;
}
