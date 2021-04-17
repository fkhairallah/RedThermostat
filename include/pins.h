/*
 * ********************************************************************************
 * 
 * This program runs undercabinet LED lights AND temperature sensor from a single
 * WEMOS D1 esp8266
 * 
 * 
 * 
 * Hardware configuration:
 * 
 *   - Blue LED connected to pin 2
 *   - 1Wire thermocouple connected to pin 12
 

    Hardware Notes:

    - GPIO-0 must be tied to ground for programming
    - GPIO-0 floats to run program
    - GPIO-0 runs Red LED on Huzzah
    - GPIO-2 is tied to Blue Led (*NOT* a PWM pin)
    - GPIO-13 is RESERVED

 * ********************************************************************************
 */
#ifndef _PINS_H
#define _PINS_H

#define pgm_pin 0
#define blueLED 2 // blue LED light on ESP12e
#define SCL_pin 1
#define SDA_pin 3
#define up_pin 5
#define down_pin 4
#define ONE_WIRE_BUS 12
#define heat_pin 13

#endif