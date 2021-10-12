# About
Alarm system that users can program to count up or down from 9 minutes and 59 seconds by utilizing a 4x4 matrix keypad.
# Features
* Count up or count down from a maximum time of 9 minutes and 59 seconds
* Turn off the system by pressing B on the keypad
* Input time or turn on the system by pressing D on the keypad
* Begin timer by pressing A on the keypad
* Press C on the keypad to switch countdown modes
* Blinks LEDs when time is reach or a key is pressed

# Required Materials
* Nucleo L4R5ZI
* 4x4 Matrix Keypad
* I2C Controller
* 4 LEDs
* Resistors
* Wires

# Resources and References
* MBED Bare Metal Guide - https://os.mbed.com/docs/mbed-os/v6.15/bare-metal/index.html

* STM32L48 User Guide - https://www.st.com/resource/en/reference_manual/rm0351-stm32l47xxx-stm32l48xxx-stm32l49xxx-and-stm32l4axxx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf 

# Getting Started
Once the program is built and run, the 4x4 matrix keypad is used to control the system. D turns on the system and switches to Input Time mode, C switches timer direction to count up or down, B turns off the timer, and A is used to start the timer.

# Modules

## main.cpp:
Main file which contains the initialization and code to run the timer system.

### Things Declared: 
* debounce_ticker [Ticker] - 1 millisecond interval ticker to ensure that key presses are debounced to validate input
* timer_ticker [Ticker] - 1 second interval ticker to handle the timer when in mode 2
* LCD [CSE321_LCD] - LCD instance as defined by lcd1602.cpp
* col_0, col_1, col_2, col_3 [InterruptIn] - Interrupts associated with the 4x4 matrix keypad columns. NOTE: pins sets to PullDown mode to ensure pin is pulled to 0V
* keypad char[4][4] - Nested array to represent keypad buttons
* mode [int] -  0 -> Off, 1 -> Input, 2 -> Timer
* row [int] - Current keypad row to power
* key_pressed [int] - Flag to determine if there is a key currently pressed to debounce and handle
* debounce_buffer [int] - Debounce flag to know when a key press is valid
* debounced [int] - Flag to know if key has been debounced already
* cursor [int] - Keeps track of what spot user is entering numbers into M:SS
* count_direction [int] - 0 -> Down, 1 -> Up,
* time_remaining [int] - In seconds (count_direction = 0)
* time_passed [int] - In Seconds (count_direction = 1)

### API and Built-In Elements Used:
* Mbed – Microcontroller API used for InterruptIn initialization

### Custom Functions:
* isr_col(void) - Rising edge Interrupt Service Routine for column pins [PF_14, PE_11, PE_9, PF_13]
* isr_falling_edge(void) - Falling edge Interrupt Service Routine for column pins [PF_14, PE_11, PE_9, PF_13]
* key_handler(void) - Determines if a key press is valid (debounced) and then calls validKey or powerOnTimer
* validKey(char letter) - Handles input based on current mode
* powerOnTimer(void) - Initalizes flags and LCD then calls validKey('D')
* timer_handler(void) - Ticker to count in 1s increments if mode = 2. Queues timer() to run blocking code
* timer(void) - Handles checking remaining time, incrementing timer, and printing the associated output
* blinkLED(void) - Handles turning on and off LED with a specified blinking interval

## stm_methods.cpp:
Contains initialization code for the RCC and GPIO pins and code to write to MODER

### API and Built-In Elements Used:
* Mbed – Microcontroller API used for pin definitions

### Custom Functions:
* write_to_pin(pin, *port, value) - Writes the designated value (0/1) to the entered pin and port
* set_pin_mode(pin, *port, mode) - Set the designed pin/port to be an input/output
* enable_rcc(*port) - Enable the reset control clock for the specified GPIO port

## lcd1602.cpp:
File that declares the initialization and methods to operate the 1602 LCD for printing, clearing, and powering the display.

### Things Declared: 
* CSE321_LCD : Class definition to enable and utilize the 1602 LCD

### API and Built-In Elements Used:
* Mbed – Microcontroller API used for pin definitions
* lcd1602 - API to initialize and control 1602 LCD

### Custom Functions:
* clear() - Clear display and reset cursor to (0,0)
* setCursor(a,b) -  Puts cursor in col a and row b, note indexing starts at 0
* print("string") - Prints strings to LCD