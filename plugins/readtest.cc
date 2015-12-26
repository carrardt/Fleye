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
		int width=0, height=0;
		const uint32_t* base_ptr = (const uint32_t*) render_buffer->readBack(width,height);	
		if( ctx->frameCounter%1024 == 0 )
			std::cout<<"base_ptr="<<base_ptr<<", w="<<width<<", h="<<height<<"\n";
	}
};

FLEYE_REGISTER_PLUGIN(readtest);
