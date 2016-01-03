#ifndef __fleye_thirfparty_gpioController_h
#define __fleye_thirfparty_gpioController_h

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int init_gpio();
void gpio_write_bits(unsigned long gpio_bits);
void gpio_write_xy_i(unsigned int xi, unsigned int yi, int laserSwitch);
void gpio_write_xy_f(float xf, float yf, int laserSwitch);
void gpio_write_theta_phi(float theta, float phi, int laserSwitch);

#ifdef __cplusplus
}
#endif

#endif
