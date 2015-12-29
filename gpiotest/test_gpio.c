#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>

#include "../thirdparty/bcm2835.h"

#define SERVO_X_VALUE_MIN 250
#define SERVO_X_ANGLE_MIN -45.0
#define SERVO_X_VALUE_MAX 750
#define SERVO_X_ANGLE_MAX 45.0

#define SERVO_Y_VALUE_MIN 250
#define SERVO_Y_ANGLE_MIN -45.0
#define SERVO_Y_VALUE_MAX 750
#define SERVO_Y_ANGLE_MAX 45.0


#define GPIO_LOCK_BIT 	26
#define GPIO_AXIS_BITS 	10
#define GPIO_LASER_BIT 	(2*GPIO_AXIS_BITS)
#define GPIO_DATA_BITS 	(GPIO_LASER_BIT+1)


int init_gpio()
{
  int i;
    if (!bcm2835_init())
  {
    printf("bcm2835_init failed\n");
    return -1;
  }

  // reserve GPIO pins for output (2 x 10bits servo + 1 binary for laser)
  for(i=0;i<GPIO_DATA_BITS;++i)
  {
	bcm2835_gpio_fsel(i, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_write(i, LOW);
  }
  // GPIO 26 for locking data HIGH=data locked, LOW=data can be read
  bcm2835_gpio_fsel(GPIO_LOCK_BIT, BCM2835_GPIO_FSEL_OUTP);
  bcm2835_gpio_write(GPIO_LOCK_BIT, HIGH);
  
  return 0;
}


void gpio_write_bits(uint32_t gpio_bits)
{
	uint32_t set_mask = 0;
	uint32_t clr_mask = 0;
	
	bcm2835_gpio_set_multi( 1UL<<GPIO_LOCK_BIT ); // set lock bit

	//printf("gpio_bits 0x%08X\n",gpio_bits);
	for(int i=0;i<=GPIO_DATA_BITS;++i)
	{
		uint32_t bitMask = (1UL << i);
		if( gpio_bits & bitMask ) { set_mask |= bitMask; }
		else { clr_mask |= bitMask; }
	}

	bcm2835_gpio_clr_multi( clr_mask );
	
	bcm2835_gpio_set_multi( set_mask );

	bcm2835_gpio_clr_multi( 1UL<<GPIO_LOCK_BIT ); // clear lock bit
}

void gpio_write_xy_i(unsigned int xi, unsigned int yi, int laserSwitch)
{
	uint32_t bits;
	
	if( xi>=1024 ) xi=1023;
	if( yi>=1024 ) yi=1023;

	laserSwitch = laserSwitch ? 1 : 0;
	bits = (laserSwitch<<GPIO_LASER_BIT) | (yi<<GPIO_AXIS_BITS) | xi;
	
	gpio_write_bits(bits);
}

void gpio_write_xy_f(float xf, float yf, int laserSwitch)
{
	unsigned int xi,yi;

	if(xf<0.0f) xf=0.0f;
	else if(xf>1.0f) xf=1.0f;

	if(yf<0.0f) yf=0.0f;
	else if(yf>1.0f) yf=1.0f;

	xi = SERVO_X_VALUE_MIN + (unsigned int)( xf * (SERVO_X_VALUE_MAX-SERVO_X_VALUE_MIN) );
	yi = SERVO_Y_VALUE_MIN + (unsigned int)( yf * (SERVO_Y_VALUE_MAX-SERVO_Y_VALUE_MIN) );

	gpio_write_xy_i(xi,yi,laserSwitch);
}

// theta and phi are angle in radians
void gpio_write_theta_phi(float theta, float phi, int laserSwitch)
{
	gpio_write_xy_f(
		(theta*57.29577951308232-SERVO_X_ANGLE_MIN)/(SERVO_X_ANGLE_MAX-SERVO_X_ANGLE_MIN) ,
		(  phi*57.29577951308232-SERVO_Y_ANGLE_MIN)/(SERVO_Y_ANGLE_MAX-SERVO_Y_ANGLE_MIN) ,
		laserSwitch );
}




int main(int argc, char* argv[])
{
	uint32_t i=0,X=0,Y=0,L=0;
	double Xf=0.0, Yf=0.0;
	
	X = argc>2 ? atol(argv[2]) : 0;
	Y = argc>3 ? atol(argv[3]) : 0;
	L = argc>4 ? atol(argv[4]) : 0;
	
	Xf = X * 0.017453292519943295 ;
	Yf = Y * 0.017453292519943295 ;
	
	init_gpio();
	
	switch( argv[1][0] )
	{	
		case 'x':
			gpio_write_bits(X);
			break;

		case 't':
			{
				struct timeval te1;
				struct timeval te2;
				double T1, T2;
				gpio_write_bits( (X-1)<<L );
				sleep(1);
				gettimeofday(&te1, NULL);
				for(int l=0;l<Y;l++)
				{
					for(i=0;i<X;i++)
					{
						gpio_write_bits(i<<L);
						usleep(1);
					}
				}
				gettimeofday(&te2, NULL);
				T1 = te1.tv_sec * 1000.0 + te1.tv_usec * 0.001;
				T2 = te2.tv_sec * 1000.0 + te2.tv_usec * 0.001;
				gpio_write_bits(0);
				printf("Time = %g ms\n",T2-T1);
			}
			break;

		case 'r':
			gpio_write_xy_i(X,Y,L);
			break;

		case 'S':
			for(i=1;i<1024;++i)
			{
				gpio_write_xy_i(X,i,L);
				usleep(X);
			}
			gpio_write_xy_i(X,0,L);
			break;

		case 's':
			for(i=1;i<1024;++i)
			{
				gpio_write_xy_i(i,Y,L);
				usleep(Y);
			}
			gpio_write_xy_i(0,Y,L);
			break;
			
		case 'a':
			printf("angle: theta = %g, phi=%g, laser=%d\n",Xf,Yf,L);
			gpio_write_theta_phi(Xf,Yf,L);
			break;
	}

	return 0;
}
