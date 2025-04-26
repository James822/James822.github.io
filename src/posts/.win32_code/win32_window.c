/*
Windows window and WGL/OpenGL example program.


NOTES:
- ASCII win32 functions are used because they are easier, but you might to consider using the wide-character unicode functions that have a 'W' suffix.

WIN32 RESOURCES:
- memory managment: https://learn.microsoft.com/en-us/windows/win32/memory/memory-management
- windows, messages, and message queues: https://learn.microsoft.com/en-us/windows/win32/winmsg/windowing

RESOURCES FOR OPENGL TUTORIALS:
- https://mariuszbartosik.com/opengl-4-x-initialization-in-windows-without-a-framework/
*/




/* @@ we have to ignore all these errors because the windows.h header itself won't even compile without error with "/Wall" in MSVC, which is quite ironic that microsoft's own headers don't compile without warning in their compiler. */
#pragma warning( push )
#pragma warning( disable : 4668 )
#include <windows.h>
#include <windowsx.h>
#pragma warning( pop )
/* @! */

#include <stdio.h>

#include <gl/gl.h>

/* these two headers need to manually installed, windows only provides "gl/gl.h" as part of the Windows SDK (installed when you install the Build Tools for Visual Studio). You can grab both of these headers from the OpenGL registry: https://registry.khronos.org/OpenGL/index_gl.php */
#include <gl/glext.h>
#include <gl/wglext.h>




LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DummyGL_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
static int Check_Extension_Available(const char* extensions_list, const char* extension);
void* Load_WGL_Proc(const char* proc_name);




