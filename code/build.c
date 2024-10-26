#define STB_TRUETYPE_IMPLEMENTATION 
#include "stb_truetype.h"

// because we use stbtt_packedchar this is below
#include "monarch.h"

#define MOE_WIN32_GL_IMPLEMENTATION
#include "win32.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "input.c"
#include "memory.c"
#include "render.c"
#include "opengl_impl.c"
#include "str.c"
#include "utils.c"
#include "moemath.c"
#include "font.c"

//#include "pong.c"
//#include "main.c"
#include "sound.c"
