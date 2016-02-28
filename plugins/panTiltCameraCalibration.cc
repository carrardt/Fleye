#include "fleye/plugin.h"
#include "fleye/FleyeContext.h"

#include "services/PanTiltService.h"
#include "services/TrackingService.h"
#include "services/TextService.h"
#include <cmath>
#include <iostream>
#include <sstream>
#include <fstream>
#include <assert.h>

struct panTiltCameraCalibration : public FleyePlugin
{
	static constexpr int32_t SpeedTestCycles = 4;
	static constexpr int32_t SpeedTestStart = 400;
	static constexpr int32_t SpeedTestStep = 100;

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
		, m_calibrationMode(0)
		, m_controlSpeed(0.003)
		, m_panMin(0.2)
		, m_panMax(0.8)
		, m_tiltMin(0.2)
		, m_tiltMax(0.8)
	{
	}
	
	void setup(FleyeContext* ctx)
	{
		m_ptsvc = PanTiltService_instance();
		m_tracksvc = TrackingService_instance();
		m_txt = TextService_instance()->addPositionnedText(0.2,0.2);
		m_logFile.open("/tmp/calibration.json");
		m_logFile << "{\n\t\"GridLayout\" : { ";
		m_logFile << "\"ControlGridX\":" << PositionSteps << ", ";
		m_logFile << "\"ControlGridY\":" << PositionSteps << ", ";
		m_logFile << "\"ScreenGridX\":" << 2 << ", ";
		m_logFile << "\"ScreenGridY\":" << 2 << " }\n";
		m_logFile.flush();
		std::cout<<"panTiltCameraCalibration ready : PanTiltService @"<<m_ptsvc<<", TrackingService @"<<m_tracksvc<< "\n";
	}

