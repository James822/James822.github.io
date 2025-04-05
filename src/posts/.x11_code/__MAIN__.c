/*
This is the platform-independent file that applications use as their new main entrypoint. You basically "pretend" that __MAIN__() is the entrypoint of the application, when in reality the platform code defines the real main() function and initializes things for you in there, and then calls __MAIN__() shortly after.

The only requirement by the application is to define the __MAIN__() function, this file contains a basic starter template that you can copy over into a new project.


@TODO:
- consider if the GLX event: "GLX_PbufferClobber" is important to check for.
- handle X11 errors, have them logged or something
- do more advanced GLXFBConfig sorting and selecting
- according to the GL spec, GL types are precisely defined with width, so maybe I can use my types instead of GL types, we'll see. Honestly, it might be better to just suck it up and use GL types, idk.
- use glEnable(DEBUG_OUPUT) and also enable debug context at context creation and use it as a debug or developer build option. Also, use glDisable(DEBUG_OUPUT) to ensure in a release build that it doesn't happen. According to the GL spec, you can still do glEnable(DEBUG_OUPUT) on a non-debug context, although debug messages may not be generated at all. Still, I htink it would be good to do glEnable(DEBUG_OUPUT) even if a debug context is not supported. Similarly, even if the context is not a debug context, it's good to call glDisable(DEBUG_OUPUT) if we want to be sure there is no debug output on a release build for example.
- along with debug output and debug contexts, it also makes sense to turn on or off error mode along with debug output. For this we need to use the GLX_ARB_create_context_no_error extension, and then we can use the GLX_CONTEXT_OPENGL_NO_ERROR_ARB flag bit in the attribs passed to the glXCreateContextAttribsARB() function.
- make it an option to select the openGL major and minor version, but core is not optional, you can only use modern core profile openGL here. It's useful to select the core major and minor version because 4.6 might still not be supported on some hardware.
- make sure that XFlush() isn't necessary to flush the output buffer to draw the framebuffer, basically just removing and flushing (XPendingEvents()) and see if things still draw double buffered as normal.
- we could probably create the glx_context after window creation (and glx window creation), so that we can create it and make it current in one fell swoop, which is more what people expect.

- consider the list of GLX extensions:
   - "GLX_ARB_create_context_no_error": apparently this allows us to great a GL context with no errors, which might be very useful for performance.
   - "GLX_ARB_framebuffer_sRGB": basically just helps us do gamma encoding, we should test this against manually doing it
   - "GLX_ARB_multisample": basically for MSAA
   - "GLX_ARB_create_context_robustness": helps with accessing out of bounds stuff for security or whatnot?
   - "GLX_OML_sync_control": timer for precise synching
   - "GLX_SGI_video_sync": also related to synching, but maybe not needed
 */

#define TRUE 1
#define FALSE 0

int search_str_in_str(const char* string, const char* search_string)
{
      for(int i = 0; string[i] != '\0'; ++i) {
	    if(string[i] == search_string[0]) {
		  // now we can start to see if the rest is search_string
		  int ans = TRUE;
		  for(int j = 0; search_string[j] != '\0'; ++j) {
			if(string[i + j] != search_string[j]) {
			      ans = FALSE;
			      break;
			}
		  }
		  if(ans == TRUE) {
			return TRUE;
		  }
	    }
      }
      return FALSE;
}


