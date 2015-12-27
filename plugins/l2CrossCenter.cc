#include "fleye/cpuworker.h"
#include "fleye/plugin.h"
#include "fleye/FleyeRenderWindow.h"
#include "fleye/fbo.h"
#include "fleye/compiledshader.h"
#include "fleye/imageprocessing.h"
#include "fleye/FleyeContext.h"

#include "services/TrackingService.h"

#include <iostream> 

struct l2CrossCenter : public FleyePlugin
{
	inline l2CrossCenter() : render_buffer(0), obj1(0), obj2(0) {}
	
	void setup(FleyeContext* ctx)
	{
		render_buffer = ctx->ip->getRenderBuffer("l2c-render-buffer");
		TrackingService* track_svc = TrackingService_instance();
		obj1 = track_svc->addTrackedObject(0);
		obj2 = track_svc->addTrackedObject(1);
		std::cout<<"L2CrossCenter setup : render_buffer @"<<render_buffer<<", obj1@"<<obj1<<", obj2@"<<obj2<< "\n";
	}

	void run(FleyeContext* ctx)
	{
		int width=0, height=0;
		const uint32_t* base_ptr = (const uint32_t*) render_buffer->readBack(width,height);
		uint32_t obj1_sumx=0,obj1_sumy=0;
		uint32_t obj2_sumx=0,obj2_sumy=0;
		uint32_t obj1_count=0, obj2_count=0;
		int obj1_L2max=1, obj2_L2max=1;
		
		for(uint32_t y=0;y<height;y++)
		{
			const uint32_t* p = base_ptr + y*width;
			for(uint32_t x=0;x<width;x++)
			{
				uint32_t value = p[x];
				if( value >= 0xFF010000 )
				{
					int r = ( value >> 3) & 0x0000001F;
					int u = ( value >> 11) & 0x0000001F;
					int m = (r>u) ? r : u;	
					if( value < 0xFF800000 )
					{
						if( m > obj1_L2max )
						{ 
							obj1_count = 0;
							obj1_sumx = 0;
							obj1_sumy = 0;
							obj1_L2max = m;
						}
						if( m == obj1_L2max )
						{
							obj1_sumx += x;
							obj1_sumy += y;
							++ obj1_count;
						}
					}
					else
					{
						if( m > obj2_L2max )
						{ 
							obj2_count = 0;
							obj2_sumx = 0;
							obj2_sumy = 0; 
							obj2_L2max = m;
						}
						if( m == obj2_L2max )
						{
							obj2_sumx += x;
							obj2_sumy += y;
							++ obj2_count;
						}
					}
				}
			}
		}
			
		if(obj1_count>0)
		{
			float wx = (float)obj1_sumx / (float)obj1_count;
			float wy = (float)obj1_sumy / (float)obj1_count;
			obj1->posX = wx / (float)width;
			obj1->posY = 1.0f - wy / (float)height;
			obj1->timestamp = ctx->frameCounter;
		}

		if(obj2_count>0)
		{
			float wx = (float)obj2_sumx / (float)obj2_count;
			float wy = (float)obj2_sumy / (float)obj2_count;
			obj2->posX = wx / (float)width;
			obj2->posY = 1.0f - wy / (float)height;
			obj2->timestamp = ctx->frameCounter;
		}
	}
	
	FleyeRenderWindow* render_buffer;
	TrackedObject* obj1;
	TrackedObject* obj2;
};

FLEYE_REGISTER_PLUGIN(l2CrossCenter);
