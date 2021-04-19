
#ifndef RED_GLOBALS_H
#define RED_GLOBALS_H
#include <dConsole.h>
#include <PubSubClient.h>
#include <Ticker.h>

#ifndef _PINS_H
#include <pins.h>
#endif

/*
 * ********************************************************************************
 *            START CONFIGURATION SECTION
 * ********************************************************************************
*/

#define TEMP_SENSOR_PRESENT // indicates a temperature sensor is present
//#define DISPLAY_PRESENT                 // indicates a temperature sensor is present

#ifdef TEMP_SENSOR_PRESENT
#define _TEMP_SENSOR_PERIOD 10000         // in ms the frequency of temperature sensor reading
#define _SEND_ROOM_TEMP_INTERVAL_MS 60000 // in ms how often the temperature is sent back to the server
#define TEMPERATURE_PRECISION 9           // Lower resolution
#endif

#define VERSION "V1.3"  // N.B: document changes in README.md
#define MQTT_TOPIC_PREFIX "thermostat" // prefix for all MQTT topics

// in WiFiConf
extern char myHostName[];
extern char deviceLocation[];
extern char mqttServer[];
extern char mqttPort[];
extern char mqttUser[];
extern char mqttPwd[];
void configureESP();       // load configuration from FLASH & configure WIFI
void checkConnection(); // check WIFI connection
void writeConfigToDisk();
void configureOTA(char *hostName);

// in MQTTConfig
void configureMQTT();
bool checkMQTTConnection();
void mqttDisconnect();
void mqttPublishTemperature(char* tempStr);

// in console.ino
extern dConsole console;
void setupConsole();
void handleConsole();

#ifdef TEMP_SENSOR_PRESENT
// in Sensors2.ino
void configSensors(long interval, void (*sensorCallback)(float insideTemp, float outsideTemp));
void serviceSensors();
#endif


// in RedThermostat
void ledON();
void ledOFF();
void tick();
void updateTemperature(float temp, float outdoorTemp);
#ifdef DISPLAY_PRESENT
extern int requiredTemperature;
void wakeButtonPressed();
void statusButtonPressed();
void upButtonPressed();
void downButtonPressed();
#endif


#endif
