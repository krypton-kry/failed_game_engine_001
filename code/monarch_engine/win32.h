#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifdef MOE_WIN32_GL_IMPLEMENTATION

void* moe_os_reserve_memory(u64 size)
{
  return VirtualAlloc(0, size, MEM_RESERVE, PAGE_NOACCESS);
}

void moe_os_commit_memory(char* memory, u64 size)
{
  VirtualAlloc(memory, size, MEM_COMMIT, PAGE_READWRITE);
}

void moe_os_free_memory(void* memory)
{
  VirtualFree(memory, 0, MEM_RELEASE);
}

f64 moe_os_time()
{
  LARGE_INTEGER frequency, counter;
  if (!QueryPerformanceFrequency(&frequency) || !QueryPerformanceCounter(&counter)) {
    return -1.0;
  }
  return (double)counter.QuadPart / (double)frequency.QuadPart;
}

//TODO Add more keys
MOE_LIB moe_input_key moe_os_to_key(void* key){
  u64 os_key = (u64) key;
  if( os_key >= 'A' && os_key <= 'z'){
    return (moe_input_key) os_key;
  }
  switch (os_key) {
    case VK_ESCAPE: return KEY_ESCAPE;
    case VK_DOWN: return KEY_DOWN;
    case VK_RIGHT: return KEY_RIGHT;
    case VK_LEFT: return KEY_LEFT;
    case VK_UP: return KEY_UP;
    default: return KEY_NIL;
  }
}

void moe_os_destroy_window(moe_window* window){
  DestroyWindow(window->handle);
}

MOE_LIB LRESULT 
moe_window_proc(HWND handle, UINT msg, WPARAM wparam, LPARAM lparam)
{
  switch(msg)
  {
    case WM_PAINT:
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(handle, &ps);
      FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
      EndPaint(handle, &ps);
    case WM_CHAR:
      if( wparam == VK_ESCAPE )
      {
        //DestroyWindow(handle);
      }
      break;
    case WM_SIZE:
      RECT rect;
      if( GetClientRect(handle, &rect) )
      {
        int width = rect.right - rect.left;
        int height = rect.bottom - rect.top;
        moe_handle_window_resize(width, height);
      }
      break;
    case WM_MOUSEMOVE:	
      // For ortho projection from windows
      f32 y = ctx.window.height - (f32) HIWORD(lparam);
      moe_handle_mouse_motion((f32)LOWORD(lparam), y);
      break;
    case WM_KEYUP:
    case WM_KEYDOWN:
      u8 is_down = ((lparam & (1 << 31)) == 0);
      moe_handle_inputs(moe_os_to_key((void*)wparam), is_down);
      break;
    case WM_CLOSE:
      DestroyWindow(handle);
      break;
    case WM_DESTROY:
      PostQuitMessage(0);
      break;
    default:
      return DefWindowProc(handle, msg, wparam, lparam);
  }
  return 0;
}

moe_window moe_os_create_window(u32 width, u32 height, moe_string title)
{
  HWND dummy_hwnd = CreateWindowExW(
      0, L"STATIC", L"DummyOGLWindow", WS_OVERLAPPED,
      CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
      NULL, NULL, NULL, NULL);

  if( dummy_hwnd == NULL ) ERROR_EXIT("DUMMY HWND Creation Fails");

  HDC dummy_dc = GetDC(dummy_hwnd);
  if( dummy_dc == NULL ) ERROR_EXIT("DC Fails");

  PIXELFORMATDESCRIPTOR dummy_pfd =
  {
    .nSize = sizeof(dummy_pfd),
    .nVersion = 1,
    .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
    .iPixelType = PFD_TYPE_RGBA,
    .cColorBits = 32,// ? 24 & cstencilBits? & cdepthBits? 
  };

  int format = ChoosePixelFormat(dummy_dc, &dummy_pfd);
  if( format == 0 ) ERROR_EXIT("PDF Choose Fails");

  int desc_pfd = DescribePixelFormat(dummy_dc, format, sizeof(dummy_pfd), &dummy_pfd);
  if( desc_pfd == 0 ) ERROR_EXIT("PDF Desc Fails");

  if(!SetPixelFormat(dummy_dc, format, &dummy_pfd)) ERROR_EXIT("PDF Set fails");

  HGLRC dummy_rc = wglCreateContext(dummy_dc);
  if( dummy_rc == NULL ) ERROR_EXIT("Dummy RC Fails");

  desc_pfd = wglMakeCurrent(dummy_dc, dummy_rc);

  // Load glFuncs
  moe_gl_load_functions();

  PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)moe_gl_getproc("wglChoosePixelFormatARB");
  if( wglChoosePixelFormatARB == NULL ) ERROR_EXIT("wglChoosePixelFormatARB not found");
  PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)moe_gl_getproc("wglCreateContextAttribsARB");
  if( wglCreateContextAttribsARB == NULL ) ERROR_EXIT("wglCreateContextAttribsARB not found");

  wglMakeCurrent(NULL, NULL);
  ReleaseDC(dummy_hwnd, dummy_dc);
  wglDeleteContext(dummy_rc);
  DestroyWindow(dummy_hwnd);

  // onto the actual window
  HINSTANCE instance = GetModuleHandle(NULL);;
  if( instance == NULL ) ERROR_EXIT("Instance Fails");
  WNDCLASSEXW window_class =
  {
    .cbSize = sizeof(window_class),
    .lpfnWndProc = moe_window_proc,
    .hInstance = instance,
    .hIcon = LoadIcon(NULL, IDI_APPLICATION),
    .hCursor = LoadCursor(NULL, IDC_ARROW),
    .lpszClassName = (LPCWSTR)TEXT("MOEWindowClass"),
    .style = CS_HREDRAW | CS_VREDRAW,
  };

  ATOM atom = RegisterClassExW(&window_class);
  if( atom == 0 ) ERROR_EXIT("Failed to register window class");

  RECT r = {.right = width, .bottom = height}; 
  AdjustWindowRectEx(&r, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_OVERLAPPEDWINDOW);

  HWND window = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW,
      (LPCWSTR)TEXT("MOEWindowClass"),// needs to be the same in window_class! wasted 30 mins
      (LPCWSTR)title.str,
      WS_OVERLAPPEDWINDOW,
      0, 0,
      r.right - r.left,
      r.bottom - r.top,
      NULL,
      NULL,
      instance,
      NULL);


  if( window == NULL ) ERROR_EXIT("moe_window Creation Failed!");

  HDC dc = GetDC(window);
  if( dc == NULL ) ERROR_EXIT("DC Fails");

  // new pixel format

  int pixel_attributes[] = {
    WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
    WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
    WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
    WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
    WGL_COLOR_BITS_ARB,     24,
    WGL_DEPTH_BITS_ARB,     24,
    WGL_STENCIL_BITS_ARB,   8,
    0
  };

  int pixel_format_arb;
  UINT pixel_formats_found;

  if (!wglChoosePixelFormatARB(dc, pixel_attributes, NULL, 1, &pixel_format_arb, &pixel_formats_found)) ERROR_EXIT("OpenGL does not support required pixel format!");

  PIXELFORMATDESCRIPTOR pfd =
  {
    .nSize = sizeof(pfd),
    .nVersion = 1,
    .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
    .iPixelType = PFD_TYPE_RGBA,
    .cColorBits = 32,// ? 24 & cstencilBits? & cdepthBits? 
  };

  int form = ChoosePixelFormat(dc, &pfd);
  if( form == 0 ) ERROR_EXIT("PDF Choose Fails");

  int d_pfd = DescribePixelFormat(dc, form, sizeof(pfd), &pfd);
  if( d_pfd == 0 ) ERROR_EXIT("PDF Desc Fails");

  if(!SetPixelFormat(dc, form, &pfd)) ERROR_EXIT("PDF Set fails");

  GLint context_attributes[] = {
    WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
    WGL_CONTEXT_MINOR_VERSION_ARB, 0,
    WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
    0
  };

  HGLRC rc = wglCreateContextAttribsARB(dc, NULL, context_attributes);
  if( rc == NULL ) ERROR_EXIT("RC Fails");


  wglMakeCurrent(dc, rc);
  ReleaseDC(window, dc);

  ShowWindow(window, SW_SHOW);

  moe_window win = {0};
  win.handle = (void*) window; 
  win.width = width;
  win.height = height;
  win.title = title;
  ctx.window = win;
  return win;
}

