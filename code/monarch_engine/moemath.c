mat4x4 m4_scalar(f32 scalar) {
  mat4x4 m = {0};

  m.raw[0][0] = scalar;
  m.raw[1][1] = scalar;
  m.raw[2][2] = scalar;
  m.raw[3][3] = scalar;
  
  return m;
}

mat4x4 m4_make_orthographic_projection(f32 left, f32 right, f32 bottom, f32 top, f32 _near, f32 _far) {
  mat4x4 m = m4_scalar(1.0f);

  m.raw[0][0] = 2.0f / (right - left);
  m.raw[1][1] = 2.0f / (top - bottom);
  m.raw[2][2] = -2.0f / (_far - _near);
  m.raw[0][3] = -(right + left) / (right - left);
  m.raw[1][3] = -(top + bottom) / (top - bottom);
  m.raw[2][3] = -(_far + _near) / (_far - _near);
  m.raw[3][3] = 1.0f;
  
  return m;
}
