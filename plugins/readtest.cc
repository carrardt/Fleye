#include "fleye/cpuworker.h"
#include "fleye/plugin.h"
#include "fleye/FleyeRenderWindow.h"
#include "fleye/fbo.h"
#include "fleye/compiledshader.h"
#include "fleye/imageprocessing.h"
#include "fleye/FleyeContext.h"

#include <iostream>

static FleyeRenderWindow* render_buffer = 0;

struct readtest : public FleyePlugin
{
	
	void setup(FleyeContext* ctx)
	{
		render_buffer = ctx->ip->getRenderBuffer("readtest");
		std::cout<<"readtest setup : render_buffer @"<<render_buffer<<"\n";
	}

	void run(FleyeContext* ctx)
	{
		int w = render_buffer->width();
		int h = render_buffer->height();
		uint8_t* ptr = render_buffer->getCopyBuffer();
		//const uint32_t* base_ptr = (const uint32_t*) render_buffer->readBack();
		render_buffer->copyToBuffer(0,0,w,h);
		//glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,ptr);
	}
};

FLEYE_REGISTER_PLUGIN(readtest);
