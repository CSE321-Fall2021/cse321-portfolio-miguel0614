/*
 * Author: Miguel Bautista
 * File Purpose: Periodically toggles an external LED on PB_8
 * Modules:
 * Subroutines:
 * Assignment: Stage 1 Project 2
 * Inputs: 4x4 Matrix Keypad
 * Outputs: PB_8 (external LED)
 * Constraints:
 * References:
 */

#include <lcd1602.h>
#include <mbed.h>
#include <stm_methods.h>
#include <string>

#define ASCII_ZERO 48
#define ASCII_FIVE 53
#define ASCII_NINE 57

void isr_col(void);
void isr_falling_edge(void);

void row_handler(void);
void key_handler(void);
void counter_handler(void);
void counter(void);
void powerOnCounter(void);
void powerOffCounter(void);
void blinkLED(void);
void validKey(char letter);

Thread row_controller; // Independent thread to control keypad row power

EventQueue queue;

Ticker debounce_ticker;
Ticker counter_ticker;

CSE321_LCD LCD(16, 2, LCD_5x8DOTS, PB_9, PB_8);

InterruptIn col_0(PF_14, PullDown);
InterruptIn col_1(PE_11, PullDown);
InterruptIn col_2(PE_9, PullDown);
InterruptIn col_3(PF_13, PullDown);

char keypad[4][4] = {{'1', '2', '3', 'A'},
                     {'4', '5', '6', 'B'},
                     {'7', '8', '9', 'C'},
                     {'*', '0', '#', 'D'}};
int row = 0;
int key_pressed = 0;
int debounce_buffer = 0;
int debounced = 0;

int cursor = 0;
int mode = 0;            // 0->off, 1->input time, 2->start
int count_direction = 0; //  0->down, 1->up,
int time_remaining = 0;  // in seconds
int time_passed = 0;

int main() {
  // Enable clock control register for GPIO A, B, & C
  enable_rcc('a');
  enable_rcc('b');
  enable_rcc('c');

  // Declare GPIOA pin 5 as an output
  set_pin_mode(5, GPIOA, 1);
  // Declare GPIOA pin 5 as an output
  set_pin_mode(6, GPIOA, 1);

  // Declare GPIOA pin 3 as an output
  set_pin_mode(3, GPIOA, 1);
  // Declare GPIOC pin 0 as an output
  set_pin_mode(0, GPIOC, 1);
  // Declare GPIOC pin 3 as an output
  set_pin_mode(3, GPIOC, 1);
  // Declare GPIOC pin 1 as an output
  set_pin_mode(1, GPIOC, 1);

  LCD.begin();
  LCD.noBacklight();

  col_0.rise(&isr_col);
  col_1.rise(&isr_col);
  col_2.rise(&isr_col);
  col_3.rise(&isr_col);

  col_0.fall(&isr_falling_edge);
  col_1.fall(&isr_falling_edge);
  col_2.fall(&isr_falling_edge);
  col_3.fall(&isr_falling_edge);

  row_controller.start(row_handler);

  debounce_ticker.attach(&key_handler, 1ms);
  counter_ticker.attach(&counter_handler, 1s);

  queue.dispatch_forever();
}

void isr_col(void) { key_pressed = 1; }

void isr_falling_edge(void) {
  key_pressed = 0;
  debounce_buffer = 0;
  debounced = 0;
  write_to_pin(5, GPIOA, 0);
}

void row_handler(void) {
  while (1) {
    if (!key_pressed) {
      row++;
      row %= 4;
      switch (row) {
      case 0:
        write_to_pin(0, GPIOC, 0);
        write_to_pin(3, GPIOC, 0);
        write_to_pin(1, GPIOC, 0);
        write_to_pin(3, GPIOA, 1);
        break;
      case 1:
        write_to_pin(3, GPIOA, 0);
        write_to_pin(3, GPIOC, 0);
        write_to_pin(1, GPIOC, 0);
        write_to_pin(0, GPIOC, 1);
        break;
      case 2:
        write_to_pin(3, GPIOA, 0);
        write_to_pin(0, GPIOC, 0);
        write_to_pin(1, GPIOC, 0);
        write_to_pin(3, GPIOC, 1);
        break;
      case 3:
        write_to_pin(3, GPIOA, 0);
        write_to_pin(0, GPIOC, 0);
        write_to_pin(3, GPIOC, 0);
        write_to_pin(1, GPIOC, 1);
        break;
      }
    }
  }
}

