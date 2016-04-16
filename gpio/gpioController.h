#ifndef __fleye_thirfparty_gpioController_h
#define __fleye_thirfparty_gpioController_h

#include <stdint.h>

#define OUTPUT_MODE 1
#define INPUT_MODE 0

#ifdef __cplusplus
extern "C" {
#endif

extern int init_gpio();
extern void gpio_set_mode(int gpioPin, int mode /*1 for output, 0 for input*/ );
extern void gpio_write_bits(unsigned long gpio_bits);
extern unsigned long gpio_read_bits(int firstPin, int nPins);
extern int gpio_pin_count();
#ifdef __cplusplus
}
#endif

#endif
