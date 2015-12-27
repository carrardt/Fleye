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
		const uint32_t* base_ptr = (const uint32_t*) render_buffer->readBack();	
	}
};

FLEYE_REGISTER_PLUGIN(readtest);
