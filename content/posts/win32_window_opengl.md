+++
date = '2025-05-02'
draft = false
title = 'Creating a win32 Window and OpenGL Context on Windows From Scratch'
+++

In a [previous post](/posts/x11_window_opengl/) I made a tutorial showing how to get a window open on linux/X11 with an OpenGL context. I'd like to make the same tutorial but for windows now, so here it is.

This tutorial follows the same format as the last one, so I won't really go into too much detail on things that I've already covered. I'm moreso only going to outline the windows/win32 aspects here, but don't worry, the full source code is available [here](https://github.com/James822/win32-opengl-window) in my github repository.

Just like the last tutorial, we'll be using C to do everything, but you can also follow along in C++ with only a few modifications.


## Win32
Unlike linux, everything we'll need for this tutorial is under the win32 API. The win32 API is the programming interface to the windows operating system. It gives us the ability to perform file I/O, allocate memory, and namely: create windows.

As opposed to linux (or rather linux distributions), the windowing system is not seperate from the kernel or OS interface, but is provided as an integral part of win32. We'll shortly see the functions we can use to create and manage windows.

If you had already read my previous tutorial for making a window on linux with GLX and X11, then win32 essentially combines all these things together into one interface. The equivalent to GLX on windows is called "WGL", or the "Windows Extension to OpenGL". There is no equivalent to a windowing system like X11 or wayland, as the windowing system is an integral part of win32.


## Documentation
The primary source of documentation is MSDN (Microsoft Developer Network). Technically, MSDN was removed and integrated into a new space called "Microsoft Learn", but everyone still refers to this portion of Microsoft Learn as what it used to be, which is MSDN.

Unfortunately, Microsft Learn/MSDN is very disorganized. I would provide you a link, but that would be more unhelpful than just providing a few select links on specific topics.

Along with MSDN, you should also have the [OpenGL registry](https://registry.khronos.org/OpenGL/index_gl.php) on hand.


## Compiling/Building on Windows with MSVC
MSVC or the "Microsoft Visual C++" compiler is the main C/C++ compiler for windows which we will be using. Even though it only has "C++" in the title, it also supports C compilation. When it comes to microsoft, almost nothing is named what it should be, and everything seems to be almost deliberately confusing. Bear with me, as it only gets worse the further into it you go. Many of the MSDN docs will refer to "C++" tutorials or references, but these also apply to plain C as well.

MSVC is the compiler used in Visual Studio, and typically, people invoke it when they use the Visual Studio IDE. If you want to use VS that's up to you, but I'll demonstrate how to use MSVC from the command line directly which I find be easier to use, and it allows you to use whatever IDE you like. For this section, the primary reference will be this [MSDN documentation](https://learn.microsoft.com/en-us/cpp/build/building-on-the-command-line?view=msvc-170).

The first thing you need to do is grab the "Build Tools for Visual Studio" which is referenced in the MSDN doc, and can be found [here](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022). This will give you the Visual Studio installer, which you should launch. You're going to want to select the "Desktop development with C++" install package, which is again poorly named, considering that it also gives us access to just plain C development tools. This install option is most likely all you'll need, but check the other options to see if you need something else. Again, just like we stated before, many of the tutorials/docs are named for C++ but are also for C as well, so don't worry.

In order for these build tools to work, a lot of environment variables need to be set correctly, which in my opinion is a very bad design decision on the behalf of windows. Thankfully, they at least provided a fix for this issue by giving us our choice of specific developer CMDs, or by using a vcvars<>.bat shell script to setup and configure any CMD for proper use in compiling with MSVC. We will go with the vcvars<>.bat option for this tutorial, but you can also just use one of the developer command prompts which does the same thing.

Unfortunately, all of these configurations and environment variables are specific to the machine/installation that you are building on, so you can't just copy the developer command prompts or vcvars<>.bat files over to another machine/installation and expect it to work, because it probably won't. So basically, this process needs to be repeated for each installation of windows. The vcvars<>.bat files can also vary from version to version, so from one VS version to the next, the location of the file and contents of the file might change. That being said, major version updates for VS don't come too often, but you should be looking to keep this process up to date at least a couple times or once per year.

The location of these vcvars<>.bat files is dependent, but I found mine for VS 2022 in the following location: `C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build`. The location can also be in many different places, such as the "Progam Files" without the x86, but this one is the correct one because it is under the "BuildTools" directory which corresponds to the Build Tools that I just referenced earlier in this tutorial for you to make sure to install. Again, windows is awful for this as there's multiple locations for these build files.

We'll use the `vcvars64.bat` one because according to the MSDN docs: "Use the 64-bit x64-native tools to build 64-bit x64 code", so it builds us 64-bit code. DO NOT cut or copy this file anywhere else. These files are so delicate and I fear that even chaging the file location could screw it up because of how delicate these things are. The cwd doesn't seem to matter at least, but do not mess with these files as they are incredibly delicate as we will see soon.

Then, run the vcvars<>.bat file only ONCE in the CMD you are using, then you can use the MSVC compiler. The vcvars<>.bat files are NOT meant to be run any more than ONE time in a given CMD, otherwise they will screw up the PATH environment variable among other environment variables. They will just add to it instead of properly changing it, so you can only run ONE of these per CMD window/instance and not more than once. A good idea would be to write a script that sets a variable to see if it has been run in your CMD already, so that it only runs it once.

Once this is done in your CMD, MSVC is now avaible to run with "cl" or "cl.exe" as the compiler, and "link" or "link.exe" as the linker. These will be valid for the rest of the lifetime of the CMD you are in.


### Windows SDK and Installing (OpenGL) Headers
Before we do anything else, we should also install headers.

The latest Windows SDK (also confusingly called "Windows Kits" for who knows what reason) is usually installed on your system when you install the Build Tools for Visual Studio as outlined above. The SDK contains many headers, such as the OpenGL headers, which we will need for this tutorial.

OpenGL extension headers are not included for whatever reason (glext.h, wglext.h, etc.), so you need to manually install them yourself. Any headers you want to install can be done manually with this method:

Find the location of the Windows SDK, which on my machine is installed in the location: `C:\Program Files (x86)\Windows Kits\`, which is confusing as they use the name "Windows Kits" instead of what should be "Windows SDK".

I found the headers in the `C:\Program Files (x86)\Windows Kits\10\` subfolder which probably refers to Windows 10 (as that is the OS being used at the time of writing this post). The headers, library files, and many other important files are here for windows development.

For example, the OpenGL headers are in the location: `C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\um\gl\` where `10.0.22621.0` is the latest SDK that was installed when installing the Build Tools for Visual Studio. You can drag and drop extension headers such as `glext.h` or `wglext.h` in here, and then it will add it to your system headers which will be recognized as `<gl/glext.h>` and `<gl/wglext.h>`. (NOTE: Windows/NTFS doesn't care about case in filenames, so "Windows.h" = "windows.h" = "WINDOWS.h", etc. By default, prefer lowercase).

You'll need to add the `khrplatform.h` header as well to your system as `glext.h` depends on `<khr/khrplatform.h>` being a valid location. This is easy, just create a `khr` folder in the `C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\um\` directory and then place the `khrplatform.h` file in that folder so that the location looks like this: `C:\Program Files (x86)\Windows Kits\10\Include\10.0.22621.0\um\khr\khrplatform.h`. Then, the system include `<khr/khrplatform.h>` or equivalently `<KHR/khrplatform.h>` will be recognized by MSVC.

You can also just place your headers in your local project folder, and then add the path to MSVC with the `/I` flag, but I prefer this method with installing it manually in the Windows SDK (or "Windows Kits") location as it keeps it all in one place, and is current to the specific version of the SDK you use. If you get a new SDK version, you'll need to repeat this process.


### Starter Code and Compilation with cl.exe (MSVC)
Once you have the headers installed and a CMD open and have run the vcvars<>.bat file (or if you have a developer command prompt, which does the same thing), type `cl` or `cl.exe` in your CMD and you should get output like this:

```
Microsoft (R) C/C++ Optimizing Compiler Version 19.43.34809 for x64
Copyright (C) Microsoft Corporation.  All rights reserved.

usage: cl [ option... ] filename... [ /link linkoption... ]
```

Otherwise, if you get some kind of error like this:

```
'cl' is not recognized as an internal or external command,
operable program or batch file.
```

Then that means you haven't run the vcvars<>.bat file (or you aren't using a developer CMD).

Once you have cl.exe available and ready, we can finally compile a basic starter "Hello World" program to make sure everything is setup correctly. Start with this C code and name it something like "win32_window.c":

```c
/*
Windows window and WGL/OpenGL example program.
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


int main(void)
{
      printf("Hello World!\n");
  
      HINSTANCE hInstance = GetModuleHandleA(NULL); /* since we aren't using wWinMain() or WinMain(), we need to grab the HINSTANCE with this function. */
      int program_running = 1;
      int fullscreen = 1; /* set to '1' if you want fullscreen, '0' if you don't */

      /* @@ user variables */
      const char* window_name = "win32 window";
      int window_width = 960;
      int window_height = 540;
      /* @! */
  
      return 0;
}
```

then, compile with this command:

```
cl User32.lib Gdi32.lib Opengl32.lib win32_window.c
```

Then run it with:

```
win32_window.exe
```

and you should get "Hello World!" as output in your CMD.

The libraries `User32.lib`, `Gdi32.lib`, and `Opengl32.lib` are all required for what we are going to do. `User32.lib` is neccessary for some basic win32 functions, and the remaining two are necessary for creating an OpenGL context.


## Dummy Window/Context
Before we jump into anything else, I need to make something clear about a certain quirk on windows that you need to be aware of when it comes to OpenGL.

For 2 reasons on windows, it is neccessary to create both a "dummy" window & GL context. We will need to first create the dummy window, get a dummy (legacy) OpenGL context, and then use that dummy GL context to load the necessary extension procedures to create a modern core-profile GL context.

The procedure that we need to create a modern core-profile OpenGL context is mainly `wglCreateContextAttribsARB`, and also a few others. The problem here is that we can't load these extension procedures unless we *already* have an OpenGL context! The procedure loading function on windows is `wglGetProcAddress`, and it requires a context to be current in order to function properly. In fact, the MSDN docs say that the function pointers returned by `wglGetProcAddress` are only valid given a current-context, and it may not be valid for any other contexts.

So how do we solve this apparent Catch-22? The answer is that we are still able to create a *legacy* OpenGL context as windows provides legacy GL functions to do so, namely `wglCreateContext`. Once we have this GL context and have made it current, we can then use it to query for and load procedures that we use to create the new or *modern* OpenGL context, and then we can get rid of the old one.

Along with that, windows only allows us to set the pixel format of a window once. If we want to change the pixel format of window, we can't. The only way to change pixel formats is to create a new window. Since it is required to set a pixel format for a window to get an OpenGL context, we need to set the pixel format of the window and *then* use that window to create the dummy (legacy) GL context. This would be all fine, except that modern GL context creation has extended pixel formats that we will want to support, with extension functions provided such as `wglChoosePixelFormatARB` to give us more options for pixel formats. So because of all those reasons, we also find it necessary to create a new window to take advantage of new pixel formats that come with modern GL.

So in short, because of both the nature of loading procedures requiring an existing context AND pixel formats only being able to be set once; we need a dummy window AND a dummy GL context, of which both we will destroy once we have the "real" window and "real" GL context.


### Dummy Window/Context Creation
Finally we can start doing some real work!

In order to create the dummy window and dummy GL context, you can use this code:

```c
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
```

The first we thing we need to do is create a window class for the dummy window and register it with `RegisterClassA()`. Window classes in win32 are basically just window information for a window class, they don't really serve much of a purpose beyond the fact that they can be reused across more than one window. We won't be doing such thing, so we'll create the window class for our dummy window.

We also need to pass in a pointer to the `RegisterClassA()` function that specifies something called the "window procedure", a special callback that we must register for each window. This window callback is a function called by the OS which receives all kinds of events or messages that we need to handle.

Since this is a dummy window that we will destroy, we don't actually need to worry about how to handle the window procedure so much, so we can just write the function with `DefWindowProc()` which is the "default window procedure" provided to us by win32 that automatically handles every message:

```c
LRESULT CALLBACK DummyGL_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
      return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
```

Then, we can pass `DummyGL_WindowProc` to the `RegisterClassA()` which will register this window callback to the one used for any windows created with this window class. We will go over what the Window Procedure is and how to write it in more detail later, but for the dummy window, we just need default behaviour as we'll never actually use the window for anything else besides getting a context. The user of this application will never see the dummy window.

Next, we then create the dummy window with `CreateWindowA()`, and we also grab the window DC (device context) which is necessary for context creation.

To create an OpenGL context in windows, we need to first set a pixel format. We choose it first with `ChoosePixelFormat()`, and then set it with `SetPixelFormat()` which is set per-window. Even though we pass in the device context to this function, it refers to our window.

Right from the [MSDN docs](https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-setpixelformat):

> If hdc references a window, calling the SetPixelFormat function also changes the pixel format of the window. Setting the pixel format of a window more than once can lead to significant complications for the Window Manager and for multithread applications, so it is not allowed. An application can only set the pixel format of a window one time. Once a window's pixel format is set, it cannot be changed.

So this is what is behind the reason for needing a dummy window, as we will see that when we load the extension procedures for extension pixel formats, we need a fresh window to set a new pixel format.

The next thing we do after that is create the dummy context with the wgl function `wglCreateContext()` which just takes in our window DC. Then, we need to make it current with `wglMakeCurrent()` so that we can start loading procedures.


### Loading WGL Extension Procedures
Now that we have our (legacy) dummy GL context, we can load the important procedures that will be neccessary for a modern core-profile GL context.

There are three extensions that we need to create a new context: `WGL_ARB_extensions_string`, `WGL_ARB_pixel_format`, and `WGL_ARB_create_context_profile`.

The first extension, `WGL_ARB_extensions_string`, is important as it gives us the procedure `wglGetExtensionsStringARB()` which gives us the ability to query what extensions are available.

Unfortunately, due to some oversight, `wglGetExtensionsStringARB()` is not provided statically, so we just have to load it and hope that it is present, otherwise we have no ability to query what extensions are available. This is also another Catch-22 situation, as we need the `wglGetExtensionsStringARB()` function to be present in order to query if extensions are present, including `WGL_ARB_extensions_string`. GLX handles this much better as there is a statically provided `glXQueryExtensionsString` which does the same thing.

So, we just try to load `wglGetExtensionsStringARB()` and hope that it is available. As for *how* to load extension procedures, we use the wgl function `wglGetProcAddress()`.

`wglGetProcAddress()` apparently does not function as documented in MSDN, as according to this [Khronos wiki entry](https://www.khronos.org/opengl/wiki/Load_OpenGL_Functions):

> While the MSDN documentation says that wglGetProcAddress returns NULL on failure, some implementations will return other values. 1, 2, and 3 are used, as well as -1.

So, we wrap `wglGetProcAddress()` in our function `Load_WGL_Proc()` like this:

```c
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
```

We also need a function to parse the extension string as it is a space-seperated list of extensions names (there are no spaces in an extension name). In python, this might be a simple one-liner, but in C, it takes a bit more work. My implementation is here, I call the function `Check_Extension_Available()`:

```c
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
```

I don't claim this is the best or most secure, it's just what I quickly wrote up.

Now that we have that, we can finally load the procedures:

```c
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
```

The second extension: `WGL_ARB_pixel_format`, gives us the procedures needed to create new types of pixel formats mainly with the procedure: `wglChoosePixelFormatARB()`.

The third extensions: `WGL_ARB_create_context_profile`, gives us the important function `wglCreateContextAttribsARB()` which is what is used to actually create the modern core-profile OpenGL context.

Now we are ready to create the real GL context!

## Window Procedure
Before we do indeed create the real window and real GL context, we need to revisit the window procedure.

Unfortunately, win32 is designed in a way so that it is necessary to have a window procedure callback that you must define so that it can handle potentially *thousands* of different kinds of messages. A typical window procedure might look like the following:

```c
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // All painting occurs here, between BeginPaint and EndPaint.

            FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));

            EndPaint(hwnd, &ps);
        }
        return 0;

    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
