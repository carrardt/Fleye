#include "fleye/cpuworker.h"
#include "fleye/plugin.h"
#include "fleye/FleyeRenderWindow.h"
#include "fleye/fbo.h"
#include "fleye/compiledshader.h"
#include "fleye/imageprocessing.h"
#include "fleye/FleyeContext.h"

#include "services/TrackingService.h"
#include "services/TextService.h"

#include <iostream> 


#define DECLARE_MINMAX_STAT(x) int32_t x##Min=256, x##Max=0

#define UPDATE_MINMAX_STAT(v) \
	if(v<v##Min) { \
		v##Min=v; \
	} if(v>v##Max) { \
		v##Max=v; \
	}
	

#define PRINT_MINMAX_STAT(S,x) \
	S<<' '<<#x<<"=["<<x##Min<<';'<<x##Max<<']';

#define UPDATE_OBJ(o,M,X,Y) \
	if( M > o##_L2max ) \
	{  \
		o##_count = 0; \
		o##_sumx = 0; \
		o##_sumy = 0; \
		o##_L2max = M; \
	} \
	if( M == o##_L2max ) \
	{ \
		o##_sumx += X; \
		o##_sumy += Y; \
		++ o##_count; \
	}

#define UPDATE_OBJ_CENTER(M,X,Y) \
if( M>0 ) { \
	if( M<128 ) { \
		M = M >> 3; \
		UPDATE_OBJ(obj1,M,X,Y); \
	} else { \
		M = (M-128) >> 3; \
		UPDATE_OBJ(obj2,M,X,Y); \
	} \
}


struct l2CrossCenterArea : public FleyePlugin
{
	inline l2CrossCenterArea() : render_buffer_odd(0), render_buffer_even(0), obj1(0), obj2(0), m_buffer(0) {}
	
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
		obj1 = track_svc->getTrackedObject(0);
		obj2 = track_svc->getTrackedObject(1);		
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

		/*DECLARE_MINMAX_STAT(m0);
		DECLARE_MINMAX_STAT(m1);
		DECLARE_MINMAX_STAT(m2);
		DECLARE_MINMAX_STAT(m3);*/

		// if no render buffer found, it means we're using main frame buffer with glReadPixels
		if( render_buffer == 0 )
		{
			width = ctx->render_window->width();
			height = ctx->render_window->height();
			if( m_buffer == 0 )
			{
				m_buffer = new uint32_t[width*height];
			}
			glReadPixels(0,0,width,height/4,GL_RGBA,GL_UNSIGNED_BYTE,m_buffer);
			base_ptr = m_buffer;
			
			for(uint32_t y=0;y<height/4;y++)
			{
				const uint32_t* p = m_buffer + y*width;
				for(uint32_t x=0;x<width;x++)
				{
					uint32_t value = p[x];
					uint32_t m0 = ( value ) & 0x000000FF;
					uint32_t m1 = ( value >> 8) & 0x000000FF;
					uint32_t m2 = ( value >> 16) & 0x000000FF;
					uint32_t m3 = ( value >> 24) & 0x000000FF;
					UPDATE_OBJ_CENTER( m0, x, (y*4+0) );
					UPDATE_OBJ_CENTER( m1, x, (y*4+1) );
					UPDATE_OBJ_CENTER( m2, x, (y*4+2) );
					UPDATE_OBJ_CENTER( m3, x, (y*4+3) );
				}
			}
		}
		else
		{
			width = render_buffer->width();
			height = render_buffer->height();
			//render_buffer->copyToBuffer(0,height/2,width,height/2);
			render_buffer->copyToBuffer(0,height-(height/3),width,height/3);
			base_ptr = (const uint32_t*) render_buffer->getCopyBuffer();
			base_ptr += width*(height-(height/3));
			for(uint32_t y=0;y<height/3;y++)
			{
				const uint32_t* p = base_ptr + y*width;
				for(uint32_t x=0;x<width;x++)
				{
					uint32_t value = p[x];
					uint32_t m0 = ( value ) & 0x000000FF;
					uint32_t m1 = ( value >> 8 ) & 0x000000FF;
					uint32_t m2 = ( value >> 16 ) & 0x000000FF;
					/*UPDATE_MINMAX_STAT(m0);
					UPDATE_MINMAX_STAT(m1);
					UPDATE_MINMAX_STAT(m2);*/
					//int Yi = height - (y*2) - 1;
					UPDATE_OBJ_CENTER( m0, x, (height-(y*3)-1) );
					UPDATE_OBJ_CENTER( m1, x, (height-(y*3+1)-1) );
					UPDATE_OBJ_CENTER( m2, x, (height-(y*3+2)-1) );
				}
			}
		}
		
		/*PRINT_MINMAX_STAT(std::cout,m0);
		PRINT_MINMAX_STAT(std::cout,m1);
		PRINT_MINMAX_STAT(std::cout,m2);
		std::cout <<"\n";*/
		
		//TextService_instance()->console()<<obj1_L2max<<"\n";
		//if(obj1_count>0 || obj2_count>0) TextService_instance()->console() <<"\n";

		if(obj1_count>0)
		{
			double W = (double)obj1_count;
			double wx = (double)obj1_sumx ;
			double wy = (double)obj1_sumy ;
			double nx = wx / (double)width;
			double ny = wy / (double)height;
			obj1->posX = nx ;
			obj1->posY = ny ;
			obj1->area = W * (1UL<<obj1_L2max);;
			obj1->weight = W;
			obj1->timestamp = ctx->frameCounter;
			//TextService_instance()->console() << "Obj1@"<<obj1->posX<<","<<obj1->posY<<" ";
		}

		if(obj2_count>0)
		{
			double W = (double)obj2_count;
			double wx = (double)obj2_sumx ;
			double wy = (double)obj2_sumy ;
			double nx = wx / (double)width;
			double ny = wy / (double)height;
			obj2->posX = nx ;
			obj2->posY = ny ;
			obj2->area = W * (1UL<<obj2_L2max);
			obj2->weight = W;
			obj2->timestamp = ctx->frameCounter;
			//TextService_instance()->console() << "Obj2@"<<obj2->posX<<","<<obj2->posY;
		}
	}
	
	FleyeRenderWindow* render_buffer_odd;
	FleyeRenderWindow* render_buffer_even;
	TrackedObject* obj1;
	TrackedObject* obj2;
	uint32_t* m_buffer;
};

FLEYE_REGISTER_PLUGIN(l2CrossCenterArea);
