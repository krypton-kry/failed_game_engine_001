#ifndef MONARCH_H
#define MONARCH_H

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "moegl.h"

/*
 * =======================================================
 *						HELPERS
 * =======================================================
 */

#ifndef MOE_API
#define MOE_API extern
#endif

#ifndef MOE_LIB
#define MOE_LIB static
#endif

#define internal static inline 
#define global static

#define Kilobytes(bytes) bytes * 1024
#define Megabytes(bytes) Kilobytes(bytes) * 1024
#define Gigabytes(bytes) Megabytes(bytes) * 1024

#define ERROR_EXIT(...) exit((fprintf(stderr, "[ERROR] "__FILE__##": "##__VA_ARGS__), 1))

#define ASSERT_PRINT(...) fprintf(stderr, "Assert failed on %s, line %d", __FILE__ __VA_ARGS__, __LINE__);
#define ASSERT(c, ...) \
do {\
if( !(c) ){\
ASSERT_PRINT(__VA_ARGS__)\
*(int*)0 = 0;\
}\
} while(0)


#define MIN(x, y) ((x) > (y) ? (y) : (x))
#define MAX(x, y) ((x) < (y) ? (y) : (x))

#define STRINGIFY_S(x) #x
#define STRINGIFY(x) STRINGIFY_S(x)

#define MOE_TRUNCATE_TO_NEAREST(X, Y) ((size_t)(X) & ~((Y) - 1))

#define ARRAY_COUNT(x) (sizeof(x)/sizeof((x)[0]))

/*
 * =======================================================
 *						TYPES
 * =======================================================
 */

typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16; 
typedef int8_t i8; 

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef float f32;
typedef double f64;

typedef enum { moe_false, moe_true } moe_bool;

/*
 * =======================================================
 *						API
 * =======================================================
 */
//API
typedef struct moe_context moe_context;

// input
typedef struct moe_input moe_input;
typedef struct moe_mouse moe_mouse;
typedef enum moe_input_event moe_input_event;
typedef enum moe_input_key moe_input_key;
typedef struct moe_key moe_key;

// math
typedef union vec2 vec2;
typedef union vec3 vec3;
typedef union vec4 vec4;
typedef union mat4x4 mat4x4;

//platform
typedef struct moe_window moe_window;

//codebase
typedef struct moe_string moe_string;
typedef struct moe_arena moe_arena;

//utils
typedef struct moe_image moe_image;

//rendering
typedef struct moe_frame moe_frame;
typedef struct moe_quad moe_quad;
typedef struct moe_renderer moe_renderer;
typedef struct moe_vertex moe_vertex;
typedef struct moe_font moe_font;

//audio
typedef struct moe_wav_file moe_wav_file;

/*
 * =======================================================
 *						MATH
 * =======================================================
 */

union vec2 {
    struct { f32 x,y; };
    f32 raw[2];	
};

union vec3 {
    struct { f32 x,y,z; };
    struct { f32 r,b,g; };
    struct { vec2 xy; };
    f32 raw[3];
};

union vec4 {
    struct { f32 x,y,z,w; };
    struct { f32 r,b,g,a; };
    struct { vec2 xy; };
    f32 raw[4];
};

union mat4x4
{
    f32 raw[4][4];
    vec4 columns[4];
};

#define V2(x, y) (vec2){x, y}
#define V3(x, y, z) (vec3){x, y, z}
#define V4(x, y, z, w) (vec4){x, y, z, w}

MOE_API mat4x4 m4_make_orthographic_projection(f32 left, f32 right, f32 bottom, f32 top, f32 _near, f32 _far);
MOE_API mat4x4 m4_scalar(f32 scalar);
MOE_API mat4x4 m4_make_scale(vec3 scale);
MOE_API mat4x4 m4_mul(mat4x4 a, mat4x4 b);
MOE_API vec2 v2_add(vec2 a, vec2 b);
MOE_API f32 v2_length(vec2 v);
MOE_API vec2 v2_mulf(vec2 v, f32 f);
MOE_API vec2 v2_divf(vec2 v, f32 f);
MOE_API vec2 v2_normalize(vec2 v);

/*
 * =======================================================
 *						CODEBASE
 * =======================================================
 */
//memory
struct moe_arena {
    void* memory;
    u64		commit_position;
    u64		alloc_position;
    u64		max;
};

#define DEFAULT_MAX_ARENA Megabytes(64)
#define ARENA_COMMIT_SIZE Kilobytes(8)

