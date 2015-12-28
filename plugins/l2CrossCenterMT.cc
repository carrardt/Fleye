extern "C" {
#include "interface/vcos/vcos.h"
}

#include "fleye/cpuworker.h"
#include "fleye/plugin.h"
#include "fleye/FleyeRenderWindow.h"
#include "fleye/fbo.h"
#include "fleye/compiledshader.h"
#include "fleye/imageprocessing.h"
#include "fleye/FleyeContext.h"

#include "services/TrackingService.h"

#include <iostream> 

struct l2CrossCenterMT : public FleyePlugin
{
	inline l2CrossCenterMT() : render_buffer_odd(0), render_buffer_even(0), obj1(0), obj2(0) {}
	
	void setup(FleyeContext* ctx)
	{
		render_buffer_even = render_buffer_odd = ctx->ip->getRenderBuffer("l2c-render-buffer");
		if( render_buffer_odd == 0 )
		{
			render_buffer_odd = ctx->ip->getRenderBuffer("l2c-render-buffer-odd");
			render_buffer_even = ctx->ip->getRenderBuffer("l2c-render-buffer-even");
		}

		TrackingService* track_svc = TrackingService_instance();
		obj1 = track_svc->addTrackedObject(0);
		obj2 = track_svc->addTrackedObject(1);
		
		memset(&mutex,0,sizeof(mutex));
		assert( vcos_mutex_create(&mutex,"l2CrossMutex") == VCOS_SUCCESS );
		
		std::cout<<"l2CrossCenterMT setup : render_buffer_odd="<<render_buffer_odd<<", render_buffer_even="<<render_buffer_even 
				 <<", obj1="<<obj1<<", obj2="<<obj2<< "\n";
	}

	void run(FleyeContext* ctx, int threadId)
	{
		// in case alternate render buffers are used, switched between the two
		FleyeRenderWindow* render_buffer = (ctx->frameCounter%2==0) ? render_buffer_odd : render_buffer_even;
		if( render_buffer == 0 ) return;

		uint32_t width = render_buffer->width();
		uint32_t height = render_buffer->height();
		
		uint32_t obj1_sumx=0,obj1_sumy=0;
		uint32_t obj2_sumx=0,obj2_sumy=0;
		uint32_t obj1_count=0, obj2_count=0;
		uint32_t obj1_L2max=1, obj2_L2max=1;

		threadId -= 1; // we don't use the main thread (0)
		assert( threadId>=0 && threadId<PROCESSING_ASYNC_THREADS );
		
		uint32_t nThreads = PROCESSING_ASYNC_THREADS;
		uint32_t nSplits = 4;
		uint32_t nSlices = nThreads * nSplits;
				
		const uint32_t* base_ptr = (const uint32_t*) render_buffer->getCopyBuffer();

		for(uint32_t split=0;split<nSplits;split++)
		{
			uint32_t sliceIndex = split*nThreads + threadId;

			uint32_t hStart = ( height * sliceIndex ) / nSlices ;
			uint32_t hEnd = ( height * (sliceIndex+1) ) / nSlices ;

			render_buffer->copyToBuffer(0,hStart,width,hEnd-hStart);
			//std::cout<<width<<"x"<<height<<"\n";
						
			for(uint32_t y=hStart;y<hEnd;y++)
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
		
		
		vcos_mutex_lock(&mutex);
		
		if(obj1_count>0)
		{
			if( ctx->frameCounter > obj1->timestamp )
			{
				obj1->priority = 0;
			}
			float W = (float)obj1_count;
			float wx = (float)obj1_sumx ;
			float wy = (float)obj1_sumy ;
			float nx = wx / (float)width;
			float ny = wy / (float)height;
			if( obj1_L2max > obj1->priority )
			{
				obj1->priority = obj1_L2max;
				obj1->posX = 0.0f;
				obj1->posY = 0.0f;
				obj1->area = 0.0f;
				obj1->weight = 0.0f;
				obj1->timestamp = ctx->frameCounter;
			}
			if( obj1_L2max == obj1->priority )
			{
				obj1->posX += nx ;
				obj1->posY += ny ;
				obj1->area += W;
				obj1->weight += W;
				//std::cout<<"add "<<nx<<','<<ny<<','<<W<<"\n";
			}
		}

		if(obj2_count>0)
		{
			if( ctx->frameCounter > obj2->timestamp )
			{
				obj2->priority = 0;
			}
			float W = (float)obj2_count;
			float wx = (float)obj2_sumx ;
			float wy = (float)obj2_sumy ;
			float nx = wx / (float)width;
			float ny = wy / (float)height;
			if( obj2_L2max > obj2->priority )
			{
				obj2->priority = obj2_L2max;
				obj2->posX = 0.0f;
				obj2->posY = 0.0f;
				obj2->area = 0.0f;
				obj2->weight = 0.0f;
				obj2->timestamp = ctx->frameCounter;
			}
			if( obj2_L2max == obj2->priority )
			{
				obj2->posX += nx ;
				obj2->posY += ny ;
				obj2->area += W;
				obj2->weight += W;
			}
		}
		
		vcos_mutex_unlock(&mutex);

	}
	
	VCOS_MUTEX_T mutex;
	FleyeRenderWindow* render_buffer_odd;
	FleyeRenderWindow* render_buffer_even;
	TrackedObject* obj1;
	TrackedObject* obj2;
};

FLEYE_REGISTER_PLUGIN(l2CrossCenterMT);
