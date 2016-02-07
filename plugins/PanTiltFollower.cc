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
#include <assert.h>

struct PanTiltFollower : public FleyePlugin
{	
	struct TrackingSample
	{
		float Px,Py;
		float dPxdCx, dPydCx;
		float dPxdCy, dPydCy;
		
		inline float dist2(float x, float y)
		{
			float vx = Px - x;
			float vy = Py - y;
			return vx*vx + vy*vy;
		}
		
		inline void normalize()
		{
			float s = 1.0f / sqrtf( dPxdCx*dPxdCx + dPydCx*dPydCx );
			dPxdCx *= s; dPydCx *= s;
			s = 1.0f / sqrtf( dPxdCy*dPxdCy + dPydCy*dPydCy );
			dPxdCy *= s; dPydCy *= s;
		}
		
		static inline TrackingSample interpolate( TrackingSample s1, TrackingSample s2, TrackingSample s3, float d1, float d2, float d3 )
		{
			TrackingSample r;
			r.Px = s1.Px * d1 + s2.Px * d2 + s3.Px * d3 ;
			r.Py = s1.Py * d1 + s2.Py * d2 + s3.Py * d3 ;
			r.dPxdCx = s1.dPxdCx * d1 + s2.dPxdCx * d2 + s3.dPxdCx * d3 ;
			r.dPydCx = s1.dPydCx * d1 + s2.dPydCx * d2 + s3.dPydCx * d3 ;
			r.dPxdCy = s1.dPxdCy * d1 + s2.dPxdCy * d2 + s3.dPxdCy * d3 ;
			r.dPydCy = s1.dPydCy * d1 + s2.dPydCy * d2 + s3.dPydCy * d3 ;
			return r;
		}
	};

	struct CalibrationSample
	{
		inline float dist2(float x, float y)
		{
			float vx = Cx - x;
			float vy = Cy - y;
			return vx*vx + vy*vy;
		}
		
		float Cx,Cy;
		std::vector<TrackingSample> m_track;
	};


	static inline float lineDist(float m1x, float m1y, float m2x, float m2y, float px, float py)
	{
		float a = m1y - m2y;
		float b = m2x - m1x;
		// with this commented out, it becomes an oriented area, which is what we want for barycentric coordinates
		/*float l = sqrtf( a*a + b*b );
		a /= l;
		b /= l;*/
		float c = - ( a*m1x + b*m1y );
		return a*px + b*py + c;
	}

	inline PanTiltFollower()
		: m_ptsvc(0)
		, m_txt(0)
		, m_px(0.5)
		, m_py(0.5)
	{
	}
	
