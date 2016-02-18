#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>

#include "../thirdparty/bcm2835.h"
#include "gpioController.h"

#define GPIO_LOCK_BIT 	25

int init_gpio()
{
  if (!bcm2835_init())
  {
    printf("bcm2835_init failed\n");
    return -1;
  }

  // BCM 25 for locking data HIGH=data locked, LOW=data can be read
  bcm2835_gpio_fsel(GPIO_LOCK_BIT, BCM2835_GPIO_FSEL_OUTP);
  bcm2835_gpio_write(GPIO_LOCK_BIT, HIGH);
  
  return 0;
}

void gpio_set_mode(int i, int mode)
{
	bcm2835_gpio_fsel(i, mode ? BCM2835_GPIO_FSEL_OUTP : BCM2835_GPIO_FSEL_INPT );
	bcm2835_gpio_write(i, LOW);
}

unsigned long gpio_read_bits(int firstPin, int nPins)
{
	unsigned long r = 0;
	for(int i=firstPin+nPins-1;i>=firstPin;--i)
	{
		r = (r<<1) | bcm2835_gpio_lev(i);
	}
	return r;
}

void gpio_write_bits(unsigned long gpio_bits)
{
	uint32_t set_mask = 0;
	uint32_t clr_mask = 0;
	int i;
	
	bcm2835_gpio_set_multi( 1UL<<GPIO_LOCK_BIT ); // set lock bit

	//printf("gpio_bits 0x%08X\n",gpio_bits);
	for(i=0;i<GPIO_LOCK_BIT;++i)
	{
		uint32_t bitMask = (1UL << i);
		if( gpio_bits & bitMask ) { set_mask |= bitMask; }
		else { clr_mask |= bitMask; }
	}

	bcm2835_gpio_clr_multi( clr_mask );
	
	bcm2835_gpio_set_multi( set_mask );

	bcm2835_gpio_clr_multi( 1UL<<GPIO_LOCK_BIT ); // clear lock bit
}
