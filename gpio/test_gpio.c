#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>

#include "gpioController.h"

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
	for(int i=0;i<20;i++) gpio_set_mode(i,OUTPUT_MODE);
	
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
	}

	return 0;
}
