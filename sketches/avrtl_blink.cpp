#include <AvrTL.h>
#include <SerialConsole.h>
#include <PrintStream.h>

using namespace avrtl;

#define LED_PIN 13

//constexpr auto led = pin(&PINB,&PORTB,&DDRB,5);
/*
constexpr auto led = pin( portInputRegister(digitalPinToPort(LED_PIN))
						, portOutputRegister(digitalPinToPort(LED_PIN))
						, portModeRegister(digitalPinToPort(LED_PIN))
						,digitalPinToBit(LED_PIN) );
*/
static constexpr auto led = pin(LED_PIN);
SerialConsole serialConsole;
PrintStream<SerialConsole> cout(serialConsole);

void setup()
{
	led.SetOutput(); //pinMode(LED_PIN,OUTPUT);
	serialConsole.begin(9600);
}

void loop()
{
	cout<<"Hello World ;)"<<endl;
	for(int j=0;j<5;j++)
	{
		for(int i=0;i<6;i++)
		{
		led = (i%2==0);
		DelayMicroseconds(500000UL*j);
		}
	}
}
