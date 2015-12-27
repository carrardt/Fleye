#ifndef __fleye_services_TrackingService_h
#define __fleye_services_TrackingService_h

#include <vector>
#include <string>
#include <iostream>
#include "fleye/service.h"

struct TrackedObject
{
	float posX,posY;
	float speedX, speedY;
	const uint32_t objectId;
	uint32_t timestamp; // logical time when object has been tracked for the last time
	inline TrackedObject(uint32_t id) : posX(0.0f), posY(0.0f), speedX(0.0f), speedY(0.0f), objectId(id), timestamp(0) {}
};

class TrackingService : public FleyeService
{
  public:
	inline TrackedObject* addTrackedObject(uint32_t id)
	{
		TrackedObject* t = new TrackedObject(id);
		m_trackedObjects[id] = t;
		return t;
	}

	inline const std::map<uint32_t,TrackedObject*>& getTrackedObjects() const { return m_trackedObjects; };
	
  private:
	std::map<uint32_t,TrackedObject*> m_trackedObjects;
};

FLEYE_DECLARE_SERVICE(TrackingService)

#endif
