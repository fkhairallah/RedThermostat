# RedThermostat

[RED]Thermostat is an ESP8266-based IOT that implements thermostat functionality

It can exist as a simple temperature sensor

or it can have a display, ability to set required temp, up/down buttons and can drive supplemental electric heat

***Because of the minimal functionaly, this is considered the master template for all other [RED] devices***

# Version History

## V1.1
    Initiale release on Arduino IDE

## V1.2
    Moved to Platform.IO

## V1.3
    Upgraded to support ArduinoJSON 6 
    WifiConfig & MQTTConfig now contain customizable sections

## I2C-Display Branch
Full implementation of I2C OLD display


# Hardware Notes

This program will probably run on a multitude of hardware. One variance is a naked temperature reporting board, the other is equipped with a display up/down keys and acts as a traditional thermostat. The two version are differentied with the ***#define DISPLAY_PRESENT***

- Pin 0  is the program pin but also wakes the device and turn the screen on
- Pin 2 blueLED on ESP12e
- Pin 1 SCL drives the display
- Pin 3 SDA drives the display
- Pin 5 UP button
- Pin 4 DOWN button
- Pin 12 OneWire bus for temp sensors
- Pin 13 Drives supplemental heat relay