void key_handler(void) {
  if (key_pressed) {
    if (!debounce_buffer) {
      debounce_buffer = 1;
    } else {
      if (!debounced) {
        switch (mode) {
        case 0:
          if (col_3.read() && keypad[row][3] == 'D') {
            queue.call(&powerOnCounter);
          }
          break;
        case 1:
          if (col_0.read() && keypad[row][0] != '*') {
            queue.call(&validKey, keypad[row][0]);
          } else if (col_1.read()) {
            queue.call(&validKey, keypad[row][1]);
          } else if (col_2.read() && keypad[row][2] != '#') {
            queue.call(&validKey, keypad[row][2]);
          } else if (col_3.read() && keypad[row][3] != 'D' &&
                     keypad[row][3] != 'C') {
            queue.call(&validKey, keypad[row][3]);
          }
          break;
        case 2:
          if (col_3.read() && keypad[row][3] != 'A') {
            queue.call(&validKey, keypad[row][3]);
          }
          break;
        }
      }
      debounce_buffer = 0;
      debounced = 1;
    }
  } else
    debounce_buffer = 0;
}

void counter_handler(void) {
  if (mode == 2) {
    queue.call(counter);
  }
}

void counter(void) {
  if (mode == 2) {
    time_remaining--;
    time_passed++;
    if (!time_remaining) {
      mode = 0;
      LCD.clear();
      count_direction ? LCD.print("Times Up") : LCD.print("Time Reached");
      for (int i = 0; i < 4; i++) {
        write_to_pin(6, GPIOA, 1);
        thread_sleep_for(200);
        write_to_pin(6, GPIOA, 0);
        thread_sleep_for(200);
      }
      thread_sleep_for(2000);
      LCD.clear();
      LCD.noBacklight();
    } else {
      LCD.clear();
      count_direction ? LCD.print("Time Passed: ")
                      : LCD.print("Time Remaining: ");
      LCD.setCursor(0, 1);

      string time = count_direction ? to_string(time_passed / 60) + "M " +
                                          to_string(time_passed % 60) + "S"
                                    : to_string(time_remaining / 60) + "M " +
                                          to_string(time_remaining % 60) + "S";

      char *time_array = &time[0];

      LCD.print(time_array);
    }
  }
}

void powerOnCounter(void) {
  mode = 1;

  write_to_pin(5, GPIOA, 1);
  thread_sleep_for(250);
  write_to_pin(5, GPIOA, 0);

  LCD.backlight();
  validKey('D');
}

void powerOffCounter(void) {
  mode = 0;
  LCD.clear();
  LCD.noBacklight();
}

void blinkLED(void) {
  write_to_pin(5, GPIOA, 1);
  thread_sleep_for(250);
  write_to_pin(5, GPIOA, 0);
}

void validKey(char letter) {
  switch (letter) {
  case 'A':
    if (time_remaining > 0) {
      mode = 2;
      cursor = 0;
      blinkLED();
    }

    break;

  case 'B':
    mode = 0;
    cursor = 0;
    time_remaining = 0;
    time_passed = 0;
    LCD.clear();
    LCD.noBacklight();
    break;

  case 'C':
    count_direction = !count_direction;
    blinkLED();
    break;

  case 'D':
    mode = 1;
    LCD.clear();
    LCD.print("Enter Time:");
    time_remaining = 0;
    time_passed = 0;
    blinkLED();
    break;
  }

  if (letter >= ASCII_ZERO && letter <= ASCII_NINE) {
    int valid = 0;
    if (cursor == 0) {
      time_remaining += (letter - ASCII_ZERO) * 60;
      valid = 1;

    } else if (cursor == 1 && letter <= ASCII_FIVE) {
      time_remaining += (letter - ASCII_ZERO) * 10;
      valid = 1;

    } else if (cursor == 2) {
      time_remaining += letter - ASCII_ZERO;
      valid = 1;
    }
    if (valid) {
      cursor++;
      string time = to_string(time_remaining / 60) + "M " +
                    to_string(time_remaining % 60) + "S";

      char *time_array = &time[0];
      LCD.clear();
      LCD.print("Enter Time:");
      LCD.setCursor(0, 1);
      LCD.print(time_array);
      blinkLED();
    }
  }
}