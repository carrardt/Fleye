#include "fleye/cpuworker.h"
#include "fleye/FleyeContext.h"
#include "fleye/imageprocessing.h"

#include "iostream"

void *cpuWorker(void *arg)
{
	CpuWorkerState* state = (CpuWorkerState*) arg;
	FleyeContext * ctx = state->ctx;
	
	if( ctx->verbose ) { std::cout<<"CPU worker #"<<state->tid<<" started\n"; }
	state->cpuFunc = 0;
	
	//vcos_semaphore_wait( & state->start_processing_sem );
	ctx->waitStartProcessingSem( state->tid );
	
	while( state->do_processing )
	{
		if( state->cpu_processing[ state->cpuFunc ] !=0 )
		{
			state->cpu_processing[ state->cpuFunc ]->run( ctx, state->tid + 1 );
			
			// signal that one more task has finished
			ctx->postEndProcessingSem( state->tid );
			//vcos_semaphore_post( & state->end_processing_sem );
			
			// step to the next function to execute
			++ state->cpuFunc;
		}
		else
		{
			state->cpuFunc = 0;
		}

		// wait signal to start next procesing 
		ctx->waitStartProcessingSem( state->tid );
		//vcos_semaphore_wait( & state->start_processing_sem );
	}
	
	return NULL;
}