int main(void)
{
      HINSTANCE hInstance = GetModuleHandleA(NULL); /* since we aren't using wWinMain() or WinMain(), we need to grab the HINSTANCE with this function. */
      int program_running = 1;
      int fullscreen = 0; /* set to '1' if you want fullscreen, '0' if you don't */



      
      /* @@ user variables */
      const char* window_name = "win32 window";
      int window_width = 960;
      int window_height = 540;
      /* @! */




      /* @@ creating dummy GL window */
      const char* dummygl_window_class_name = "DUMMYGL_WINDOW_CLASS";
      WNDCLASSA dummygl_wnd_class;
      dummygl_wnd_class.style = CS_OWNDC;
      dummygl_wnd_class.lpfnWndProc = DummyGL_WindowProc;
      dummygl_wnd_class.cbClsExtra = 0;
      dummygl_wnd_class.cbWndExtra = 0;
      dummygl_wnd_class.hInstance = hInstance;
      dummygl_wnd_class.hIcon = NULL;
      dummygl_wnd_class.hCursor = NULL;
      dummygl_wnd_class.hbrBackground = 0;
      dummygl_wnd_class.lpszMenuName = NULL;
      dummygl_wnd_class.lpszClassName = dummygl_window_class_name;
      if(RegisterClassA(&dummygl_wnd_class) == 0) {
	    DWORD win32_error_val = GetLastError();
	    printf("ERROR: RegisterClassA() failed to register window class: %s - win32 error code: %ld\n", dummygl_window_class_name, win32_error_val);
      }

      HWND dummygl_window_handle = CreateWindowA
	    (dummygl_window_class_name,
	     "dummygl_window",
	     WS_DISABLED,
	     CW_USEDEFAULT,
	     CW_USEDEFAULT,
	     CW_USEDEFAULT,
	     CW_USEDEFAULT,
	     NULL,
	     NULL,
	     hInstance,
	     NULL);
      if(dummygl_window_handle == NULL) {
	    DWORD win32_error_val = GetLastError();
	    printf("ERROR: CreateWindowA() failed to create dummygl window - win32 error code: %ld\n", win32_error_val);
	    return 1;
      }

      HDC dummygl_DC = GetDC(dummygl_window_handle);
      if(dummygl_DC == NULL) {
	    printf("ERROR: GetDC() failed to get DC for dummygl window\n");
	    return 1;
      }
      /* @! */



      
      /* @@ creating dummy GL context */

      /* it doesn't matter what the pixel format descriptor members are, just as long as this gets us a context, so we just pick members that would give us a high chance of getting a context on every possible system */
      PIXELFORMATDESCRIPTOR dummygl_pfd;
      dummygl_pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
      dummygl_pfd.nVersion = 1;
      dummygl_pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
      dummygl_pfd.iPixelType = PFD_TYPE_RGBA;
      dummygl_pfd.cColorBits = 32;
      dummygl_pfd.cRedBits = 0;
      dummygl_pfd.cRedShift = 0;
      dummygl_pfd.cGreenBits = 0;
      dummygl_pfd.cGreenShift = 0;
      dummygl_pfd.cBlueBits = 0;
      dummygl_pfd.cBlueShift = 0;
      dummygl_pfd.cAlphaBits = 0;
      dummygl_pfd.cAlphaShift = 0;
      dummygl_pfd.cAccumBits = 0;
      dummygl_pfd.cAccumRedBits = 0;
      dummygl_pfd.cAccumGreenBits = 0;
      dummygl_pfd.cAccumBlueBits = 0;
      dummygl_pfd.cAccumAlphaBits = 0;
      dummygl_pfd.cDepthBits = 24;
      dummygl_pfd.cStencilBits = 8;
      dummygl_pfd.cAuxBuffers = 0;
      dummygl_pfd.iLayerType = PFD_MAIN_PLANE;
      dummygl_pfd.bReserved = 0;
      dummygl_pfd.dwLayerMask = 0;
      dummygl_pfd.dwVisibleMask = 0;
      dummygl_pfd.dwDamageMask = 0;

      int dummygl_pixelformat_index = ChoosePixelFormat(dummygl_DC, &dummygl_pfd);
      if(dummygl_pixelformat_index == 0) {
	    DWORD win32_error_val = GetLastError();
	    printf("ERROR: ChoosePixelFormat() failed to get a pixel format that matched - win32 error code: %ld\n", win32_error_val);
	    return 1;
      }

      if(SetPixelFormat(dummygl_DC, dummygl_pixelformat_index, &dummygl_pfd) != TRUE) {
	    DWORD win32_error_val = GetLastError();
	    printf("ERROR: SetPixelFormat() failed to set the pixel format - win32 error code: %ld\n", win32_error_val);
	    return 1;
      }

      HGLRC dummygl_context = wglCreateContext(dummygl_DC);
      if(dummygl_context == NULL) {
	    DWORD win32_error_val = GetLastError();
	    printf("ERROR: wglCreateContext() failed to create a dummy GL context - win32 error code: %ld\n", win32_error_val);
	    return 1;
      }
      if(wglMakeCurrent(dummygl_DC, dummygl_context) != TRUE) {
	    DWORD win32_error_val = GetLastError();
	    printf("ERROR: wglMakeCurrent() failed to make context current - win32 error code: %ld\n", win32_error_val);
	    return 1;
      }
      /* @! */



      
      /* @@ loading extensions to create our real Gl context. Now that we have a dummy context, we can load the necessary extension procedures in order to create the real context. */

      /* the procedure wglGetExtensionsStringARB() is used to query extensions, but since it is itself part of an extension (WGL_ARB_extensions_string), we can't use the procedure to query itself before we even know if it exists. So, we just have to try to load it. */
      PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB = NULL;      
      wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)
	    Load_WGL_Proc("wglGetExtensionsStringARB");
      if(wglGetExtensionsStringARB == NULL) {
	    printf("ERROR: failed to load wglGetExtensionsStringARB()\n");
	    return 1;
      }
      
      
      const char * extensions_string = wglGetExtensionsStringARB(dummygl_DC);
      if(extensions_string == NULL) {
	    DWORD win32_error_val = GetLastError();
	    printf("ERROR: wglGetExtensionsStringARB() failed to get the extensions string - win32 (or or WGL_ARB_extensions_string extension) error code: %ld\n", win32_error_val);
	    return 1;
      }	  

      if(Check_Extension_Available(extensions_string, "WGL_ARB_pixel_format") != 1) {
	    printf("ERROR: WGL_ARB_pixel_format extension not found\n");
	    return 1;
      }
      if(Check_Extension_Available(extensions_string, "WGL_ARB_create_context_profile") != 1) {
	    printf("ERROR: WGL_ARB_create_context_profile extension not found\n");
	    return 1;
      }

      
      /* These procedure pointers are neccessary to aquire/load in the dummy context, but the remainder of the WGL (and GL) extension procedures should be loaded with the actual context. wglGetExtensionsStringARB() should be loaded again with the new context, but these ones are only neccessary to for context creation, so we don't need them again. */
      PFNWGLGETPIXELFORMATATTRIBIVARBPROC wglGetPixelFormatAttribivARB = NULL;
      PFNWGLGETPIXELFORMATATTRIBFVARBPROC wglGetPixelFormatAttribfvARB = NULL;
      PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = NULL;
      PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
      
      wglGetPixelFormatAttribivARB = (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)
	    Load_WGL_Proc("wglGetPixelFormatAttribivARB");
      wglGetPixelFormatAttribfvARB = (PFNWGLGETPIXELFORMATATTRIBFVARBPROC)
	    Load_WGL_Proc("wglGetPixelFormatAttribfvARB");
      wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)
	    Load_WGL_Proc("wglChoosePixelFormatARB");
      wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)
	    Load_WGL_Proc("wglCreateContextAttribsARB");

      if(wglGetPixelFormatAttribivARB == NULL) {
	    printf("ERROR: failed to load proc: \"wglGetPixelFormatAttribivARB\"\n");
	    return 1;
      }
      if(wglGetPixelFormatAttribfvARB == NULL) {
	    printf("ERROR: failed to load proc: \"wglGetPixelFormatAttribfvARB\"\n");
	    return 1;
      }
      if(wglChoosePixelFormatARB == NULL) {
	    printf("ERROR: failed to load proc: \"wglChoosePixelFormatARB\"\n");
	    return 1;
      }
      if(wglCreateContextAttribsARB == NULL) {
	    printf("ERROR: failed to load proc: \"wglCreateContextAttribsARB\"\n");
	    return 1;
      }
      /* @! */



      
      /* @@ Creating the real window */
      WNDCLASSEXA wnd_class;
      const char* window_class_name = "WINDOW_CLASS";
      wnd_class.cbSize = sizeof(WNDCLASSEXA);
      wnd_class.style = CS_OWNDC; /* it's probably neccessary to set CS_OWNDC flag for OpenGL context creation */
      wnd_class.lpfnWndProc = WindowProc;
      wnd_class.cbClsExtra = 0;
      wnd_class.cbWndExtra = 0;
      wnd_class.hInstance = hInstance;
      wnd_class.hIcon = NULL;
      wnd_class.hCursor = NULL;
      wnd_class.hbrBackground = 0;
      wnd_class.lpszMenuName = NULL;
      wnd_class.lpszClassName = window_class_name;
      wnd_class.hIconSm = NULL;

      if(RegisterClassExA(&wnd_class) == 0) {
	    DWORD win32_error_val = GetLastError();
	    printf("ERROR: RegisterClassA() failed to register window class - win32 error code: %ld\n", win32_error_val);
	    return 1;
      }
      
      HWND window_handle = CreateWindowExA
	    (0,
	     window_class_name,
	     window_name,
	     WS_OVERLAPPEDWINDOW,
	     CW_USEDEFAULT,
	     CW_USEDEFAULT,
	     window_width,
	     window_height,
	     NULL,
	     NULL,
	     hInstance,
	     NULL);
      if(window_handle == NULL) {
	    DWORD win32_error_val = GetLastError();
	    printf("ERROR: CreateWindowExA() failed to create window - win32 error code: %ld\n", win32_error_val);
	    return 1;
      }

      HDC window_DC = GetDC(window_handle);
      if(window_DC == NULL) {
	    printf("ERROR: GetDC() failed to get DC for window\n");
	    return 1;
      }
      /* @! */



      
      /* @@ creating the real Gl context */
      #define PI_ATTRIB_LIST_LENGTH 19
      int pi_attrib_list[PI_ATTRIB_LIST_LENGTH];
      pi_attrib_list[0]  = WGL_DRAW_TO_WINDOW_ARB; pi_attrib_list[1]  = TRUE;
      pi_attrib_list[2]  = WGL_SUPPORT_OPENGL_ARB; pi_attrib_list[3]  = TRUE;
      pi_attrib_list[4]  = WGL_DOUBLE_BUFFER_ARB;  pi_attrib_list[5]  = TRUE;
      pi_attrib_list[6]  = WGL_PIXEL_TYPE_ARB;     pi_attrib_list[7]  = WGL_TYPE_RGBA_ARB;
      pi_attrib_list[8]  = WGL_ACCELERATION_ARB;   pi_attrib_list[9]  = WGL_FULL_ACCELERATION_ARB;
      pi_attrib_list[10] = WGL_COLOR_BITS_ARB;     pi_attrib_list[11] = 32;
      pi_attrib_list[12] = WGL_ALPHA_BITS_ARB;     pi_attrib_list[13] = 8;
      pi_attrib_list[14] = WGL_DEPTH_BITS_ARB;     pi_attrib_list[15] = 24;
      pi_attrib_list[16] = WGL_STENCIL_BITS_ARB;   pi_attrib_list[17] = 8;
      pi_attrib_list[18] = 0;
      /* look into WGL_SAMPLE_BUFFERS_ARB for MSAA */

      int pixel_format_id;
      UINT num_formats;
      if(wglChoosePixelFormatARB(window_DC, pi_attrib_list, NULL, 1, &pixel_format_id, &num_formats) != TRUE) {
	    DWORD win32_error_val = GetLastError();
	    printf("ERROR: failed to get pixel format with wglChoosePixelFormatARB() - win32 (or WGL_ARB_pixel_format extension) error code: %ld\n", win32_error_val);
	    return 1;
      }
      if(num_formats == 0) {
	    printf("ERROR: no pixel formats found as queried with wglChoosePixelFormatARB()\n");
	    return 1;
      }

      PIXELFORMATDESCRIPTOR pixel_fd;
      if(DescribePixelFormat(window_DC, pixel_format_id, sizeof(PIXELFORMATDESCRIPTOR), &pixel_fd) == 0) {
	    DWORD win32_error_val = GetLastError();
	    printf("ERROR: DescribePixelFormat() failed - win32 error code: %ld\n", win32_error_val);
	    return 1;
      }
      if(SetPixelFormat(window_DC, pixel_format_id, &pixel_fd) != TRUE) {
	    DWORD win32_error_val = GetLastError();
	    printf("ERROR: failed to set pixel format with SetPixelFormat() - win32 error code: %ld\n", win32_error_val);
	    return 1;
      }


      #define ATTRIB_LIST_LENGTH 7
      int attrib_list[ATTRIB_LIST_LENGTH];
      attrib_list[0] = WGL_CONTEXT_MAJOR_VERSION_ARB; attrib_list[1] = 4;
      attrib_list[2] = WGL_CONTEXT_MINOR_VERSION_ARB; attrib_list[3] = 6;
      attrib_list[4] = WGL_CONTEXT_PROFILE_MASK_ARB; attrib_list[5] = WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
      attrib_list[6] = 0;

      HGLRC wgl_context = wglCreateContextAttribsARB(window_DC, 0, attrib_list);
      if(wgl_context == NULL) {
	    DWORD win32_error_val = GetLastError();
	    printf("ERROR: failed to create WGL (GL) context - win32 (or WGL_ARB_create_context/WGL_ARB_create_context_profile extension) error code: %ld\n", win32_error_val);
	    return 1;
      }
      /* @! */



      
      /* @@ cleanup of dummygl stuff */
      if(wglMakeCurrent(dummygl_DC, NULL) != TRUE) { /* making dummy GL context not current */
	    printf("ERROR: wglMakeCurrent() failed to make context NOT current\n");
	    return 1;
      }
      if(wglDeleteContext(dummygl_context) != TRUE) { /* deleting dummy GL context */
	    printf("ERROR: wglDeleteContext() failed to delete the dummy GL context\n");
	    return 1;
      }
      if(DeleteDC(dummygl_DC) == 0) { /* deleting dummy GL device context */
	    printf("ERROR: DeleteDC() failed to delete the dummy GL Device Context\n");
	    return 1;
      }
      if(DestroyWindow(dummygl_window_handle) == 0) { /* destroying dummy GL window */
	    printf("ERROR: DestroyWindow() failed to destroy the dummy GL window\n");
	    return 1;
      }
      if(UnregisterClassA(dummygl_window_class_name, hInstance) == 0) { /* unregistering dummy GL window class */
	    printf("ERROR: UnregisterClassA() failed to unregister the dummy GL window class: %s\n", dummygl_window_class_name);
	    return 1;
      }
      /* @! */



      
      /* @@ now we make the real context current, the WGL context */
      if(wglMakeCurrent(window_DC, wgl_context) != TRUE) {
	    printf("ERROR: wglMakeCurrent() failed to make context current\n");
      }
      /* @! */



      
      /* @@ loading WGL extension procedures (again) for the real Gl context. */
      
      /* We reload the string extension procedure (as it's the only one we will reuse), and then load the rest of the WGL and GL procedures now that we have the real context. According to the MSDN docs for wglGetProcAddress(): "Extension functions supported in one rendering context are not necessarily available in a separate rendering context. Thus, for a given rendering context in an application, use the function addresses returned by the wglGetProcAddress function only.". In practice, we may not need to do this, but as a matter of *good* practice we should. */
      wglGetExtensionsStringARB = NULL;
      wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)
	    Load_WGL_Proc("wglGetExtensionsStringARB");
      if(wglGetExtensionsStringARB == NULL) {
	    printf("ERROR: failed to load wglGetExtensionsStringARB()\n");
	    return 1;
      }
      extensions_string = wglGetExtensionsStringARB(window_DC); /* grabbing the extensions again */

      
      PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = NULL;
      PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT = NULL;

      /* it's not necessary to require that these extensions be present, we could leave it as an optional feature, but for our program, we will require them. */
      if(Check_Extension_Available(extensions_string, "WGL_EXT_swap_control") != 1) {
	    printf("ERROR: WGL_EXT_swap_control extension not found\n");
	    return 1;
      }
      if(Check_Extension_Available(extensions_string, "WGL_EXT_swap_control_tear") != 1) {
	    printf("ERROR: WGL_EXT_swap_control_tear extension not found\n");
	    return 1;
      }

      wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)
	    Load_WGL_Proc("wglSwapIntervalEXT");
      wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)
	    Load_WGL_Proc("wglGetSwapIntervalEXT");

      if(wglSwapIntervalEXT == NULL) {
	    printf("ERROR: failed to load proc: \"wglSwapIntervalEXT\"\n");
	    return 1;
      }
      if(wglGetSwapIntervalEXT == NULL) {
	    printf("ERROR: failed to load proc: \"wglGetSwapIntervalEXT\"\n");
	    return 1;
      }     
      /* @! */


      /* past this point we have everything we need, a window and OpenGL context. Now we just need to load OpenGL function pointers (which we do manually), and then we can start doing some rendering! */

      
      /* @@ Loading OpenGL procedures (we need the "glext.h" header for the function typedefs and declarations). */
      PFNGLGETSTRINGIPROC glGetStringi = NULL;
      PFNGLGENBUFFERSPROC glGenBuffers = NULL;
      PFNGLBINDBUFFERPROC glBindBuffer = NULL;
      PFNGLBUFFERDATAPROC glBufferData = NULL;
      PFNGLCREATESHADERPROC glCreateShader = NULL;
      PFNGLSHADERSOURCEPROC glShaderSource = NULL;
      PFNGLCOMPILESHADERPROC glCompileShader = NULL;
      PFNGLCREATEPROGRAMPROC glCreateProgram = NULL;
      PFNGLATTACHSHADERPROC glAttachShader = NULL;
      PFNGLLINKPROGRAMPROC glLinkProgram = NULL;
      PFNGLDELETESHADERPROC glDeleteShader = NULL;
      PFNGLUSEPROGRAMPROC glUseProgram = NULL;
      PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = NULL;
      PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = NULL;
      PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = NULL;
      PFNGLBINDVERTEXARRAYPROC glBindVertexArray = NULL;

      
      glGetStringi = (PFNGLGETSTRINGIPROC)Load_WGL_Proc((const char *)"glGetStringi");
      glGenBuffers = (PFNGLGENBUFFERSPROC)Load_WGL_Proc((const char *)"glGenBuffers");
      glBindBuffer = (PFNGLBINDBUFFERPROC)Load_WGL_Proc((const char *)"glBindBuffer");
      glBufferData = (PFNGLBUFFERDATAPROC)Load_WGL_Proc((const char *)"glBufferData");
      glCreateShader = (PFNGLCREATESHADERPROC)Load_WGL_Proc((const char *)"glCreateShader");
      glShaderSource = (PFNGLSHADERSOURCEPROC)Load_WGL_Proc((const char *)"glShaderSource");
      glCompileShader = (PFNGLCOMPILESHADERPROC)Load_WGL_Proc((const char *)"glCompileShader");
      glCreateProgram = (PFNGLCREATEPROGRAMPROC)Load_WGL_Proc((const char *)"glCreateProgram");
      glAttachShader = (PFNGLATTACHSHADERPROC)Load_WGL_Proc((const char *)"glAttachShader");
      glLinkProgram = (PFNGLLINKPROGRAMPROC)Load_WGL_Proc((const char *)"glLinkProgram");
      glDeleteShader = (PFNGLDELETESHADERPROC)Load_WGL_Proc((const char *)"glDeleteShader");
      glUseProgram = (PFNGLUSEPROGRAMPROC)Load_WGL_Proc((const char *)"glUseProgram");
      glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)Load_WGL_Proc((const char *)"glVertexAttribPointer");
      glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)Load_WGL_Proc((const char *)"glEnableVertexAttribArray");
      glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)Load_WGL_Proc((const char *)"glGenVertexArrays");
      glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)Load_WGL_Proc((const char *)"glBindVertexArray");

      
      if(glGetStringi == NULL) {
	    printf("ERROR: \"glGetStringi\" function pointer NULL\n");
	    return 1;
      }
      if(glGenBuffers == NULL) {
	    printf("ERROR: \"glGenBuffers\" function pointer NULL\n");
	    return 1;
      }
      if(glBindBuffer == NULL) {
	    printf("ERROR: \"glBindBuffer\" function pointer NULL\n");
	    return 1;
      }
      if(glBufferData == NULL) {
	    printf("ERROR: \"glBufferData\" function pointer NULL\n");
	    return 1;
      }
      if(glCreateShader == NULL) {
	    printf("ERROR: \"glCreateShader\" function pointer NULL\n");
	    return 1;
      }
      if(glShaderSource == NULL) {
	    printf("ERROR: \"glShaderSource\" function pointer NULL\n");
	    return 1;
      }
      if(glCompileShader == NULL) {
	    printf("ERROR: \"glCompileShader\" function pointer NULL\n");
	    return 1;
      }
      if(glCreateProgram == NULL) {
	    printf("ERROR: \"glCreateProgram\" function pointer NULL\n");
	    return 1;
      }
      if(glAttachShader == NULL) {
	    printf("ERROR: \"glAttachShader\" function pointer NULL\n");
	    return 1;
      }
      if(glLinkProgram == NULL) {
	    printf("ERROR: \"glLinkProgram\" function pointer NULL\n");
	    return 1;
      }
      if(glDeleteShader == NULL) {
	    printf("ERROR: \"glDeleteShader\" function pointer NULL\n");
	    return 1;
      }
      if(glUseProgram == NULL) {
	    printf("ERROR: \"glUseProgram\" function pointer NULL\n");
	    return 1;
      }
      if(glVertexAttribPointer == NULL) {
	    printf("ERROR: \"glVertexAttribPointer\" function pointer NULL\n");
	    return 1;
      }
      if(glEnableVertexAttribArray == NULL) {
	    printf("ERROR: \"glEnableVertexAttribArray\" function pointer NULL\n");
	    return 1;
      }
      if(glGenVertexArrays == NULL) {
	    printf("ERROR: \"glGenVertexArrays\" function pointer NULL\n");
	    return 1;
      }
      if(glBindVertexArray == NULL) {
	    printf("ERROR: \"glBindVertexArray\" function pointer NULL\n");
	    return 1;
      }
      /* @! */



      
      /* @@ user settings */
      if(SetWindowTextA(window_handle, window_name) == 0) {
	    printf("ERROR: SetWindowTextA() failed!\n");
      }

      wglSwapIntervalEXT(-1); /* setting vsync to adaptive vsync (-1), we could also set it to 1 for normal vsync */
      /* @! */



      
      /* @@ setting up rendering of hello triangle */
