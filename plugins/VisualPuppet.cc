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

struct VisualPuppet : public FleyePlugin
{	
	inline VisualPuppet()
		: m_ptsvc(0)
		, m_txt(0)
		, m_obj1(0)
		, m_obj2(0)
	{
	}
	
	void setup(FleyeContext* ctx)
	{
		m_ptsvc = PanTiltService_instance();
		m_ptsvc->setNumberOfControllers(2);
		m_obj1 = TrackingService_instance()->getTrackedObject(0);
		m_obj2 = TrackingService_instance()->getTrackedObject(1);
		m_txt = TextService_instance()->addPositionnedText(0.1,0.2);
		m_txt->out()<<"Initializing...";
		std::cout<<"PanTiltFollower ready : PanTiltService @"<<m_ptsvc<<", obj1 @"<<m_obj1<<", obj2 @"<<m_obj2<< "\n";
	}

	void run(FleyeContext* ctx,int threadId)
	{
		float W1 = m_obj1->weight;
		float W2 = m_obj2->weight;
		if( W1<1.0f ) W1=1.0f;
		if( W2<1.0f ) W2=1.0f;
		W1=1.0f/W1;
		W2=1.0f/W2;
		
		float xf1 = m_obj1->posX * W1;
		float yf1 = m_obj1->posY * W1;
		float xf2 = m_obj2->posX * W2;
		float yf2 = m_obj2->posY * W2;

		m_txt->out() << "X1 = "<<xf1<<"\nY1 = "<<yf1<<"\nX2 = "<<xf2<<"\nY2 = "<<yf2;

		m_ptsvc->setPan( xf1, 0 );
		m_ptsvc->setTilt( yf1, 0 );
		m_ptsvc->setPan( xf2, 1 );
		m_ptsvc->setTilt( yf2, 1 );
	}

	PanTiltService* m_ptsvc;
	PositionnedText* m_txt;
	TrackedObject* m_obj1;
	TrackedObject* m_obj2;
};

FLEYE_REGISTER_PLUGIN(VisualPuppet);