MOE_API void* arena_alloc(moe_arena* arena, u64 size);
MOE_API void arena_init(moe_arena* arena);
MOE_API void arena_init_sized(moe_arena* arena, u64 size);
MOE_API void* arena_alloc_array_sized(moe_arena* arena, u64 elem_size, u64 count);
MOE_API void arena_free(moe_arena* arena);
MOE_API void arena_dealloc(moe_arena* arena, u64 size);

#define arena_alloc_array(arena, elem_type, count) arena_alloc_array_sized(arena, sizeof(elem_type), count)


//str 
struct moe_string {
    u8* str;
    u64 size;
};

#define str_lit(s) (moe_string){.str = (u8*)s, .size = sizeof(s)-1} 

MOE_API moe_string str_concat(moe_arena* arena, moe_string a, moe_string b);
MOE_API u64 str_count_substr(moe_arena* arena, moe_string text, moe_string pattern);
MOE_API moe_bool str_find_first(moe_arena* arena, moe_string text, moe_string pattern);

/*
 * =======================================================
 *						PLATFORM	
 * =======================================================
 */

struct moe_window {
    void* handle;
    f32 width;
    f32 height;
    moe_string title;
};

MOE_API moe_window moe_os_create_window(u32 width, u32 height, moe_string title);
MOE_API void moe_os_destroy_window(moe_window* window);
MOE_API void moe_os_swap_buffers(moe_context* ctx);
MOE_API void moe_os_show_window(moe_context* ctx);
MOE_API void moe_os_handle_messages();

MOE_API void* moe_os_reserve_memory(u64 size);
MOE_API void moe_os_commit_memory(char* memory, u64 size);
MOE_API void moe_os_free_memory(void* memory);

MOE_API moe_string moe_os_read_file(moe_arena* arena, moe_string filename);
MOE_API u8* moe_os_read_binary_file(moe_arena* arena, moe_string filename);

MOE_API void  moe_os_print(char* fmt, ...);
MOE_API f64 moe_os_time();

MOE_API void moe_os_init_sound();

MOE_LIB moe_input_key moe_os_to_key(void* key);
MOE_LIB void moe_hide_cursor();
MOE_LIB moe_bool moe_os_file_exists(moe_string path);

/*
 * =======================================================
 *							INPUT
 * =======================================================
 */

struct moe_mouse {
    vec2 pos; // position of the mouse in relation to the window
    // left bottom is (0, 0) and right top is (w, h) of window
};

struct moe_key {
    u8 is_down;
    u8 changed;
};

// TODO Add a WINDOW_RESIZE event if needed
// TODO Add Scroll event
enum moe_input_event {
    INPUT_EVENT_KEY,
    INPUT_EVENT_MOUSE,
};

// TODO Add extra keys if needed
// https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
enum moe_input_key {
    KEY_NIL = 0,
    
    KEY_TAB = 9,
    KEY_ENTER = 13,
    
    KEY_ESCAPE  = 27,
    KEY_SPACEBAR  = 32,
    
    KEY_LEFT = 37,
    KEY_UP = 38,
    KEY_RIGHT = 39,
    KEY_DOWN = 40,
    
    // A - Z, a-z 65-122
    
    KEY_MOUSE_LEFT = 130,
    KEY_MOUSE_MIDDLE = 131,
    KEY_MOUSE_RIGHT = 132,
    
    KEY_MAX // we dont set MAX here!
};

struct moe_input {
    moe_mouse mouse;
    moe_key keys[KEY_MAX];
};

// TODO can be merged into a single function if all needed events are added
MOE_API void moe_handle_mouse_motion(f32 x, f32 y);
MOE_API void moe_handle_window_resize(f32 width, f32 height);
MOE_API void moe_handle_inputs(moe_input_key key, u8 is_down);
MOE_API u8 is_down(moe_input_key key);

/*
 * =======================================================
 *						LOGGING
 * =======================================================
 */
#define log_info(fmt, ...) moe_os_print("[INFO] "fmt"\n", __VA_ARGS__)
#if _DEBUG
#define log_verbose(fmt, ...) moe_os_print("[VERBOSE] "fmt"\n", __VA_ARGS__)
#else
#define log_verbose(...)
#endif

#define log_warn(fmt, ...) moe_os_print("[WARN] "fmt"\n", __VA_ARGS__)

#define logp(x) puts(STRINGIFY(x));
#define log_int(x) log_info(#x " -> %d", x)

/*
 * =======================================================
 *						UTILS
 * =======================================================
 */
struct moe_image {
    u32 width;
    u32 height;
    u32 channels;
    u8* data;
};

