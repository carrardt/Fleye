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
	inline motionVector() : render_buffer(0), obj(0) {}
	
	void setup(FleyeContext* ctx)
	{
		render_buffer = ctx->ip->getRenderBuffer("mv-render-buffer");
		TrackingService* track_svc = TrackingService_instance();
		obj = track_svc->addTrackedObject(0);
		/*obj00 = track_svc->addTrackedObject(1);
		obj01 = track_svc->addTrackedObject(2);
		obj10 = track_svc->addTrackedObject(3);
			obj11 = track_svc->addTrackedObject(4);*/
		std::cout<<"motionVector setup : render_buffer="<<render_buffer<< "\n";
	}

	void run(FleyeContext* ctx, int threadId /*, frameCount */)
	{		
		// in case alternate render buffers are used, switch between the two
		const uint32_t* base_ptr = 0;

		/*DECLARE_MINMAX_STAT(m0);
		DECLARE_MINMAX_STAT(m1);
		DECLARE_MINMAX_STAT(m2);
		DECLARE_MINMAX_STAT(m3);*/
		double sumX = 0;
		double sumY = 0;
		//float sumL = 0;

		int width = render_buffer->width();
		int height = render_buffer->height();
		//std::cout<<width<<'x'<<height<<"\n";
		render_buffer->copyToBuffer(0,0,width,height);
		base_ptr = (const uint32_t*) render_buffer->getCopyBuffer();
		//std::cout<<"ptr="<<base_ptr<< "\n"; std::cout.flush();

		for(uint32_t y=0;y<height;y++)
		{
			const uint32_t* p = base_ptr + y*width;
			for(uint32_t x=0;x<width;x++)
			{
				uint32_t value = p[x];
				uint32_t mx = ( value ) & 0x000000FF;
				uint32_t my = ( value >> 8 ) & 0x000000FF;
				uint32_t ml = ( value >> 16 ) & 0x000000FF;
				
				double Sx = 0.00390625*mx;
				double Sy = 0.00390625*my;
				double L = 0.00390625*ml;
				sumX += (Sx-0.5)*L;
				sumY += (Sy-0.5)*L;
				//sumL += L; 
				
				/*UPDATE_MINMAX_STAT(m0);
				UPDATE_MINMAX_STAT(m1);
				UPDATE_MINMAX_STAT(m2);*/
				//int Yi = height - (y*2) - 1;
			}
		}

		//std::cout<<"ok\n"; std::cout.flush();
		
		double L = sqrt( sumX*sumX + sumY*sumY );
		if(L>1.0)
		{
			sumX /= L;
			sumY /= L;
		}
		std::cout <<sumX<<","<<sumY<<" l="<<L<<"\n";
		
		obj->posX = 0.5 ;
		obj->posY = 0.5 ;
		obj->speedX = sumX/4.0 ;
		obj->speedY = sumY/4.0 ;
		obj->area = 1;
		obj->weight = 1;
		obj->timestamp = ctx->frameCounter;
	}
	
	FleyeRenderWindow* render_buffer;
	TrackedObject* obj;
};

FLEYE_REGISTER_PLUGIN(motionVector);
