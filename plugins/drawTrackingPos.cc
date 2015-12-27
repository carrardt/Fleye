#include "fleye/plugin.h"
#include "fleye/compiledshader.h"
#include "fleye/FleyeContext.h"
#include "fleye/imageprocessing.h"
#include "fleye/cpuworker.h"
#include "services/TrackingService.h"

#include <iostream>

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

	void draw(struct FleyeContext* ctx, CompiledShader* compiledShader, int pass)
	{
		glEnableVertexAttribArray(compiledShader->shader.attribute_locations[0]);
		
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
			drawCross(compiledShader,x,y,h);
			p.second->priority = 0;
			p.second->weight = 0;
			p.second->area = 0;
			p.second->posX = 0;
			p.second->posY = 0;
			++i;
		}

		glDisableVertexAttribArray(compiledShader->shader.attribute_locations[0]);
	}

	void drawCross(struct CompiledShader* cs, float posx, float posy, float hue)
	{
		GLfloat varray[12];
		for(int i=0;i<4;i++)
		{
			int x = i%2;
			int y = ((i/2)+x)%2;
			double ox = x ? -0.05 : 0.05;
			double oy = y ? -0.05 : 0.05;
			varray[i*3+0] = posx +ox;
			varray[i*3+1] = posy +oy;
			varray[i*3+2] = hue;
		}
		glVertexAttribPointer(cs->shader.attribute_locations[0], 3, GL_FLOAT, GL_FALSE, 0, varray);
		glDrawArrays(GL_LINES, 0, 4);
	}
	
	TrackingService* m_track_svc;
};

FLEYE_REGISTER_PLUGIN(drawTrackingPos);

