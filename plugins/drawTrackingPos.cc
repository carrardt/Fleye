#include "fleye/plugin.h"
#include "fleye/compiledshader.h"
#include "fleye/FleyeContext.h"
#include "fleye/imageprocessing.h"
#include "fleye/cpuworker.h"
#include "services/TrackingService.h"

#include <iostream>
#include <math.h>

/*
 * use it with zhue_vs vertex shader
 */

struct drawTrackingPos : public FleyePlugin
{
	static constexpr int BufferVertices = 256;
	
	inline drawTrackingPos() : m_track_svc(0), m_bufVertices(0), m_varray(0), m_carray(0) {}

	void setup(struct FleyeContext* ctx)
	{
		m_track_svc = TrackingService_instance();
		m_varray = new GLfloat[BufferVertices*2];
		m_carray = new GLfloat[BufferVertices*3];
		std::cout<<"drawTrackingPos: Tracking service @"<<m_track_svc<<"\n";
	}

	void draw(struct FleyeContext* ctx, CompiledShader* cs, int pass)
	{
		cs->enableVertexArray(FLEYE_GL_VERTEX);
		cs->enableVertexArray(FLEYE_GL_COLOR);
		
		cs->vertexAttribPointer(FLEYE_GL_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, m_varray);
		cs->vertexAttribPointer(FLEYE_GL_COLOR, 3, GL_FLOAT, GL_FALSE, 0, m_carray);

		int i=0;
		//std::cout<<"m_track_svc="<<m_track_svc<<"\n";
		float N = m_track_svc->getTrackedObjects().size();
		for( auto p : m_track_svc->getTrackedObjects() )
		{
			float W = p.second->weight;
			if(W==0.0f) W=1.0f;
			W = 1.0f/W;
			float x = ( p.second->posX*W - 0.5f ) * 2.0f ;
			float y = ( p.second->posY*W - 0.5f ) * 2.0f ;
			float h = i / N;
			//std::cout<<i<<" : x="<<x<<", y="<<y<<", W="<<W<< "\n";
			drawObject(cs,x,y,p.second->speedX, p.second->speedY, h);
			++i;
		}
		flushVertices();
		
		cs->disableVertexArray(FLEYE_GL_COLOR);
		cs->disableVertexArray(FLEYE_GL_VERTEX);
	}

	void flushVertices()
	{
		if(m_bufVertices>0)
		{
			glDrawArrays(GL_LINES, 0, m_bufVertices);
			m_bufVertices = 0;
		}
	}

	void drawObject(struct CompiledShader* cs, float posx, float posy, float sx, float sy, float hue)
	{
		int nv = 4;
		float color[3] = { 1.0f-hue, hue, fabsf(0.5f-hue) };
		for(int i=0;i<4;i++)
		{
			int x = i%2;
			int y = ((i/2)+x)%2;
			double ox = x ? -0.05 : 0.05;
			double oy = y ? -0.05 : 0.05;
			m_varray[m_bufVertices*2+0] = posx +ox;
			m_varray[m_bufVertices*2+1] = posy +oy;
			for(int i=0;i<3;i++) m_carray[m_bufVertices*3+i] = color[i];
			++ m_bufVertices;
		}
		if( sx!=0.0 || sy!=0.0 )
		{
			m_varray[m_bufVertices*2+0] = posx;
			m_varray[m_bufVertices*2+1] = posy;
			for(int i=0;i<3;i++) m_carray[m_bufVertices*3+i] = color[i];
			++m_bufVertices;
			m_varray[m_bufVertices*2+0] = posx+sx;
			m_varray[m_bufVertices*2+1] = posy+sy;
			for(int i=0;i<3;i++) m_carray[m_bufVertices*3+i] = color[i];
			++m_bufVertices;
		}
		if( m_bufVertices >= (BufferVertices-6) ) { flushVertices(); }
	}
	
	TrackingService* m_track_svc;
	int m_bufVertices; 
	GLfloat* m_varray;
	GLfloat* m_carray;
};

FLEYE_REGISTER_PLUGIN(drawTrackingPos);

