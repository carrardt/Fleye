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

	static inline ScreenSample interpolate(const ScreenSample& s1, const ScreenSample& s2, float t)
	{
		ScreenSample r;
		r.P = s1.P*(1.0f-t) + s2.P*t;
		r.dPdCx = s1.dPdCx*(1.0f-t) + s2.dPdCx*t;
		r.dPdCy = s1.dPdCy*(1.0f-t) + s2.dPdCy*t;
		return r;
	}

	struct CalibrationSample
	{
		Vec2f C;
		std::vector<ScreenSample> sgrid;
	};

	static inline CalibrationSample interpolate(const CalibrationSample& c1, const CalibrationSample& c2, float t)
	{
		int n = c1.sgrid.size();
		assert( n == c2.sgrid.size() );
		CalibrationSample r;
		r.sgrid.resize( n );
		r.C = c1.C*(1.0f-t) + c2.C*t;
		for(int i=0;i<n;i++) r.sgrid[i] = interpolate(c1.sgrid[i],c2.sgrid[i],t);
		return r;
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
	
	inline CalibrationSample& cgrid(int i,int j)
	{ 
		return m_cgrid[j*m_nci+i]; 
	}
	
	inline ScreenSample& sgrid( CalibrationSample& c, int i, int j )
	{
		return c.sgrid[j*m_npi+i];
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
		const Vec2f target( 0.5f, 0.5f );
		
		float W = m_obj->weight;
		if( W < 1.e-9f ) return;
		W=1.0f/W;
		float px = m_obj->posX * W;
		float py = m_obj->posY * W;

		Vec2f dP = target - Vec2f(px,py);
		if( dP.norm2() < 0.001 ) return;
		
		float cx = m_ptsvc->pan();
		float cy = m_ptsvc->tilt();

		int ci=0, cj=0;
		while( ci<(m_nci-2) && cgrid(ci,cj).C.x > cx ) ++ci;
		while( cj<(m_ncj-2) && cgrid(ci,cj).C.y > cy ) ++cj;
		
		// ensure this is an orthogonal grid
		assert( cgrid(ci,cj).C.y == cgrid(ci+1,cj).C.y );
		assert( cgrid(ci,cj+1).C.y == cgrid(ci+1,cj+1).C.y );
		assert( cgrid(ci,cj).C.x == cgrid(ci,cj+1).C.x );
		assert( cgrid(ci+1,cj).C.x == cgrid(ci+1,cj+1).C.x );
		
		float tcx = ( cx - cgrid(ci,cj).C.x ) / ( cgrid(ci+1,cj).C.x - cgrid(ci,cj).C.x ) ;
		float tcy = ( cy - cgrid(ci,cj).C.y ) / ( cgrid(ci,cj+1).C.y - cgrid(ci,cj).C.y ) ;
		
		CalibrationSample c1 = interpolate( cgrid(ci,cj) , cgrid(ci+1,cj) , tcx );
		CalibrationSample c2 = interpolate( cgrid(ci,cj+1) , cgrid(ci+1,cj+1) , tcx );
		CalibrationSample c = interpolate( c1, c2, tcy );
		
		int pi=0,pj=0;
		while( pi<(m_npi-2) && sgrid(c,pi,pj).P.x > px ) ++pi;
		while( pj<(m_npj-2) && sgrid(c,pi,pj).P.y > py ) ++pj;
		
		// ensure this is an orthogonal grid
		assert( sgrid(c,pi,pj).P.y == sgrid(c,pi+1,pj).P.y );
		assert( sgrid(c,pi,pj+1).P.y == sgrid(c,pi+1,pj+1).P.y );
		assert( sgrid(c,pi,pj).P.x == sgrid(c,pi,pj+1).P.x );
		assert( sgrid(c,pi+1,pj).P.x == sgrid(c,pi+1,pj+1).P.x );

		float tpx = ( px - sgrid(c,pi,pj).P.x ) / ( sgrid(c,pi+1,pj).P.x - sgrid(c,pi,pj).P.x );
		float tpy = ( py - sgrid(c,pi,pj).P.y ) / ( sgrid(c,pi,pj+1).P.y - sgrid(c,pi,pj).P.y );

		ScreenSample s1 = interpolate( sgrid(c,pi,pj) , sgrid(c,pi+1,pj) , tpx );
		ScreenSample s2 = interpolate( sgrid(c,pi,pj+1) , sgrid(c,pi+1,pj+1) , tpx );
		ScreenSample s = interpolate( s1 , s2 , tpy );
		
		float nCx = dot( s.dPdCx , dP );
		float nCy = dot( s.dPdCy , dP );
		
		m_txt->out() <<"nCx="<<nCx<<"\nnCy="<<nCy;

		m_ptsvc->setPan( m_ptsvc->pan() -nCx*0.04 );
		m_ptsvc->setTilt( m_ptsvc->tilt() -nCy*0.04 );
	}

	PanTiltService* m_ptsvc;
	PositionnedText* m_txt;
	TrackedObject* m_obj;
	std::vector<CalibrationSample> m_cgrid;
	int m_nci, m_ncj, m_npi, m_npj;
};

FLEYE_REGISTER_PLUGIN(PanTiltFollower);

