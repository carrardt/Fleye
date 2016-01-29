#include "fleye/cpuworker.h"
#include "fleye/plugin.h"
#include "fleye/FleyeRenderWindow.h"
#include "fleye/fbo.h"
#include "fleye/compiledshader.h"
#include "fleye/imageprocessing.h"
#include "fleye/FleyeContext.h"

#include "services/TrackingService.h"

#include <iostream> 
#include <math.h>


#define DECLARE_MINMAX_STAT(x) int32_t x##Min=256, x##Max=0

#define UPDATE_MINMAX_STAT(v) \
	if(v<v##Min) { \
		v##Min=v; \
	} if(v>v##Max) { \
		v##Max=v; \
	}
	

#define PRINT_MINMAX_STAT(S,x) \
	S<<' '<<#x<<"=["<<x##Min<<';'<<x##Max<<']';


struct motionVector : public FleyePlugin
{
	inline motionVector() : render_buffer(0)
	{
		for(int i=0;i<4;i++)
		{
			corner[i] = 0;
		}
	}

	void setup(FleyeContext* ctx)
	{
		render_buffer = ctx->ip->getRenderBuffer("mv-render-buffer");
		TrackingService* track_svc = TrackingService_instance();

		for(int i=0;i<4;i++)
		{
			corner[i] = track_svc->getTrackedObject(i);
			corner[i]->posX = 0.25 + (i%2)*0.5;
			corner[i]->posY = 0.25 + (i/2)*0.5;
			corner[i]->area = 1;
			corner[i]->weight = 1;
		}

		std::cout<<"motionVector setup : render_buffer="<<render_buffer<< "\n";
	}

	void run(FleyeContext* ctx, int threadId /*, frameCount */)
	{
		const uint32_t* base_ptr = 0;
		int width = render_buffer->width();
		int height = render_buffer->height();
		render_buffer->copyToBuffer(0,0,width,height);
		base_ptr = (const uint32_t*) render_buffer->getCopyBuffer();
		for(uint32_t y=0;y<height;y++)
		{
			const uint32_t* p = base_ptr + y*width;
			int cy = y<(height/2) ? 0 : 1;
			for(uint32_t x=0;x<width;x++)
			{
				int cx = x<(width/2) ? 0 : 1;
				int c = cy*2+cx;
				uint32_t value = p[x];
				uint32_t mx = ( value ) & 0x000000FF;
				uint32_t my = ( value >> 8 ) & 0x000000FF;
				uint32_t ml = ( value >> 16 ) & 0x000000FF;
				
				double Sx = 0.00390625*mx;
				double Sy = 0.00390625*my;
				double L = 0.00390625*ml;
				corner[c]->speedX += (Sx-0.5)*L;
				corner[c]->speedY += (Sy-0.5)*L;
			}
		}
		for(int i=0;i<4;i++)
		{
			float sx= corner[i]->speedX;
			float sy= corner[i]->speedY;
			double L = sqrt( sx*sx + sy*sy );
			//L=4096.0;
			//if(L>1.0)
			{
				corner[i]->speedX /= (width*height)/4; //(4.0*L);
				corner[i]->speedY /= (width*height)/4; //(4.0*L);
			}
		}
	}
	
	FleyeRenderWindow* render_buffer;
	TrackedObject* corner[4];
};

FLEYE_REGISTER_PLUGIN(motionVector);
