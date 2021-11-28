/*
 * Author: Miguel Bautista (50298507)
 *
 * File Purpose:
 *
 * Modules:
 *      CSE321_project2_mabautis_stm_methods - Contains initialization code for
 * the RCC and GPIO pins and code to write to MODER
 *      CSE321_project2_mabautis_lcd1602 - Contains intiialization and operation
 * code for a 1602 LCD
 *
 * Subroutines:
 *
 * Assignment: Project 3
 *
 * Inputs:
 *
 * Outputs:
 *
 * Constraints:
 *
 * References:
 *      MBED Bare Metal Guide -
 * https://os.mbed.com/docs/mbed-os/v6.15/bare-metal/index.html STM32L48 User
 * Guide -
 * https://www.st.com/resource/en/reference_manual/rm0351-stm32l47xxx-stm32l48xxx-stm32l49xxx-and-stm32l4axxx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf
 */

#include "mbed_thread.h"
#include <CSE321_project3_mabautis_lcd1602.h>
#include <CSE321_project3_mabautis_stm_methods.h>
#include <cstdio>
#include <mbed.h>
#include <string>

#define ASCII_ZERO 48
#define ASCII_FIVE 53
#define ASCII_NINE 57

void isr_col(void); // Rising edge Interrupt Service Routine for column pins
                    // [PF_14, PE_11, PE_9, PF_13]
void isr_falling_edge(void); // Falling edge Interrupt Service Routine for
                             // column pins [PF_14, PE_11, PE_9, PF_13]

void row_handler(void); // Handles the powering of rows on the matrix keypad
void key_handler(void);

void power_on_mode(void);
void unarmed_mode(void);
void armed_mode(void);
void triggered_mode(void);

void idle_timeout_handler(void);
void set_display_off(void);

int key_pressed = 0;
int debounced = 0;

int display_on = 1;

string password = "****";
string password_entered = "****";
int password_position = 0;
int entering_password = 0;

CSE321_LCD LCD(16, 2, LCD_5x8DOTS, PB_9, PB_8); // Initialize LCD

// Initialize interrupts for columns of keypad. Set pull down to pull port down
// to 0 volts
InterruptIn col_0(PF_14, PullDown);
InterruptIn col_1(PE_11, PullDown);
InterruptIn col_2(PE_9, PullDown);
InterruptIn col_3(PF_13, PullDown);

Thread row_thread;
Thread key_thread;

Mutex resource_lock;

EventQueue queue; // Initialize EventQueue to queue blocking code from ISR

Timeout idle_timeout;

char keypad[4][4] = {{'1', '2', '3', 'A'},
                     {'4', '5', '6', 'B'},
                     {'7', '8', '9', 'C'},
                     {'*', '0', '#', 'D'}}; // Enumerate keypad matrix

int mode = 0; // 0 -> Power On Mode (Define Code), 1 -> Unarmed, 2 -> Armed, 3
              // -> Triggered

int row = 0; // Current keypad row to power

int main() {
  col_0.enable_irq();
  col_1.enable_irq();
  col_2.enable_irq();
  col_3.enable_irq();
  // Enable clock control register for GPIO A & C
  enable_rcc('a');
  enable_rcc('c');

  // Declare GPIOA pin 3 as an output
  set_pin_mode(3, GPIOA, 1);
  // Declare GPIOC pin 0 as an output
  set_pin_mode(0, GPIOC, 1);
  // Declare GPIOC pin 3 as an output
  set_pin_mode(3, GPIOC, 1);
  // Declare GPIOC pin 1 as an output
  set_pin_mode(1, GPIOC, 1);

  LCD.begin(); // Initialize LCD
  LCD.print("Set Passcode: ");
  LCD.setCursor(0, 1);

  // Declare interrupts for rising edge of each column of keypad
  col_0.rise(&isr_col);
  col_1.rise(&isr_col);
  col_2.rise(&isr_col);
  col_3.rise(&isr_col);

  // Declare interrupts for falling edge of each column of keypad
  col_0.fall(&isr_falling_edge);
  col_1.fall(&isr_falling_edge);
  col_2.fall(&isr_falling_edge);
  col_3.fall(&isr_falling_edge);

  idle_timeout.attach(&idle_timeout_handler, 10s);
  key_thread.start(key_handler);
  row_thread.start(row_handler);
}

void isr_col(void) { key_pressed = 1; } // Set flag to handle in debounce ticker

void isr_falling_edge(void) { // Set flag to indicate key no longer pressed
  key_pressed = 0;
  debounced = 0;
}