	void setup(FleyeContext* ctx)
	{
		m_ptsvc = PanTiltService_instance();
		m_obj = TrackingService_instance()->getTrackedObject(0);
		m_txt = TextService_instance()->addPositionnedText(0.1,0.2);
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
				sample.normalize();
				csample.m_track.push_back( sample );
			}
			m_samples.push_back(csample);
		}

		auto cobj = TrackingService_instance()->getTrackedObject(66);
		cobj->posX = 0.5;
		cobj->posY = 0.5;

		std::cout<<"PanTiltFollower ready : PanTiltService @"<<m_ptsvc<<", obj @"<<m_obj<< "\n";
	}

	void run(FleyeContext* ctx,int threadId)
	{
		const float targetX = 0.5;
		const float targetY = 0.5;
		
		float W = m_obj->weight;
		if( W < 1.e-9f ) return;
		W=1.0f/W;
		float px = m_obj->posX * W;
		float py = m_obj->posY * W;
		m_px = px;
		m_py = py;

		float dPx = targetX - px;
		float dPy = targetY - py;
		float targetDist2 = dPx*dPx+dPy*dPy;
		if( targetDist2 < 0.001 ) return;
		
		float cx = m_ptsvc->pan();
		float cy = m_ptsvc->tilt();

		std::sort( m_samples.begin(), m_samples.end() ,
			[cx,cy] (CalibrationSample s1, CalibrationSample s2) -> bool { return s1.dist2(cx,cy) < s2.dist2(cx,cy); }
			);

		CalibrationSample s1 = m_samples[0];
		CalibrationSample s2 = m_samples[1];
		CalibrationSample s3 = m_samples[2];
		
		// ensure s1,s2 and s3 are ccw
		{
			float v1x = s2.Cx - s1.Cx;
			float v1y = s2.Cy - s1.Cy;
			float v2x = s3.Cx - s1.Cx;
			float v2y = s3.Cy - s1.Cy;
			if( (v1x*v2x+v1y*v2y) < 0.0f ) { std::swap(s2,s3); }
		}
		
		// compute barycentric coordinate
		float D1 = lineDist( s2.Cx, s2.Cy, s3.Cx, s3.Cy, s1.Cx, s1.Cy );
		float d1 = lineDist( s2.Cx, s2.Cy, s3.Cx, s3.Cy, cx, cy ) / D1;
		
		float D2 = lineDist( s3.Cx, s3.Cy, s1.Cx, s1.Cy, s2.Cx, s2.Cy );
		float d2 = lineDist( s3.Cx, s3.Cy, s1.Cx, s1.Cy, cx, cy ) / D2;

		float D3 = lineDist( s1.Cx, s1.Cy, s2.Cx, s2.Cy, s3.Cx, s3.Cy );
		float d3 = lineDist( s1.Cx, s1.Cy, s2.Cx, s2.Cy, cx, cy ) / D3;
		
		if( d1<0.0 || d2<0.0 || d3<0.0 )
		{
			std::cout<<"out of control space\n";
			return;
		}
		
		float Fx = cx - ( d1*s1.Cx + d2*s2.Cx + d3*s3.Cx ) ;
		float Fy = cy - ( d1*s1.Cy + d2*s2.Cy + d3*s3.Cy ) ;
		if( (Fx*Fx+Fy*Fy) > 1.e-9  )
		{
			std::cout<<"Barycentric error\n";
			return;
		}
		
		// now, screen space interpolation
		int nTrack = s1.m_track.size();
		assert( nTrack == s2.m_track.size() );
		assert( nTrack == s3.m_track.size() );
		m_itrack.resize( nTrack );
		for(int i=0;i<nTrack;i++)
		{
			assert( s1.m_track[i].Px == s2.m_track[i].Px );
			assert( s1.m_track[i].Py == s2.m_track[i].Py );
			assert( s1.m_track[i].Px == s3.m_track[i].Px );
			assert( s1.m_track[i].Py == s3.m_track[i].Py );
			m_itrack[i] = TrackingSample::interpolate(s1.m_track[i],s2.m_track[i],s3.m_track[i],d1,d2,d3);
		}
		
		std::sort( m_itrack.begin(), m_itrack.end() ,
			[px,py] (TrackingSample s1, TrackingSample s2) -> bool { return s1.dist2(px,py) < s2.dist2(px,py); }
			);
		
		TrackingSample ts1 = m_itrack[0];
		TrackingSample ts2 = m_itrack[1];
		TrackingSample ts3 = m_itrack[2];

		// ensure ts1, ts2 and ts3 are ccw
		{
			float v1x = ts2.Px - ts1.Px;
			float v1y = ts2.Py - ts1.Py;
			float v2x = ts3.Px - ts1.Px;
			float v2y = ts3.Py - ts1.Py;
			
			if( (v1x*v2x+v1y*v2y) < 0.0f ) { std::swap(ts2,ts3); }
		}
		
		float tD1 = lineDist( ts2.Px, ts2.Py, ts3.Px, ts3.Py, ts1.Px, ts1.Py );
		float td1 = lineDist( ts2.Px, ts2.Py, ts3.Px, ts3.Py, px, py ) / tD1;
		float tD2 = lineDist( ts3.Px, ts3.Py, ts1.Px, ts1.Py, ts2.Px, ts2.Py );
		float td2 = lineDist( ts3.Px, ts3.Py, ts1.Px, ts1.Py, px, py ) / tD2;
		float tD3 = lineDist( ts1.Px, ts1.Py, ts2.Px, ts2.Py, ts3.Px, ts3.Py );
		float td3 = lineDist( ts1.Px, ts1.Py, ts2.Px, ts2.Py, px, py ) / tD3;

		if( td1<0.0 || td2<0.0 || td3<0.0 )
		{
			std::cout<<"out of screen space\n";
			return;
		}

		Fx = px - ( td1*ts1.Px + td2*ts2.Px + td3*ts3.Px ) ;
		Fy = py - ( td1*ts1.Py + td2*ts2.Py + td3*ts3.Py ) ;
		if( (Fx*Fx+Fy*Fy) > 1.e-9  )
		{
			std::cout<<"Barycentric error\n";
			return;
		}
		

		TrackingSample F = TrackingSample::interpolate(ts1,ts2,ts3,td1,td2,td3);
		
		float nCx = F.dPxdCx * dPx + F.dPydCx * dPy ;
		float nCy = F.dPxdCy * dPx + F.dPydCy * dPy ;
		
		m_txt->out() <<"dPxdCx="<<F.dPxdCx<<"\ndPydCx="<<F.dPydCx
					<<"\ndPxdCy="<<F.dPxdCy<<"\ndPydCy="<<F.dPydCy
					<<"\nnCx="<<nCx<<"\nnCy="<<nCy;

		m_ptsvc->setPan( m_ptsvc->pan() -nCx*0.04 );
		m_ptsvc->setTilt( m_ptsvc->tilt() -nCy*0.04 );
		/*if( fabsf(nCx) > fabsf(nCy) )
		{			
			float dc = nCx>0 ? -0.002 : 0.002 ;
			m_ptsvc->setPan( m_ptsvc->pan() + dc );
		}
		else
		{
			float dc = nCy>0 ? -0.002 : 0.002 ;
			m_ptsvc->setTilt( m_ptsvc->tilt() + dc );
		}*/

	}

	PanTiltService* m_ptsvc;
	PositionnedText* m_txt;
	TrackedObject* m_obj;
	float m_px, m_py;
	std::vector<CalibrationSample> m_samples;
	std::vector<TrackingSample> m_itrack;
};

FLEYE_REGISTER_PLUGIN(PanTiltFollower);

