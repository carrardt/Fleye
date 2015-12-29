#include <EGL/egl.h>
#include <stdio.h>
#include <assert.h>

#include "fleye/FleyeRenderWindow.h"
#include "native_window.h"

FleyeRenderWindow::FleyeRenderWindow(int x,int y,int width, int height, FleyeRenderWindow* sharedCtxWin, bool offscreen)
{
	static const EGLint egl_config_attribs[] =
	{
	   EGL_RED_SIZE,   8,
	   EGL_GREEN_SIZE, 8,
	   EGL_BLUE_SIZE,  8,
	   EGL_ALPHA_SIZE, 8,
	   EGL_DEPTH_SIZE, 16,
	   EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
	   EGL_NONE
	};

	static const EGLint egl_fbo_config_attribs[] =
	{
	   EGL_RED_SIZE,   8,
	   EGL_GREEN_SIZE, 8,
	   EGL_BLUE_SIZE,  8,
	   EGL_ALPHA_SIZE, 8,
	   EGL_DEPTH_SIZE, 16,
	   EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
	   //EGL_BIND_TO_TEXTURE_RGBA, EGL_TRUE,
	   //EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
	   EGL_NONE
	};

	EGLint egl_fbo_pbuffer_attribs[] =
	{
	   EGL_WIDTH,   width,
	   EGL_HEIGHT,   height,
	   EGL_TEXTURE_FORMAT, EGL_TEXTURE_RGBA,
	   EGL_TEXTURE_TARGET, EGL_TEXTURE_2D,
	   //EGL_RENDER_BUFFER, EGL_SINGLE_BUFFER,
	   EGL_MIPMAP_TEXTURE,EGL_FALSE,
	   //EGL_SWAP_BEHAVIOR, EGL_BUFFER_DESTROYED,
	   EGL_NONE
	};


	this->read_back_buffer = 0;

	if(offscreen)
	{
		this->fleye_window = create_offscreen_native_window(0,0,width,height,255);
		assert(this->fleye_window!=NULL && "Failed to create native window");
		//this->create_egl_pbuffer_context(egl_fbo_config_attribs,egl_fbo_pbuffer_attribs,sharedCtxWin);
		this->create_egl_context(egl_fbo_config_attribs,sharedCtxWin);

		posix_memalign((void**) & this->read_back_buffer, 4096, width*height*4);
		memset(this->read_back_buffer,0,width*height*4);
	}
	else 
	{
		this->fleye_window = create_native_window(x,y,width,height,255);
		assert(this->fleye_window!=NULL && "Failed to create native window");
		this->create_egl_context(egl_config_attribs,sharedCtxWin);
	}
}

