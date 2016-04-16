#ifndef __fleye_services_IOService_h
#define __fleye_services_IOService_h

#include <vector>
#include <string>
#include <iostream>
#include "fleye/service.h"

class IOService : public FleyeService
{
  public:
    
	inline IOService()
		: m_analogOutput(0)
		, m_digitalOutput(0)
	{
	}
	
	int getNumberOfAnalogOutputs()
	{
		return m_analogOutput.size();
	}

	void setNumberOfAnalogOutputs(int n)
	{
		m_analogOutput.resize(n,0.5);
	}

	float getAnalogOutput(int i)
	{
		if(i<0 || i>=m_analogOutput.size()) return 0.5;
		return m_analogOutput[i];
	}
	
	void setAnalogOutput(int i, float x)
	{
		if(i<0 || i>=m_analogOutput.size()) return;
		m_analogOutput[i] = x;
	}
		
	int getNumberOfDigitalOutputs()
	{
		return m_digitalOutput.size();
	}

	void setNumberOfDigitalOutputs(int n)
	{
		m_digitalOutput.resize(n,false);
	}
	
	void setDigitalOutput(int i, bool x)
	{
		if(i<0 || i>=m_digitalOutput.size()) return;
		m_digitalOutput[i] = x;
	}

	bool getDigitalOutput(int i)
	{
		if(i<0 || i>=m_digitalOutput.size()) return false;
		return m_digitalOutput[i];
	}
	
  private:
	std::vector<float>  m_analogOutput;
	std::vector<bool>   m_digitalOutput;
};

FLEYE_DECLARE_SERVICE(IOService)

#endif
