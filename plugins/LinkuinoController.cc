#include "fleye/plugin.h"
#include "fleye/FleyeContext.h"
#include "services/IOService.h"
#include "services/TextService.h"
#include "Linkuino/LinkuinoClient.h"

#include "services/MotorDriveService.h"

#include <cmath>
#include <iostream>

/* 
 * Rapsberry Pi GPIO based controller exposes 2 analog outputs
 */

struct LinkuinoController : public FleyePlugin
{
	inline LinkuinoController() : m_iosvc(0), m_motorsvc(0), m_link(0) {}
	
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
		
		m_motorsvc = MotorDriveService_instance();
		m_motorsvc->setNumberOfMotors(2);

		m_txt = nullptr; //TextService_instance()->addPositionnedText(0.1,0.5);
		
		write_linkuino();
		
		std::cout<<"LinkuinoController IOService @"<<m_iosvc<<", MotorDriveService @"<<m_motorsvc<<", LinkuinoClient @"<<m_link<<"\n";
	}

	void run(FleyeContext* ctx,int threadId)
	{
		write_linkuino();
	}
	
	void write_linkuino()
	{
		int na = m_iosvc->getNumberOfAnalogOutputs();
		if( na > Linkuino::PWM_COUNT ) na = Linkuino::PWM_COUNT;
		std::ostringstream oss;
		for(int i=0;i<na;i++)
		{
			int32_t value = m_iosvc->getAnalogOutput(i) * 1500 + 500;
			if( value < 500 ) value = 500;
			if( value > 2000 ) value = 2000;
			//oss << "PWM"<<i<<"="<<value<<"\n";
			m_link->setPWMValue( i, value );
		}
		//m_txt->out() << oss.str();
		int nd = m_iosvc->getNumberOfDigitalOutputs();
		if( nd > 8 ) nd = 8;
		uint8_t bitfield = 0;
		for(int i=0;i<nd;i++)
		{
			bitfield <<= 1;
			bitfield |= m_iosvc->getDigitalOutput(nd-i-1) ? 1 : 0;
		}
		m_link->setRegisterValue( Linkuino::DOUT_ADDR , bitfield );
		
		if( m_motorsvc->getMotorCommand(0).m_targetPosition!=0.0f ||  m_motorsvc->getMotorCommand(1).m_targetPosition!=0.0f )
		{
			int tr = m_motorsvc->getMotorCommand(0).m_targetSpeed * 16.0f; // speed in [0;1]
			int rr = m_motorsvc->getMotorCommand(0).m_targetPosition * 5.0f; // number of rounds
			int tl = m_motorsvc->getMotorCommand(1).m_targetSpeed * 16.0f;
			int rl = m_motorsvc->getMotorCommand(1).m_targetPosition * 5.0f;
			uint32_t a=0, b=0, c=0, d=0;
			a = std::max( 0, std::min(tr,15) );
			b = std::max( 0, std::min(rr,255) );
			c = std::max( 0, std::min(tl,15) );
			d = std::max( 0, std::min(rl,255) );
			std::cout<<a<<", "<<b<<", "<<c<<", "<<d<<"\n";
			uint32_t data = a<<20 | b<<12 | c<<8 | d;
			uint16_t d0 = (data>>18) & 0x3F;
			uint16_t d1 = (data>>12) & 0x3F;
			uint16_t d2 = (data>>6) & 0x3F;
			uint16_t d3 = data & 0x3F;
			m_link->setRegisterValue(Linkuino::REQ_ADDR, Linkuino::REQ_FWD_SERIAL);
			m_link->setRegisterValue(Linkuino::REQ_DATA0_ADDR, d0);
			m_link->setRegisterValue(Linkuino::REQ_DATA1_ADDR, d1);
			m_link->setRegisterValue(Linkuino::REQ_DATA2_ADDR, d2);
			m_link->setRegisterValue(Linkuino::REQ_DATA3_ADDR, d3);
			m_motorsvc->setMotorCommand( 0, 0.0f, 0.0f );
			m_motorsvc->setMotorCommand( 1, 0.0f, 0.0f );
		}
		else
		{
			m_link->setRegisterValue(Linkuino::REQ_ADDR, Linkuino::REQ_NOOP);
		}
		
		m_link->send();
	}
	
	IOService* m_iosvc;
	MotorDriveService* m_motorsvc;
	LinkuinoClient* m_link;
	PositionnedText* m_txt;
};

FLEYE_REGISTER_PLUGIN(LinkuinoController);

