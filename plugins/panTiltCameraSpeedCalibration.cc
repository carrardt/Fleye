#include "fleye/plugin.h"
#include "fleye/FleyeContext.h"

#include "services/PanTiltService.h"
#include "services/TrackingService.h"
#include "services/TextService.h"
#include <cmath>
#include <iostream>
#include <sstream>
#include <fstream>

struct panTiltCameraSpeedCalibration : public FleyePlugin
{
	static constexpr int32_t SpeedTestCycles = 3;
	static constexpr int32_t SpeedTestStart = 450;
	static constexpr int32_t SpeedTestStep = 100;
	inline panTiltCameraSpeedCalibration() : m_ptsvc(0), m_tracksvc(0), m_txt(0), m_cycle(0), m_iteration(0)
	{
		for(int i=0;i<SpeedTestCycles;i++) m_avgSpeed[i]=0.0;
	}
	
	void setup(FleyeContext* ctx)
	{
		m_ptsvc = PanTiltService_instance();
		m_tracksvc = TrackingService_instance();
		m_txt = TextService_instance()->addPositionnedText(0.2,0.2);
		std::cout<<"panTiltCameraCalibration ready : PanTiltService @"<<m_ptsvc<<", TrackingService @"<<m_tracksvc<< "\n";
	}

	void run(FleyeContext* ctx,int threadId)
	{
		int cycle = m_cycle;
		
		if( cycle < SpeedTestCycles )
		{
			if(m_iteration==0)
			{
				std::ostringstream oss;
				oss <<"Cycle "<<(cycle+1)<<" / "<<SpeedTestCycles;
				m_txt->setText( oss.str() );
			}
			else
			{
				for( auto obj : m_tracksvc->getTrackedObjects() )
				{
					if( obj.first != 0 ) { m_avgSpeed[cycle] += obj.second->speed(); }
				}
			}
			int32_t f = SpeedTestStart + (cycle)*SpeedTestStep;
			float theta = m_iteration*2.0*M_PI/f;
			m_ptsvc->setPan( cos(theta)*0.5+0.5 );
			m_ptsvc->setTilt( sin(theta)*0.5+0.5 );
			++ m_iteration;
			if( m_iteration >= f ) { m_avgSpeed[cycle]/=m_iteration; ++m_cycle; m_iteration=0; }
			return;
		}
		cycle -= SpeedTestCycles;
		
		if( m_iteration == 0 )
		{
			std::cout<<"Speed test :\n";
			int bestI = 0;
			double bestS=m_avgSpeed[0];
			for(int i=1;i<SpeedTestCycles;i++)
			{ 
				if( m_avgSpeed[i] > bestS ) { bestI=i; bestS=m_avgSpeed[i]; }
			}
			std::cout<<"best score @ cycle "<<bestI<<"\n";
			double bestSpeedPerFrame = M_PI / (SpeedTestStart+bestI*SpeedTestStep);
			std::cout<<"optimal motor speed per frame = "<<bestSpeedPerFrame<<"\n";
			{
				std::ostringstream oss;
				oss <<"{ \"PanTiltSpeed\" : " << bestSpeedPerFrame <<" }\n";
				std::ofstream file("/tmp/calibration.txt");
				file << oss.str(); file.flush(); file.close();
				m_txt->setText( oss.str() );
			}
			++ m_iteration;
		}
		
	}

	PanTiltService* m_ptsvc;
	TrackingService* m_tracksvc;
	PositionnedText* m_txt;
	uint32_t m_cycle;
	uint32_t m_iteration;
	double m_avgSpeed[SpeedTestCycles];
};

FLEYE_REGISTER_PLUGIN(panTiltCameraSpeedCalibration);

