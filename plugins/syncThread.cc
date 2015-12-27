#include <stdio.h>

#include "fleye/cpuworker.h"
#include "fleye/plugin.h"
#include "fleye/FleyeContext.h"
#include "fleye/imageprocessing.h"

struct syncThread : public FleyePlugin
{
	void syncThread_setup(FleyeContext* ctx)
	{
		printf("syncThread plugin ready\n");
	}

	void syncThread_run(FleyeContext* ctx)
	{
		for(int i=0;i<PROCESSING_ASYNC_THREADS;i++)
		{
			CpuWorkerState * state = & ctx->ip->cpu_tracking_state[i];
			int nToWait = state->nAvailCpuFuncs - state->nFinishedCpuFuncs;
			while( nToWait > 0 )
			{
				ctx->waitEndProcessingSem( i );
				-- nToWait;
			}
			state->nFinishedCpuFuncs = state->nAvailCpuFuncs; 
		}
	}
};

FLEYE_REGISTER_PLUGIN(syncThread);
