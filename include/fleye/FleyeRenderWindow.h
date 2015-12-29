#ifndef __fleye_rednerwindow_H_
#define __fleye_rednerwindow_H_

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>

struct FleyeNativeWindow;

struct FleyeRenderWindow
{    
	FleyeRenderWindow(int x, int y, int width, int height, FleyeRenderWindow* sharedCtxWin=NULL, bool offscreen=false);
	~FleyeRenderWindow();
	
	void copyToBuffer(int x,int y, int w,int h);
	inline uint8_t* getCopyBuffer() { return read_back_buffer; }
	
	inline uint8_t* readBack()
	{
		this->copyToBuffer(0,0,width(), height());
		return this->getCopyBuffer();
	}

	int width();
	int height();
    
   	FleyeNativeWindow* 	fleye_window;
    EGLDisplay 			display;
    EGLSurface 			surface;
    EGLContext 			context;

  private:

	void create_egl_context(const EGLint * attribs, FleyeRenderWindow* sharedCtxWin);
	void create_egl_pbuffer_context(const EGLint * attribs, const EGLint * pbAttribs, FleyeRenderWindow* sharedCtxWin);

    uint8_t * 			read_back_buffer;

};

#endif /* fleye_window */