#define VERT_SIZE 9
      GLfloat vertices[VERT_SIZE];
      vertices[0] = -0.5f;
      vertices[1] = -0.5f;
      vertices[2] = 0.0f;
      vertices[3] = 0.5f;
      vertices[4] = -0.5f;
      vertices[5] = 0.0f;
      vertices[6] = 0.0f;
      vertices[7] = 0.5f;
      vertices[8] = 0.0f;

      /* @TODO: use the new glCreateBuffers() and glNamedBufferStorage() and see if that works! */

      const char * vert_shader_source = "#version 460 core\n"
	    "layout (location = 0) in vec3 vpos;\n"
	    "void main()\n"
	    "{\n"
	    "gl_Position = vec4(vpos.x, vpos.y, vpos.z, 1.0);\n"
	    "}\n\0";
      const char * frag_shader_source = "#version 460 core\n"
	    "out vec4 frag_color;\n"
	    "void main()\n"
	    "{\n"
	    "frag_color = vec4(0.1f, 0.7f, 0.5f, 1.0f);\n"
	    "}\n\0";

      GLuint vert_shader;
      GLuint frag_shader;
      GLuint shader_program;
      
      vert_shader = glCreateShader(GL_VERTEX_SHADER);
      frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
      shader_program = glCreateProgram();
      
      glShaderSource(vert_shader, 1, &vert_shader_source, NULL);
      glShaderSource(frag_shader, 1, &frag_shader_source, NULL);

      glCompileShader(vert_shader);
      glCompileShader(frag_shader);

      glAttachShader(shader_program, vert_shader);
      glAttachShader(shader_program, frag_shader);

      glLinkProgram(shader_program);

      glDeleteShader(vert_shader);
      glDeleteShader(frag_shader);


      GLuint vao;
      GLuint vbo;
      glGenVertexArrays(1, &vao);      
      glGenBuffers(1, &vbo);

      glBindVertexArray(vao);
      
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBufferData(GL_ARRAY_BUFFER, VERT_SIZE * sizeof(GLfloat), (const void *)vertices, GL_STATIC_DRAW);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void *)0);
      glEnableVertexAttribArray(0);
      /* @! */



      
      /* @@ setting fullscreen */
      if(fullscreen) {
	    HMONITOR monitor_handle = MonitorFromWindow(window_handle, MONITOR_DEFAULTTONULL);
	    if(monitor_handle == NULL) {
		  printf("ERROR: MonitorFromWindow() failed to get a monitor that the window is on. Probably the window doesn't intersect any monitors\n");
		  return 1;
	    }
      
	    MONITORINFO mi;
	    mi.cbSize = sizeof(MONITORINFO);
	    if(GetMonitorInfoA(monitor_handle, &mi) == 0) {
		  printf("ERROR: GetMonitorInfoA() failed\n");
		  return 1;
	    }

	    LONG_PTR window_style = GetWindowLongPtrA(window_handle, GWL_STYLE);

	    SetLastError(0); /* need to do for SetWindowLongPtrA() */
	    if(SetWindowLongPtrA(window_handle, GWL_STYLE, window_style & ~WS_OVERLAPPEDWINDOW) == 0) {
		  /* the return value of SetWindowLongPtrA() could be 0 because the previous window style is 0, but it also returns 0 because of error. This horrible API design means that if 0 is returned, it could either be because of an error, or the function succeeded and returned the window style value for example (which was 0). To solve this, we first check to see if SetWindowLongPtrA() is 0 (after calling SetLastError(0)), and then if GetLastError() is non-zero, then we know there is an error.*/
		  DWORD win32_error_val = GetLastError();
		  if(win32_error_val != 0) {
			printf("ERROR: SetWindowLongPtrA() failed to set the new window style - win32 error code: %ld\n", win32_error_val);
			return 1;
		  }
	    }
	    if( SetWindowPos(window_handle,
			     HWND_TOP,
			     mi.rcMonitor.left,
			     mi.rcMonitor.top,
			     mi.rcMonitor.right - mi.rcMonitor.left,
			     mi.rcMonitor.bottom - mi.rcMonitor.top,
			     SWP_NOOWNERZORDER | SWP_FRAMECHANGED) == 0 ) {
		  DWORD win32_error_val = GetLastError();
		  printf("ERROR: SetWindowPos() failed to change to fullscreen mode - win32 error code: %ld\n", win32_error_val);
		  return 1;
	    }
      }
      /* @! */



      
      /* @@ setting glViewport() */
      RECT window_size;
      GetClientRect(window_handle, &window_size);

      glViewport(0, 0, window_size.right - window_size.left, window_size.bottom - window_size.top);
      /* @! */



      
      /* @@ Finally at the end, we show the window, similar to XMapRaised() or XMapWindow() for X11 */
      ShowWindow(window_handle, SW_SHOWNORMAL);
      /* @! */



      
      /* @@ main loop */
      MSG msg;
      while(program_running) {	    
	    /* @@ flush/process/get messages */
	    while(PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE) != 0) {
		  if(LOWORD(msg.message) == WM_QUIT) {
			program_running = 0;
			break;
		  }

		  TranslateMessage(&msg);		  
		  DispatchMessage(&msg);
	    }
	    /* @! */

	    
	    /* @@ rendering */
	    glClearColor(0.1f, 0.15f, 0.19f, 1.0f);
	    glClear(GL_COLOR_BUFFER_BIT);
	    glUseProgram(shader_program);
	    glBindVertexArray(vao);
	    glDrawArrays(GL_TRIANGLES, 0, 3);
	    /* @! */
	    

	    /* @@ swapping and synching */
	    wglSwapLayerBuffers(window_DC, WGL_SWAP_MAIN_PLANE);
	    glFinish(); /* blocks until all previous GL commands finish, including the buffer swap. */
	    /* @! */
      }
      /* @! */



      
      /* @@ Cleanup and Exit */
      if(wglMakeCurrent(window_DC, NULL) != TRUE) { /* making WGL context not current */
	    printf("ERROR: wglMakeCurrent() failed to make context NOT current\n");
      }
      if(wglDeleteContext(wgl_context) != TRUE) { /* deleting WGL context */
	    printf("ERROR: wglDeleteContext() failed to delete WGL context\n");
      }
      if(DeleteDC(window_DC) == 0) { /* deleting window device context */
	    printf("ERROR: DeleteDC() failed to delete window Device Context\n");
      }
      if(DestroyWindow(window_handle) == 0) {
	    DWORD win32_error_val = GetLastError();
	    printf("ERROR: failed to destroy window with DestroyWindow() - win32 error code: %ld\n", win32_error_val);
      }
      if(UnregisterClassA(window_class_name, hInstance) == 0) { /* unregistering window class */
	    printf("ERROR: UnregisterClassA() failed to unregister window class: %s\n", window_class_name);
      }

      
      return 0;
      /* @! */    
}




