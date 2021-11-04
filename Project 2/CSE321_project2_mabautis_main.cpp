/*
 * Author: Miguel Bautista (50298507)
 *
 * File Purpose: Implements timer functionality that is operated by a 4x4 matrix keypad
 *
 * Modules: 
 *      CSE321_project2_mabautis_stm_methods - Contains initialization code for the RCC and GPIO pins and code to write to MODER
 *      CSE321_project2_mabautis_lcd1602 - Contains intiialization and operation code for a 1602 LCD
 *
 * Subroutines:
 *      void isr_col(void) - Rising edge Interrupt Service Routine for column pins [PF_14, PE_11, PE_9, PF_13]
 *      void isr_falling_edge(void) - Falling edge Interrupt Service Routine for column pins [PF_14, PE_11, PE_9, PF_13]
 *      void key_handler(void) - Determines if a key press is valid (debounced) and then calls validKey or powerOnTimer
 *      void validKey(char letter) - Handles input based on current mode
 *      void powerOnTimer(void) - Initalizes flags and LCD then calls validKey('D')
 *      void timer_handler(void) - Ticker to count in 1s increments if mode = 2. Queues timer() to run blocking code
 *      void timer(void) - Handles checking remaining time, incrementing timer, and printing the associated output
 *      void blinkLED(void) - Handles turning on and off LED with a specified blinking interval
 *
 * Assignment: Project 2
 *
 * Inputs: 
 *      4x4 Matrix Keypad - Rows: [PA_3, PC_0, PC_3, PC_1] Columns: [PF_14, PE_11, PE_9, PF_13]
 *
 * Outputs: 
 *      1602 LCD - SDA: PB_9 SCL: PB_8
 *      Valid Key Press LED - PA_5
 *      Timer Done LEDs - PA_6
 *
 * Constraints:
 *
 * References: 
 *      MBED Bare Metal Guide - https://os.mbed.com/docs/mbed-os/v6.15/bare-metal/index.html
 *      STM32L48 User Guide - https://www.st.com/resource/en/reference_manual/rm0351-stm32l47xxx-stm32l48xxx-stm32l49xxx-and-stm32l4axxx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf 
 */

#include <CSE321_project2_mabautis_lcd1602.h>
#include <mbed.h>
#include <CSE321_project2_mabautis_stm_methods.h>
#include <string>

#define ASCII_ZERO 48
#define ASCII_FIVE 53
#define ASCII_NINE 57

void isr_col(void); //Rising edge Interrupt Service Routine for column pins [PF_14, PE_11, PE_9, PF_13]
void isr_falling_edge(void); // Falling edge Interrupt Service Routine for column pins [PF_14, PE_11, PE_9, PF_13]

void key_handler(void); // Determines if a key press is valid (debounced) and then calls validKey or powerOnTimer
void validKey(char letter); // Handles input based on current mode
void powerOnTimer(void); // Initalizes flags and LCD then calls validKey('D')

void timer_handler(void); // Ticker to count in 1s increments if mode = 2. Queues timer() to run blocking code
void timer(void); // Handles checking remaining time, incrementing timer, and printing the associated output

void blinkLED(void); // Handles turning on and off LED with a specified blinking interval

EventQueue queue; // Initialize EventQueue to queue blocking code from ISR

// Initialize tickers
Ticker debounce_ticker;
Ticker timer_ticker;

CSE321_LCD LCD(16, 2, LCD_5x8DOTS, PB_9, PB_8); // Initialize LCD

// Initialize interrupts for columns of keypad. Set pull down to pull port down to 0 volts
InterruptIn col_0(PF_14, PullDown);
InterruptIn col_1(PE_11, PullDown);
InterruptIn col_2(PE_9, PullDown);
InterruptIn col_3(PF_13, PullDown);

char keypad[4][4] = {{'1', '2', '3', 'A'},
                     {'4', '5', '6', 'B'},
                     {'7', '8', '9', 'C'},
                     {'*', '0', '#', 'D'}}; // Enumerate keypad matrix

int mode = 0; // 0 -> Off, 1 -> Input, 2 -> Timer

int row = 0; // Current keypad row to power

int key_pressed = 0; // Flag to determine if there is a key currently pressed to debounce and handle

// Debounce flags to know when a key press is valid
int debounce_buffer = 0;
int debounced = 0;

int cursor = 0; // Keeps track of what spot user is entering numbers into M:SS

int count_direction = 0; //  0 -> Down, 1 -> Up,
int time_remaining = 0;  // In seconds (count_direction = 0)
int time_passed = 0; // In Seconds (count_direction = 1)

