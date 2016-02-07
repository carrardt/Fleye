#include "fleye/plugin.h"
#include "fleye/FleyeContext.h"

#include "services/PanTiltService.h"
#include "services/TrackingService.h"
#include "services/TextService.h"
#include <cmath>
#include <iostream>
#include <sstream>
#include <fstream>
#include <json/json.h>
#include <vector>
#include <algorithm>

struct TrackingSample
{
	float Px,Py;
	float dPxdCx, dPydCx;
	float dPxdCy, dPydCy;
};

struct CalibrationSample
{
	float Cx,Cy;
	std::vector<TrackingSample> m_track;
};

struct PanTiltFollower : public FleyePlugin
{	
	inline PanTiltFollower()
		: m_ptsvc(0)
		, m_txt(0)
	{
	}
	
	void setup(FleyeContext* ctx)
	{
		m_ptsvc = PanTiltService_instance();
		m_obj = TrackingService_instance()->getTrackedObject(0);
		m_txt = TextService_instance()->addPositionnedText(0.2,0.2);
		m_txt->out()<<"Initializing...";

		std::ifstream dataFile(std::string(FLEYE_DATA_DIR)+"/calibration.json");
		Json::Value root; 
		Json::Reader reader;
		bool parsingSuccessful = reader.parse( dataFile, root );
		if ( !parsingSuccessful ){
			// report to the user the failure and their locations in the document.
			std::cerr<< "Failed to parse calibration file\n"<< reader.getFormattedErrorMessages();
			return ;
		}
		
		for( auto cycleName : root.getMemberNames() )
		{
			std::cout<<"cycle '"<<cycleName<<"'\n";
			Json::Value cycle = root[cycleName];
			CalibrationSample csample;
			for( auto trackName : cycle.getMemberNames() )
			{
				std::cout<<"\ttrack '"<<trackName<<"'\n";
				Json::Value track = cycle[trackName];
				TrackingSample sample;
				csample.Cx = track["Cx"].asDouble();
				csample.Cy = track["Cy"].asDouble();
				sample.Px = track["Px"].asDouble();
				sample.Py = track["Py"].asDouble();
				sample.dPxdCx = track["dPxdCx"].asDouble();
				sample.dPydCx = track["dPydCx"].asDouble();
				sample.dPxdCy = track["dPxdCy"].asDouble();
				sample.dPydCy = track["dPydCy"].asDouble();
				csample.m_track.push_back( sample );
			}
			m_samples.push_back(csample);
		}

		std::cout<<"PanTiltFollower ready : PanTiltService @"<<m_ptsvc<<", obj @"<<m_obj<< "\n";
	}

	void run(FleyeContext* ctx,int threadId)
	{
		float W = m_obj->weight;
		if(W==0.0f) W=1.0f;
		else W=1.0f/W;
		float px = m_obj->posX * W;
		float py = m_obj->posY * W;
		float cx = m_ptsvc->pan();
		float cy = m_ptsvc->tilt();

		std::sort( m_samples.begin(), m_samples.end() ,
			[cx,cy] (CalibrationSample s1, CalibrationSample s2) -> bool
			{
				float v1[2] = {s1.Cx-cx,s1.Cy-cy};
				float v2[2] = {s2.Cx-cx,s2.Cy-cy};
				float d1 = v1[0]*v1[0] + v1[1]*v1[1];
				float d2 = v2[0]*v2[0] + v2[1]*v2[1];
				return d1 < d2;
			}
			);

		CalibrationSample s1 = m_samples[0];
		CalibrationSample s2 = m_samples[1];
		CalibrationSample s3 = m_samples[2];

		m_txt->out() <<"s1=("<<s1.Cx<<","<<s1.Cy<<") s2=("<<s2.Cx<<","<<s2.Cy<<") s3=("<<s3.Cx<<","<<s3.Cy<<")";
	}

	PanTiltService* m_ptsvc;
	PositionnedText* m_txt;
	TrackedObject* m_obj;
	std::vector<CalibrationSample> m_samples;
};

FLEYE_REGISTER_PLUGIN(PanTiltFollower);