/* main window procedure for our win32 window */
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
      /* printf("MSG CODE: %u\n", uMsg); */
      LRESULT result = 0;

      
      switch (uMsg) {
	    /* @@ these are messages that we want to process ourselves */
      case WM_PAINT: {
	    printf("WM_PAINT\n");
	    ValidateRect(hwnd, NULL); /* validates the entire client region of the window so that the OS doesn't keep spamming WM_PAINT messages and thus stalling our application message loop. */
      } break;
      case WM_CLOSE: {
	    printf("WM_CLOSE\n");
	    PostQuitMessage(0);
      } break;
	    /* @! */

	    
	    /* @@ mouse input */
      case WM_LBUTTONDOWN: {
	    printf("WM_LBUTTONDOWN\n");
      } break;
	    
      case WM_LBUTTONUP: {
	    printf("WM_LBUTTONUP\n");
      } break;
	    
      case WM_MBUTTONDOWN: {
	    printf("WM_MBUTTONDOWN\n");
      } break;
	    
      case WM_MBUTTONUP: {
	    printf("WM_MBUTTONUP\n");
      } break;

      case WM_RBUTTONDOWN: {
	    printf("WM_RBUTTONDOWN\n");
      } break;
	    
      case WM_RBUTTONUP: {
	    printf("WM_RBUTTONUP\n");
      } break;

      case WM_XBUTTONDOWN: {
	    printf("WM_XBUTTONDOWN: ");
	    if(GET_XBUTTON_WPARAM(wParam) == XBUTTON1) {
		  printf("XBUTTON1");
	    } else if(GET_XBUTTON_WPARAM(wParam) == XBUTTON2) {
		  printf("XBUTTON2");
	    }
	    printf("\n");
      } break;
	    
      case WM_XBUTTONUP: {
	    printf("WM_XBUTTONUP: ");
	    if(GET_XBUTTON_WPARAM(wParam) == XBUTTON1) {
		  printf("XBUTTON1");
	    } else if(GET_XBUTTON_WPARAM(wParam) == XBUTTON2) {
		  printf("XBUTTON2");
	    }
	    printf("\n");
      } break;

      case WM_MOUSEMOVE: {
	    printf("WM_MOUSEMOVE\n");
	    int x_pos = GET_X_LPARAM(lParam);
	    int y_pos = GET_Y_LPARAM(lParam);
      } break;

      case WM_MOUSEWHEEL: {
	    printf("WM_MOUSEWHEEL\n");
      }
	    /* @! */

	    
	    /* @@ keyboard Input */
      case WM_SYSKEYDOWN: {
	    printf("WM_SYSKEYDOWN\n");
      } break;
	    
      case WM_SYSKEYUP: {
	    printf("WM_SYSKEYUP\n");
      } break;
	    
      case WM_KEYDOWN: {
	    printf("WM_KEYDOWN\n");
	    if(wParam == VK_ESCAPE) {
		  /* quit if user presses the ESC key */
		  PostQuitMessage(0);
	    }
      } break;
	    
      case WM_KEYUP: {
	    printf("WM_KEYUP\n");
      } break;

      case WM_CHAR: {
	    printf("WM_CHAR\n");
      } break;
	    
      case WM_SYSCHAR: {
	    printf("WM_SYSCHAR\n");
      } break;
	    /* @!*/

	    
      default: {
	    result = DefWindowProc(hwnd, uMsg, wParam, lParam);
	    /* result = 0; */
      } break;

	    
      }
      
      
      return result;
}



