+++
date = '2025-04-05'
draft = false
title = 'Creating an X11 Window and OpenGL Context on Linux From Scratch'
+++


Part of the motivation for this post/tutorial was how arcane it is to open a basic window and start drawing graphics. Most beginner programmers typically start out learning basic graphics, but it's usually through a library or framework that sort of robs you of learning the underlying mechanisms.

I was more interested in how to get pixels on the screen *without* having to use a library such as Qt or JavaFX. I wanted to figure out how to at least open a basic window that you could start drawing to by programming the operating system directly.

After opening a window the next logical step would be hardware acceleration, and for that I've chosen **OpenGL**, but the same basic principles apply for other graphics standards such as Vulkan. It's not strictly necessary to do graphics with dedicated hardware, but software rendering is typically slower and more difficult as you're pushing everything on the CPU.

Also, because different operating systems have different programming interfaces; I've chosen Linux to start with. I'll be tackling Microsoft Windows in a future post, but for now we'll focus on Linux only in this post.


## X11 & GLX
To talk about opening a native window on any platform we need to understand what the native windowing system is. In the case of Microsoft Windows, the windowing system is built into the general platform API known as win32. In the case of Linux, windowing systems are separate from the kernel (properly speaking, separate from Linux itself), but the most widely used windowing system across different Linux distributions is **X11** by far. Wayland is another option, but I won't be touching that.

X11, or *The X Window system* is a bitmap display system based on a client-server architecture with a specified protocol called the "X Protocol". Applications communicate to the X Server (which may be on the same device, or across a network) with commands to peform actions.

Programming X11 is therefore writing a client application that can communicate with an X11 server, via the X Protocol. Typically, the X server is on the same device as the client, and in fact we will be counting on that in our OpenGL application as we don't want to consider networked graphics applications.

It is within possibility to learn the X Protocol and manually implement it within your client application, but this is beyond what I would consider practical. Rather than that, Xlib is provided for us as a C library that implements the X Protocol so that we can communicate between server and client with an API interface. This is the option I will be going with for this tutorial, but the X protocol is standardized and you absolutely could go look at it for yourself.

In order to create and use an OpenGL context, we will need an extension to X11/Xlib called **GLX**, or the *OpenGL extension to the X Window System*, which is what allows us to create a framebuffer and use it within an OpenGL context. If we were on windows, we would use win32/WGL. GLX is what allows us to do most of the important work to link our X11 Window and OpenGL context so that we can start doing graphics rendering.


### Documentation for X11/Xlib and GLX
It's important to be aware of the documentation for the two main APIs involved here, X11/Xlib, and GLX, of which GLX is really just an API extension to Xlib.

Unfortunately, the documentation for creating a window and GL context with Xlib/GLX is rather spread out all over the place. Further, the documentation that does exist is poor and outdated in some areas, which is part of the motivation for me making this tutorial.

