#ifndef __fleye_services_PanTiltService_h
#define __fleye_services_PanTiltService_h

#include <vector>
#include <string>
#include <iostream>
#include "fleye/service.h"

class PanTiltService : public FleyeService
{
  public:
	inline PanTiltService()
	{
		setNumberOfControllers(1);
	}
	
	void setNumberOfControllers(int n)
	{
		m_panCommand.resize(n,0.5f);
		m_tiltCommand.resize(n,0.5f);
		m_laserCommand.resize(n,false);
	}
	
	float getPanMin(int cid=0) { return 0.1; }
	float getPanMax(int cid=0) { return 0.9; }
	float getTiltMin(int cid=0) { return 0.1; }
	float getTiltMax(int cid=0) { return 0.9; }
	
	float pan(int cid=0) const { return m_panCommand[cid]; }
	float tilt(int cid=0) const { return m_tiltCommand[cid]; }
	
	bool laser(int cid=0) const { return m_laserCommand[cid]; }
	
	void setPan(float x,int cid=0)
	{
		if( x<getPanMin(cid) ) x=getPanMin(cid);
		if( x>getPanMax(cid) ) x=getPanMax(cid);
		m_panCommand[cid] = x;
	}
	void setTilt(float x,int cid=0)
	{
		if( x<getTiltMin(cid) ) x=getTiltMin(cid);
		if( x>getTiltMax(cid) ) x=getTiltMax(cid);
		m_tiltCommand[cid] = x;
	}
	void setLaser(bool l,int cid=0) { m_laserCommand[cid] = l; }
	
  private:
	std::vector<float>  m_panCommand;
	std::vector<float>   m_tiltCommand;
	std::vector<bool>   m_laserCommand;
};

FLEYE_DECLARE_SERVICE(PanTiltService)

#endif
