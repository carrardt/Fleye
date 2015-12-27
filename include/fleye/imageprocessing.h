#ifndef __fleye_ImageProcessing_H_
#define __fleye_ImageProcessing_H_

#include "fleye/config.h"
#include "fleye/processingstep.h"
#include "fleye/cpuworker.h"

#include <string>
#include <map>
#include <vector>

struct GLTexture;
struct FrameBufferObject;
struct FleyeRenderWindow;
struct FleyeContext;

struct ImageProcessingState
{
	inline ImageProcessingState(FleyeContext* _ctx)
		: ctx(_ctx)
		{
			for(int i=0;i<PROCESSING_ASYNC_THREADS;i++)
			{
				cpu_tracking_state[i].ctx = _ctx;
				cpu_tracking_state[i].tid = i;
			}
		}
	
	FleyeRenderWindow* getRenderBuffer(const std::string& name) const;
	int readScriptFile();	

	FleyeContext* ctx;
	CpuWorkerState cpu_tracking_state[PROCESSING_ASYNC_THREADS];
	std::vector<ProcessingStep> processing_step;
	std::map<std::string,GLTexture*> texture;
	std::map<std::string,FrameBufferObject*> fbo;
};

#endif
