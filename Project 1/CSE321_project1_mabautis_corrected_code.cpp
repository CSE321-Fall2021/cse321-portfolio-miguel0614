#include "mbed.h"

// Create a thread to drive an LED to have an on time of 2000ms and off time of 500ms

Thread controller; // Allows event based execution, scheduling, and priority management

// Function prototypes
void led_handler();     // Toggles blue LED on and off
void set_toggle_flag(); // Allow state to be toggled
void toggle_state();    // Toggle state to halt the blinking LED

DigitalOut blue_led(LED2);     // Set blue LED as an output
InterruptIn button_1(BUTTON1); // Set button 1 as an input

int _state = 0;       // Active low state which allows the thread handler to toggle the blue LED
int toggle_flag = 0; // Toggle flag must be 1 to allow for state change

int main()
{
  printf("----------------START----------------\n");
  printf("Starting state of thread: %d\n", controller.get_state());
  controller.start(led_handler); // Begin thread execution
  printf("State of thread right after start: %d\n", controller.get_state());
  button_1.rise(set_toggle_flag); // On rising edge, set the toggle flag
  button_1.fall(toggle_state);    // On falling edge, attempt to toggle the state

  return 0;
}

// Thread handler to toggle blue LED on and off
void led_handler()
{
  while (true)
  {
    if (_state == 0)
    {
      blue_led = !blue_led;
      printf("Toggle the blue LED on");
      thread_sleep_for(2000); // Sleep thread for 2000ms

      blue_led = !blue_led;
      printf("Toggle the blue LED off");
      thread_sleep_for(500); // Sleep thread for 500ms
    }
  }
}

void set_toggle_flag()
{
  toggle_flag = 1; // Set flag to toggle blue LED handler
}

void toggle_state()
{
  if (toggle_flag == 1)
  {
    // Increment state and take remainder to keep the state either 0 or 1
    _state++;
    _state %= 2;

    toggle_flag = 0; // Set toggle flag back to 0 until next rising edge
  }
}