void key_handler() {
  while (1) {
    resource_lock.lock();
    if (key_pressed) {
      if (!debounced) {
        thread_sleep_for(10);
        if (key_pressed) {
          debounced = 1;
          if (!display_on) {
            display_on = 1;
            LCD.backlight();
          }
          idle_timeout.detach();
          idle_timeout.attach(&idle_timeout_handler, 10s);
          printf("MODE: %d\n", mode);

          switch (mode) {
          case 0:
            power_on_mode();
            break;
          case 1:
            unarmed_mode();
            break;
          case 2:
            armed_mode();
            break;
          case 3:
            triggered_mode();
            break;
          }
        }
      }
    }
    resource_lock.unlock();
  }
}

void row_handler() {
  while (1) {
    resource_lock.lock();
    if (!key_pressed) {
      row++;    // Increment row
      row %= 4; // Keep row between 0 and 3
      switch (row) {
      case 0: // Turn off other rows, turn on row 0
        write_to_pin(0, GPIOC, 0);
        write_to_pin(3, GPIOC, 0);
        write_to_pin(1, GPIOC, 0);
        write_to_pin(3, GPIOA, 1);
        break;
      case 1: // Turn off other rows, turn on row 1
        write_to_pin(3, GPIOA, 0);
        write_to_pin(3, GPIOC, 0);
        write_to_pin(1, GPIOC, 0);
        write_to_pin(0, GPIOC, 1);
        break;
      case 2: // Turn off other rows, turn on row 2
        write_to_pin(3, GPIOA, 0);
        write_to_pin(0, GPIOC, 0);
        write_to_pin(1, GPIOC, 0);
        write_to_pin(3, GPIOC, 1);
        break;
      case 3: // Turn off other rows, turn on row 3
        write_to_pin(3, GPIOA, 0);
        write_to_pin(0, GPIOC, 0);
        write_to_pin(3, GPIOC, 0);
        write_to_pin(1, GPIOC, 1);
        break;
      }
    }
    queue.dispatch_once();
    resource_lock.unlock();
  }
}

void power_on_mode() {
  if (col_0.read() && keypad[row][0] != '*') {
    password[password_position] = keypad[row][0];
  } else if (col_1.read()) {
    password[password_position] = keypad[row][1];
  } else if (col_2.read() && keypad[row][2] != '#') {
    password[password_position] = keypad[row][2];
  }
  if ((col_0.read() && keypad[row][0] != '*') || col_1.read() ||
      (col_2.read() && keypad[row][2] != '#')) {
    password_position++;
    LCD.print("*");
    if (password_position == 4) {
      password_position = 0;
      mode = 1;
      LCD.clear();
      LCD.print("Unarmed");
    }
  }
}

void unarmed_mode() {
  if (col_0.read() && keypad[row][0] != '*' && entering_password) {
    password_entered[password_position] = keypad[row][0];
  } else if (col_1.read() && entering_password) {
    password_entered[password_position] = keypad[row][1];

  } else if (col_2.read() && keypad[row][2] != '#' && entering_password) {
    password_entered[password_position] = keypad[row][2];
  } else if (col_3.read() && keypad[row][3] == 'A' && !entering_password) {
    entering_password = 1;
    LCD.clear();
    LCD.print("Enter Passcode: ");
    LCD.setCursor(0, 1);
  }
  if ((col_0.read() && keypad[row][0] != '*' && entering_password) ||
      (col_1.read() && entering_password) ||
      (col_2.read() && keypad[row][2] != '#' && entering_password)) {
    password_position++;
    LCD.print("*");
    if (password_position == 4) {
      password_position = 0;
      entering_password = 0;
      if (password_entered == password) {
        mode = 2;
        LCD.clear();
        LCD.print("Armed");
      } else {
        LCD.clear();
        LCD.print("Incorrect");
        LCD.setCursor(0, 1);
        LCD.print("Passcode");
        thread_sleep_for(2000);
        LCD.clear();
        LCD.print("Unarmed");
      }
    }
  }
}

void armed_mode() {}

void triggered_mode() {}

void idle_timeout_handler() {
  if (display_on) {
    display_on = 0;
    queue.call(&set_display_off);
  }
}

void set_display_off() {
  LCD.noBacklight();
  LCD.clear();
  password_position = 0;
  entering_password = 0;
  switch (mode) {
  case 0:
    LCD.print("Set Passcode: ");
    LCD.setCursor(0, 1);
    break;
  case 1:
    LCD.print("Unarmed");
    break;
  case 2:
    LCD.print("Armed");
    break;
  case 3:
    LCD.print("Triggered");
    break;
  }
}