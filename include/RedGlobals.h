
#ifndef RED_GLOBALS_H
#define RED_GLOBALS_H

#include <Homie.h>
#include <dConsole.h>
#include <Ticker.h>

// Pick either the small D1 with display or the Garage One.
#define D1
//#define GARAGE
// #define CUSTOM_BOARD

#ifndef _PINS_H
#include <pins.h>
#endif

/*
 * ********************************************************************************
 *            START CONFIGURATION SECTION
 * ********************************************************************************
*/

#define TEMP_SENSOR_PRESENT // indicates a temperature sensor is present

//#ifdef D1
#define DISPLAY_PRESENT     // indicates a screen is present
#define BUTTONS_PRESENT     // indicates up/down buttons are present
//#endif
//#define HEAT_POWER          // relay to turn aux heat on

#define _TEMP_SENSOR_PERIOD 10000         // in ms the frequency of temperature sensor reading
#define TEMPERATURE_PRECISION 9           // Lower resolution
#define _SEND_ROOM_TEMP_INTERVAL_MS 60000 // in ms how often the temperature is sent back to the server
#define _DISPLAY_INTERVAL 10000           // in ms how long before the display is dimmed then turned off

#define VERSION "V2.0"  // N.B: document changes in README.md
#define MQTT_TOPIC_PREFIX "thermostat" // prefix for all MQTT topics

// in RedThermostat
extern bool debugMode;

// in esp_ota.h
void configureOTA(const char *hostName);
void handleOTA();


// in console.ino
extern dConsole console;
void setupConsole();
void handleConsole();

#ifdef TEMP_SENSOR_PRESENT
// in Sensors2.ino
extern int numberOfDevices; // Number of temperature devices found
void configSensors(long interval, void (*sensorCallback)(float insideTemp, float outsideTemp));
void serviceSensors();
#endif

// in display.cpp
#ifdef DISPLAY_PRESENT
bool configureDisplay();
void displayStatus();
void displayRequiredTemperature(int temp);
void serviceDisplay();
void displayTemperature(float averageTemp);
#endif


// in RedThermostat
extern float averageTemp;        // Average temp for the last interval -- what is displayed
void ledON();
void ledOFF();
void tick();
void updateTemperature(float temp, float outdoorTemp);

#ifdef DISPLAY_PRESENT
extern bool heatIsOn;
extern int requiredTemperature;
void wakeButtonPressed();
void statusButtonPressed();
void upButtonPressed();
void downButtonPressed();
#endif


#endif
