#ifndef __fleye_services_MotorDriveService_h
#define __fleye_services_MotorDriveService_h

#include <vector>
#include <string>
#include <iostream>
#include "fleye/service.h"

class MotorDriveService : public FleyeService
{
  struct MotorCommand
  {
	  inline MotorCommand() : m_targetPosition(0.0f), m_targetSpeed(0.0f) {}
	  float m_targetPosition;
	  float m_targetSpeed;
  };
	
  public:
    
	inline MotorDriveService()
		: m_motorCommand(0)
	{}
	
	int getNumberOfMotors()
	{
		return m_motorCommand.size();
	}

	void setNumberOfMotors(int n)
	{
		m_motorCommand.resize(n);
	}
	
	const MotorCommand& getMotorCommand(int i) { return m_motorCommand[i]; }

	void setMotorCommand(int i, float position, float speed)
	{
		if(i<0 || i>=m_motorCommand.size()) return;
		m_motorCommand[i].m_targetPosition = position;
		m_motorCommand[i].m_targetSpeed = speed;
	}

  private:
	std::vector<MotorCommand>  m_motorCommand;
};

FLEYE_DECLARE_SERVICE(MotorDriveService)

#endif
