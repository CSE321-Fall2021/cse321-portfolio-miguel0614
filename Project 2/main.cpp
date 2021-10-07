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

void isr_col_0(void);
void isr_col_1(void);
void isr_col_2(void);
void isr_col_3(void);
void isr_falling_edge(void);
void row_handler(void);

Thread row_controller; // Independent thread to control keypad row power

DigitalOut external_led(PB_8);

EventQueue queue;

InterruptIn col_0(PF_14, PullDown);
InterruptIn col_1(PE_11, PullDown);
InterruptIn col_2(PE_9, PullDown);
InterruptIn col_3(PF_13, PullDown);

DigitalOut row_0(PD_9);
DigitalOut row_1(PD_8);
DigitalOut row_2(PF_15);
DigitalOut row_3(PE_13);

char keypad[4][4] = {{'1', '2', '3', 'A'},
                     {'4', '5', '6', 'B'},
                     {'7', '8', '9', 'C'},
                     {'*', '0', '#', 'D'}};
int row = 0;
int key_pressed = 0;

// int main() {

//     // Enable clock control register for GPIO B
//     RCC -> AHB2ENR |= 0x2;

//     // Declare GPIOB port 8 as an output
//     GPIOB -> MODER &= ~(0x20000);
//     GPIOB -> MODER |= 0x10000;

//     printf("Miguel Bautista\n");

//     while(1) {
//     GPIOB -> ODR |= 0x100;
//     thread_sleep_for(1000);
//     GPIOB -> ODR &= ~(0x100);
//     thread_sleep_for(1000);
//   }

// }

// int main() {
//     printf("Miguel Bautista\n");
//     while(1) {
//         external_led.read() ? external_led = 0 : external_led = 1;
//         thread_sleep_for(100);
//     }

// }

int main() {

  col_0.rise(&isr_col_0);
  col_1.rise(&isr_col_1);
  col_2.rise(&isr_col_2);
  col_3.rise(&isr_col_3);

  col_0.fall(&isr_falling_edge);
  col_1.fall(&isr_falling_edge);
  col_2.fall(&isr_falling_edge);
  col_3.fall(&isr_falling_edge);

  row_controller.start(row_handler);
  queue.dispatch_forever();
}

void isr_col_0(void) {
  if (!key_pressed) {
    key_pressed = 1;
    external_led = 1;
    queue.call(printf, "%c\n", keypad[row][0]);
  }
}

void isr_col_1(void) {
  if (!key_pressed) {
    key_pressed = 1;
    external_led = 1;
    queue.call(printf, "%c\n", keypad[row][1]);
  }
}

void isr_col_2(void) {
  if (!key_pressed) {
    key_pressed = 1;
    external_led = 1;
    queue.call(printf, "%c\n", keypad[row][2]);
  }
}

void isr_col_3(void) {
  if (!key_pressed) {
    key_pressed = 1;
    external_led = 1;
    queue.call(printf, "%c\n", keypad[row][3]);
  }
}

void isr_falling_edge(void) {
  key_pressed = 0;
  external_led = 0;
}

void row_handler(void) {
  while (1) {
    if (!key_pressed) {
      switch (row) {
      case 0:
        row_1 = 0;
        row_2 = 0;
        row_3 = 0;
        row_0 = 1;
        break;

      case 1:
        row_0 = 0;
        row_2 = 0;
        row_3 = 0;
        row_1 = 1;
        break;

      case 2:
        row_0 = 0;
        row_1 = 0;
        row_3 = 0;
        row_2 = 1;
        break;

      case 3:
        row_0 = 0;
        row_1 = 0;
        row_2 = 0;
        row_3 = 1;
        break;
      }
      row++;
      row %= 4;
    }
  }
}
