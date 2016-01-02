#include "fleye/cpuworker.h"
#include "fleye/plugin.h"
#include "fleye/FleyeRenderWindow.h"
#include "fleye/fbo.h"
#include "fleye/compiledshader.h"
#include "fleye/imageprocessing.h"
#include "fleye/FleyeContext.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <iostream>

#define VCMEM_SIZE   0x8000000
// uncached alias @  0xC0000000, given a split with 128Mb for video core sdram
// Rpi mem size is   0x38000000
#define VCMEM_OFFSET 0xF8000000ULL
#define VCMEM_DEVPATH "/dev/mem"

struct scanGPU : public FleyePlugin
{
	
	inline scanGPU() : m_fd(-1), m_hostPtr(0)  {}
	
	void setup(FleyeContext* ctx)
	{
		m_fd = open(VCMEM_DEVPATH, O_RDWR | O_SYNC);
		if ( m_fd < 0) 
		{
			std::cerr<<"Unable to open "<<VCMEM_DEVPATH<<"\n";
			return;
		}
		m_hostPtr = mmap(NULL, VCMEM_SIZE, (PROT_READ | PROT_WRITE), MAP_SHARED, m_fd, VCMEM_OFFSET );
		if (m_hostPtr == MAP_FAILED)
		{
			std::cerr<<"Unable to map "<<VCMEM_DEVPATH<<"\n";
			m_hostPtr = 0;
			return;
		}
		std::cout<<"scanGPU ready : fd="<<m_fd<<", addr="<<m_hostPtr<<"\n";
	}

	void run(FleyeContext* ctx, int threadId)
	{		
		uint32_t* dwptr = (uint32_t*) m_hostPtr;
		uint32_t nWords = VCMEM_SIZE/4;
		
		uint32_t bStart = 0;
		uint32_t bLen = 0;
		uint32_t bCmpValue = 0;

		uint32_t start = 0;
		uint32_t len = 1;
		uint32_t cmpValue = dwptr[0];
		
		for(uint32_t i=1;i<nWords;i++)
		{
			if( i%(1024*1024) == (1024*1024-1) ) { std::cout<<'.'; std::cout.flush(); }
			uint32_t value = dwptr[i];
			/*
			uint32_t x0 = ( value ) & 0x000000FF;
			uint32_t x1 = ( value >> 8) & 0x000000FF;
			uint32_t x2 = ( value >> 16) & 0x000000FF;
			uint32_t x3 = ( value >> 24) & 0x000000FF;
			if(x1<x0) std::swap(x0,x1);
			if(x2<x1) std::swap(x2,x1);
			if(x3<x2) std::swap(x2,x3);
			if(x2<x1) std::swap(x2,x1);
			if(x1<x0) std::swap(x0,x1);			
			if( (x0==63 || x0==64) && (x1==127 || x1==128) && x3==255 )
			{ 
				std::cout<<i<<": x0="<<x0<<" x1="<<x1<<" x2="<<x2<<" x3="<<x3<<"\n";
			}
			*/
			if( value == cmpValue ) ++len;
			else
			{ 
				if( cmpValue!=0 && len>bLen ) { bStart=start; bLen=len; bCmpValue=cmpValue; }
				len=1; cmpValue=value; start=i;
			}
		}
		if( cmpValue!=0 && len>bLen ) { bStart=start; bLen=len; bCmpValue=cmpValue; }
		
		std::cout<<"start="<<bStart<<", len="<<bLen<<", value="<<(void*)bCmpValue<<"\n";
	}
	
	int m_fd;
	void * m_hostPtr;
};

FLEYE_REGISTER_PLUGIN(scanGPU);