/* Returns 1 if extension is available, otherwise 0 if it is not present.

This function expects "extensions_lists" to be a space seperated list of GL (or platform, e.g. WGL/GLX) extensions, and it searches extensions_lists to find if the extension "extensions" is within it.
 */
static int Check_Extension_Available(const char* extensions_list, const char* extension)
{
      int i = 0;
      char c = extensions_list[i];
      while(c != '\0') {
	    if(c == extension[0]) {
		  if(i > 0) { /* i > 0 (i != 0) means we need to ensure the previous char is a space */
			if(extensions_list[i - 1] != ' ') {
			      break;
			}
		  }

		  /* now we check to see if the rest of the string after matches */
		  int equal = 1;
		  int j = 0;
		  while(extension[j] != '\0') {
			if(extensions_list[i + j] == '\0') {
			      equal = 0;
			      break;
			}
			if(extensions_list[i + j] != extension[j]) {
			      equal = 0;
			      break;
			}
			j += 1;
		  }
		  
		  if(equal == 1 && (extensions_list[i + j] == ' ' || extensions_list[i + j] == '\0') ) {
			return 1;
		  }
	    }

	    i += 1;
	    c = extensions_list[i];
      }

      
      return 0;
}




/* Dummy Window Procedure to use for our dummy GL window. It doesn't matter what this is as long as it just exists (and handles default behaviour so that window creation works), so we just use the default window procedure for every message. Again, it doesn't matter because this just needs to exist so that we can create a dummy window, nothing further.*/
LRESULT CALLBACK DummyGL_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
      return DefWindowProc(hwnd, uMsg, wParam, lParam);
}




/* Loads the specified GL/WGL function with the name "proc_name" using wglGetProcAddress(). Returns NULL on failure, otherwise non-NULL for success.
*/
void* Load_WGL_Proc(const char* proc_name)
{
      void* proc = NULL;
      proc = (void *)wglGetProcAddress(proc_name);
      if(proc == 0 || proc == (void *)1 || proc == (void *)2 || proc == (void *)3 || proc == (void *)-1) {
	    return NULL;
      }
      return proc;
}
