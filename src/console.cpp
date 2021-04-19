/**********************************************************************************
 * 
 *    Implements functionality for telnet and serial console
 * 
 *********************************************************************************/
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager
#include <RedGlobals.h>

dConsole console;

void setupConsole()
{
  console.enableSerial(&Serial, true);
  console.enableTelnet(23);
}


void handleConsole()
{
  // console
  if (console.check())
  {

    if (strcmp(console.commandString, "?") == 0)
    {
      console.println("\n\n\n[RED]Thermostat");
      console.print("IP address: ");
      console.println(WiFi.localIP().toString());
      console.println("Available commands are:  1, location room, mqtt server port, status, reset (Factory), reboot");
    }
    if (strcmp(console.commandString, "reset") == 0)
    {
      console.print("Reseting configuration...");
      //reset settings - for testing
      WiFiManager wifiManager;
      wifiManager.resetSettings();
      console.println(" Done.");
    }
    if (strcmp(console.commandString, "reboot") == 0)
    {
      console.print("Rebooting...");
      delay(200);
      //reset and try again, or maybe put it to deep sleep
      ESP.reset();
      delay(5000);
    }
    if (strcmp(console.commandString, "status") == 0)
    {
      console.println(deviceLocation);
      console.println(myHostName);
      console.printf("MQTT Server %s, port: %s\r\n", mqttServer, mqttPort);
    }

    if (strcmp(console.commandString, "mqtt") == 0)
    {
      strcpy(mqttServer, console.parameterString);
      writeConfigToDisk();
      console.print("MQTT server changed to ");
      console.println(mqttServer);
      mqttDisconnect();
    }
    if (strcmp(console.commandString, "location") == 0)
    {
      strcpy(deviceLocation, console.parameterString);
      writeConfigToDisk();
      console.printf("location changed to %s\r\n", deviceLocation);
      console.println("Change will take effect after next reboot");
    }
    if (strcmp(console.commandString, "1") == 0)
    {
      console.println("Scanning for 1Wire");
      configSensors(_TEMP_SENSOR_PERIOD, &updateTemperature);
    }

    console.print("[RED]> ");
  }
}
