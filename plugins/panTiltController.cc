#include "fleye/plugin.h"
#include "fleye/FleyeContext.h"

#include "services/PanTiltService.h"
#include "services/IOService.h"
#include <cmath>
#include <iostream>

struct panTiltController : public FleyePlugin
{
	inline panTiltController() : m_ptsvc(0), m_iosvc(0) {}
	
	void setup(FleyeContext* ctx)
	{		
		m_iosvc = IOService_instance();
		if( m_iosvc->getNumberOfAnalogOutputs() < 2 )
		{
			m_iosvc->setNumberOfAnalogOutputs( 2 );
		}
		if( m_iosvc->getNumberOfDigitalOutputs() < 1 )
		{
			m_iosvc->setNumberOfDigitalOutputs( 1 );
		}

		m_ptsvc = PanTiltService_instance();

		m_iosvc->setAnalogOutput( 0, m_ptsvc->pan() );
		m_iosvc->setAnalogOutput( 1, m_ptsvc->tilt() );
		m_iosvc->setDigitalOutput( 3, false ); //TODO: configurable mapping to IOService outputs
		
		std::cout<<"panTiltController ready, PanTiltService @"<<m_ptsvc<<", IOService @"<<m_iosvc<<"\n";
	}

	void run(FleyeContext* ctx,int threadId)
	{		
		 //std::cout<<m_ControlX<<" / "<<m_ControlY<<"\n";
		 if( m_ptsvc->pan() < 0.0 ) m_ptsvc->setPan(0.0);
		 else if( m_ptsvc->pan() > 1.0 ) m_ptsvc->setPan(1.0);
		 if( m_ptsvc->tilt() < 0.0 ) m_ptsvc->setTilt(0.0);
		 else if( m_ptsvc->tilt() > 1.0 ) m_ptsvc->setTilt(1.0);
		 
		m_iosvc->setAnalogOutput( 0, m_ptsvc->pan() );
		m_iosvc->setAnalogOutput( 1, m_ptsvc->tilt() );
		m_iosvc->setDigitalOutput( 3, m_ptsvc->laser() );
	}
	
	PanTiltService* m_ptsvc;
	IOService* m_iosvc;
};

FLEYE_REGISTER_PLUGIN(panTiltController);

