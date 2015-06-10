#include "cpu_tracking.h"
#include "gpio.h"
#include <stdio.h>

void *cpuTrackingWorker(void *arg)
{
	CPU_TRACKING_STATE * state = (CPU_TRACKING_STATE *) arg;
	
	fprintf(stderr,"cpuTrackingWorker started: %dx%d\n",state->width,state->height); fflush(stderr);
	
	while( state->do_processing )
	{
		vcos_semaphore_wait( & state->start_processing_sem );

		const uint32_t* p = (uint32_t*) state->image;
		int x,y;
		int sumx=0,sumy=0;
		int count=0;
		int L2max=1;
		for(y=0;y<state->height;y++)
		{
			for(x=0;x<state->width;x++)
			{
				uint32_t value = *(p++);
				uint32_t mask = ( value & 0x00000080 ) ;
				int l = ( value & 0x0000007F ) >> 4;
				int r = ( value & 0x00007F00 ) >> 12;
				int b = ( value & 0x007F0000 ) >> 20;
				int u = ( value & 0x7F000000 ) >> 28;
				if( mask )
				{
					int h = (l<r) ? l : r;
					int v = (b<u) ? b : u;
					int m = (h<v) ? h : v;
					if( m>L2max ) { count=sumx=sumy=0; L2max=m; }
					else if( m==L2max )
					{
						sumx += x;
						sumy += y;
						++count;
					}
				}
			}
		}

		if(count>0)
		{
			state->objectCount = 1;
			//printf("%d %fx%f\n",count,state->width,state->height);
			state->objectCenter[0][0] = (double)sumx / (double)( count * state->width );
			state->objectCenter[0][1] = (double)sumy / (double)( count * state->height );
			//printf("%f, %f\n",state->objectCenter[0][0],state->objectCenter[0][1]);
		}
		else
		{
			state->objectCount = 0;
			//printf("vide\n");
		}

		vcos_semaphore_post( & state->end_processing_sem );
		
		// ici, transmission des données au mcu
		if( state->objectCount > 0 )
		{
			gpio_write_xy_f(state->objectCenter[0][0], state->objectCenter[0][1]);
		}
	}
	
	return NULL;
}