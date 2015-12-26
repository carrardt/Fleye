#include "fleye/cpuworker.h"
#include "fleye/plugin.h"
#include "fleye/FleyeRenderWindow.h"
#include "fleye/fbo.h"
#include "fleye/compiledshader.h"
#include "fleye/imageprocessing.h"
#include "fleye/FleyeContext.h"

#include <iostream>

static FleyeRenderWindow* render_buffer = 0;

struct l2CrossCenter : public FleyePlugin
{

	void setup(FleyeContext* ctx)
	{
		render_buffer = ctx->ip->getRenderBuffer("l2c-render-buffer");
		std::cout<<"L2CrossCenter setup : render_buffer @"<<render_buffer<<"\n";
	}

	void run(FleyeContext* ctx)
	{
		CpuWorkerState * state = & ctx->ip->cpu_tracking_state;
		int width=0, height=0;
		const uint32_t* base_ptr = (const uint32_t*) render_buffer->readBack(width,height);
		uint32_t obj1_sumx=0,obj1_sumy=0;
		uint32_t obj2_sumx=0,obj2_sumy=0;
		uint32_t obj1_count=0, obj2_count=0;
		uint32_t obj1_L2max=1, obj2_L2max=1;
		
		for(uint32_t y=0;y<height;y++)
		{
			const uint32_t* p = base_ptr + y * width;
			for(uint32_t x=0;x<width;x++)
			{
				uint32_t value = *(p++);
				uint32_t r = ( value >> 3) & 0x0000001F;
				uint32_t u = ( value >> 11) & 0x0000001F;
				uint32_t B = ( value >> 16) & 0x000000FF;
				//uint32_t A = ( value >> 24) & 0x000000FF;
				
				if( B != 0 )
				{
					uint32_t m = (r>u) ? r : u;
					if( B<128 )
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
							//if( r1 >= 1 ) obj1_sumx -= 1<<(r1-1);
							obj1_sumy += y;
							//if( u1 >= 1 ) obj1_sumy -= 1<<(u1-1);
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
							//if( r2 >= 1 ) obj2_sumx -= 1<<(r2-1);
							obj2_sumy += y;
							//if( u2 >= 1 ) obj2_sumy -= 1<<(u2-1);
							++ obj2_count;
						}
					}
				}
			}
		}

		state->objectCount = 0;
			
		if(obj1_count>0)
		{
			obj1_sumx /= obj1_count;
			obj1_sumy /= obj1_count;	
			state->objectCenter[state->objectCount][0] = ((float)obj1_sumx) / (float)width;
			state->objectCenter[state->objectCount][1] = 1.0 - ((float)obj1_sumy) / (float)height;
			state->trackedObjects[ state->objectCount ++ ] = 0;
		}

		if(obj2_count>0)
		{
			obj2_sumx /= obj2_count;
			obj2_sumy /= obj2_count;	
			state->objectCenter[state->objectCount][0] = ((float)obj2_sumx) / (float)width;
			state->objectCenter[state->objectCount][1] = 1.0 - ((float)obj2_sumy) / (float)height;
			state->trackedObjects[ state->objectCount ++ ] = 1;
		}

		/*if( ctx->frameCounter%30 == 0 )
		{
			PRINT_MINMAX_STAT(R);
			PRINT_MINMAX_STAT(G);
			PRINT_MINMAX_STAT(B);
			PRINT_MINMAX_STAT(u1);
			PRINT_MINMAX_STAT(r1);
			PRINT_MINMAX_STAT(u2);
			PRINT_MINMAX_STAT(r2);
			std::cout<<" c1="<<obj1_count<<" p="<<obj1_sumx<<','<<obj1_sumy;
			std::cout<<" c2="<<obj2_count<<" p="<<obj2_sumx<<','<<obj2_sumy;
			std::cout<<" T="<<state->objectCenter[0][0]<<','<<state->objectCenter[0][1];
			std::cout<<" L="<<state->objectCenter[1][0]<<','<<state->objectCenter[1][1]<<"\n";
		}*/
	}
};

FLEYE_REGISTER_PLUGIN(l2CrossCenter);