	void run(FleyeContext* ctx,int threadId)
	{
		if( m_calibrationMode == 0 )
		{
			if(m_iteration==0)
			{
				m_txt->out() << "Speed Test "<<(m_cycle+1)<<" / "<<SpeedTestCycles;
				m_avgSpeed[m_cycle] = 0.0;
			}
			else
			{
				for( auto obj : m_tracksvc->getTrackedObjects() )
				{
					m_avgSpeed[m_cycle] += obj.second->speed();
				}
			}
			int32_t f = SpeedTestStart + (m_cycle)*SpeedTestStep;
			float theta = (m_iteration*2.0*M_PI)/f;
			m_ptsvc->setPan( cos(theta)*0.25+0.5 );
			m_ptsvc->setTilt( sin(theta)*0.25+0.5 );
			++ m_iteration;
			if( m_iteration >= f ) { m_avgSpeed[m_cycle]/=m_iteration; ++m_cycle; m_iteration=0; }
			
			if( m_cycle >= SpeedTestCycles )
			{
				int bestI = 0;
				double bestS = m_avgSpeed[0];
				for(int i=1;i<SpeedTestCycles;i++)
				{ 
					if( m_avgSpeed[i] > bestS ) { bestI=i; bestS=m_avgSpeed[i]; }
				}
				std::cout<<"best motion @ cycle "<<bestI<<"\n";
				double bestSpeedPerFrame = 0.5*M_PI / (SpeedTestStart+bestI*SpeedTestStep);
				std::cout<<"optimal motor control speed per frame = "<<bestSpeedPerFrame<<"\n";
				{
					m_logFile <<"\t,\"MotionMotorSpeed\" : " << bestSpeedPerFrame <<"\n";
					m_logFile.flush();
					m_txt->out() << "Speed = "<< bestSpeedPerFrame;
				}
				m_controlSpeed = bestSpeedPerFrame;
				++ m_calibrationMode;
				m_cycle = 0;
				m_iteration = 0;
			}
		}
		else if(m_calibrationMode==1)
		{
			float StartX = 0.5f;
			float StartY = 0.5f;
			bool axis = ( (m_cycle/2) == 0 );
			bool dir = ( (m_cycle%2) == 0 );
			int nbIteration = static_cast<int>( 0.5 / m_controlSpeed );
			int totalIteration = 30 + nbIteration;

			if( m_iteration == 0 ) { m_speedRecord.clear(); m_speedRecord.resize(nbIteration,-1.0);	}
			
			if( m_iteration<30 )
			{
				m_ptsvc->setPan( StartX );
				m_ptsvc->setTilt( StartY );
			}
			else if( m_iteration < totalIteration )
			{
				int step = m_iteration-30;
				float t = (step*0.5f)/(float)nbIteration;
				float d = dir ? t : -t;
				float posx=0.5f, posy=0.5f;
				if(axis) posy += d;
				else posx += d;
				m_ptsvc->setPan( posx );
				m_ptsvc->setTilt( posy );
				double Sx=0.0,Sy=0.0;
				for(int i=0;i<4;i++)
				{
					TrackedObject* obj = m_tracksvc->getTrackedObject(i);
					Sx += obj->speedX;
					Sy += obj->speedY;
				}
				assert( step < m_speedRecord.size() );
				m_speedRecord[step]=sqrt(Sx*Sx+Sy*Sy);
			}
			
			if(m_iteration >= totalIteration )
			{
				double minVal = 1.e18;
				double maxVal = 0.0;
				double avg=0.0;
				int N=m_speedRecord.size();
				int i=0;
				for(i=0;i<N;i++ )
				{
					double s = m_speedRecord[i];
					if( s >= 0.0 )
					{
						//std::cout<<i<<":"<<s<<" ";
						avg += s;
						if( s < minVal ) minVal = s;
						if( s > maxVal ) maxVal = s;
					}
				}
				//std::cout<<"\n";
				avg /= N;
				//std::cout<<"avg="<<avg;
				avg = (avg+minVal)*0.5;
				//std::cout<<", seuil="<<avg;
				i = N-1;
				int o=0;
				while( i>0 && o<2 )
				{
					if( m_speedRecord[i]>avg ) ++o;
					 --i;
				}
				//std::cout<<"limit: i="<<i<<", s="<<m_speedRecord[i]<<"\n";
				if( axis )
				{
					if( dir ) m_tiltMax = 0.5 + i*0.5/(double)N;
					else m_tiltMin = 0.5 - i*0.5/(double)N;
				}
				else
				{
					if( dir ) m_panMax = 0.5 + i*0.5/(double)N;
					else m_panMin = 0.5 - i*0.5/(double)N;
				}
				
				m_iteration=0;
				++ m_cycle;
			}
			else { ++m_iteration; }
			
			if( m_cycle >= 4 )
			{
				std::ostringstream oss;
				oss <<"\t,\"PanMin\" : " << m_panMin <<"\n";
				oss <<"\t,\"PanMax\" : " << m_panMax <<"\n";
				oss <<"\t,\"TiltMin\" : " << m_tiltMin <<"\n";
				oss <<"\t,\"TiltMax\" : " << m_tiltMax <<"\n";
				m_logFile << oss.str();
				m_logFile.flush();
				m_txt->setText( oss.str() );
				std::cout << oss.str();
				m_cycle = 0;
				m_iteration = 0;
				++m_calibrationMode;
			}
		}
		else if(m_calibrationMode==2)
		{
			if(m_iteration==0 && m_cycle==0)
			{
				
				m_logFile<<"\t,\n\t\"CalibrationData\":\n\t[\n";
				m_logFile.flush();
			}
			
			int posCycle = m_cycle / 2;
			int dirCycle = m_cycle % 2;
			
			if( posCycle == CycleCount )
			{
				m_logFile << "\t]\n}\n";
				m_logFile.close();
				std::cout<<"Calibration done\n";
				m_cycle += 2;
			};
			if( posCycle > CycleCount )
			{
				m_iteration=0; m_cycle=0; ++m_calibrationMode; return;
			}

			double panDivSize = DivisionSize * (m_panMax-m_panMin);
			double tiltDivSize = DivisionSize * (m_tiltMax-m_tiltMin);

			int posI = posCycle%PositionSteps;
			int posJ = posCycle/PositionSteps;
			int iposX = PositionMargin + posI - 1;
			int iposY = PositionMargin + posJ - 1;
			double startX = m_panMin + iposX * panDivSize;
			double startY = m_tiltMin + iposY * tiltDivSize;
			double dirX = dirCycle ? 0.0 : 2*panDivSize;
			double dirY = dirCycle ? 2*tiltDivSize : 0.0;
			double pathLength = sqrt(dirX*dirX+dirY*dirY);
			int32_t nbIteration = 30 + static_cast<int32_t>( pathLength / m_controlSpeed );
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
					oss<<"\t\t"<< ((posCycle==0)?" ":",") <<"\n\t\t{\n";
					oss<<"\t\t\t\"Ci\":"<<posI<<", \"Cj\":"<<posJ<<", \"Cx\":"<<startX+DivisionSize<<", \"Cy\":"<<startY+DivisionSize<<",\n";
					oss<<"\t\t\t\"Samples\" :\n\t\t\t[\n";
					for(int i=0;i<4;i++)
					{
						oss<<"\t\t\t\t"<<  ((i==0)?" ":",") <<"{ ";
						oss<<"\"Pi\":"<<i%2<<", \"Pj\":"<<i/2;
						TrackedObject* obj = m_tracksvc->getTrackedObject(i);
						oss<<", \"Px\":"<<obj->posX<<", \"Py\":"<<obj->posY;
						oss<<", \"dPxdCx\":"<<m_dPxdCx[i]<<", \"dPydCx\":"<<m_dPydCx[i];
						oss<<", \"dPxdCy\":"<<m_dPxdCy[i]<<", \"dPydCy\":"<<m_dPydCy[i]<<" }\n";
					}
					oss<<"\t\t\t]\n\t\t}\n";
					std::cout << oss.str();
					m_logFile << oss.str();
					m_logFile.flush();
				}
				++m_cycle;
				m_iteration=0;
			}
		}
	}

	PanTiltService* m_ptsvc;
	TrackingService* m_tracksvc;
	PositionnedText* m_txt;
	uint32_t m_cycle;
	uint32_t m_iteration;
	uint32_t m_calibrationMode;
	double m_avgSpeed[SpeedTestCycles];
	double m_controlSpeed;
	std::vector<float> m_speedRecord;
	double m_panMin;
	double m_panMax;
	double m_tiltMin;
	double m_tiltMax;
	double m_dPxdCx[4];
	double m_dPydCx[4];
	double m_dPxdCy[4];
	double m_dPydCy[4];

	std::ofstream m_logFile;
};

FLEYE_REGISTER_PLUGIN(panTiltCameraCalibration);

