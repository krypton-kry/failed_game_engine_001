mat4x4 m4_scalar(f32 scalar) {
  mat4x4 m = {0};

  m.raw[0][0] = scalar;
  m.raw[1][1] = scalar;
  m.raw[2][2] = scalar;
  m.raw[3][3] = scalar;
  
  return m;
}

mat4x4 m4_mul(mat4x4 a, mat4x4 b) {
  mat4x4 result;
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      result.raw[i][j] = a.raw[i][0] * b.raw[0][j] +
        a.raw[i][1] * b.raw[1][j] +
        a.raw[i][2] * b.raw[2][j] +
        a.raw[i][3] * b.raw[3][j];
    }
  }
  return result;
}

mat4x4 m4_make_orthographic_projection(f32 left, f32 right, f32 bottom, f32 top, f32 _near, f32 _far) {
  mat4x4 m = m4_scalar(1.0f);

  m.raw[0][0] = 2.0f / (right - left);
  m.raw[1][1] = 2.0f / (top - bottom);
  m.raw[2][2] = -2.0f / (_far - _near);
  m.raw[3][0] = -(right + left) / (right - left);
  m.raw[3][1] = -(top + bottom) / (top - bottom);
  m.raw[3][2] = -(_far + _near) / (_far - _near);
  m.raw[3][3] = 1.0f;

  return m;
}

mat4x4 m4_make_scale(vec3 scale) {
  mat4x4 m = m4_scalar(1.0);
  m.raw[0][0] = scale.x;
  m.raw[1][1] = scale.y;
  m.raw[2][2] = scale.z;
  m.raw[3][3] = 1.0f;
  return m;
}

vec2 v2_add(vec2 a, vec2 b){
  return V2(a.x+b.x, a.y+b.y);
}

f32 v2_length(vec2 v){
  return sqrt((v.x * v.x) + (v.y * v.y));
}

vec2 v2_mulf(vec2 v, f32 f){
  return V2(v.x*f, v.y*f);
}

vec2 v2_divf(vec2 v, f32 f){
  return V2(v.x/f, v.y/f);
}

vec2 v2_normalize(vec2 v){
  f32 length = v2_length(v);
  if(length == 0) return V2(0, 0);
  return v2_divf(v, length);
}