```

where you either decide to intercept messages yourself, or let them be handled by the Default Window Procedure `DefWindowProc()`.

Microsoft doesn't even document these messages properly, with their documentation being scattered all over MSDN and not in a central place. Luckily, the folks over at WINE have taken it upon themselves do to so which can be found [here](https://gitlab.winehq.org/wine/wine/-/wikis/Wine-Developer's-Guide/List-of-Windows-Messages).

What's not so great is that the window procedure can be called by windows (the OS) at *any time* and there is no way of predicting when! Rather than just having a simple message loop that blocks until it gets messages, the creators behind win32 also opted to go for a callback design with the window procedure message, which is nothing less than an awful design choice no matter what historical context prompted it.

The window procedure can either be called in the message loop by `DispatchMessage()`, which is a function we will call (we'll get to the message loop later), OR, it can be called at any point by the OS. There are essentially two different event-handling paradigms (callback/queued) that are both present in win32. This is the worst of both worlds, not the best.

There is also absolutely no clear and cut way to know what messages you should handle, whether you should intercept some and handle completely yourself, or if you should handle it and <i>then</i> pass it to the default window procedure - OR - if you should only use the default window procedure period. Since we don't have the source code for `DefWindowProc()`, half the time we just have to guess what it does, as most of the documentation for it is in the MSDN description for a message and is usually vague and somtimes completely absent.

What's even worse is that if you choose not to handle some messages, your window or application will break in completely unpredictable ways, as the message model that win32 operates on is very odd in several places. Sometimes messages will be passed in for purely informational purposes (such as when a window is created), but you still have to return a specific value otherwise certain functions will break (such as `CreateWindow()`).

What the OS is essentially doing is "recruiting" our process (specifically the thread the window was created in) to do the window processing that it should be handling itself. This is a terrible design decision and could have only ever been cooked up by microsoft. It is overly complex and very error-prone, with absolutely no benefit.

Rather than handling it in the OS (where it should be) a lot of critical default behaviour is offloaded onto your thread, which causes unexpected behaviour that can be bad in very subtle ways. See this [link](https://github.com/libsdl-org/SDL/issues/1059) for example of how much of a headache this caused for the folks at SDL. This is just one particular example.

All of this *might* be acceptable if it was well-documented exactly what the default window procedure does, as we are essentially forced into calling it on our own process thread, rather than the OS handling itself (which it should). But of course, as we would expect, the documentation for `DefWindowProc()` is spread out all over MSDN for each message (there are thousands of messages!). Most of the time, only vague descriptions are given as to what the default window procedure does, which is next to useless.

Thankfully, the folks at WINE have once again come to the rescue and they have reverse engineered the default window procedure which can be found [here](https://gitlab.winehq.org/wine/wine/-/blob/master/dlls/win32u/defwnd.c#L2388).

To deal with all of this nonsense, we basically just have to call the default window procedure, and try to figure out which cases we need to handle ourselves. This is a tedious trial-and-error process and is unfortunately the only way to go. Once again, absolutely terrible design on the behalf of microsoft.

On the bright side, we can learn from existing codebases (such as the reverse-engineered one in WINE) for how to write our window procedures.

The implementation we will use for this tutorial is given here:

```c
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
```

This minimal implementation has empty cases for all kinds of keyboard input and mouse input. The `WM_PAINT` message in particular is handled ourselves as we don't want the default procedure painting our window. Since we will be using OpenGL for graphics, we only call `ValidateRect()` to let the OS know that the whole window reigon is "validated" so that it won't continue to spam more `WM_PAINT` and stall our application.


## Creating the Real Window and Real GL Context
Now we can finally create the real window and real GL context!

To do so, use this code:

```c
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
```

Creating the window is the same as before, except that this is the real window so you want to set properties knowing that it's the one that will be visible. After window creation, we grab the window DC again which will be used in creation of the real context.

Now, we can create the new GL context which is where our new extensions come into play.

We need to specify an attribute list: `pi_attrib_list`, which we set to have a number of important properties. This attribute list will be used in creation of our new pixel format with the `wglChoosePixelFormatARB()` function, and is the main reason why we needed this extension function in the first place. With this attribute list we can also choose double buffering and/or MSAA, which was not possible with the legacy context.

Then, we use `DescribePixelFormat()` to get a pixel descriptor, and then pass that to `SetPixelFormat()` so that we can finally set the pixel format for this window.

We then create the context with `wglCreateContextAttribsARB()` which we loaded prior, passing in an attribute list to specify the version of OpenGL we will be using and some other things.

Once we are done that, we cleanup the old dummy window and dummy context as we no longer need it. Right after that, we finally make the new GL context current with `wglMakeCurrent()` and now we have a modern OpenGL context created current to our thread!

Technically we're done at this point as we have a modern GL context now, but there's still more to go over.

### Loading WGL Extension Procedures (Again)
Now that we have a new context, we should reload any procedures that we had in the previous context. This is a matter of good practice, as according to the [MSDN documentation](https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-wglgetprocaddress):

> The OpenGL library supports multiple implementations of its functions. Extension functions supported in one rendering context are not necessarily available in a separate rendering context. Thus, for a given rendering context in an application, use the function addresses returned by the wglGetProcAddress function only

So it's best to reload any extension procedures that we need.

```c
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
```

In this case, we also load `wglSwapIntervalEXT()` which is used to set v-sync.


### Loading GL procedures
Let's also load some GL procedures that we might need for a typical short OpenGL "hello triangle" application:

```c
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
```

This code makes use of the `Load_WGL_Proc()` wrapper function that we wrote beforehand.


## Setting up Hello Triangle
Once we have the GL functions loaded, we can actually start doing some OpenGL rendering. Here is some code for creating a basic "hello triangle" that we will render:

```c
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
```

Once this is done, we'll be able to render a simple triangle to the screen in our main loop, which we'll talk about next.

Before we do that, we should set the GL viewport with `glViewport()` and also show our window with `ShowWindow()` so that it is visible:

```c
/* @@ setting glViewport() */
RECT window_size;
GetClientRect(window_handle, &window_size);