FleyeRenderWindow::~FleyeRenderWindow()
{
   /* Terminate EGL */
   eglMakeCurrent(this->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
   eglDestroyContext(this->display, this->context);
   eglDestroySurface(this->display, this->surface);
   //eglTerminate(this->display);

	destroy_native_window( this->fleye_window );
	this->fleye_window = NULL;
	
	if(read_back_buffer!=0) delete [] read_back_buffer;
}

int FleyeRenderWindow::width()
{
	return this->fleye_window->width;
}

int FleyeRenderWindow::height()
{
	return this->fleye_window->height;
}

void FleyeRenderWindow::copyToBuffer(int x,int y, int w,int h)
{
	read_offscreen_image_rect(this->fleye_window,x,y,w,h,this->read_back_buffer);
}

void FleyeRenderWindow::create_egl_context(const EGLint * attribs, FleyeRenderWindow* sharedCtxWin)
{
   const EGLint context_attribs[] =
   {
      EGL_CONTEXT_CLIENT_VERSION, 2,
      EGL_NONE
   };

   EGLConfig config;
   EGLint num_configs;

   // printf("%s\n", __PRETTY_FUNCTION__);

   if (this->fleye_window->native_window == NULL)
   {
      fprintf(stderr,"%s: No native window\n", __PRETTY_FUNCTION__);
	  fprintf(stderr,"%s: EGL error 0x%08x\n", __PRETTY_FUNCTION__, eglGetError());
      //fleyeutil_gl_term(fleye_state);
      abort();
   }

   this->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
   if (this->display == EGL_NO_DISPLAY)
   {
      fprintf(stderr,"%s: Failed to get EGL display\n", __PRETTY_FUNCTION__);
	  fprintf(stderr,"%s: EGL error 0x%08x\n", __PRETTY_FUNCTION__, eglGetError());
      //fleyeutil_gl_term(fleye_state);
      abort();
   }

   if (! eglInitialize(this->display, 0, 0))
   {
      fprintf(stderr,"%s: eglInitialize failed\n", __PRETTY_FUNCTION__);
	  fprintf(stderr,"%s: EGL error 0x%08x\n", __PRETTY_FUNCTION__, eglGetError());
      //fleyeutil_gl_term(fleye_state);
      abort();
   }

   if (! eglChooseConfig(this->display, attribs, &config,
            1, &num_configs))
   {
      fprintf(stderr,"%s: eglChooseConfig failed\n", __PRETTY_FUNCTION__);
	  fprintf(stderr,"%s: EGL error 0x%08x\n", __PRETTY_FUNCTION__, eglGetError());
      //fleyeutil_gl_term(fleye_state);
      abort();
   }

   this->surface = eglCreateWindowSurface(this->display,
         config, this->fleye_window->native_window, NULL);
   if (this->surface == EGL_NO_SURFACE)
   {
      fprintf(stderr,"%s: eglCreateWindowSurface failed\n", __PRETTY_FUNCTION__);
	  fprintf(stderr,"%s: EGL error 0x%08x\n", __PRETTY_FUNCTION__, eglGetError());
      //fleyeutil_gl_term(fleye_state);
      abort();
   }

   this->context = eglCreateContext(this->display, config, sharedCtxWin ? sharedCtxWin->context : EGL_NO_CONTEXT, context_attribs);
   if (this->context == EGL_NO_CONTEXT)
   {
      fprintf(stderr,"%s: eglCreateContext failed\n", __PRETTY_FUNCTION__);
	  fprintf(stderr,"%s: EGL error 0x%08x\n", __PRETTY_FUNCTION__, eglGetError());
      //fleyeutil_gl_term(fleye_state);
      abort();
   }

   if (!eglMakeCurrent(this->display, this->surface, this->surface, this->context))
   {
      fprintf(stderr,"%s: Failed to activate EGL context\n", __PRETTY_FUNCTION__);
	  fprintf(stderr,"%s: EGL error 0x%08x\n", __PRETTY_FUNCTION__, eglGetError());
      //fleyeutil_gl_term(fleye_state);
      abort();
   }

   eglSwapInterval(this->display,0); // no VSync
}

void FleyeRenderWindow::create_egl_pbuffer_context(const EGLint * attribs, const EGLint * pbAttribs, FleyeRenderWindow* sharedCtxWin)
{
   const EGLint context_attribs[] =
   {
      EGL_CONTEXT_CLIENT_VERSION, 2,
      EGL_NONE
   };

   EGLConfig config;
   EGLint num_configs;

   // printf("%s\n", __PRETTY_FUNCTION__);

   if (this->fleye_window->native_window == NULL)
   {
      fprintf(stderr,"%s: No native window\n", __PRETTY_FUNCTION__);
	  fprintf(stderr,"%s: EGL error 0x%08x\n", __PRETTY_FUNCTION__, eglGetError());
      //fleyeutil_gl_term(fleye_state);
      abort();
   }

   this->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
   if (this->display == EGL_NO_DISPLAY)
   {
      fprintf(stderr,"%s: Failed to get EGL display\n", __PRETTY_FUNCTION__);
	  fprintf(stderr,"%s: EGL error 0x%08x\n", __PRETTY_FUNCTION__, eglGetError());
      //fleyeutil_gl_term(fleye_state);
      abort();
   }

   if (! eglInitialize(this->display, 0, 0))
   {
      fprintf(stderr,"%s: eglInitialize failed\n", __PRETTY_FUNCTION__);
	  fprintf(stderr,"%s: EGL error 0x%08x\n", __PRETTY_FUNCTION__, eglGetError());
      //fleyeutil_gl_term(fleye_state);
      abort();
   }

   if (! eglChooseConfig(this->display, attribs, &config, 1, &num_configs))
   {
      fprintf(stderr,"%s: eglChooseConfig failed\n", __PRETTY_FUNCTION__);
	  fprintf(stderr,"%s: EGL error 0x%08x\n", __PRETTY_FUNCTION__, eglGetError());
      //fleyeutil_gl_term(fleye_state);
      abort();
   }

   this->surface = eglCreatePbufferSurface(this->display, config, pbAttribs);
   if (this->surface == EGL_NO_SURFACE)
   {
      fprintf(stderr,"%s: eglCreateWindowSurface failed\n", __PRETTY_FUNCTION__);
	  fprintf(stderr,"%s: EGL error 0x%08x\n", __PRETTY_FUNCTION__, eglGetError());
      //fleyeutil_gl_term(fleye_state);
      abort();
   }

   this->context = eglCreateContext(this->display, config, sharedCtxWin ? sharedCtxWin->context : EGL_NO_CONTEXT, context_attribs);
   if (this->context == EGL_NO_CONTEXT)
   {
      fprintf(stderr,"%s: eglCreateContext failed\n", __PRETTY_FUNCTION__);
	  fprintf(stderr,"%s: EGL error 0x%08x\n", __PRETTY_FUNCTION__, eglGetError());
      //fleyeutil_gl_term(fleye_state);
      abort();
   }

   if (!eglMakeCurrent(this->display, this->surface, this->surface, this->context))
   {
      fprintf(stderr,"%s: Failed to activate EGL context\n", __PRETTY_FUNCTION__);
	  fprintf(stderr,"%s: EGL error 0x%08x\n", __PRETTY_FUNCTION__, eglGetError());
      //fleyeutil_gl_term(fleye_state);
      abort();
   }

   eglSwapInterval(this->display,0); // no VSync
}
