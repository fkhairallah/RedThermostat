/*******************   See README.md  *****************************/
#ifndef _PINS_H
#define _PINS_H



#ifdef D1
// pin definition for Wemos D1 Mini & wiring colors
#define pgm_pin 0   // Wemos D3
#define up_pin 15   // Not Used
#define down_pin 14 // Not Used
#define blueLED 2   // Wemos D4
#define SCL_pin 5   // Wemos D1 (BLUE)
#define SDA_pin 4   // Wemos D2 (GREEN)
#define ONE_WIRE_BUS 12 // Wemos D6 (YELLOW)
#endif

#ifdef GARAGE
// pin definition for Wemos D1 Mini & wiring colors
#define pgm_pin 0       
#define blueLED 2       
#define ONE_WIRE_BUS 5
#endif

#ifdef CUSTOM_BOARD
#define pgm_pin 0
#define blueLED 2 // blue LED light on ESP12e
#define SCL_pin 1
#define SDA_pin 3
#define up_pin 5
#define down_pin 4
#define ONE_WIRE_BUS 12
#define heat_pin 13
#endif


#endif