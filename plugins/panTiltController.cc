#include "fleye/plugin.h"
#include "fleye/FleyeContext.h"

#include "gpio/gpioController.h"
#include <cmath>

#define SERVO_X_VALUE_MIN 128
#define SERVO_X_ANGLE_MIN -67.5
#define SERVO_X_VALUE_MAX 896
#define SERVO_X_ANGLE_MAX 67.5

#define SERVO_Y_VALUE_MIN 128
#define SERVO_Y_ANGLE_MIN -67.5
#define SERVO_Y_VALUE_MAX 768
#define SERVO_Y_ANGLE_MAX 45

#define GPIO_AXIS_BITS 	10
#define GPIO_LASER_BIT 	(2*GPIO_AXIS_BITS)
#define GPIO_DATA_BITS 	(GPIO_LASER_BIT+1)

struct panTiltController : public FleyePlugin
{
	void setup(FleyeContext* ctx)
	{
		for(int i=0;i<GPIO_DATA_BITS;i++)
		{
			gpio_set_mode(i,OUTPUT_MODE);
		}
	}

	void run(FleyeContext* ctx,int threadId)
	{
		
		 float theta = atan( (std::cos( ctx->frameCounter*0.02 )) /4.0 ) ;
		 float phi = atan( (std::sin( ctx->frameCounter*0.02 )) /4.0 ) ;
		 gpio_write_theta_phi(theta,phi, ctx->frameCounter%2 );
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

	// theta and phi are angle in radians
	void gpio_write_theta_phi(float theta, float phi, bool laser)
	{
		gpio_write_xy_f(
			(theta*57.29577951308232-SERVO_X_ANGLE_MIN)/(SERVO_X_ANGLE_MAX-SERVO_X_ANGLE_MIN) ,
			(  phi*57.29577951308232-SERVO_Y_ANGLE_MIN)/(SERVO_Y_ANGLE_MAX-SERVO_Y_ANGLE_MIN) , laser);
	}

};

FLEYE_REGISTER_PLUGIN(panTiltController);

