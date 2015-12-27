#ifndef __FLEYE_CPU_TRACKING_H
#define __FLEYE_CPU_TRACKING_H

#include "fleye/config.h"
#include "fleye/plugin.h"
#include <stdint.h>

struct FleyeContext;
typedef FleyePlugin* FleyePluginP;
typedef volatile FleyePluginP VFleyePluginP;

struct CpuWorkerInternal;

typedef struct CpuWorkerState
{
  inline CpuWorkerState()
	: ctx(0)
	, tid(-1)
	, do_processing(0)
	, nAvailCpuFuncs(0)
	, nFinishedCpuFuncs(0)
	, cpuFunc(0) {}

  FleyeContext* ctx;
  int tid;
  
  VFleyePluginP cpu_processing[IMGPROC_MAX_CPU_FUNCS];
  volatile int do_processing;
  volatile int nAvailCpuFuncs;
  volatile int nFinishedCpuFuncs;
  volatile int cpuFunc;

} CpuWorkerState;

// entry point for cpu processing thread
void *cpuWorker(void *arg);

#endif
