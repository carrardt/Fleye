#include "fleye/plugin.h"
#include "fleye/FleyeContext.h"
#include "gpio/gpioController.h"
#include "services/IOService.h"

#include <cmath>
#include <iostream>

/* 
 * Rapsberry Pi GPIO based controller exposes 2 analog outputs
 */

#define ANALOG_RESOLUTION 1024
#define ANALOG_BITS 10
#define ANALOG_OUTPUTS 2

struct GPIOController : public FleyePlugin
{
	inline GPIOController() : m_iosvc(0) {}
	
	void setup(FleyeContext* ctx)
	{
		// initialize gpio module
		init_gpio();
		for(int i=0;i<(ANALOG_OUTPUTS*ANALOG_BITS);i++)
		{
			gpio_set_mode(i,OUTPUT_MODE);
		}
		gpio_set_mode(ANALOG_OUTPUTS*ANALOG_BITS, OUTPUT_MODE);
		
		m_iosvc = IOService_instance();
		m_iosvc->setNumberOfAnalogOutputs( 2 );
		m_iosvc->setNumberOfDigitalOutputs( 1 );
		write_gpio();
		std::cout<<"GPIOController ready, IOService @"<<m_iosvc<<"\n";
	}

	void run(FleyeContext* ctx,int threadId)
	{		
		 //std::cout<<m_ControlX<<" / "<<m_ControlY<<"\n";
		 write_gpio();
	}
	
	void write_gpio()
	{
		float xf,yf;
		bool l;
		xf = m_iosvc->getAnalogOutput(0);
		yf = m_iosvc->getAnalogOutput(1);
		l = m_iosvc->getDigitalOutput(0);
		int32_t xi = xf*ANALOG_RESOLUTION;
		int32_t yi = yf*ANALOG_RESOLUTION;
		if(xi<0) xi=0;
		if(xi>=ANALOG_RESOLUTION) xi=ANALOG_RESOLUTION-1;
		if(yi<0) yi=0;
		if(yi>=ANALOG_RESOLUTION) yi=ANALOG_RESOLUTION-1;
		uint32_t bits = (yi<<ANALOG_BITS) | xi ;
		if( l ) bits |= 1<<(2*ANALOG_BITS);
		gpio_write_bits(bits);
	}

	IOService* m_iosvc;
};

FLEYE_REGISTER_PLUGIN(GPIOController);