glViewport(0, 0, window_size.right - window_size.left, window_size.bottom - window_size.top);
/* @! */


/* @@ Finally at the end, we show the window, similar to XMapRaised() or XMapWindow() for X11 */
ShowWindow(window_handle, SW_SHOWNORMAL);
/* @! */
```

The window is technically visible at this point, but the application will return almost as soon as it starts so we might not see it. To see our window (and also render the triangle), we need to write the main loop.


### Main Event/Message Loop & Cleanup
Now we need to write the main event or message loop which will process window messages and also do rendering.

The code is as follows:

```c
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
```

There's not much that's different here than on other platforms, besides of course the `PeekMessageA()` message loop. Basically, we get all the messages of the queue and process them one-by-one. `DispatchMessageA()` is the function that actually sends our message to the window procedure `WindowProc()`. Keep in mind that not all messages are sent with `DispatchMessageA()`, many are just sent directly to the window procedure.

Lastly, once the main loop exits, we need to cleanup everything and return:

```c
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
```

And that's it! Once you have this working, you're on your way to program native OpenGL applications on windows!


## Source Code
As promised, the full source code (and more) is provided in my [github repo](https://github.com/James822/win32-opengl-window). Feel free to use it as you wish. There's some minor differences, namely there's a section of code that turns on fullscreen which I left out in the tutorial here as I viewed that to be windows-specific and not really related to OpenGL.


## References
- MSDN tutorial for using MSVC on the command line: [https://learn.microsoft.com/en-us/cpp/build/building-on-the-command-line?view=msvc-170](https://learn.microsoft.com/en-us/cpp/build/building-on-the-command-line?view=msvc-170)
- Build Tools for Visual Studio: [https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022)
- Khronos OpenGL registry: [https://registry.khronos.org/OpenGL/index_gl.php](https://registry.khronos.org/OpenGL/index_gl.php)
- Khronos Opengl Context Creation Tutorial: [https://www.khronos.org/opengl/wiki/Tutorial:_OpenGL_3.0_Context_Creation_(GLX)](https://www.khronos.org/opengl/wiki/Tutorial:_OpenGL_3.0_Context_Creation_(GLX))
- Khronos tutorial on loading OpenGL functions: (https://www.khronos.org/opengl/wiki/Load_OpenGL_Functions)[https://www.khronos.org/opengl/wiki/Load_OpenGL_Functions]
- WINE list of windows messages: [https://gitlab.winehq.org/wine/wine/-/wikis/Wine-Developer's-Guide/List-of-Windows-Messages](https://gitlab.winehq.org/wine/wine/-/wikis/Wine-Developer's-Guide/List-of-Windows-Messages)
- WINE implementation of default window procedure: [https://gitlab.winehq.org/wine/wine/-/blob/master/dlls/win32u/defwnd.c#L2388](https://gitlab.winehq.org/wine/wine/-/blob/master/dlls/win32u/defwnd.c#L2388)
- The amazing and classic learnOpenGL tutorial: [https://learnopengl.com/](https://learnopengl.com/)
