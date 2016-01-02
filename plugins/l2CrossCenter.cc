#include "fleye/cpuworker.h"
#include "fleye/plugin.h"
#include "fleye/FleyeRenderWindow.h"
#include "fleye/fbo.h"
#include "fleye/compiledshader.h"
#include "fleye/imageprocessing.h"
#include "fleye/FleyeContext.h"

#include "services/TrackingService.h"

#include <iostream> 


#define DECLARE_MINMAX_STAT(x) uint32_t x##Min=256, x##Max=0

#define UPDATE_MINMAX_STAT(v) \
	if(v<v##Min) { \
		v##Min=v; \
	} else if(v>v##Max) { \
		v##Max=v; \
	}

#define PRINT_MINMAX_STAT(x) \
	std::cout<<' '<<#x<<"=["<<x##Min<<';'<<x##Max<<']';



struct l2CrossCenter : public FleyePlugin
{
	inline l2CrossCenter() : render_buffer_odd(0), render_buffer_even(0), obj1(0), obj2(0), m_buffer(0) {}
	
	void setup(FleyeContext* ctx)
	{
		render_buffer_even = render_buffer_odd = ctx->ip->getRenderBuffer("l2c-render-buffer");
		if( render_buffer_odd == 0 )
		{
			render_buffer_odd = ctx->ip->getRenderBuffer("l2c-render-buffer-odd");
			render_buffer_even = ctx->ip->getRenderBuffer("l2c-render-buffer-even");
		}
		
		//if(render_buffer_odd==0 || render_buffer_even==0) { use_glRead = true; }
		
		TrackingService* track_svc = TrackingService_instance();
		obj1 = track_svc->addTrackedObject(0);
		obj2 = track_svc->addTrackedObject(1);		
		std::cout<<"L2CrossCenter setup : render_buffer_odd="<<render_buffer_odd<<", render_buffer_even="<<render_buffer_even 
				 <<", obj1="<<obj1<<", obj2="<<obj2<< "\n";
	}

	void run(FleyeContext* ctx, int threadId /*, frameCount */)
	{		
		// in case alternate render buffers are used, switch between the two
		FleyeRenderWindow* render_buffer = (ctx->frameCounter%2==0) ? render_buffer_even : render_buffer_odd;
		
		uint32_t obj1_sumx=0,obj1_sumy=0;
		uint32_t obj2_sumx=0,obj2_sumy=0;
		uint32_t obj1_count=0, obj2_count=0;
		int obj1_L2max=1, obj2_L2max=1;
		int width = 0;
		int height = 0;
		const uint32_t* base_ptr = 0;

		// if non render buffer found, it means we're using main frame buffer
		if( render_buffer == 0 )
		{
			width = ctx->render_window->width();
			height = ctx->render_window->height();
			if( m_buffer == 0 )
			{
				m_buffer = new uint32_t[width*height];
			}
			glReadPixels(0,0,width,height,GL_RGBA,GL_UNSIGNED_BYTE,m_buffer);
			base_ptr = m_buffer;
			
			DECLARE_MINMAX_STAT(m);

			for(uint32_t y=0;y<height;y++)
			{
				const uint32_t* p = m_buffer + y*width;
				for(uint32_t x=0;x<width;x++)
				{
					uint32_t value = p[x];
					uint32_t m = ( value ) & 0x000000FF;
					//uint32_t u = ( value >> 8) & 0x000000FF;
					//uint32_t o = ( value >> 16) & 0x000000FF;
					//uint32_t a = ( value >> 24) & 0x000000FF;
					//int m = (r>u) ? r : u;
					
					
					if( m>0 )
					{
						if( m<128 )
						{
							m = m >> 3;
							UPDATE_MINMAX_STAT(m);
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
							m = (m-128) >> 3;
							UPDATE_MINMAX_STAT(m);
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
			
			PRINT_MINMAX_STAT(m);
			std::cout<<'\n';
		}
		else
		{
			width = render_buffer->width();
			height = render_buffer->height();
			render_buffer->copyToBuffer(0,0,width,height);
			base_ptr = (const uint32_t*) render_buffer->getCopyBuffer();
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
								obj1_sumy += (height-y-1);
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
								obj2_sumy += (height-y-1);
								++ obj2_count;
							}
						}
					}
				}
			}
		}
				

		if(obj1_count>0)
		{
			float W = (float)obj1_count;
			float wx = (float)obj1_sumx ;
			float wy = (float)obj1_sumy ;
			float nx = wx / (float)width;
			float ny = wy / (float)height;
			obj1->posX = nx ;
			obj1->posY = ny ;
			obj1->area = W;
			obj1->weight = W;
			obj1->timestamp = ctx->frameCounter;
		}

		if(obj2_count>0)
		{
			float W = (float)obj2_count;
			float wx = (float)obj2_sumx ;
			float wy = (float)obj2_sumy ;
			float nx = wx / (float)width;
			float ny = wy / (float)height;
			obj2->posX = nx ;
			obj2->posY = ny ;
			obj2->area = W;
			obj2->weight = W;
			obj2->timestamp = ctx->frameCounter;
		}
	}
	
	FleyeRenderWindow* render_buffer_odd;
	FleyeRenderWindow* render_buffer_even;
	TrackedObject* obj1;
	TrackedObject* obj2;
	uint32_t* m_buffer;
};

FLEYE_REGISTER_PLUGIN(l2CrossCenter);
