#ifndef __fleye_services_TrackingService_h
#define __fleye_services_TrackingService_h

#include <vector>
#include <string>
#include <iostream>
#include <cmath>

#include "fleye/service.h"

struct TrackedObject
{
	float posX,posY;
	float speedX, speedY;
	float area;
	float weight;
	uint32_t priority;
	uint32_t timestamp;
	const uint32_t objectId;
	
	inline TrackedObject(uint32_t id)
		: posX(0.0f)
		, posY(0.0f)
		, speedX(0.0f)
		, speedY(0.0f)
		, area(0.0f)
		, weight(0.0f)
		, priority(0)
		, timestamp(0)
		, objectId(id) {}
		
	inline float speed() const { return std::sqrt(speedX*speedX+speedY*speedY); }
};

class TrackingService : public FleyeService
{
  public:  
	inline TrackedObject* getTrackedObject(uint32_t id)
	{
		if( m_trackedObjects.find(id) != m_trackedObjects.end() ) { return m_trackedObjects[id]; }
		TrackedObject* t = new TrackedObject(id);
		m_trackedObjects[id] = t;
		return t;
	}

	inline std::map<uint32_t,TrackedObject*>& getTrackedObjects() { return m_trackedObjects; };
	
  private:
	std::map<uint32_t,TrackedObject*> m_trackedObjects;
};

FLEYE_DECLARE_SERVICE(TrackingService)

#endif
