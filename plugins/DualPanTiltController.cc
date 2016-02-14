#include "fleye/plugin.h"
#include "fleye/FleyeContext.h"

#include "gpio/gpioController.h"
#include "services/PanTiltService.h"
#include <cmath>
#include <iostream>

#define SERVO_X_VALUE_MIN 0
#define SERVO_X_VALUE_MAX 1023

#define SERVO_Y_VALUE_MIN 0
#define SERVO_Y_VALUE_MAX 1023

#define GPIO_AXIS_BITS 	10
#define GPIO_LASER_BIT 	(2*GPIO_AXIS_BITS)
#define GPIO_DATA_BITS 	(GPIO_LASER_BIT+1)

struct DualPanTiltController : public FleyePlugin
{
	inline DualPanTiltController() : m_ptsvc(0) {}

	void setup(FleyeContext* ctx)
	{
		for(int i=0;i<GPIO_DATA_BITS;i++)
		{
			gpio_set_mode(i,OUTPUT_MODE);
		}
		m_ptsvc = PanTiltService_instance();
		m_ptsvc->setNumberOfControllers(2);
		gpio_write_xy_f( m_ptsvc->pan(0), m_ptsvc->tilt(0) , m_ptsvc->pan(1), m_ptsvc->tilt(1) );
		std::cout<<"panTiltController ready, PanTiltService @"<<m_ptsvc<<"\n";
	}

	void run(FleyeContext* ctx,int threadId)
	{		
		gpio_write_xy_f( m_ptsvc->pan(0), m_ptsvc->tilt(0) , m_ptsvc->pan(1), m_ptsvc->tilt(1) );
	}

	void gpio_write_xy_i(unsigned int xi1, unsigned int yi1, unsigned int xi2, unsigned int yi2)
	{
		uint32_t bits;
		if( xi1>=32 ) xi1=32;
		if( yi1>=32 ) yi1=32;
		if( xi2>=32 ) xi2=32;
		if( yi2>=32 ) yi2=32;
		bits = (yi1<<15) | (xi1<<10) | (yi2<<5) | xi2;
		gpio_write_bits(bits);
	}

	static inline float clamp(float x, float l, float h)
	{
		if( x<l ) return l;
		if( x>h ) return h;
		return x;
	}

	void gpio_write_xy_f(float xf1, float yf1, float xf2, float yf2)
	{
		unsigned int xi1,yi1, xi2,yi2;

		xi1 = (unsigned int)( clamp(xf1,0,1) * 32 );
		yi1 = (unsigned int)( clamp(yf1,0,1) * 32 );
		xi2 = (unsigned int)( clamp(xf2,0,1) * 32 );
		yi2 = (unsigned int)( clamp(yf2,0,1) * 32 );

		gpio_write_xy_i(xi1,yi1,xi2,yi2);
	}

	PanTiltService* m_ptsvc;
};

FLEYE_REGISTER_PLUGIN(DualPanTiltController);

