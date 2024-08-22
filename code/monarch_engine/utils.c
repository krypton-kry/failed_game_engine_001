vec4 hex_to_rgba(u64 hex){
  return (vec4){
    .r = ((hex >> 24) & 0xFF) / 255.0f,
    .g = ((hex >> 16) & 0xFF) / 255.0f,
    .b = ((hex >> 8) & 0xFF) / 255.0f,
    .a = ((hex) & 0xFF) / 255.0f,
  };
}

moe_image moe_load_image(moe_string path){
  // NOTE(krypton) : if you flip this turn the texture to negative
  //stbi_set_flip_vertically_on_load(1);
  int width, height, channels;
  u8* data = stbi_load(path.str, &width, &height, &channels, 4);

  if(!data) ERROR_EXIT("Failed to load image");
  log_verbose("Image %s loaded -> width = %d, height = %d, channels = %d", path.str, width, height, channels); 
  //stbi_image_free(data);

  return (moe_image){
    .width=width,
    .height=height,
    .channels=channels,
    .data=data,
  };
}
