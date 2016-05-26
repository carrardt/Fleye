#include "fleye/plugin.h"
#include "fleye/FleyeContext.h"
#include "fleye/vec2f.h"

#include "services/TrackingService.h"
#include "services/TextService.h"
#include "services/MotorDriveService.h"
#include "services/PanTiltService.h"

#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <assert.h>

#define _USE_MATH_DEFINES
#include <cmath>

struct SmartCar : public FleyePlugin
{
	inline SmartCar()
		: m_ptsvc(0)
		, m_motorsvc(0)
		, m_obj1(0)
		, m_obj2(0)
		, m_txt(0)
	{
	}
		
	void setup(FleyeContext* ctx)
	{
		//m_txt = TextService_instance()->addPositionnedText(0.1,0.4);
		//m_txt->out()<<"Raise !";

		m_obj1 = TrackingService_instance()->getTrackedObject(0);
		m_obj2 = TrackingService_instance()->getTrackedObject(1);

		m_motorsvc = MotorDriveService_instance();
		m_motorsvc->setNumberOfMotors(2);

		m_ptsvc = PanTiltService_instance();

		std::cout<<"SmartCar ready : MotorDriveService @"<<m_motorsvc<<"\n";
	}

	void onStaticObject()
	{
		std::ostringstream oss;
		oss<<"angle="<<m_targetHorizAngle<<"\ndistance="<<m_targetDistance;
		if( m_targetDistance > 0.03f )
		{
			if( m_targetHorizAngle < -0.5f ) // turn right
			{
				oss<<"\nTurn Right";
				m_motorsvc->setMotorCommand( 0, 0.0f , 0.0f ); // right wheel
				m_motorsvc->setMotorCommand( 1, 0.5f , 1.0f ); // left wheel					
			}
			else if( m_targetHorizAngle < -0.25f ) // turn right
			{
				oss<<"\nTurn half-Right";
				m_motorsvc->setMotorCommand( 0, 0.0f , 0.0f ); // right wheel
				m_motorsvc->setMotorCommand( 1, 0.25f , 0.75f ); // left wheel					
			}			
			else if( m_targetHorizAngle > 0.5f ) // turn left
			{
				oss<<"\nTurn Left";
				m_motorsvc->setMotorCommand( 0, 0.5f , 1.0f ); // right wheel
				m_motorsvc->setMotorCommand( 1, 0.0f , 0.0f ); // left wheel
			}
			else if( m_targetHorizAngle > 0.25f ) // turn left
			{
				oss<<"\nTurn half-Left";
				m_motorsvc->setMotorCommand( 0, 0.25f , 0.75f ); // right wheel
				m_motorsvc->setMotorCommand( 1, 0.0f , 0.0f ); // left wheel
			}
			else // go ahead
			{
				oss<<"\nGo Ahead";
				m_motorsvc->setMotorCommand( 0, 1.0f , 0.75f ); // right wheel
				m_motorsvc->setMotorCommand( 1, 1.0f , 0.75f ); // left wheel
			}
		}
		//m_txt->setText( oss.str() );
		std::cout<<oss.str()<<"\n";
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

		bool bigEnough = ( A2 > 64.0f );
		
		if( ! bigEnough ) { return; }

		// this will be the position to track
		Vec2f P = P2; //P1; //(P1+P2)*0.5f;
		Vec2f dP = target - P;

		if( std::abs(dP.x) < 0.01 )
		{
			float horiz_angle = (m_ptsvc->pan() - 0.5f)*M_PI;
			float distance = 1024.0f / A2;
			++ m_targetLockedFrames;
			m_targetHorizAngle += horiz_angle;
			m_targetDistance += distance;
			if( m_targetLockedFrames > 90 )
			{
				m_targetHorizAngle /= m_targetLockedFrames;
				m_targetDistance /= m_targetLockedFrames;
				this->onStaticObject();		
				m_targetLockedFrames = 0;
				m_targetHorizAngle = 0.0f;
				m_targetDistance = 0.0f;
			}
		}
		else
		{
			m_targetLockedFrames = 0;
			m_targetHorizAngle = 0.0f;
			m_targetDistance = 0.0f;
		}
	}

	PanTiltService* m_ptsvc;
	TrackedObject* m_obj1;
	TrackedObject* m_obj2;
	MotorDriveService* m_motorsvc;
	PositionnedText* m_txt;
	float m_targetHorizAngle, m_targetDistance;
	int m_targetLockedFrames;
};

FLEYE_REGISTER_PLUGIN(SmartCar);

