#include "fleye/plugin.h"
#include "fleye/FleyeContext.h"

#include "gpio/gpioController.h"
#include "services/PanTiltService.h"
#include <cmath>
#include <iostream>

#define SERVO_X_VALUE_MIN 256
#define SERVO_X_VALUE_MAX 768

#define SERVO_Y_VALUE_MIN 256
#define SERVO_Y_VALUE_MAX 768

#define GPIO_AXIS_BITS 	10
#define GPIO_LASER_BIT 	(2*GPIO_AXIS_BITS)
#define GPIO_DATA_BITS 	(GPIO_LASER_BIT+1)

struct panTiltController : public FleyePlugin
{
	inline panTiltController() : m_ptsvc(0) {}
	
	void setup(FleyeContext* ctx)
	{
		for(int i=0;i<GPIO_DATA_BITS;i++)
		{
			gpio_set_mode(i,OUTPUT_MODE);
		}
		m_ptsvc = PanTiltService_instance();
		gpio_write_xy_f( m_ptsvc->pan(), m_ptsvc->tilt(), 0);
		std::cout<<"panTiltController ready, PanTiltService @"<<m_ptsvc<<"\n";
	}

	void run(FleyeContext* ctx,int threadId)
	{		
		 //std::cout<<m_ControlX<<" / "<<m_ControlY<<"\n";
		 gpio_write_xy_f( m_ptsvc->pan(), m_ptsvc->tilt(), 0);
	}
	
	void gpio_write_xy_i(unsigned int xi, unsigned int yi, bool laser)
	{
		uint32_t bits;
		if( xi>=1024 ) xi=1023;
		if( yi>=1024 ) yi=1023;
		bits = (yi<<GPIO_AXIS_BITS) | xi ;
		bits |= laser ? 1UL<<GPIO_LASER_BIT : 0;
		gpio_write_bits(bits);
	}

	void gpio_write_xy_f(float xf, float yf, bool laser)
	{
		unsigned int xi,yi;

		if(xf<0.0f) xf=0.0f;
		else if(xf>1.0f) xf=1.0f;

		if(yf<0.0f) yf=0.0f;
		else if(yf>1.0f) yf=1.0f;

		xi = SERVO_X_VALUE_MIN + (unsigned int)( xf * (SERVO_X_VALUE_MAX-SERVO_X_VALUE_MIN) );
		yi = SERVO_Y_VALUE_MIN + (unsigned int)( yf * (SERVO_Y_VALUE_MAX-SERVO_Y_VALUE_MIN) );

		gpio_write_xy_i(xi,yi,laser);
	}

	PanTiltService* m_ptsvc;
};

FLEYE_REGISTER_PLUGIN(panTiltController);