The Xlib documentation is found here: [https://www.x.org/wiki/ProgrammingDocumentation/](https://www.x.org/wiki/ProgrammingDocumentation/) <a href=""></a>, and the GLX documentation is found within the Khronos OpenGL registry here: [https://registry.khronos.org/OpenGL/index_gl.php](https://registry.khronos.org/OpenGL/index_gl.php). Even though GLX is a library extension to Xlib, it is listed in the OpenGL Registry which can be somewhat confusing. Out of all the documentation cited in this post, GLX is easily the worst, so I'll do my best to explain it. You should really grab the documentation for Xlib, GLX, and also the OpenGL core spec as well, which can also be found in the Khronos OpenGL registry.

GLX 1.4 is the latest version and the one we will be using, but the documentation for it is outdated and flawed in many ways. It appears that the maintainers have given up on updating it since probably the early 2010s. For example, this documentation here: [https://registry.khronos.org/OpenGL/extensions/EXT/GLX_EXT_swap_control_tear.txt](https://registry.khronos.org/OpenGL/extensions/EXT/GLX_EXT_swap_control_tear.txt) suggests edits and additions to the GLX 1.4 spec that are not actually present. If you grab the latest spec you won't find these additions or changes anywhere. There are several other examples where these proposed "additions" are not to be found anywhere with the GLX 1.4 spec found in the Khronos OpenGL registry. It's possible that these extensions were intended for the next version of GLX, so "GLX 1.5", but that never came so we are left with somewhat of a mess.

What this really means for us is that we just need to read the extension documentation alongside the GLX 1.4 spec, which is not really that bad.


## Starter Program and Compilation with GCC
I'm going to be compiling with GCC and be using C, but you can also follow along in C++ with only a few modifications. If you read the Xlib documentation you'll note that most functions are provided as Macros, this is exactly so that either C or C++ can use the macro and get the right function. I just call the functions directly personally.

Anyways, start with this C code, and name it something like "x11_window.c":

```c
#include <stdio.h>;

#define TRUE 1
#define FALSE 0
  
int main()
{
      printf("Hello World!\n");
      return 0;
}
```

then, compile with this command:

```
gcc x11_window.c -o x11_window
```

run it with:

```
./x11_window
```

and you should get "Hello World!" as output.

Also, at the end of the main function add a cleanup section like this:

```c
#include <stdio.h>

#define TRUE 1
#define FALSE 0
    
int main()
{
      printf("Hello World!\n");


 CLEANUP_AND_EXIT:
      return 0;
}
```

for now the cleanup section just returns from the main function. We'll be referring back to this cleanup section later.


## Setting up X11 Display
The first task is to open a connection to the X11 Display Server, the *X Server*. If no server exists on the host machine (or on the network if we specify a device across the network), then this will fail and we will have no way of displaying anything. We NEED an X server to have an X window, so either prompt the user to install/run the X server or something else.

Anyways, we can open a connection to the X server and get a Display back using the `XOpenDisplay()` function.

```c
/* @@ opening connection to X server */
Display* display = NULL;
int display_exists = FALSE;
int default_screen_id = 0;

display = XOpenDisplay(NULL);
if(display == NULL) {
      printf("ERROR: failed to open X11 display, display name: \"%s\"\n", XDisplayName(NULL));
      goto CLEANUP_AND_EXIT;
} else {
      display_exists = TRUE;
      default_screen_id = XDefaultScreen(display);
}
/* @! */
```

make sure to also have `#include <X11/Xlib.h>` and also link to X11 with GCC by adding the flag: `-lX11`.

This code snippet opens a connection to the X Server, and if it succeeds, also grabs the screen ID which we will need later. `NULL` is passed in to `XOpenDisplay()` which will give us the default display and screen of the device we are on.

In the cleanup section, we need to close the display with `XCloseDisplay()` like this:

```c
/* @@ cleanup and program exit */
CLEANUP_AND_EXIT:
     if(display_exists == TRUE) {
	   XCloseDisplay(display);
     }

     return 0;
     /* @! */
```

And that's it for now.


## Setting up GLX
Before we open an X11 window or do anything else, we should query X to see if GLX is supported. We do that using the `glXQueryExtension()` function, and we also need to include GLX with: `#include <GL/glx.h>`. We also need to link to GLX with GCC using: `-lGLX`.

The basic code for setting up GLX is like this:

```c
/* @@ glx initial setup */
int error_base;
int event_base;
if(glXQueryExtension(display, &error_base, &event_base) != True) {
      printf("ERROR: GLX extension not supported for this X Server\n");
      goto CLEANUP_AND_EXIT;
}

int glx_major_version;
int glx_minor_version;
if(glXQueryVersion(display, &glx_major_version, &glx_minor_version) != True) {
      printf("ERROR: glXQueryVersion() failed\n");
      goto CLEANUP_AND_EXIT;
}
if( glx_major_version < 1 || (glx_major_version == 1 && glx_minor_version < 4) ) {
      /* The reason why we require GLX 1.4 (or higher, although it's doubtful a new version will be released) is because the GLX_ARB_create_context/GLX_ARB_create_context_profile extensions (which we need) requires GLX 1.4, and we need this extension in order to create a modern OpenGL context, which is what we are doing. "Modern" is somewhat of an outdated term as GLX 1.4 was released in 2005/2006, so there's nothing really unportable as we require the user to have hardware that can at least support this "modern" OpenGL. */
      printf("ERROR: GLX version is less than 1.4\n");
      goto CLEANUP_AND_EXIT;
}
/* @! */
```

With this code snippet, we query to see if the GLX extension to X11 is available for this X display using `glXQueryExtension()`. Then, we check which version of GLX is supported and we make sure it is at least 1.4, otherwise we won't be able to create a modern OpenGL context due to the `GLX_ARB_create_context_profile` extension requiring GLX 1.4, according to the documentation for the extension found [here](https://registry.khronos.org/OpenGL/extensions/ARB/GLX_ARB_create_context.txt).

I won't be considering supporting old or legacy OpenGL in this tutorial, as I view that is a niche topic with less practical application.


### Checking for GLX extensions
After having made sure we have GLX 1.4 supported, we then need to check to see what extensions are supported. Some extensions are mandatory for creating a modern OpenGL context, but some are also very useful such as querying for swap control which gives us access to double-buffered vsync swapping. Note that GLX extensions are NOT the same thing as OpenGL or GL extensions, they are mutually exclusive with some exceptions. GLX extensions are properly extensions to GLX itself that gives you greater functionality, one of these extensions is `GLX_ARB_create_context_profile` which is necessary for a core OpenGL profile.

To query for extensions we use the `glXQueryExtensionsString()` function, which gives us a space-separated list of extensions that we can check through. It's a good idea to print out this string and check each extension for yourself to see what kinds of things GLX extensions accomplish.

For our interests, we only care about two other extensions besides `GLX_ARB_create_context_profile`, which are `GLX_EXT_swap_control` and `GLX_EXT_swap_control_tear`. These two swap control extensions give us normal vsync and adaptive vsync respectively. We don't strictly need them, so you can decide whether they are required for your application or not.

```c
/* @@ Checking for GLX extensions */
const char* glx_extensions_string = glXQueryExtensionsString(display, default_screen_id);      

/* @NOTE: GLX_ARB_create_context_profile is absolutely necessary in order to create a GL context, we cannot proceed without it. Technically, we need only check if GLX_ARB_create_context is supported rather than GLX_ARB_create_context_profile, because the former implies the latter is supported, IF the implementation supports OpenGL 3.2 or later, but we are definitely using a version higher than OpenGL 3.2, version 4 actually. Still, just in case, we explicitly check if GLX_ARB_create_context_profile is supported to be as robust as possible. See this documentation for more details: https://registry.khronos.org/OpenGL/extensions/ARB/GLX_ARB_create_context.txt, specifically check the "Dependencies on OpenGL 3.2 and later OpenGL versions" section. */
if(search_str_in_str(glx_extensions_string, "GLX_ARB_create_context_profile") != TRUE) {
      printf("ERROR: \"GLX_ARB_create_context\" extension not supported, cannot create GL context!");
      goto CLEANUP_AND_EXIT;
}
/* EXT_swap_control give us the ability to synchronize the swap (v-sync) by an integer amount of frames greater or equal to 0. */
if(search_str_in_str(glx_extensions_string, "GLX_EXT_swap_control") != TRUE) {
      printf("ERROR: \"GLX_EXT_swap_control\" extension not supported!");
      goto CLEANUP_AND_EXIT;
}
/* This allows us to use adaptive v-sync for when we miss a swap by specifying negative integer intervals. Adaptive vsync is a generally a good compromise with double-buffering. This extension depends upon the GLX_EXT_swap_control extension to be available/supported. */
if(search_str_in_str(glx_extensions_string, "GLX_EXT_swap_control_tear") != TRUE) {
      printf("ERROR: \"GLX_EXT_swap_control_tear\" extension not supported!");
      goto CLEANUP_AND_EXIT;
}
/* @! */
```

The `search_str_in_str()` function can be whatever you want, but all it does is find if a string is contained within another string.

Otherwise, that's basically it for searching for GLX extensions. You can check for any other additional extensions if you like, but we'll be sticking with these three.


### Loading GLX extension procedures
All GLX/GL extensions add functionality in one way or another. Some of them just change how things work, but some of them add new procedures that we have to load at runtime. These are not available at runtime precisely because they are extensions, so depending on the platform, they may or may not be available. Not only that, but the way they are implemented is also different depending on the driver's implementation.

Supposedly, extensions should function as they are described by the standards, but in reality, there is not universal consistency and sometimes things don't work as expected. This is annoying, but we have to deal with this as graphics programmers and just do our best.

The most important extension is the `GLX_ARB_create_context_profile` extension which gives us the function `glXCreateContextAttribsARB()`, which is what we need to use to create a modern (core profile) OpenGL context.

Since `glXCreateContextAttribsARB()` is an extension procedure, we need to load it with function pointers. But how can we load this function? It turns out, GLX provides us with a function `glXGetProcAddress()` which allows us to load both GLX extension procedures AND OpenGL (GL) extension procedures. There is also the similar `glXGetProcAddressARB()` function, which only differs in the "ARB" suffix. The ARB function is the extension function, as this used to be an extension to GLX but is now integrated into GLX 1.4 as a main function.

```c
/* @@ loading GLX extension procedures (we need the "glxext.h" header for the function typedefs and declarations). */
PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB = NULL;
PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT = NULL;

glXCreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC)glXGetProcAddress((const GLubyte *)"glXCreateContextAttribsARB");      
glXSwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)glXGetProcAddress((const GLubyte *)"glXSwapIntervalEXT");

if(glXCreateContextAttribsARB == NULL) {
      printf("ERROR: glXCreateContextAttribsARB() function pointer NULL, cannot create GL context!\n");
      goto CLEANUP_AND_EXIT;
}
if(glXSwapIntervalEXT == NULL) {
      printf("ERROR: \"glXSwapIntervalEXT\" function pointer NULL\n");
      goto CLEANUP_AND_EXIT;
}
/* @! */
```

Along with loading the `glXCreateContextAttribsARB()` proc, we also load the `glXSwapIntervalEXT()` proc which allows us to specify a swap interval, giving us the ability to control vsync. Of course, since drivers can override this feature (such as if a user forcibly disables vsync in their control panel for their graphics card), then we can't ensure that it actually works exactly as we request it.

Also, make sure you add `#include <GL/glxext>` which is the extension header for GLX that gives us the extension procedure typedefs and many other useful things. Make sure to include it AFTER the `#include <GL/glx.h>` otherwise it won't compile.


### Selecting GLX framebuffer config
Now that we have the GLX extension procedure(s) loaded, we need to start thinking about creating a window or framebuffer. We need to create an X11 window that is also compatible with OpenGL rendering, specifically with things like double buffering and other information.

To accomplish this, GLX provides something called a *framebuffer config* or `GLXFBConfig`, which encapsulates the information we need to create a consistent window/framebuffer between OpenGL and X11.

Our task is therefore to select a suitable `GLXFBConfig`, and we can ask for specific settings such as the size of the GL depth buffer, color-bit depth, double-buffering enabled, etc., to get a set of fb configs that match our desired settings:

```c
/* @@ selecting a GLX FB Config */
/* This fbc attrib array is what specifies the kind of framebuffer we will get, so it's important to specify everything we need and not leave anything to defaults (unless we don't care what it is). It's possible that we specify a configuration that isn't supported, which means either we need to choose another one that is supported, or we deem the platform to be unsupported by our application. */
int fbc_attribs[] = {
      GLX_LEVEL, 0,
      GLX_DOUBLEBUFFER, True,
      GLX_STEREO, False,
      GLX_RED_SIZE, 8,
      GLX_GREEN_SIZE, 8,
      GLX_BLUE_SIZE, 8,
      GLX_ALPHA_SIZE, 8,
      GLX_DEPTH_SIZE, 24,
      GLX_STENCIL_SIZE, 8,
      GLX_RENDER_TYPE, GLX_RGBA_BIT,
      GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
      GLX_X_RENDERABLE, True,
      GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
      GLX_CONFIG_CAVEAT, GLX_NONE,
      GLX_TRANSPARENT_TYPE, GLX_NONE,
      None /* we must put this "None" value here as this array is passed in with no size parameter */
};

int num_fb_configs = 0;
GLXFBConfig *fb_configs = glXChooseFBConfig(display, default_screen_id, fbc_attribs, &num_fb_configs);
if(fb_configs == NULL || num_fb_configs == 0) {
      printf("ERROR: glXChooseFBConfig() failed, or it returned no matching fb configs\n");
      goto CLEANUP_AND_EXIT;
}
GLXFBConfig best_fb_config = fb_configs[0]; /* for now we are just picking the first FBConfig in the array, but you may want to actually sort through the array and select the one to your liking. */
XFree(fb_configs);
/* @! */
```

The `fbc_attribs` is an array of pairs, where the first item is a symbolic constant specifying the attribute, and the second item is the value for that attribute.

There are many attributes here, I won't go over all of them but some are important which I will detail here:

- `GLX_DOUBLEBUFFER` is a boolean setting for whether we want to have doublebuffering or not. We set it to True because we want double buffering, which is generally what you want.
- `GLX_RED_SIZE`, `GLX_GREEN_SIZE`, and `GLX_BLUE_SIZE` set the bit-depth of the red, green, and blue color channels respectively. We want 8-bit color here which is fairly normal, so we set all channels to 8.
- `GLX_DEPTH_SIZE`, and `GLX_STENCIL_SIZE`, are the sizes of the depth buffer and stencil buffer respectively, which we set to 24 and 8, for a total of 32-bits.

The other settings are important too, check the GLX/Xlib documentation to see what they mean and to see what the other attribs are.

Once we have our attrib array, we need to pass that into the `glXChooseFBConfig()` function and see if there are any framebuffer configs on the system that matches our requested attributes. The call to `glXChooseFBConfig()` will return a list of GLXFBConfigs or NULL on failure. Then, we just pick the first `GLXFBConfig` out of the list and save it for later.

Once we are done, we can free the `fb_config` array with `XFree()`. If you're wondering why you can't find `XFree()` in the GLX 1.4 spec, it's because it's not there. `XFree()` is an Xlib function, which may seem weird as to why we're using an Xlib function for an array returned by a GLX function. But don't forget, GLX is just an *extension* to Xlib/X11, so it operates on the same basic inherited things that Xlib does, such as memory management in this case. Another thing GLX inherits are the Xlib boolean definitions `True` and `False`, which are separate from my own personal `TRUE` and `FALSE`.


## Creating an X11 Window and GLX Window
Finally we have what we need to create the Window, the main subject of this tutorial along with an OpenGL context.

To create an X11 window, we need to use the framebuffer config from before, and we also need to configure some other X11 attributes. We also need to create a GLX window after, which is basically just a pointer to the X11 window that we have, but it does accomplish some specific things. I've seen tutorials and people on the internet claim that the `GLXWindow` type is essentially useless, but this is far from true. More on that later anyways.

The code to do all this is:
  
```c
/* @@ creating X11 Window and GLX window */
Window parent_window = XDefaultRootWindow(display);
Window window;
int window_width = 960;
int window_height = 540;

XVisualInfo* visual_info = glXGetVisualFromFBConfig(display, best_fb_config); /* this GLX function gives us the structure we need to pass into X11 window create function so that our GLX fb config is consistent with the X11 window config. */
if(visual_info == NULL) {
      printf("ERROR: glXGetVisualFromFBConfig() failed to get a XVisualInfo struct\n");
      goto CLEANUP_AND_EXIT;
}

XSetWindowAttributes window_attributes;
window_attributes.background_pixmap = None;
window_attributes.background_pixel = XWhitePixel(display, default_screen_id);

Colormap cmap = XCreateColormap(display, parent_window, visual_info->visual, AllocNone);
window_attributes.colormap = cmap;

window = XCreateWindow(display, parent_window, 0, 0, (unsigned int)window_width, (unsigned int)window_height, 0, visual_info->depth, InputOutput, visual_info->visual, CWBackPixmap | CWBackPixel | CWColormap, &window_attributes);
window_exists = TRUE;
XFree(visual_info);


/* now we create the GLX window which is basically just a handle to our X11 window, but it does accomplish very important things so we absolutely need it */
/* we need to use the same exact FB config as used to create the context AND to create the X11 window, so we just pass that in here as well. */
GLXWindow glx_window = glXCreateWindow(display, best_fb_config, window, NULL);
glx_window_exists = TRUE;

/* from now we use glx_window instead of window for any GLX/GL functions. For normal X functions, we continue to use the normal X window. */
/* @! */
```

Firstly, we need to get a X11 Visual Config from our previous fb config with the `glXGetVisualFromFBConfig()` function. It may seem pointless that we have the fb config in the first place when we just convert it to an `XVisualInfo` struct, but the GLX fb config is necessary for the GLX window, and especially for creating an OpenGL context.

Then, we need to fill out a `XSetWindowAttributes` struct which contains some basic properties. I've only set some basic stuff here like making the window white by default, and getting rid of a background pixmap which we don't need.

We also need a colormap which can be created by a call to `XCreateColormap()`. I just use the default colormap which is just taken from the parent window. We also need to pass in the `visual` attribute from the `visual_info` struct we created from the GLX fb config.

With those set, we can pass those into the `XCreateWindow()` function and finally get our window. We also pass in other settings, like the window width and height. Once we have the window we can free the `visual_info` struct with `XFree()`.

Along with the X window, we also need to create a GLX window which is very simple and only requires a call to `glXCreateWindow()`, and we need to pass in our GLX fb config which we created earlier, as well as the X window we just created.

From then on, any GLX functions should only use the `glx_window` variable instead of the X window variable. Use of the latter instead of the former is incorrect, but that is what is commonly seen in many tutorials such as these. Later, I'll go into depth as to why this is necessary. For Xlib functions, use the X window rather than the GLX window.

Also, since we created the X window and GLX window, we should clean them up in the cleanup section which will now look like this:
```c
     /* @@ cleanup and program exit */
CLEANUP_AND_EXIT:
     if(glx_window_exists == TRUE) {
	   glXDestroyWindow(display, glx_window);
     }
     if(window_exists == TRUE) {
	   XDestroyWindow(display, window);
     }
     if(display_exists == TRUE) {
	   XCloseDisplay(display);
     }

     return 0;
     /* @! */
```

If you compile the code, the window won't be visible yet because we haven't made the necessary call to `XMapRaised(display, window)` which will display our window and raise it to the top. Even doing this won't work because the X server hasn't synced its request, so we need to do that with the call: `XSync(display, False)`, but even then you won't see the window just yet. The window will close as soon as it opens, so we need to add a sleep() call or something like that to see the window.

You could do this if you want to see the window and make sure that it is working, but delete that code for later because we'll handle this in a different way.


## Creating a GLX Context (OpenGL context) and making it current
Now we can create an OpenGL context, and we do this with GLX. If it's confusing as to why an OpenGL context is created with GLX rather than OpenGL itself, that's because OpenGL doesn't "exist" at this point without a context, so something that doesn't exist cannot create itself. Sure, we can include the GL headers, but the actual context state has not been created yet. Furthermore, much about the context or context state is implementation or platform dependent, so it must be created at runtime, hence the need to create a GL context.

Creating a *modern* OpenGL context or *core profile* context is only supported as a (GLX) extension, specifically, the `GLX_ARB_create_context_profile` we queried for earlier is needed to do so. The Windows equivalent for this is `WGL_ARB_create_context_profile`.

The `GLX_ARB_create_context_profile` gives us the extension procedure `glXCreateContextAttribsARB()` which is what we need to use for a modern or core profile OpenGL. GLX also provides the functions `glXCreateNewContext()` and `glXCreateContext()`, but these functions are for OLD pre-modern OpenGL, so we don't want to use them.

To create the context, we use the `glXCreateContextAttribsARB()` function as so:

```c
/* @@ Creating GLX (GL) context, and making context current. */
int context_attribs[] = {
      /* @@ with GLX_CONTEXT_MAJOR_VERSION_ARB and GLX_CONTEXT_MAJOR_MINOR_VERSION, the context creation function may actually give us a higher version as long as it is backwards compatible, so we need to query the real OpenGL version with glGet. If we ask for 4.6, it's unlikely we'll get a higher version as they probably won't update OpenGL further. */
      GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
      GLX_CONTEXT_MINOR_VERSION_ARB, 6,
      /* @! */

      GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,	    
      GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB, /* @@ Forward compatible bit is set so that we avoid deprecated functions. This is optional and won't really change anything since OpenGL isn't going to change. */
      None
};

GLXContext glx_context = glXCreateContextAttribsARB(display, best_fb_config, NULL, True, context_attribs);
if(glx_context == NULL) {
      printf("ERROR: failed to create glx context with glXCreateContextAttribsARB()\n");
      goto CLEANUP_AND_EXIT;
} else {
      glx_context_exists = TRUE;
}

if(glXIsDirect(display, glx_context) != True) {
      printf("ERROR: OpenGL Context was NOT created in direct mode (DRI). It is not supported to run a networked X server setup with the client and server not on the same device. Otherwise, some other error occurred.\n");
      goto CLEANUP_AND_EXIT;
}

if(glXMakeCurrent(display, glx_window, glx_context) != True) {
      printf("ERROR: failed to make context current\n");	    
      goto CLEANUP_AND_EXIT;
} else {
      glx_context_current = TRUE;
}
/* @! */
```

First, we fill in an array of attribs `context_attribs`, which mostly just sets the OpenGL profile major and minor version. Then we pass that, along with our fb_config and some other variables into `glXCreateContextAttribsARB()`. The `NULL` is passed in because we don't want to use any shared contexts (we only want one context for this application), and the `True` is passed in to indicate that we want a Direct Rendering context which is very important, we need that if we want any chance of having decent performance.

Next, we check to make sure using `glXIsDirect()` that our context is indeed direct, otherwise we exit with failure. Reasons for failing include that the display is not a local X server, or that there are other contexts on the system taking up the max amount of direct contexts supported.

Finally, we make the context current by a call to `glXMakeCurrent()` passing in our context, GLX window, and display. This will not work properly with a normal X window, you need to create and use a GLX window.

Lastly, we need to add this to our cleanup section:

```c
     /* @@ cleanup and program exit */
CLEANUP_AND_EXIT:
     if(glx_context_current == TRUE) {
           /* This function releases the current context when called with appropriate parameters, we have to first make the context not current or released/unbound to be able to destroy it properly */
	   glXMakeContextCurrent(display, None, None, NULL);
     }
     if(glx_context_exists == TRUE) {
	   glXDestroyContext(display, glx_context);
     }
     if(glx_window_exists == TRUE) {
	   glXDestroyWindow(display, glx_window);
     }
     if(window_exists == TRUE) {
	   XDestroyWindow(display, window);
     }
     if(display_exists == TRUE) {
	   XCloseDisplay(display);
     }

     return 0;
     /* @! */
```

We just added here to make the context uncurrent and then destroy it.

At this point we are technically done, but there are still a couple of other things.


### Loading OpenGL procedures
To use many of the OpenGL procedures, they need to be loaded at runtime. We load GL procedures just like GLX procedures, and there is also a similar extension header to include that is `<GL/glext.h>`, but we also need to include the headers for OpenGL itself which is `<GL/gl.h>`, and we also need to link to GL with `-lGL` in GCC.

Once that is done, we can load GL procedures with the following code. I haven't loaded every function, but these are a couple of functions that we'll be using for the rest of the tutorial along with some others:

```c
/* @@ loading OpenGL procedures (we need the "glext.h" header for the function typedefs and declarations, "glext.h" and "glxext.h" are NOT the same, they are spelled very similar but the former is for OpenGL and the latter is for GLX). */
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

glGetStringi = (PFNGLGETSTRINGIPROC)glXGetProcAddress((const GLubyte *)"glGetStringi");
glGenBuffers = (PFNGLGENBUFFERSPROC)glXGetProcAddress((const GLubyte *)"glGenBuffers");
glBindBuffer = (PFNGLBINDBUFFERPROC)glXGetProcAddress((const GLubyte *)"glBindBuffer");
glBufferData = (PFNGLBUFFERDATAPROC)glXGetProcAddress((const GLubyte *)"glBufferData");
glCreateShader = (PFNGLCREATESHADERPROC)glXGetProcAddress((const GLubyte *)"glCreateShader");
glShaderSource = (PFNGLSHADERSOURCEPROC)glXGetProcAddress((const GLubyte *)"glShaderSource");
glCompileShader = (PFNGLCOMPILESHADERPROC)glXGetProcAddress((const GLubyte *)"glCompileShader");
glCreateProgram = (PFNGLCREATEPROGRAMPROC)glXGetProcAddress((const GLubyte *)"glCreateProgram");
glAttachShader = (PFNGLATTACHSHADERPROC)glXGetProcAddress((const GLubyte *)"glAttachShader");
glLinkProgram = (PFNGLLINKPROGRAMPROC)glXGetProcAddress((const GLubyte *)"glLinkProgram");
glDeleteShader = (PFNGLDELETESHADERPROC)glXGetProcAddress((const GLubyte *)"glDeleteShader");
glUseProgram = (PFNGLUSEPROGRAMPROC)glXGetProcAddress((const GLubyte *)"glUseProgram");
glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)glXGetProcAddress((const GLubyte *)"glVertexAttribPointer");
glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)glXGetProcAddress((const GLubyte *)"glEnableVertexAttribArray");
glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)glXGetProcAddress((const GLubyte *)"glGenVertexArrays");
glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)glXGetProcAddress((const GLubyte *)"glBindVertexArray");
/* @! */
```

Note that some functions like `glClear()` do not need to be loaded because they are old functions, part of the original OpenGL version. It's only newer procedures (that were once purely extensions, and hence are loaded at runtime) that have to be loaded like this.

People keep complaining about how hard it is to load GL procedures, but it's literally as simple as I have shown you. There are a lot of other caveats, such as checking the GL version and extension to make sure that certain functions are available, but the core idea is fairly simple.

Considering that OpenGL will not be updated, and that we've had 4.6 for almost a decade now, it won't be that difficult to determine which extensions are generally present on most systems. Obviously vendor-specific extension procedures will need to be queried for in a different manner, but the core OpenGL version 4 functions are going to be pretty much available everywhere.


## Configuring Window/Framebuffer Settings
There are a couple of Window settings we can configure before we get to actual GL rendering. We can set the name of the window, ask X11 to receive certain events (keyboard input), and also turn on vsync:

```c
/* @@ configuring window parameters */
XStoreName(display, window, "X11 OpenGL Window");
XSelectInput(display, window, KeyPressMask); /* so we can get keyboard inputs in the X11 event loop */

/* even though we checked to see if the extension is present, we should still check again with glXQueryDrawable() to make sure it is there. This is a robust and defensive way of programming that you need to do. */
unsigned int query_val;
glXQueryDrawable(display, glx_window, GLX_LATE_SWAPS_TEAR_EXT, &query_val);
if(query_val == 1) { /* late swaps is supported, i.e. adaptive v-sync */
      glXSwapIntervalEXT(display, glx_window, -1); /* -1 is adaptive v-sync */	    
      glXQueryDrawable(display, glx_window, GLX_SWAP_INTERVAL_EXT, &query_val);
      if((int)query_val != -1) { /* even again here we check to make sure it was set properly */
	    printf("ERROR: failed to set swapinterval val correctly. \"-1\" was requested but we did not get it\n");
	    goto CLEANUP_AND_EXIT;
      }
} else {
      glXSwapIntervalEXT(display, glx_window, 1); /* 1 is normal v-sync */
      glXQueryDrawable(display, glx_window, GLX_SWAP_INTERVAL_EXT, &query_val);
      if(query_val != 1) {
	    printf("ERROR: failed to set swapinterval val correctly. \"1\" was requested but we did not get it\n");
	    goto CLEANUP_AND_EXIT;
      }
}

glViewport(0, 0, window_width, window_height);  
/* @! */
```

First we give our window a name with `XStoreName(display, window, "X11 OpenGL Window")`, and then we ask it to get keyboard input with `XSelectInput()` and the `KeyPressMask` flag.

Then using `glXQueryDrawable()`, we double check if what the extensions support is actually there, just to be safe. Depending on what seems to be supported, we either select for late swapping (adaptive vsync) or normal vsync. We set the swap interval with the GLX extension provided procedure `glXSwapIntervalEXT()`.

Lastly, we set the GL viewport with `glViewport()`.


## Basic OpenGL Hello Triangle
As an example, just to prove that this whole tutorial does indeed work - let us setup a basic hello triangle example with OpenGL. I will not write code for this correctly or explain how it works as that is out of the scope of this tutorial. See: [LearnOpenGL](https://learnopengl.com/) as an excellent resource for learning how to program OpenGL, of which this example is taken from.

You can just copy-paste this code just as a test to see if everything works:

```c
/* @@ setting up rendering of basic hello triangle */
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


XMapRaised(display, window);
/* @! */
```

The only thing I want to explain is that we make a call to `XMapRaised()` so that we can finally see the window. This still won't make the window visible, both because the program ends too quickly and also because we need to setup the event loop and actually have X commands flushed to the server.


### X Event Loop & Basic Rendering
The last thing to do is write a basic event loop with rendering the triangle we setup, responding to keyboard events, and swapping buffers.

```c
/* @@ main loop */
int running = 1;
while(running) {
      glClearColor(0.1f, 0.15f, 0.19f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);


      /* int num_events = XPending(display); /\* identical to XEventsQueued(display, QueuedAfterFlush) *\/ */


      XFlush(display); /* rather than calling XFlush() here, we could use the XPending() function instead of XEventsQueued() which will flush the X buffer if needed, I want to ensure that it flushes every frame consistently so that's why this is done */
      int num_events = XEventsQueued(display, QueuedAlready);

      XEvent event;
      while(num_events > 0) {
	    XNextEvent(display, &event);
	    if(event.type == KeyPress) {
		  running = 0;
	    }

	    num_events = num_events - 1;
      }


      /* @@ rendering */
      glUseProgram(shader_program);
      glBindVertexArray(vao);
      glDrawArrays(GL_TRIANGLES, 0, 3);
      /* @! */


      /* @@ the order of these last few procedures are critical */
      glXSwapBuffers(display, glx_window);
      glFinish(); /* blocks until all previous GL commands to finish, including the buffer swap. */
      /* @! */
}
/* @! */
```

In the loop, we first clear the framebuffer with `glClear()`. Then, we use `XFlush()` to flush the X buffer so that we can get access to the latest events, such as keyboard input or window events.

We then get the number of events with `XEventsQueued()`, it's important that we specify `QueuedAlready` instead of something like `QueuedAfterFlush`, because we are manually flushing ourselves with `XFlush()`, so we don't need to use `QueuedAfterFlush`.

Once we have the number of events in the event queue, we simply read off that number of events in a loop with the `XNextEvent()` function which returns the next event. Be careful when calling the `XNextEvent()` function, because it will block if there are no events in the queue and wait until another event comes. For some applications this behaviour is desirable, but for a realtime application such as a graphics engine, we definitely do not want that.

In our case, we just check if there is keyboard input and kill the program if there is by setting `running = 0`. Then, we do some basic OpenGL rendering using the setup before.

The last two things in the loop is to swap the buffers and then synchronize with `glFinish()`. `glXSwapBuffers()` will behave as according to how we controlled the swap interval with `glXSwapIntervalEXT()`. If we set the swap interval to `0`, then it would not try to synchronize to the refresh rate or vertical blank whatsoever.

Calling `glFinish()` here is not usually seen, but it is important to understand that the `glXSwapBuffers()` function does not block or wait for the buffer swap to be complete, all the function does is queue up a buffer swap and lets the driver decide when to actually make the swap (according to vsync rules primarily). If you just call this function without a call to `glFinish()`, then you might end up queuing subsequent frames early, before the previous frame is visible. Worst-case scenario, you would be possibly a whole frame behind because you're simulation/rendering runs much faster than the refresh rate.

Synchronization is out of scope for this post, but suffice to say, a call to `glFinish()` will block until the buffers are swapped. I do not claim to be an expert on this, but for more information, see [this](https://community.khronos.org/t/swapbuffers-and-synchronization/107667) blog post.

And that's it! I'll provide the full source code at the end of the page and the GCC options to compile.


## Addendums
There are a couple of extra points I want to clarify or expand upon that were hinted in the post.

### glXGetProcAddress vs glXGetProcAddressARB
So it may be confusing which of these functions to use. For one, the author(s) of the official Khronos API doesn't even seem to know what to use, as according to this [link](https://www.khronos.org/opengl/wiki/Load_OpenGL_Functions):

> The function glXGetProcAddress (or glXGetProcAddressARB; it is not entirely clear as to which should be used) retrieves function pointers on X-Windows-based systems.

So that's not much help, but I've done some digging myself on the matter.

The `glXGetProcAddressARB()` function was written against the GLX 1.3 specification, as evidenced by [this](https://registry.khronos.org/OpenGL/extensions/ARB/GLX_ARB_get_proc_address.txt) documentation for the `GLX_ARB_get_proc_address` extension. However, in the GLX 1.4 spec, the `glXGetProcAddressARB()` function was promoted to the core `glXGetProcAddress()` function. You can find this in the "GLX Versions" section of the GLX 1.4 spec which is at the very end. Therefore, if we use GLX 1.4 (which we are), then we can just use `glXGetProcAddress()`.

If you want to support pre GLX 1.4 or legacy OpenGL, consider then using `glXGetProcAddressARB()`.


### GLXWindow vs X Window
So a point I brought up was that we need to create a GLX Window, although some people disagree. [This](https://stackoverflow.com/questions/60795622/xlibs-window-vs-glxs-glxwindow-confusion) stackoverflow post is incorrect that the GLX window is "useless".

For one, this GLFW [github issue](https://github.com/glfw/glfw/issues/507) shows a request to use GLFW windows for GLX 1.4 support.

Secondly, the [documentation](https://registry.khronos.org/OpenGL/extensions/EXT/EXT_swap_control.txt) for the swap control extension mandates that we need to query a GLX window rather than an X window for swap interval properties, otherwise it will not work correctly. Right from these docs:

> The current swap interval and implementation-dependent max swap interval for a particular drawable can be obtained by calling glXQueryDrawable with the attributes GLX_SWAP_INTERVAL_EXT and GLX_MAX_SWAP_INTERVAL_EXT respectively. The value returned by glXQueryDrawable is undefined if the drawable is not a **GLXWindow** and these attributes are given.

And in my own testing, I found that using a normal X window instead of a GLX window cause errors just as this spec described. I would get incorrect values for the max swap interval, such as 0, when it returns a much higher number with a GLX window.

Thirdly, if you try to set the swap interval without a GLX window, not only will it crash, but trying to do basic GL rendering won't even work at all.

The bottom line is that you can't trust what you see on the internet. I understand that's rich coming from the author here, but I have also provided evidence for my claims.


## Source Code and GCC Options
As promised, the full source code is available [here](https://github.com/James822/x11-OpenGL-window).

GCC options are as follows:

```
gcc x11_window.c -lX11 -lGLX -lGL -o x11_window
```


## References
- Khronos OpenGL registry (GLX spec is available here): [https://registry.khronos.org/OpenGL/index_gl.php](https://registry.khronos.org/OpenGL/index_gl.php)
- Xorg/Xlib Documentation: [https://www.x.org/wiki/ProgrammingDocumentation/](https://www.x.org/wiki/ProgrammingDocumentation/)
- Khronos Opengl Context Creation Tutorial: [https://www.khronos.org/opengl/wiki/Tutorial:_OpenGL_3.0_Context_Creation_(GLX)](https://www.khronos.org/opengl/wiki/Tutorial:_OpenGL_3.0_Context_Creation_(GLX))
- Tutorial by [ColleagueRiley](https://github.com/ColleagueRiley) on Window/GL context creation: [https://github.com/ColleagueRiley/OpenGL-Context-Creation](https://github.com/ColleagueRiley/OpenGL-Context-Creation)
- Tutorial by [Apoorva Joshi](https://apoorvaj.io/), also on Window/GL context creation: [https://apoorvaj.io/creating-a-modern-opengl-context/](https://apoorvaj.io/creating-a-modern-opengl-context/)
- Khronos tutorial on loading OpenGL functions: [https://www.khronos.org/opengl/wiki/Load_OpenGL_Functions](https://www.khronos.org/opengl/wiki/Load_OpenGL_Functions)
- The amazing and classic learnOpenGL tutorial: [https://learnopengl.com/](https://learnopengl.com/)
- Excellent tutorial that mirrors my own: [https://www.tspi.at/2021/06/13/openglcontextx11.html](https://www.tspi.at/2021/06/13/openglcontextx11.html)
  