void moe_os_swap_buffers(moe_context* ctx)
{
  HDC dc = GetDC(ctx->window.handle);
  SwapBuffers(dc);
  ReleaseDC(ctx->window.handle, dc);
}

void moe_os_show_window(moe_context* ctx)
{
  ShowWindow(ctx->window.handle, SW_SHOW);
}

void moe_os_handle_messages()
{
  MSG msg;
  while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
    if (msg.message == WM_QUIT) ExitProcess(0);
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}
//TODO: use binary_read
moe_string moe_os_read_file(moe_arena* arena, moe_string filename)
{
  HANDLE file = CreateFileA(filename.str, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (file == INVALID_HANDLE_VALUE) {
    printf("[WARNING] Failed to open file\n");
    return (moe_string){0};
  }

  DWORD fileSize = GetFileSize(file, NULL);
  if (fileSize == INVALID_FILE_SIZE) {
    printf("[WARNING] Failed to get file size\n");
    CloseHandle(file);
    return (moe_string){0};
  }

  u8* buffer = arena_alloc_array(arena, u8, fileSize + 1);
  if (!buffer) {
    printf("[WARNING] Memory allocation failed\n");
    CloseHandle(file);
    return (moe_string){0};
  }

  DWORD bytesRead;
  if (!ReadFile(file, buffer, fileSize, &bytesRead, NULL)) {
    printf("[WARNING] Failed to read file\n");
    arena_dealloc(arena, sizeof(u8) * (fileSize + 1));
    CloseHandle(file);
    return (moe_string){0};
  }

  buffer[bytesRead] = '\0';

  moe_string fileContents = {.str = buffer, .size = bytesRead};

  CloseHandle(file);
  return fileContents;
}

u8* moe_os_read_binary_file(moe_arena* arena, moe_string filename)
{
  HANDLE file = CreateFileA(filename.str, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (file == INVALID_HANDLE_VALUE) {
    printf("[WARNING] Failed to open file\n");
    return NULL;
  }

  DWORD fileSize = GetFileSize(file, NULL);
  if (fileSize == INVALID_FILE_SIZE) {
    printf("[WARNING] Failed to get file size\n");
    CloseHandle(file);
    return NULL;
  }

  u8* buffer = arena_alloc_array(arena, u8, fileSize);
  if (!buffer) {
    printf("[WARNING] Memory allocation failed\n");
    CloseHandle(file);
    return NULL;
  }

  DWORD bytesRead;
  if (!ReadFile(file, buffer, fileSize, &bytesRead, NULL)) {
    printf("[WARNING] Failed to read file\n");
    arena_dealloc(arena, sizeof(u8) * fileSize);
    CloseHandle(file);
    return NULL;
  }

  CloseHandle(file);
  return buffer;
}

void  moe_os_print(char* fmt, ...){
  va_list args;
  va_start(args, fmt);
  // TODO - Change to actual platform code
  vprintf(fmt, args);
  va_end(args);
}

#endif /* MOE_WIN32_GL_IMPLEMENTATION  */
