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
	inline drawTrackingPos() : m_track_svc(0) {}

	void setup(struct FleyeContext* ctx)
	{
		m_track_svc = TrackingService_instance();
		std::cout<<"drawTrackingPos: Tracking service @"<<m_track_svc<<"\n";
	}

	void draw(struct FleyeContext* ctx, CompiledShader* cs, int pass)
	{
		cs->enableVertexArray(FLEYE_GL_VERTEX);
		cs->enableVertexArray(FLEYE_GL_COLOR);
		
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
			drawCross(cs,x,y,h);
			++i;
		}

		cs->disableVertexArray(FLEYE_GL_COLOR);
		cs->disableVertexArray(FLEYE_GL_VERTEX);
	}

	void drawCross(struct CompiledShader* cs, float posx, float posy, float hue)
	{
		GLfloat varray[8];
		GLfloat carray[16];
		for(int i=0;i<4;i++)
		{
			int x = i%2;
			int y = ((i/2)+x)%2;
			double ox = x ? -0.05 : 0.05;
			double oy = y ? -0.05 : 0.05;
			varray[i*2+0] = posx +ox;
			varray[i*2+1] = posy +oy;
			
			carray[i*4+0] = 1.0f-hue;
			carray[i*4+1] = hue;
			carray[i*4+2] = fabsf(0.5f-hue);
			carray[i*4+3] = 1.0f;
		}
		cs->vertexAttribPointer(FLEYE_GL_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, varray);
		cs->vertexAttribPointer(FLEYE_GL_COLOR, 4, GL_FLOAT, GL_FALSE, 0, carray);
		glDrawArrays(GL_LINES, 0, 4);
	}
	
	TrackingService* m_track_svc;
};

FLEYE_REGISTER_PLUGIN(drawTrackingPos);

