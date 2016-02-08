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
	struct Vec2f
	{
		float x,y;

		inline Vec2f() : x(0.0f) , y(0.0f) {}
		inline Vec2f(float u, float v) : x(u) , y(v) {}
		inline Vec2f(const Vec2f& v) : x(v.x) , y(v.y) {}

		inline Vec2f operator + (Vec2f v) const { return Vec2f(x+v.x,y+v.y); }
		inline Vec2f operator - (Vec2f v) const { return Vec2f(x-v.x,y-v.y); }
		inline Vec2f operator * (float s) const { return Vec2f(x*s,y*s); }
		inline Vec2f operator / (float s) const { return Vec2f(x/s,y/s); }
		
		inline float norm2() const { return x*x+y*y; }
		inline float norm() const { return sqrtf( norm2() ); }
		inline float dist2(Vec2f v) const {	return (v-*this).norm2(); }
		inline float dist(Vec2f v) const {	return (v-*this).norm(); }
		
		inline Vec2f normalize() const { return *this / norm(); }
	};
	
	static inline float dot(Vec2f u, Vec2f v) { return u.x * v.x + u.y * v.y; }

	static inline Vec2f weightedSum( Vec2f x1, Vec2f x2, Vec2f x3, float w1, float w2, float w3 )
	{
		return x1*w1 + x2*w2 + x3*w3;
	}
	
	struct ScreenSample
	{
		Vec2f P;
		Vec2f dPdCx;
		Vec2f dPdCy;
		
		inline void normalize()
		{
			dPdCx = dPdCx.normalize();
			dPdCy = dPdCy.normalize();
		}
		
		inline void print()
		{
			std::cout<<"P=("<<P.x<<','<<P.y<<") dPdCx=("<<dPdCx.x<<','<<dPdCx.y<<") dPdCy=("<<dPdCy.x<<','<<dPdCy.y<<")\n";
		}
	};

	static inline ScreenSample weightedSum( ScreenSample s1, ScreenSample s2, ScreenSample s3, float w1, float w2, float w3 )
	{
		ScreenSample r;
		r.P = weightedSum( s1.P, s2.P, s3.P, w1, w2, w3 );
		r.dPdCx = weightedSum( s1.dPdCx, s2.dPdCx, s3.dPdCx, w1, w2, w3 );
		r.dPdCy = weightedSum( s1.dPdCy, s2.dPdCy, s3.dPdCy, w1, w2, w3 );
		return r;
	}

	struct CalibrationSample
	{
		Vec2f C;		
		std::vector<ScreenSample> sgrid;
	};

	static inline float lineDist(Vec2f m1, Vec2f m2, Vec2f p)
	{
		Vec2f N( m1.y - m2.y , m2.x - m1.x );
		// with this commented out, it becomes an oriented area, which is what we want for barycentric coordinates
		//N = N.normalize();
		float c = - dot(N,m1); 
		return dot(N,p) + c;
	}

	inline PanTiltFollower()
		: m_ptsvc(0)
		, m_txt(0)
		, m_nci(0)
		, m_ncj(0)
		, m_npi(0)
		, m_npj(0)
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
		
		Json::Value layout = root["GridLayout"];
		m_nci = layout["ControlGridX"].asInt();
		m_ncj = layout["ControlGridY"].asInt();
		m_npi = layout["ScreenGridX"].asInt();
		m_npj = layout["ScreenGridY"].asInt();
		std::cout<<"nCi="<<m_nci<<" nCj="<<m_ncj<<" nPi="<<m_npi<<" nPj="<<m_npj;
		m_cgrid.resize( m_nci * m_ncj );
		
		Json::Value data = root["CalibrationData"];
		for( const Json::Value& cycle : data )
		{
			int i = cycle["Ci"].asInt();
			int j = cycle["Cj"].asInt();
			int k = j * m_nci + i;
			m_cgrid[k].C.x = cycle["Cx"].asDouble();
			m_cgrid[k].C.y = cycle["Cy"].asDouble();
			std::cout<<"cycle : Ci="<<i<<", j="<<j<<", Cx="<<m_cgrid[k].C.x<<", Cy="<<m_cgrid[k].C.y<<"\n";
			m_cgrid[k].sgrid.resize( m_npi*m_npj );
			Json::Value trackList = cycle["Samples"];
			for( const Json::Value& track : trackList )
			{
				int Pi = track["Pi"].asInt();
				int Pj = track["Pj"].asInt();
				std::cout<<"track "<<Pi<<","<<Pj<<"\n";
				int Pk = Pj * m_npi + Pi;
				m_cgrid[k].sgrid[Pk].P.x = track["Px"].asDouble();
				m_cgrid[k].sgrid[Pk].P.y = track["Py"].asDouble();
				m_cgrid[k].sgrid[Pk].dPdCx.x = track["dPxdCx"].asDouble();
				m_cgrid[k].sgrid[Pk].dPdCx.y = track["dPydCx"].asDouble();
				m_cgrid[k].sgrid[Pk].dPdCy.x = track["dPxdCy"].asDouble();
				m_cgrid[k].sgrid[Pk].dPdCy.y = track["dPydCy"].asDouble();
				m_cgrid[k].sgrid[Pk].normalize();
				m_cgrid[k].sgrid[Pk].print();
			}
		}
		
		// TODO: tracked object have drawing style and color attributes
		auto cobj = TrackingService_instance()->getTrackedObject(66);
		cobj->posX = 0.5;
		cobj->posY = 0.5;

		std::cout<<"PanTiltFollower ready : PanTiltService @"<<m_ptsvc<<", obj @"<<m_obj<< "\n";

	}

	void run(FleyeContext* ctx,int threadId)
	{
#if 0
		const float targetX = 0.5;
		const float targetY = 0.5;
		
		float W = m_obj->weight;
		if( W < 1.e-9f ) return;
		W=1.0f/W;
		float px = m_obj->posX * W;
		float py = m_obj->posY * W;

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
#endif
	}

	PanTiltService* m_ptsvc;
	PositionnedText* m_txt;
	TrackedObject* m_obj;
	std::vector<CalibrationSample> m_cgrid;
	int m_nci, m_ncj, m_npi, m_npj;
};

FLEYE_REGISTER_PLUGIN(PanTiltFollower);