int main() {
  // Enable clock control register for GPIO A & C
  enable_rcc('a');
  enable_rcc('c');

  // Declare GPIOA pin 5 as an output
  set_pin_mode(5, GPIOA, 1);
  // Declare GPIOA pin 6 as an output
  set_pin_mode(6, GPIOA, 1);

  // Declare GPIOA pin 3 as an output
  set_pin_mode(3, GPIOA, 1);
  // Declare GPIOC pin 0 as an output
  set_pin_mode(0, GPIOC, 1);
  // Declare GPIOC pin 3 as an output
  set_pin_mode(3, GPIOC, 1);
  // Declare GPIOC pin 1 as an output
  set_pin_mode(1, GPIOC, 1);

  LCD.begin(); // Initialize LCD
  LCD.noBacklight(); // Turn backlight off, starting in mode 0

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

  debounce_ticker.attach(&key_handler, 1ms); // Start ticker to handle debouncing keys
  timer_ticker.attach(&timer_handler, 1s); // Start ticker to increment timer every second

  while (1) {
    if (!key_pressed) {
      row++; // Increment row
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
   queue.dispatch_once(); // Dispatch waiting events [valid_key, timer, powerOnTimer]
  }
}

void isr_col(void) { key_pressed = 1; } // Set flag to handle in debounce ticker

void isr_falling_edge(void) {
  // Set flags to 0
  key_pressed = 0;
  debounce_buffer = 0;
  debounced = 0;
}

void key_handler(void) {
  if (key_pressed) {
    if (!debounce_buffer) {
      debounce_buffer = 1; // Set flag
    } else {
      if (!debounced) {
        switch (mode) {
        case 0: // Check for power on button in Off Mode
          if (col_3.read() && keypad[row][3] == 'D') {
            queue.call(&powerOnTimer); // Power on Timer
          }
          break;
        case 1: // Check for number or valid mode letter in Input Mode
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
        case 2: // Check for valid mode letter in Timer Mode
          if (col_3.read() && keypad[row][3] != 'A') {
            queue.call(&validKey, keypad[row][3]);
          }
          break;
        }
      }
      // Set flags
      debounce_buffer = 0;
      debounced = 1;
    }
  } else
    debounce_buffer = 0; 
}

void timer_handler(void) {
  if (mode == 2) {
    queue.call(timer); // Call timer handler if in Timer Mode
  }
}

void timer(void) {
  if (mode == 2) {
    LCD.clear(); // Clear Screen 
    if (!time_remaining) { // Check if timer is over
      mode = 0; // Set mode to Off
      count_direction ? LCD.print("Times Up") : LCD.print("Time Reached"); // Print prompt based on counting direction
      for (int i = 0; i < 4; i++) { // Blink LED 4 times [1600ms]
        write_to_pin(6, GPIOA, 1);
        thread_sleep_for(200);
        write_to_pin(6, GPIOA, 0);
        thread_sleep_for(200);
      }
      thread_sleep_for(2000); // 2 second delay and then turn off display
      LCD.clear();
      LCD.noBacklight();
    } else {
      count_direction ? LCD.print("Time Passed: ")
                      : LCD.print("Time Remaining: "); // Print prompt based on counting direction
      LCD.setCursor(0, 1); // Set cursor to second row

      string time = count_direction ? to_string(time_passed / 60) + "M " +
                                          to_string(time_passed % 60) + "S"
                                    : to_string(time_remaining / 60) + "M " +
                                          to_string(time_remaining % 60) + "S"; // Print time based on counting direction

      LCD.print(&time[0]); // Pass in pointer to beginning of string to print
    }
    time_remaining--; // Decrement time remaining
    time_passed++; // Incremement time passed
  }
}

void powerOnTimer(void) {
  mode = 1; // Set mode to Input
  LCD.backlight(); // Turn on LCD backlight
  validKey('D'); // Handle mode change
}

void blinkLED(void) {
  write_to_pin(5, GPIOA, 1); // Turn LED on
  thread_sleep_for(250); // Wait .25 seconds
  write_to_pin(5, GPIOA, 0); // Turn LED off
}

void validKey(char letter) {
  switch (letter) { //Handle valid mode change
  case 'A':
      mode = 2; // Enter Timer Mode
      cursor = 0; // Reset cursor flag
      blinkLED(); // Valid key press -> blink LED
    break;

  case 'B':
    mode = 0; // Turn off Timer
    // Reset flags
    cursor = 0;
    time_remaining = 0;
    time_passed = 0;
    
    LCD.clear(); // Clear LCD
    LCD.noBacklight(); // Turn off LCD
    break;

  case 'C':
    count_direction = !count_direction; // Toggle direction flag
    blinkLED(); // Valid key press -> blink LED
    break;

  case 'D':
    mode = 1;
    // Reset flags
    time_remaining = 0;
    time_passed = 0;
    
    LCD.clear(); // Clear LCD
    LCD.print("Enter Time:"); // Print prompt
    LCD.setCursor(0, 1); // Set cursor to second row
    LCD.print("0:00"); // Print timer prompt

    blinkLED(); // Valid key press -> blink LED
    break;
  }

  if (letter >= ASCII_ZERO && letter <= ASCII_NINE) {
    int valid = 0; // Set flag to determine if number is valid
    if (cursor == 0) { // Any number valid in minutes spot
      time_remaining += (letter - ASCII_ZERO) * 60; // Subtract number by ASCII ZERO to convert to integer then convert to seconds
      valid = 1; // Set flag
    } else if (cursor == 1 && letter <= ASCII_FIVE) { // Only 0-5 allowed in 1st seconds spot
      time_remaining += (letter - ASCII_ZERO) * 10; // Subtract number by ASCII ZERO to convert to integer and multiply by 10 to represent 10s place
      valid = 1; //Set flag
    } else if (cursor == 2) { // Any number valid in 2nd seconds spot
      time_remaining += letter - ASCII_ZERO; // Subtract number by ASCII ZERO to convert to integer 
      valid = 1; // Set flag
    }
    if (valid) { // Check if valid number entered before handling
      cursor++; // Increment timer cursor
      string time = to_string(time_remaining / 60) + "M " +
                    to_string(time_remaining % 60) + "S"; // Print time based on counting direction

      LCD.clear(); // Clear LCD
      LCD.print("Enter Time:"); // Print prompt
      LCD.setCursor(0, 1); // Set cursor to second row
      LCD.print(&time[0]); // Pass in pointer to beginning of string to print
      blinkLED(); // Valid key press -> blink LED
    }
  }
}
