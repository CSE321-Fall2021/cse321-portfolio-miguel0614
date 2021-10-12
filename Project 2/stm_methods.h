 #include "mbed.h"

void set_pin_mode(unsigned int pin, GPIO_TypeDef *port, unsigned int mode);
void enable_rcc(unsigned int port);
void write_to_pin(unsigned int pin, GPIO_TypeDef *port, unsigned int value);