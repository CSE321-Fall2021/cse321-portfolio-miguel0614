/*
 * Author: Miguel Bautista
 * File Purpose: 
 * Modules:
 * Subroutines: 
 * Assignment: 
 * Inputs:
 * Outputs: PB_8
 * Constraints:
 * References:
 */

#include <mbed.h>

int main() {

    // Enable clock control register for GPIO B
    RCC -> AHB2ENR |= 0x2;

    // Declare GPIOB port 8 as an output
    GPIOB -> MODER &= ~(0x20000);
    GPIOB -> MODER |= 0x10000;
    
    printf("Miguel Bautista\n");

    while(1) {
    GPIOB -> ODR |= 0x100;
    thread_sleep_for(1000);
    GPIOB -> ODR &= ~(0x100);
    thread_sleep_for(1000);
  }

}