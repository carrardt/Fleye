#include "fleye/plugin.h"
#include "fleye/FleyeContext.h"
#include "services/IOService.h"

#include "Linkuino/LinkuinoClient.h"

#include <cmath>
#include <iostream>

/* 
 * Rapsberry Pi GPIO based controller exposes 2 analog outputs
 */

struct LinkuinoController : public FleyePlugin
{
	inline LinkuinoController() : m_iosvc(0), m_link(0) {}
	
	void setup(FleyeContext* ctx)
	{
		std::string serialDevPath = ctx->vars["SERIAL"];
		if( serialDevPath.empty() ) serialDevPath = "/dev/ttyUSB0";

		std::cout<<"LinkuinoController: open serial device '"<<serialDevPath<<"'\n";
		int serial_fd = LinkuinoClient::openSerialDevice( serialDevPath.c_str() );
		if( serial_fd < 0 )
		{
			std::cerr << "Error opening serial device.\n";
			return ;
		}
		m_link = new LinkuinoClient(serial_fd);
		
		m_iosvc = IOService_instance();
		m_iosvc->setNumberOfAnalogOutputs( Linkuino::PWM_COUNT );
		m_iosvc->setNumberOfDigitalOutputs( 6 );
		
		write_linkuino();
		
		std::cout<<"LinkuinoController IOService @"<<m_iosvc<<", LinkuinoClient @"<<m_link<<"\n";
	}

	void run(FleyeContext* ctx,int threadId)
	{
		write_linkuino();
	}
	
	void write_linkuino()
	{
		int na = m_iosvc->getNumberOfAnalogOutputs();
		if( na > Linkuino::PWM_COUNT ) na = Linkuino::PWM_COUNT;
		for(int i=0;i<na;i++)
		{
			int32_t value = m_iosvc->getAnalogOutput(i) * 1500 + 500;
			if( value < 500 ) value = 500;
			if( value > 2000 ) value = 2000;
			m_link->setPWMValue( i, value );
		}
		int nd = m_iosvc->getNumberOfDigitalOutputs();
		if( nd > 8 ) nd = 8;
		uint8_t bitfield = 0;
		for(int i=0;i<nd;i++)
		{
			bitfield <<= 1;
			bitfield |= m_iosvc->getDigitalOutput(nd-i-1) ? 1 : 0;
		}
		m_link->setRegisterValue( Linkuino::DOUT_ADDR , bitfield );
		m_link->send();
	}
	
	IOService* m_iosvc;
	LinkuinoClient* m_link;
};

FLEYE_REGISTER_PLUGIN(LinkuinoController);

