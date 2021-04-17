/*
 * ********************************************************************************

   This code implements all sensor related function
 * ********************************************************************************
*/
#include <OneWire.h>
#include <DallasTemperature.h>
#include <pins.h>
#include <RedGlobals.h>

#ifdef TEMP_SENSOR_PRESENT

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

int numberOfDevices; // Number of temperature devices found

DeviceAddress tempDeviceAddress;        // inside Sensor
DeviceAddress outsideTempDeviceAddress; // Outside Sensor

void (*sensorCallBackFunction)(float temp, float outsideTemp); // holds the callback function pointer

unsigned long lastSensorQuery;    // holds time since last sensor query
unsigned long sensorSendInterval; // interval to send temp update

/*
 * ********************************************************************************

   configure sensors, get # and set resolution

 * ********************************************************************************
*/

void configSensors(long interval, void (*sensorCallback)(float temp, float outsideTemp))
{
  DeviceAddress deviceAddress;

  sensorSendInterval = interval;           // remember the interval to send temp update
  sensorCallBackFunction = sensorCallback; // remember the callback function

  // Start up the library
  sensors.begin();
  console.println("OneWire Library Begun");

  // Grab a count of devices on the wire
  numberOfDevices = sensors.getDeviceCount();
  console.print("Found " + String(numberOfDevices));
  console.println(" 1wire devices.");

  // Loop through each device, print out address
  for (int i = 0; i < numberOfDevices; i++)
  {
    // Search the wire for address
    if (sensors.getAddress(deviceAddress, i))
    {
      // set the resolution to TEMPERATURE_PRECISION bit (Each Dallas/Maxim device is capable of several different resolutions)
      console.print("0x");
      for (int j = 0; j < 8; j++)
      {
        if (deviceAddress[j] < 16)
        {
          console.print('0');
        }
        console.print(deviceAddress[j], HEX);
        if (i < 7)
        {
          console.print(" ");
        }
      }
      console.println();
    }
  }
  // Run this to initially discover addresses on the bus and then
  // use those addresses and manually assign them (see above) once you know
  // the devices on your bus (and assuming they don't change).
  //
  // the two sensor are positioned and marked by address
  sensors.getAddress(tempDeviceAddress, 0);        // Get address of inside sensor
  sensors.getAddress(outsideTempDeviceAddress, 1); // Get address of outsideside sensor

  sensors.setResolution(tempDeviceAddress, TEMPERATURE_PRECISION);
  sensors.setResolution(outsideTempDeviceAddress, TEMPERATURE_PRECISION);

  lastSensorQuery = millis() + sensorSendInterval; // Set the time so we do a reading at first boot
}

/*
 * ********************************************************************************

   Service sensors routine requests temperature reading every period
   It then wait 1 second, reads the temperature and calls a callback
   routine to report it back to the main code
 * ********************************************************************************
*/

void serviceSensors()
{
  float outsideTempF = -100;

  if ((unsigned long)(millis() - lastSensorQuery) >= sensorSendInterval)
  {
    console.println("Requesting Temp...");
    sensors.requestTemperatures(); // Send the command to get temperatures

    float tempF = sensors.getTempF(tempDeviceAddress);
    if (numberOfDevices == 2)
      outsideTempF = sensors.getTempF(outsideTempDeviceAddress);

    sensorCallBackFunction(tempF, outsideTempF);
    lastSensorQuery = millis();
  }
}

#endif