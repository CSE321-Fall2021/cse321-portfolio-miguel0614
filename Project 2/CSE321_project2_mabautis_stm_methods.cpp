/*
 * Author: Miguel Bautista
 *
 * File Purpose: Contains initialization code for the RCC and GPIO pins and code to write to MODER
 *
 * Modules: 
 * Subroutines:
 * void set_pin_mode(unsigned int pin, GPIO_TypeDef *port, unsigned int mode) - Set the designed pin/port to be an input/output
 * void enable_rcc(unsigned int port) - Enable the reset control clock for the specified GPIO port
 * void write_to_pin(unsigned int pin, GPIO_TypeDef *port, unsigned int value) - Writes the designated value (0/1) to the entered pin and port
 *
 * Assignment: Project 2
 * Inputs: 
 * Outputs: 
 * Constraints:
 * References: 
 *      MBED Bare Metal Guide - https://os.mbed.com/docs/mbed-os/v6.15/bare-metal/index.html
 *      STM32L48 User Guide - https://www.st.com/resource/en/reference_manual/rm0351-stm32l47xxx-stm32l48xxx-stm32l49xxx-and-stm32l4axxx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf 
 */
#include <mbed.h>

#define UPPERCASE 65
#define LOWERCASE 97

void set_pin_mode(unsigned int pin, GPIO_TypeDef *port, unsigned int mode);
void enable_rcc(unsigned int port);
void write_to_pin(unsigned int pin, GPIO_TypeDef *port, unsigned int value);

void enable_rcc(unsigned int port) {
  // Enable RCC for GPIO port
  unsigned int offset =
      (port - LOWERCASE) < 0
          ? port - UPPERCASE
          : port - LOWERCASE; // Determine if character is uppercase/lowercase
  RCC->AHB2ENR |= (0x1 << offset); // Enable port using port offset
}

void set_pin_mode(unsigned int pin, GPIO_TypeDef *port, unsigned int mode) {
  // MODE 0 -> Input Mode || MODE 1 -> Output Mode
  unsigned int offset = pin * 2; // offset pin times 2; accounts for 2-bits
  if (mode) {
    port->MODER &= ~(0x2 << (offset)); // Set leftmost bit of pin to 0
    port->MODER |= (0x1 << (offset));  // Set rightmost bit of pin to 1
  } else {
    port->MODER &= ~(0x3 << (offset)); // Set both pins to 0
  }
}

void write_to_pin(unsigned int pin, GPIO_TypeDef *port, unsigned int value) {
  // Writes logic high/low to pin
  value ? port->ODR |= (0x1 << pin) : port->ODR &= ~(0x1 << pin);
}
