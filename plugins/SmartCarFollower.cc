#include "fleye/plugin.h"
#include "fleye/FleyeContext.h"
#include "fleye/vec2f.h"

#include "services/PanTiltService.h"
#include "services/TrackingService.h"
#include "services/TextService.h"

#include "services/MotorDriveService.h"

#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <assert.h>

#define _USE_MATH_DEFINES
#include <cmath>

struct SmartCarFollower : public FleyePlugin
{
	inline SmartCarFollower()
		: m_ptsvc(0)
		, m_motorsvc(0)
		, m_txt(0)
		, m_obj1(0)
		, m_obj2(0)
		, m_panMin(0.1)
		, m_panMax(0.9)
		, m_tiltMin(0.1)
		, m_tiltMax(0.9)
		, m_start(true)
	{
	}
		
	void setup(FleyeContext* ctx)
	{
		m_ptsvc = PanTiltService_instance();
		m_obj1 = TrackingService_instance()->getTrackedObject(0);
		m_obj2 = TrackingService_instance()->getTrackedObject(1);
		m_txt = TextService_instance()->addPositionnedText(0.1,0.2);
		m_txt->out()<<"Initializing...";

		
		// TODO: tracked object have drawing style and color attributes
		auto cobj = TrackingService_instance()->getTrackedObject(99);
		cobj->posX = 0.5;
		cobj->posY = 0.5;

		m_motorsvc = MotorDriveService_instance();
		m_motorsvc->setNumberOfMotors(2);

		std::cout<<"PanTiltFollower ready : MotorDriveService @"<<m_motorsvc<<", PanTiltService @"<<m_ptsvc<<", obj1 @"<<m_obj1<<", obj2 @"<<m_obj2<< "\n";
	}

	void smartCar(float horiz_angle, float distance)
	{
		++ m_targetLockedFrames;
		m_targetHorizAngle += horiz_angle;
		m_targetDistance += distance;
		if( m_targetLockedFrames > 120 )
		{
			m_targetHorizAngle /= m_targetLockedFrames;
			m_targetDistance /= m_targetLockedFrames;			
			m_txt->out()<<"angle="<<m_targetHorizAngle<<"\ndistance="<<m_targetDistance;
			
			if( m_targetDistance > 0.03f )
			{
				if( m_targetHorizAngle < -0.5f ) // turn right
				{
					std::cout<<"Right\n";
					m_motorsvc->setMotorCommand( 0, 0.0f , 0.5f ); // right wheel
					m_motorsvc->setMotorCommand( 1, 0.5f , 1.0f ); // left wheel					
				}
				else if( m_targetHorizAngle > 0.5f ) // turn left
				{
					std::cout<<"Left\n";
					m_motorsvc->setMotorCommand( 0, 0.5f , 1.0f ); // right wheel
					m_motorsvc->setMotorCommand( 1, 0.1f , 0.5f ); // left wheel
				}
				else // go ahead
				{
					std::cout<<"Ahead\n";
					m_motorsvc->setMotorCommand( 0, 0.5f , 1.0f ); // right wheel
					m_motorsvc->setMotorCommand( 1, 0.5f , 1.0f ); // left wheel
				}
			}
			
			m_targetLockedFrames = 0;
			m_targetHorizAngle = 0.0f;
			m_targetDistance=0.0f;			
		}
	}

	void run(FleyeContext* ctx,int threadId)
	{
		// what is the target position of the tracked point
		const Vec2f target( 0.5f, 0.43f );

		float W1 = m_obj1->weight;
		float A1 = m_obj1->area;
		float S1 = (W1>=1.0f) ? 1.0f/W1 : 0.0f;
		float W2 = m_obj2->weight;
		float A2 = m_obj2->area;
		float S2 = (W2>=1.0f) ? 1.0f/W2 : 0.0f;
		Vec2f P1( m_obj1->posX * S1 , m_obj1->posY * S1 );
		Vec2f P2( m_obj2->posX * S2 , m_obj2->posY * S2 );

		bool bigEnough = ( A2 > 128.0f );

		// this will be the position to track
		Vec2f P = P2; //P1; //(P1+P2)*0.5f;
		
		Vec2f dP = target - P;
		
		// wait to have an object in the center before starting
		if( dP.norm() < 0.1 )
		{ 
			if( ! m_start ) { m_txt->out()<<"Target locked"; }
			m_start=true;
		}
		if( ! m_start ) return;

		if( ! bigEnough )
		{
			m_txt->out()<<"Target lost :(";
			return;
		}
		
		if( dP.x < 0.01 )
		{
			m_ptsvc->setLaser( ! m_ptsvc->laser() );
			this->smartCar( (m_ptsvc->pan() - 0.5f)*M_PI , 1024.0f / A2 );
			return;
		}
		m_targetLockedFrames = 0;
		m_targetHorizAngle = 0.0f;
		m_targetDistance=0.0f;
		m_ptsvc->setLaser( false );

		float cx = m_ptsvc->pan();
		float cy = m_ptsvc->tilt();

		cx -= dP.x * 0.04f;
		cx = std::max( std::min( cx , m_panMax ) , m_panMin );
		
		float tiltAmpMax = 0.5f - std::abs( 0.5f - ( cx - m_panMin ) / ( m_panMax - m_panMin ) );
		float tiltMax = (m_tiltMin+m_tiltMax)*0.5f + (m_tiltMax-m_tiltMin)*tiltAmpMax;
		float tiltMin = (m_tiltMin+m_tiltMax)*0.5f - (m_tiltMax-m_tiltMin)*tiltAmpMax;

		cy += dP.y * 0.04f;
		cy = std::max( std::min( cy , tiltMax ) , tiltMin );

		m_txt->out()<<"A1="<<A1<<"\nA2="<<A2;

		m_ptsvc->setPan( cx );
		m_ptsvc->setTilt( cy );
	}

	PanTiltService* m_ptsvc;
	MotorDriveService* m_motorsvc;
	PositionnedText* m_txt;
	TrackedObject* m_obj1;
	TrackedObject* m_obj2;
	float m_panMin,m_panMax,m_tiltMin,m_tiltMax;
	float m_targetHorizAngle, m_targetDistance;
	int m_targetLockedFrames;
	bool m_start;
};

FLEYE_REGISTER_PLUGIN(SmartCarFollower);