int __MAIN__()
{
      /* KPL_Print_Platform_Info(); */


      /* @@ KYN lib variables */
      Display* display = NULL;
      Window window = 0;
      GLXWindow glx_window = 0;
      GLXContext glx_context = NULL;

      u32 display_exists = FALSE;
      u32 window_exists = FALSE;
      u32 glx_window_exists = FALSE;
      u32 glx_context_exists = FALSE;
      u32 glx_context_current = FALSE;
      
      Window parent_window = 0;
      int default_screen_id = 0;
      GLXFBConfig best_fb_config;
      /* @! */


      
      /* @@ user variables */
      GLint window_width = 960;
      GLint window_height = 540;
      /* @! */


      
      /* @@ opening connection to X server */      
      display = XOpenDisplay(NULL);
      if(display == NULL) {
	    KPL_Log("ERROR: failed to open X11 display, display name: \"");
	    KPL_Log(XDisplayName(NULL));
	    KPL_Log("\"\n");
	    goto CLEANUP_AND_ERROR_EXIT;
      } else {
	    display_exists = TRUE;
	    default_screen_id = XDefaultScreen(display);
      }
      /* @! */

      
      
      /* @@ glx initial setup */
      int error_base;
      int event_base;
      if(glXQueryExtension(display, &error_base, &event_base) != True) {
	    KPL_Log("ERROR: GLX extension not supported for this X Server\n");
	    goto CLEANUP_AND_ERROR_EXIT;
      }

      int glx_major_version;
      int glx_minor_version;
      if(glXQueryVersion(display, &glx_major_version, &glx_minor_version) != True) {
	    KPL_Log("ERROR: glXQueryVersion() failed\n");
	    goto CLEANUP_AND_ERROR_EXIT;
      }
      if( glx_major_version < 1 || (glx_major_version == 1 && glx_minor_version < 4) ) {
	    /* The reason why we require GLX 1.4 (or higher, although it's doubtful a new version will be released) is because the GLX_ARB_create_context/GLX_ARB_create_context_profile extensions (which we need) requires GLX 1.4, and we need this extension in order to create a modern OpenGL context, which is what we are doing. "Modern" is somewhat of an outdated term as GLX 1.4 was released in 2005/2006, so there's nothing really unportable as we require the user to have hardware that can at least support this "modern" OpenGL. */
	    KPL_Log("ERROR: GLX version is less than 1.4\n");
	    goto CLEANUP_AND_ERROR_EXIT;
      }
      /* @! */

      
      
      /* @@ Checking for GLX extensions */      
      const char* glx_extensions_string = glXQueryExtensionsString(display, default_screen_id);      

      /* @NOTE: GLX_ARB_create_context_profile is absolutely neccessary in order to create a GL context, we cannot proceed without it. Technically, we need only check if GLX_ARB_create_context is supported rather than GLX_ARB_create_context_profile, because the former implies the latter is supported, IF the implementation supports OpenGL 3.2 or later, but we are definitely using a version higher than OpenGL 3.2, version 4 actually. Still, just in case, we explicity check if GLX_ARB_create_context_profile is supported to be as robust as possible. See this documentation for more details: https://registry.khronos.org/OpenGL/extensions/ARB/GLX_ARB_create_context.txt, specifically check the "Dependencies on OpenGL 3.2 and later OpenGL versions" section. */
      if(search_str_in_str(glx_extensions_string, "GLX_ARB_create_context_profile") != TRUE) {
	    KPL_Log("ERROR: \"GLX_ARB_create_context\" extension not supported, cannot create GL context!");
	    goto CLEANUP_AND_ERROR_EXIT;
      }
      /* EXT_swap_control give us the ability to synchronize the swap (v-sync) by an integer amount of frames greater or equal to 0. */
      if(search_str_in_str(glx_extensions_string, "GLX_EXT_swap_control") != TRUE) {
	    KPL_Log("ERROR: \"GLX_EXT_swap_control\" extension not supported!");
	    goto CLEANUP_AND_ERROR_EXIT;
      }
      /* This allows us to use adaptive v-sync for when we miss a swap by specifying negative integer intervals. Adaptive vsync is a generally a good compromise with double-buffering. This extension depends upon the GLX_EXT_swap_control extension to be available/supported. @TODO: we should consider if this extension isn't present that our application still supports that. Currently we require it here.*/
      if(search_str_in_str(glx_extensions_string, "GLX_EXT_swap_control_tear") != TRUE) {
	    KPL_Log("ERROR: \"GLX_EXT_swap_control_tear\" extension not supported!");
	    goto CLEANUP_AND_ERROR_EXIT;
      }
      /* @! */

      

      /* @@ loading GLX extension procedures (we need the "glxext.h" header for the function typedefs and declarations). */
      PFNGLXCREATECONTEXTATTRIBSARBPROC glXCreateContextAttribsARB = NULL;
      PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT = NULL;
      
      glXCreateContextAttribsARB = (PFNGLXCREATECONTEXTATTRIBSARBPROC)glXGetProcAddress((const GLubyte *)"glXCreateContextAttribsARB");      
      glXSwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)glXGetProcAddress((const GLubyte *)"glXSwapIntervalEXT");

      if(glXCreateContextAttribsARB == NULL) {
	    KPL_Log("ERROR: glXCreateContextAttribsARB() function pointer NULL, cannot create GL context!\n");
	    goto CLEANUP_AND_ERROR_EXIT;
      }
      if(glXSwapIntervalEXT == NULL) {
	    KPL_Log("ERROR: \"glXSwapIntervalEXT\" function pointer NULL\n");
	    goto CLEANUP_AND_ERROR_EXIT;
      }
      /* @! */


      
      /* @@ selecting a GLX FB Config */
      /* This fbc attrib array is what specifies the kind of framebuffer we will get, so it's important to specify everything we need and not leave anything to defaults (unless we don't care what it is). It's possible that we specify a configuration that isn't supported, which means either we need to choose another one that is supported, or we deem the platform to be unsupported by our application. */
      int fbc_attribs[] = {
	    GLX_LEVEL, 0, /* framebuffer level, which we set to 0 (default) */
	    GLX_DOUBLEBUFFER, True,
	    /* GLX_SAMPLE_BUFFERS, 1, /\* setting this to "1" gives us a mutlisample buffer, the only other option is 0, which is no multisample buffer *\/ */
	    /* GLX_SAMPLES, 4, /\* this is the number of MSAA samples, and this setting only makes sense if we have GLX_SAMPLE_BUFFERS set to 1, otherwise it should be 0 *\/ */
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
	    GLX_CONFIG_CAVEAT, GLX_NONE, /* GLX_NON_CONFORMANT_CONFIG doesn't even give me any fbs on my machine, GLX_SLOW_CONFIG is clearly undesriable as it is slow, so that leaves GLX_NONE as the option. @TODO: it might be better in the future to just make sure that this is NOT GLX_SLOW_CONFIG, and leave the possibility open for it to be either GLX_NON_CONFORMANT_CONFIG or GLX_NONE. But non-conformace may be bad or good, I don't know. */
	    GLX_TRANSPARENT_TYPE, GLX_NONE, /* setting it to false just in case, but we might want transparencey later */
	    None /* we must put this "None" value here as this array is passed in with no size parameter */
      };
      
      int num_fb_configs = 0;
      GLXFBConfig *fb_configs = glXChooseFBConfig(display, default_screen_id, fbc_attribs, &num_fb_configs);
      if(fb_configs == NULL || num_fb_configs == 0) {
	    KPL_Log("ERROR: glXChooseFBConfig() failed, or it returned no matching fb configs\n");
	    goto CLEANUP_AND_ERROR_EXIT;
      }
      best_fb_config = fb_configs[0]; /* @TODO: come up with better selection process than just picking the first one out of the array */      
      XFree(fb_configs);
      /* @! */


      
      /* @@ creating X11 Window and GLX window */
      parent_window = XDefaultRootWindow(display);
      
      XVisualInfo* visual_info = glXGetVisualFromFBConfig(display, best_fb_config); /* this GLX function gives us the structure we need to pass into X11 window create function so that our GLX fb config is consistent with the X11 window config. */
      if(visual_info == NULL) {
	    KPL_Log("ERROR: glXGetVisualFromFBConfig() failed to get a XVisualInfo struct\n");
	    goto CLEANUP_AND_ERROR_EXIT;
      }

      XSetWindowAttributes window_attributes;
      window_attributes.background_pixmap = None;
      window_attributes.background_pixel = XWhitePixel(display, default_screen_id);
      window_attributes.override_redirect = True; /* @TODO: this helps with fullscreen? */

      Colormap cmap = XCreateColormap(display, parent_window, visual_info->visual, AllocNone);
      window_attributes.colormap = cmap;

      /* @TODO: We need to create the X Window with the same attributes as the framebuffer configuration (GLXFBConfig) that we chose to create the GLX context. We accomplish this by simply passing in the same fb config (as converted to a visual info with glXGetVisualfromFBConfig()) as before, but it's possible that the XCreateWindow() function will fail to match the visual info. */
      window = XCreateWindow(display, parent_window, 0, 0, (unsigned int)window_width, (unsigned int)window_height, 0, visual_info->depth, InputOutput, visual_info->visual, CWBackPixmap | CWBackPixel | CWColormap, &window_attributes);
      /* @TODO: use the error handler to check errors of window creation */
      window_exists = TRUE;
      XFree(visual_info);

      
      /* now we create the GLX window which is basically just a handle to our X11 window, but it does accomplish very important things so we absolutely need it */
      /* For subtle reasons that most people overlook, it is better to use a GLXWindow as a handle to the X11 window rather than using the X window directly. That being said, most graphics applications get away without creating a GLXWindow. I found probably what is a mostly overlooked practical reason to use GLX_Window pertaining to swap intervals of all things. */
      /* we need to use the same exact FB config as used to create the context AND to create the X11 window, so we just pass that in here as well. */
      glx_window = glXCreateWindow(display, best_fb_config, window, NULL);
      glx_window_exists = TRUE;

      /* from now we use glx_window instead of window for any GLX/GL functions. For normal X functions, we continue to use the normal X window. */
      /* @! */

      
      
      /* @@ Creating GLX (GL) context, and making context current. */      
      int context_attribs[] = {
	    /* @@ with GLX_CONTEXT_MAJOR_VERSION_ARB and GLX_CONTEXT_MAJOR_MINOR_VERSION, the context creation function may actually give us a higher version as long as it is backwards compatible, so we need to query the real OpenGL version with glGet */
	    GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
	    GLX_CONTEXT_MINOR_VERSION_ARB, 6,
	    /* @! */
	    
	    GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,	    
	    GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB, /* @TODO: Forward compatible bit is set so that we avoid deprecated functions. This may or may not be a good idea depending on if we need deprecated functionality, and considering OpenGL has no future, there's no real need to set this if it interferes in any way. @TODO: We should also support an option to turn on GLX_CONTEXT_DEBUG_BIT_ARB so that we can use OpenGL in debug mode for development. */
	    None
      };
      
      glx_context = glXCreateContextAttribsARB(display, best_fb_config, NULL, True, context_attribs);
      if(glx_context == NULL) {
	    KPL_Log("ERROR: failed to create glx context with glXCreateContextAttribsARB()\n");
	    goto CLEANUP_AND_ERROR_EXIT;
      } else {
	    glx_context_exists = TRUE;
      }
      
      if(glXIsDirect(display, glx_context) != True) {
	    KPL_Log("ERROR: OpenGL Context was NOT created in direct mode (DRI). It is not supported to run a networked X server setup with the client and server not on the same device. Otherwise, some other error occured.\n");
	    goto CLEANUP_AND_ERROR_EXIT;
      }
      
      if(glXMakeCurrent(display, glx_window, glx_context) != True) {
	    KPL_Log("ERROR: failed to make context current\n");	    
	    goto CLEANUP_AND_ERROR_EXIT;
      } else {
	    glx_context_current = TRUE;
      }
      /* @! */


      
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

      if(glGetStringi == NULL) {
	    KPL_Log("ERROR: \"glGetStringi\" function pointer NULL\n");
	    goto CLEANUP_AND_ERROR_EXIT;
      }
      if(glGenBuffers == NULL) {
	    KPL_Log("ERROR: \"glGenBuffers\" function pointer NULL\n");
	    goto CLEANUP_AND_ERROR_EXIT;
      }
      if(glBindBuffer == NULL) {
	    KPL_Log("ERROR: \"glBindBuffer\" function pointer NULL\n");
	    goto CLEANUP_AND_ERROR_EXIT;
      }
      if(glBufferData == NULL) {
	    KPL_Log("ERROR: \"glBufferData\" function pointer NULL\n");
	    goto CLEANUP_AND_ERROR_EXIT;
      }
      if(glCreateShader == NULL) {
	    KPL_Log("ERROR: \"glCreateShader\" function pointer NULL\n");
	    goto CLEANUP_AND_ERROR_EXIT;
      }
      if(glShaderSource == NULL) {
	    KPL_Log("ERROR: \"glShaderSource\" function pointer NULL\n");
	    goto CLEANUP_AND_ERROR_EXIT;
      }
      if(glCompileShader == NULL) {
	    KPL_Log("ERROR: \"glCompileShader\" function pointer NULL\n");
	    goto CLEANUP_AND_ERROR_EXIT;
      }
      if(glCreateProgram == NULL) {
	    KPL_Log("ERROR: \"glCreateProgram\" function pointer NULL\n");
	    goto CLEANUP_AND_ERROR_EXIT;
      }
      if(glAttachShader == NULL) {
	    KPL_Log("ERROR: \"glAttachShader\" function pointer NULL\n");
	    goto CLEANUP_AND_ERROR_EXIT;
      }
      if(glLinkProgram == NULL) {
	    KPL_Log("ERROR: \"glLinkProgram\" function pointer NULL\n");
	    goto CLEANUP_AND_ERROR_EXIT;
      }
      if(glDeleteShader == NULL) {
	    KPL_Log("ERROR: \"glDeleteShader\" function pointer NULL\n");
	    goto CLEANUP_AND_ERROR_EXIT;
      }
      if(glUseProgram == NULL) {
	    KPL_Log("ERROR: \"glUseProgram\" function pointer NULL\n");
	    goto CLEANUP_AND_ERROR_EXIT;
      }
      if(glVertexAttribPointer == NULL) {
	    KPL_Log("ERROR: \"glVertexAttribPointer\" function pointer NULL\n");
	    goto CLEANUP_AND_ERROR_EXIT;
      }
      if(glEnableVertexAttribArray == NULL) {
	    KPL_Log("ERROR: \"glEnableVertexAttribArray\" function pointer NULL\n");
	    goto CLEANUP_AND_ERROR_EXIT;
      }

      
      PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = NULL;
      glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)glXGetProcAddress((const GLubyte *)"glGenVertexArrays");
      if(glGenVertexArrays == NULL) {
	    KPL_Log("ERROR: \"glGenVertexArrays\" function pointer NULL\n");
	    goto CLEANUP_AND_ERROR_EXIT;
      }
      PFNGLBINDVERTEXARRAYPROC glBindVertexArray = NULL;
      glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)glXGetProcAddress((const GLubyte *)"glBindVertexArray");
      if(glBindVertexArray == NULL) {
	    KPL_Log("ERROR: \"glBindVertexArray\" function pointer NULL\n");
	    goto CLEANUP_AND_ERROR_EXIT;
      }
      /* @! */


      
      /* past this point we get into user code */

      
      
      /* @@ configuring window parameters */     
      XStoreName(display, window, "Kyanite X11 Window");
      XSelectInput(display, window, KeyPressMask);

      unsigned int query_val;
      /* glXQueryDrawable(display, glx_window, GLX_MAX_SWAP_INTERVAL_EXT, &query_val); */
      /* printf("max swap interval %d\n", query_val); */
      glXQueryDrawable(display, glx_window, GLX_LATE_SWAPS_TEAR_EXT, &query_val);
      if(query_val == 1) { /* late swaps is supported, i.e. adaptive v-sync */
	    glXSwapIntervalEXT(display, glx_window, -1); /* -1 is adaptive v-sync */	    
	    glXQueryDrawable(display, glx_window, GLX_SWAP_INTERVAL_EXT, &query_val);
	    if((int)query_val != -1) {
		  KPL_Log("ERROR: failed to set swapinterval val correctly. \"-1\" was requested but we did not get it\n");
		  goto CLEANUP_AND_ERROR_EXIT;
	    }
      } else {
	    glXSwapIntervalEXT(display, glx_window, 1); /* 1 is normal v-sync */
	    glXQueryDrawable(display, glx_window, GLX_SWAP_INTERVAL_EXT, &query_val);
	    if(query_val != 1) {
		  KPL_Log("ERROR: failed to set swapinterval val correctly. \"1\" was requested but we did not get it\n");
		  goto CLEANUP_AND_ERROR_EXIT;
	    }
      }
      /* @! */

      
      
      /* @@ GL setup */
      
      /* GLint num_gl_extensions = 0; */
      /* glGetIntegerv(GL_NUM_EXTENSIONS, &num_gl_extensions); */
      /* for(GLint i = 0; i < num_gl_extensions; ++i) { */
      /* 	    KPL_Log((const char *)glGetStringi(GL_EXTENSIONS, (GLuint)i)); */
      /* 	    KPL_Log("\n"); */
      /* 	    /\* printf("%s\n", (const char *)glGetStringi(GL_EXTENSIONS, (GLuint)i)); *\/ */
      /* } */
      

      GLint max_viewport_dims[2];
      max_viewport_dims[0] = 0;
      max_viewport_dims[1] = 0;
      glGetIntegerv(GL_MAX_VIEWPORT_DIMS, max_viewport_dims);
      if( !(window_width <= max_viewport_dims[0]) ) {
	    KPL_Log("ERROR: window width is larger than the max viewport width GL can support\n");
	    goto CLEANUP_AND_ERROR_EXIT;
      }
      if( !(window_height <= max_viewport_dims[1]) ) {
	    KPL_Log("ERROR: window height is larger than the max viewport height GL can support\n");
	    goto CLEANUP_AND_ERROR_EXIT;
      }

      glViewport(0, 0, window_width, window_height);

      GLint viewport_info[4];
      viewport_info[0] = 0;
      viewport_info[1] = 0;
      viewport_info[2] = 0;
      viewport_info[3] = 0;
      glGetIntegerv(GL_VIEWPORT, viewport_info);
      if( (viewport_info[2] != window_width) || (viewport_info[3] != window_height) ) {
	    KPL_Log("ERROR: GL viewport dimensions not set properly\n");
	    goto CLEANUP_AND_ERROR_EXIT;
      }
      /* @! */      

      
      
      /* @@ logging some GLX information */
      KPL_Log("GLX info:\n");
      KPL_Log("=====================================\n");
      KPL_Log("X11 Client GLX vendor: ");
      KPL_Log(glXGetClientString(display, GLX_VENDOR));
      KPL_Log("\n");
      KPL_Log("X11 Client GLX version: ");
      KPL_Log(glXGetClientString(display, GLX_VERSION));
      KPL_Log("\n");
      
      KPL_Log("X11 Server GLX vendor: ");
      KPL_Log(glXQueryServerString(display, default_screen_id, GLX_VENDOR));
      KPL_Log("\n");
      KPL_Log("X11 Server GLX version: ");
      KPL_Log(glXQueryServerString(display, default_screen_id, GLX_VERSION));
      KPL_Log("\n");
      KPL_Log("=====================================\n\n");
      /* @! */


      
      /* @@ logging OpenGL info */
      KPL_Log("OpenGL Info:\n");
      KPL_Log("=====================================\n");
      KPL_Log("driver vendor: ");
      KPL_Log((const char *)glGetString(GL_VENDOR));
      KPL_Log("\n");
      KPL_Log("renderer: ");
      KPL_Log((const char *)glGetString(GL_RENDERER));
      KPL_Log("\n");
      KPL_Log("version: ");
      KPL_Log((const char *)glGetString(GL_VERSION));
      KPL_Log("\n");
      KPL_Log("GLSL version: ");
      KPL_Log((const char *)glGetString(GL_SHADING_LANGUAGE_VERSION));
      KPL_Log("\n");

      GLint gl_context_flags = 0;
      glGetIntegerv(GL_CONTEXT_FLAGS, &gl_context_flags);
      if(GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT & gl_context_flags) {
	    KPL_Log("GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT set\n");
      } else {
	    KPL_Log("GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT not set\n");
      }
      if(GL_CONTEXT_FLAG_DEBUG_BIT & gl_context_flags) {
	    KPL_Log("GL_CONTEXT_FLAG_DEBUG_BIT set\n");
      } else {
	    KPL_Log("GL_CONTEXT_FLAG_DEBUG_BIT not set\n");
      }
      if(GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT & gl_context_flags) {
	    KPL_Log("GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT set\n");
      } else {
	    KPL_Log("GL_CONTEXT_FLAG_ROBUST_ACCESS_BIT not set\n");
      }
      if(GL_CONTEXT_FLAG_NO_ERROR_BIT & gl_context_flags) {
	    KPL_Log("GL_CONTEXT_FLAG_NO_ERROR_BIT set\n");
      } else {
	    KPL_Log("GL_CONTEXT_FLAG_NO_ERROR_BIT not set\n");
      }
      KPL_Log("=====================================\n\n");
      /* @! */


      
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


      
      /* @@ finally after all our configuration and setup; we can map the X window, which basically makes it visible and functional. It's much better to do this as late as possible, so that the window is displayed right away with what we want, rather than displaying a window and then filling it with stuff which might cause a jarring flicker at program startup. */
      XMapRaised(display, window);
      /* XSync(display, False); /\* then we call XSync() after to make sure everything up to this point is synched *\/ */
      /* @! */

      

      /* @@ main loop */
      int running = 1;
      while(running) {
	    /* record_delta_time() /\* this must be the very first thing, or, equivalently; it can go at the very end of the loop *\/ */
	    
	    glClearColor(0.1f, 0.15f, 0.19f, 1.0f);
	    glClear(GL_COLOR_BUFFER_BIT);

	    
	    /* int num_events = XPending(display); /\* identical to XEventsQueued(display, QueuedAfterFlush) *\/ */
	    
	    XFlush(display);
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


      
      /* @@ program exit/finish/cleanup (possibly with errors) */
      int exit_code = 0;
 CLEANUP:
      KPL_Log("------ PROGRAM FINISH LOG ------\n");

      KPL_Flush_Log_Buffer(); /* flushing the log buffer first */

      if(glx_context_current == TRUE) {
	    /* This function releases the current context when called with appropriate parameters, we have to first make the context not current or released/unbound to be able to destroy it properly */
	    if(glXMakeContextCurrent(display, None, None, NULL) != True) {
		  KPL_Log("ERROR: failed to release current context\n");
	    } else {
		  KPL_Log("released current context\n");
	    }
      }
      if(glx_context_exists == TRUE) {
	    glXDestroyContext(display, glx_context); /* this function call only destroys the context if it is made uncurrent first (according to the GLX spec), hence the need to call glXMakeContextCurrent() with the right parameters to release the context first. */
	    KPL_Log("destroyed glx_context\n");
      }
      if(glx_window_exists == TRUE) {
	    glXDestroyWindow(display, glx_window);
	    KPL_Log("destroyed glx_window\n");
      }
      if(window_exists == TRUE) {
	    XDestroyWindow(display, window); KPL_Log("destroyed window\n");
      }
      if(display_exists == TRUE) {
	    XCloseDisplay(display); KPL_Log("destroyed display\n");
      }

      KPL_Log("--------------------------------\n\n");

      return exit_code;

 CLEANUP_AND_ERROR_EXIT:
      KPL_Log("\n\n\n!!!!!! - PROGRAM EXITED WITH ONE OR MORE ERROR(S) - !!!!!!\n");
      exit_code = -1;
      goto CLEANUP;
      /* @! */
}
