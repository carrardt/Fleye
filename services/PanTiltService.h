#ifndef __fleye_services_PanTiltService_h
#define __fleye_services_PanTiltService_h

#include <vector>
#include <string>
#include <iostream>
#include "fleye/service.h"

class PanTiltService : public FleyeService
{
  public:
	inline PanTiltService() : m_panCommand(0.5), m_tiltCommand(0.5) {}
	
	float pan() const { return m_panCommand; }
	float tilt() const { return m_tiltCommand; }
	
	void setPan(float x) { m_panCommand = x; }
	void setTilt(float x) { m_tiltCommand = x; }
	
  private:
	float m_panCommand;
	float m_tiltCommand;
};

FLEYE_DECLARE_SERVICE(PanTiltService)

#endif
