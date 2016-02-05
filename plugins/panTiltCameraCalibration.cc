#include "fleye/plugin.h"
#include "fleye/FleyeContext.h"

#include "services/PanTiltService.h"
#include "services/TrackingService.h"
#include "services/TextService.h"
#include <cmath>
#include <iostream>
#include <sstream>
#include <fstream>

struct panTiltCameraCalibration : public FleyePlugin
{
	static constexpr double PanTiltSpeed = 0.005;
	static constexpr int32_t PositionSteps = 3;
	static constexpr int32_t PositionMargin = 1;
	static constexpr int32_t PositionDivisions = PositionSteps+PositionMargin*2;
	static constexpr double DivisionSize = 1.0 / PositionDivisions;
	static constexpr int32_t CycleCount = PositionSteps*PositionSteps;
	
	inline panTiltCameraCalibration()
		: m_ptsvc(0)
		, m_tracksvc(0)
		, m_txt(0)
		, m_cycle(0)
		, m_iteration(0)
	{
		m_logFile.open("/tmp/calibration.json");
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
		int posCycle = m_cycle / 2;
		int dirCycle = m_cycle % 2;
		
		if( posCycle >= CycleCount ) { m_logFile.close(); return; }

		int iposY = PositionMargin + (posCycle/PositionSteps) - 1;
		int iposX = PositionMargin + (posCycle%PositionSteps) - 1;
		double startX = iposY * DivisionSize;
		double startY = iposX * DivisionSize;
		double dirX = dirCycle ? 0.0 : 2*DivisionSize;
		double dirY = dirCycle ? 2*DivisionSize : 0.0;
		double pathLength = sqrt(dirX*dirX+dirY*dirY);
		int32_t nbIteration = 30 + static_cast<int32_t>( pathLength / PanTiltSpeed );
		int32_t nbSteps = nbIteration-30;
		
		double* m_avgX = dirCycle ? m_dPxdCy : m_dPxdCx;
		double* m_avgY = dirCycle ? m_dPydCy : m_dPydCx;
		
		if(m_iteration==0)
		{
			std::ostringstream oss;
			oss <<"Cycle "<<posCycle<<", Dir "<<dirCycle;
			m_txt->setText( oss.str() );
			for(int i=0;i<4;i++)
			{
				m_avgX[i] = 0.0;
				m_avgY[i] = 0.0;
			}
			m_ptsvc->setPan( startX );
			m_ptsvc->setTilt( startY );
		}
		else if( m_iteration>=30 )
		{
			int32_t step = m_iteration-30;
			m_ptsvc->setPan( startX + (dirX*step)/nbSteps );
			m_ptsvc->setTilt( startY + (dirY*step)/nbSteps );
			for(int i=0;i<4;i++)
			{
				TrackedObject* obj = m_tracksvc->getTrackedObject(i);
				m_avgX[i] += obj->speedX;
				m_avgY[i] += obj->speedY;
			}
		}
		++ m_iteration;
		if( m_iteration >= nbIteration )
		{
			if( dirCycle )
			{
				std::ostringstream oss;
				oss<<"\"Cycle"<<posCycle<<"\" : {\n";
				for(int i=0;i<4;i++)
				{
					oss<<"\t\"Tracking"<<i<<"\" : {\n";
					TrackedObject* obj = m_tracksvc->getTrackedObject(i);
					oss<<"\t\t\"Cx\" : "<<startX+DivisionSize<<" , \"Cy\" : "<<startY+DivisionSize<<"\n";
					oss<<"\t\t\"Px\" : "<<obj->posX<<" , \"Py\" : "<<obj->posY<<"\n";
					double l = sqrt( m_dPxdCx[i]*m_dPxdCx[i] + m_dPydCx[i]*m_dPydCx[i] );
					oss<<"\t\t\"dPxdCx\" : "<<m_dPxdCx[i]<<" , \"dPydCx\" : "<<m_dPydCx[i]<<"\n";
					oss<<"\t\t\"dPxdCy\" : "<<m_dPxdCy[i]<<" , \"dPydCy\" : "<<m_dPydCy[i]<<"\n\t\t}\n";
				}
				oss<<"\t}\n\n";
				std::cout << oss.str();
				m_logFile << oss.str();
				m_logFile.flush();
			}
			++m_cycle;
			m_iteration=0;
		}
	}

	PanTiltService* m_ptsvc;
	TrackingService* m_tracksvc;
	PositionnedText* m_txt;
	uint32_t m_cycle;
	uint32_t m_iteration;
	double m_dPxdCx[4];
	double m_dPydCx[4];
	double m_dPxdCy[4];
	double m_dPydCy[4];

	std::ofstream m_logFile;
};

FLEYE_REGISTER_PLUGIN(panTiltCameraCalibration);

