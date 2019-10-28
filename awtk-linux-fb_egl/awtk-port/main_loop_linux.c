/**
 * File:   main_loop_linux.c
 * Author: AWTK Develop Team
 * Brief:  linux implemented main_loop interface
 *
 * Copyright (c) 2018 - 2018  Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 * this program is distributed in the hope that it will be useful,
 * but without any warranty; without even the implied warranty of
 * merchantability or fitness for a particular purpose.  see the
 * license file for more details.
 *
 */

/**
 * history:
 * ================================================================
 * 2018-09-09 li xianjing <xianjimli@hotmail.com> created
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <assert.h>
#include <math.h>
#include <signal.h>

#include "GLES2/gl2.h"
#include "GLES2/gl2ext.h"

#include "EGL/egl.h"
#include "fsl_egl/fsl_egl.h"
#include "native_window/native_window_fb_gl.h"


#include "base/idle.h"
#include "base/timer.h"
#include "base/font_manager.h"
#include "base/window_manager.h"
#include "main_loop/main_loop_simple.h"

#include "tslib_thread.h"
#include "input_thread.h"
#include "mouse_thread.h"
#include "main_loop_linux.h"

#define FB_DEVICE_FILENAME "/dev/fb0"
#define TS_DEVICE_FILENAME "/dev/input/event0"
#define KB_DEVICE_FILENAME "/dev/input/event1"
#define MICE_DEVICE_FILENAME "/dev/input/mice"



static EGLint               numconfigs;
static EGLDisplay           egldisplay;
static EGLConfig            eglconfig;
static EGLSurface           eglsurface;
static EGLContext           eglcontext;
static EGLNativeWindowType  eglNativeWindow;
static EGLNativeDisplayType eglNativeDisplayType;

void DeInit_GLES()
{
  eglMakeCurrent(egldisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
  assert(eglGetError() == EGL_SUCCESS);
  eglTerminate(egldisplay);
  assert(eglGetError() == EGL_SUCCESS);
  eglReleaseThread();
}

void Init_GLES()
{
  static const EGLint s_configAttribs[] =
  {
    EGL_SAMPLES,      0,
    EGL_RED_SIZE,     8,
    EGL_GREEN_SIZE,   8,
    EGL_BLUE_SIZE,    8,
    EGL_ALPHA_SIZE,   EGL_DONT_CARE,
    EGL_STENCIL_SIZE, 8,
    EGL_DEPTH_SIZE,   0,
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
    EGL_MIN_SWAP_INTERVAL, 1,
    EGL_NONE,
  };


  eglNativeDisplayType = fsl_getNativeDisplay();
  egldisplay = eglGetDisplay(eglNativeDisplayType);
  eglInitialize(egldisplay, NULL, NULL);
  assert(eglGetError() == EGL_SUCCESS);
  eglBindAPI(EGL_OPENGL_ES_API);

  eglChooseConfig(egldisplay, s_configAttribs, &eglconfig, 1, &numconfigs);
  assert(eglGetError() == EGL_SUCCESS);
  assert(numconfigs == 1);

  eglNativeWindow = fsl_createwindow(egldisplay, eglNativeDisplayType);	
  assert(eglNativeWindow);	

  eglsurface = eglCreateWindowSurface(egldisplay, eglconfig, eglNativeWindow, NULL);

  assert(eglGetError() == EGL_SUCCESS);
  EGLint ContextAttribList[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
  eglcontext = eglCreateContext( egldisplay, eglconfig, EGL_NO_CONTEXT, ContextAttribList );
  assert(eglGetError() == EGL_SUCCESS);
  eglMakeCurrent(egldisplay, eglsurface, eglsurface, eglcontext);
  assert(eglGetError() == EGL_SUCCESS);

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_SCISSOR_TEST);

  eglSwapInterval(egldisplay, 1);
  assert(eglGetError() == EGL_SUCCESS);

}



static ret_t main_loop_linux_destroy(main_loop_t* l) {
  main_loop_simple_t* loop = (main_loop_simple_t*)l;

  main_loop_simple_reset(loop);
  native_window_fb_gl_deinit();

  return RET_OK;
}

ret_t input_dispatch_to_main_loop(void* ctx, const event_queue_req_t* e, const char* msg) {
  main_loop_queue_event((main_loop_t*)ctx, e);
  input_dispatch_print(ctx, e, msg);

  return RET_OK;
}

static tk_thread_t* s_kb_thread = NULL;
static tk_thread_t* s_mice_thread = NULL;
static tk_thread_t* s_ts_thread = NULL;

static void on_app_exit(void) {
  tk_thread_destroy(s_kb_thread);
  tk_thread_destroy(s_mice_thread);
  tk_thread_destroy(s_ts_thread);
  DeInit_GLES();
}

static ret_t gles_swap_buffer(native_window_t* win) {
  eglSwapBuffers(egldisplay, eglsurface);
  return RET_OK;
}

static ret_t gles_make_current(native_window_t* win) {

  EGLint width = 0;
  EGLint height = 0;
  eglQuerySurface(egldisplay, eglsurface, EGL_WIDTH, &width);
  eglQuerySurface(egldisplay, eglsurface, EGL_HEIGHT, &height);

  eglMakeCurrent(egldisplay, eglsurface, eglsurface, eglcontext);
  assert(eglGetError() == EGL_SUCCESS);

  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  glViewport(0, 0, width, height);
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f );

  return RET_OK;
}


main_loop_t* main_loop_init(int w, int h) {
  EGLint width = 0;
  EGLint height = 0;
  main_loop_simple_t* loop = NULL;

  Init_GLES();
  eglQuerySurface(egldisplay, eglsurface, EGL_WIDTH, &width);
  eglQuerySurface(egldisplay, eglsurface, EGL_HEIGHT, &height);

  native_window_t* win = native_window_fb_gl_init(width, height, 1.0f);

  native_window_fb_gl_set_swap_buffer_func(win, gles_swap_buffer);
  native_window_fb_gl_set_make_current_func(win, gles_make_current);

  loop = main_loop_simple_init(width, height);
  loop->base.destroy = main_loop_linux_destroy;

#ifdef HAS_TSLIB
  s_ts_thread =
      tslib_thread_run(TS_DEVICE_FILENAME, input_dispatch_to_main_loop, loop, width, height);
#endif /*HAS_TSLIB*/

  s_kb_thread =
      input_thread_run(KB_DEVICE_FILENAME, input_dispatch_to_main_loop, loop, width, height);
  s_mice_thread =
      mouse_thread_run(MICE_DEVICE_FILENAME, input_dispatch_to_main_loop, loop, width, height);

  atexit(on_app_exit);

  return (main_loop_t*)loop;
}
