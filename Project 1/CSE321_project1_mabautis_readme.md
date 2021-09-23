-------------------
About
-------------------
Project Description: 
This project creates a thread which controls a blue LED. The LED blinks periodically, being on for 2000ms and off for 500ms.

Contributor List:

--------------------
Features
--------------------
- Controls blue LED 
- Blinks on for 2000ms and off for 500ms

--------------------
Required Materials
--------------------
-Nucleo L4R5ZI

--------------------
Resources and References
--------------------
Mbed Documentation - https://os.mbed.com/docs/mbed-os/v6.14/introduction/index.html

--------------------
Getting Started
--------------------
Once the program has begun running, the controller will automatically start controlling the blue LED and begin blinking with the specified period.

--------------------
CSE321_project1_mabautis_corrected_code.cpp:
--------------------
 This file contains the main method which defines the thread behavior.

The GPIO pin with the blue LED is set to DigitalOut which sets it as an output while doing all the associated initialization as well and the button_1 is set as InterruptIn which initializes its interrupt capabilities so the program can utilize the rising and falling edges of the button push.

With this functionality defined, the thread handles blinking the LED at a rate of 2000ms on and 500ms off while watching for button interrupts to pause this behavior.

----------
Things Declared
----------
blue_led – DigitalOut signal which corresponds to the blue LED GPIO pin
button_1 – InterruptIn which corresponds to button 1
_state – Active low variable to determine whether the LED should be blinking
toggle_flag – Determines whether the state should be changed to halt/begin LED blinking

----------
API and Built In Elements Used
----------
Mbed – Microcontroller API used for OS
DigitalOut – Defined GPIO output and initialization
InterruptIn – Initializes input as an interrupt
LED2 – Represents blue LED
BUTTON1 – Represents button 1 

----------
Custom Functions
----------
led_handler: 
Used by the thread to control the LED’s behavior i.e. whether it is blinking or not and at what speed.
	Inputs: None
	Global References: _state, blue_led, printf

set_toggle_flag:
On rising edge of button press, the toggle flag is set.
Inputs: None
	Global References: toggle_flag

toggle_state:
On falling edge of button press, if the toggle_flag is set, the state goes from 0 to 1 or 1 to 0 and the toggle_flag is set back to 0.
Inputs: None
	Global References: toggle_flag, _state

