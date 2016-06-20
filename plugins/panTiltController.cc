#include "fleye/plugin.h"
#include "fleye/FleyeContext.h"

#include "services/PanTiltService.h"
#include "services/IOService.h"
#include <cmath>
#include <iostream>

struct panTiltController : public FleyePlugin
{
	inline panTiltController()
		: m_ptsvc(0)
		, m_iosvc(0)
		, m_panAnalog(0)
		, m_tiltAnalog(1)
		, m_laserAnalog(2)
		, m_laserDigital(4)
		{}
	
	void setup(FleyeContext* ctx)
	{		
		std::string panAxisStr = ctx->vars["PAN"];
		if( ! panAxisStr.empty() ) { m_panAnalog = atoi(panAxisStr.c_str()); }
		
		std::string tiltAxisStr = ctx->vars["TILT"];
		if( ! tiltAxisStr.empty() ) { m_tiltAnalog = atoi(tiltAxisStr.c_str()); }

		std::string laserAnalogStr = ctx->vars["LASERA"];
		if( ! laserAnalogStr.empty() ) { m_laserAnalog = atoi(laserAnalogStr.c_str()); }

		std::string laserDigitalStr = ctx->vars["LASERD"];
		if( ! laserDigitalStr.empty() ) { m_laserDigital = atoi(laserDigitalStr.c_str()); }

		std::cout<<"panAxis="<<m_panAnalog<<", tiltAxis="<<m_tiltAnalog;
		std::cout<<", lasera="<<m_laserAnalog<<", laserd="<<m_laserDigital<<"\n";

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

		if( m_panAnalog<m_iosvc->getNumberOfAnalogOutputs() )
		{
			m_iosvc->setAnalogOutput( m_panAnalog, m_ptsvc->pan() );
		}
		if( m_tiltAnalog<m_iosvc->getNumberOfAnalogOutputs() )
		{
			m_iosvc->setAnalogOutput( m_tiltAnalog, m_ptsvc->tilt() );
		}
		if( m_laserAnalog<m_iosvc->getNumberOfAnalogOutputs() )
		{
			m_iosvc->setAnalogOutput( m_laserAnalog, 0.0 );
		}
		if( m_laserDigital<m_iosvc->getNumberOfDigitalOutputs() )
		{
			m_iosvc->setDigitalOutput( m_laserDigital, false );
		}
		
		std::cout<<"panTiltController ready, PanTiltService @"<<m_ptsvc<<", IOService @"<<m_iosvc<<"\n";
	}

	void run(FleyeContext* ctx,int threadId)
	{		
		 //std::cout<<m_ControlX<<" / "<<m_ControlY<<"\n";
		 if( m_ptsvc->pan() < 0.0 ) m_ptsvc->setPan(0.0);
		 else if( m_ptsvc->pan() > 1.0 ) m_ptsvc->setPan(1.0);
		 if( m_ptsvc->tilt() < 0.0 ) m_ptsvc->setTilt(0.0);
		 else if( m_ptsvc->tilt() > 1.0 ) m_ptsvc->setTilt(1.0);
		 
		if( m_panAnalog<m_iosvc->getNumberOfAnalogOutputs() )
		{
			m_iosvc->setAnalogOutput( m_panAnalog, m_ptsvc->pan() );
		}
		if( m_tiltAnalog<m_iosvc->getNumberOfAnalogOutputs() )
		{
			m_iosvc->setAnalogOutput( m_tiltAnalog, m_ptsvc->tilt() );
		}
		if( m_laserAnalog<m_iosvc->getNumberOfAnalogOutputs() )
		{
			float l = m_ptsvc->laser() ? 1.f : 0.f ;
			m_iosvc->setAnalogOutput( m_laserAnalog, (std::sin(ctx->frameCounter*0.1)*0.5+0.5)*l );
		}
		if( m_laserDigital<m_iosvc->getNumberOfDigitalOutputs() )
		{
			m_iosvc->setDigitalOutput( m_laserDigital, m_ptsvc->laser() );
		}
	}
	
	PanTiltService* m_ptsvc;
	IOService* m_iosvc;
	int m_panAnalog, m_tiltAnalog, m_laserAnalog, m_laserDigital;
};

FLEYE_REGISTER_PLUGIN(panTiltController);