#define COLOR_BLACK V4(0, 0, 0, 1)
#define COLOR_WHITE V4(1, 1, 1, 1)
#define COLOR_GREY V4(0.6, 0.6, 0.6, 1)
#define COLOR_RED V4(1, 0, 0, 1)

MOE_API vec4 hex_to_rgba(u64 hex);
MOE_API moe_image moe_load_image(moe_string path); // stbi_image_free this

/*
 * =======================================================
 *							Render
 * =======================================================
 */

#define TEXTURE_WIDTH 1024
#define TEXTURE_HEIGHT 1024
#define TEXTURE_FONT_SIZE 200.0f

struct moe_vertex {
    vec2 pos;
    vec2 tex_coord;
    vec4 color;
};

struct moe_renderer{ 
    u32 vao;
    u32 vbo;
    u32 shader;
    u32 texture;
    
    moe_vertex *vertices;
    u32 vertex_count;
    u32 vertex_capacity;
};

struct moe_frame {
    mat4x4 view;
    mat4x4 projection;
    moe_renderer* renderer;
};

struct moe_font {
    stbtt_packedchar *char_data;
    u32 texture;
};

MOE_API void moe_render_update();
MOE_API void moe_reset_frame(moe_frame *frame);

MOE_API void moe_draw_quad(u32 shader, vec2 size, vec2 pos, vec4 color);
MOE_API void moe_draw_rect(vec2 size, vec2 pos, vec4 color);
MOE_API void moe_draw_image(u32 texture, vec2 size, vec2 pos);

/*
 * =======================================================
 *							FONT
 * =======================================================
 */

MOE_API moe_font moe_load_font_data(moe_string font);
MOE_API void moe_render_text(moe_font *font, char* str, vec2 pos, f32 size);

/*
 * =======================================================
 *							AUDIO
 * =======================================================
 */

struct moe_wav_file {
    // RIFF Chunk
    u32 riff_id;
    u32 riff_chunk_size;
    u32 wave_id;
    
    // fmt Chunk
    u32 fmt_id;
    u32 fmt_chunk_size;
    u16 format_code;
    u16 channels;
    u32 sample_rate;
    u32 byte_rate;
    u16 block_align;
    u16 bits_per_sample;
    
    // data Chunk
    u32 data_id;
    u32 data_chunk_size;
    u16 samples[];
};

typedef struct moe_sound_buffer {
    u32 channels;
    u32 samples_per_second;
    u32 size;
    u32 bytes_per_sample;
    u32 last_cursor;
    u32 running_sample_index;
    u32 samples_to_write;
    u16* samples;
} moe_sound_buffer;

typedef struct moe_sound {
    u16* samples;
    u32 sample_count;
} moe_sound;

//MOE_API moe_sound_info moe_load_wav(moe_arena* arena, moe_string filename);
MOE_API void moe_init_audio();
/*
 * =======================================================
 *							OPENGL
 * =======================================================
 */

MOE_API u32 moe_compile_shader(moe_arena* arena, moe_string fragment_path, moe_string vertex_path);
MOE_API void moe_clear_screen();
MOE_API u32 moe_create_texture_from_image(moe_string path);
MOE_API void moe_texture(moe_renderer* re, u32 texture);
MOE_API void moe_push_vertex(moe_renderer* re, moe_vertex vertex);
MOE_API void moe_render_flush(moe_renderer* re);
MOE_API void moe_create_renderer(i32 cap);
MOE_API void moe_set_matrix_uniform(moe_string name, u32 shader, mat4x4 m);
MOE_API void moe_set_viewport(u32 x, u32 y, u32 width, u32 height);
MOE_API u32 moe_create_font_texture(u8* pixels);

/*
 * =======================================================
 *							api
 * =======================================================
 */

struct moe_context {
    moe_window window;
    moe_input input;
    moe_arena arena;
    moe_sound_buffer sound;
};

/*
 * =======================================================
 *							GLOBALS
 * =======================================================
 */

global moe_context ctx = {0};
global moe_frame draw_frame = {0};
global u32 SHAPE_SHADER = 0;
global u32 IMAGE_SHADER = 0;
global u32 TEXT_SHADER = 0;

/*
 * =======================================================
 *							INTERNAL
 * =======================================================
 */
MOE_LIB void create_z_array(moe_string str, u64 *z);
MOE_LIB u64 align_memory(u64 mem_location);
MOE_LIB LRESULT moe_window_proc(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam);


#endif /* MONARCH_H */
