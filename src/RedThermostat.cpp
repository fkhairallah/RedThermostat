/************************************************************************************

   Thermostat ESP

   Configuration parameters are found in .h file

  This code converts a dump Honneywell Thermostat to a smart one that interfaces with OpenHab through MQTT

  This code provides WIFI & MQTT configuration through the use of WIFI manager. Initial setup allow
  configuration of WIFI network and MQTT server

  It provides OTA functionality.

 *************************************************************************************/
#include <Arduino.h>
#include <pins.h>
#include <RedGlobals.h>
#include <EasyButton.h>



float tempAccumulator;   // keeps sum of  temperature as reported by remote thermostat
int tempNumberOfReading;  // keeps # of readings
unsigned long lastTempSend;
float averageTemp;        // Average temp for the last interval -- what is displayed
Ticker ticker;            // ticker

#ifdef DISPLAY_PRESENT
EasyButton wakeButton(pgm_pin);
EasyButton upButton(up_pin);
EasyButton downButton(down_pin);
#endif

void tick()
{
  //toggle state
  int state = digitalRead(blueLED); // get the current state of GPIO1 pin
  digitalWrite(blueLED, !state);    // set pin to the opposite state
}

/*
 * ********************************************************************************

   Perform the initial hardware setup and then sequence the starting of the
   various modules

 * ********************************************************************************
*/

void setup() {
  pinMode(blueLED, OUTPUT);

  // configure the pin and make sure heat isn't turned on
  pinMode(heat_pin, OUTPUT);
  digitalWrite(heat_pin, LOW);

#ifdef DISPLAY_PRESENT
  wakeButton.begin();
  wakeButton.onPressed(wakeButtonPressed);
  wakeButton.onPressedFor(1500, statusButtonPressed);
  upButton.begin();
  upButton.onPressed(upButtonPressed);
  downButton.begin();
  downButton.onPressed(downButtonPressed);
#endif

  // start ticker with 0.5 because we start in AP mode and try to connect
  ticker.attach(0.6, tick);

  setupConsole();

  //console.enableSerial(&Serial, true);
  console.print("[RED]Thermostat ");
  console.println(VERSION);


  // Configure WIFI
  configureESP(); // load configuration from FLASH & configure WIFI

  digitalWrite(blueLED, LOW);
  console.enableTelnet(23);
  //console.enableUDP(WiFi.localIP(), 10110);
  console.print("Telnet Enabled on ");
  console.println(WiFi.localIP().toString());

  configureMQTT();


  //  // configure thermostat sensor
  tempAccumulator = 0;
  tempNumberOfReading = 0;
  averageTemp = -9999;
  lastTempSend = millis();
  configSensors(_TEMP_SENSOR_PERIOD, &updateTemperature);

#ifdef DISPLAY_PRESENT
  // switch UART pins to digital mode so they can be used by the display
  console.disableSerial();
  pinMode(SCL_pin, FUNCTION_3);
  pinMode(SDA_pin, FUNCTION_3);
  pinMode(SCL_pin, OUTPUT);
  pinMode(SDA_pin, OUTPUT);

  configureDisplay();
  displayStatus();
#endif
}


/*
 * ********************************************************************************

   main loop services all modules: Wifi, MQTT, HP, console and webserver

 * ********************************************************************************
*/
void loop() {

  checkConnection();  // check WIFI connection

  // service temperature and other sensos
  serviceSensors();

#ifdef DISPLAY_PRESENT
  // service the buttons
  wakeButton.read();
  upButton.read();
  downButton.read();

  // service display taking care of diming it and turning it off after _DISPLAY_INTERVAL
  serviceDisplay();
#endif

  checkMQTTConnection(); // check MQTT

  handleConsole(); // handle any commands from console

}

/*
 * ********************************************************************************

   Button control rotines

 * ********************************************************************************
*/
#ifdef DISPLAY_PRESENT

void wakeButtonPressed()
{
  console.println("Button: Wake");
  displayTemperature(averageTemp);
}

void statusButtonPressed()
{
  console.println("Button: status");
  displayStatus();
}

void upButtonPressed()
{
  char str[32];

  console.println("Button: up");
  requiredTemperature++;
  displayRequiredTemperature(requiredTemperature);
  sprintf(str, "%i", requiredTemperature);
  mqtt_client.publish(mqtt_requiredTemperature_topic, str);

}

void downButtonPressed()
{
  char str[32];

  console.println("Button: down");
  requiredTemperature--;
  displayRequiredTemperature(requiredTemperature);
  sprintf(str, "%i", requiredTemperature);
  mqtt_client.publish(mqtt_requiredTemperature_topic, str);

}
#endif


/*
 * ********************************************************************************

   This is callback routine that is called when the temperature sensor receives
   a reading. It collects the data into a long term average then,
   every _TEMP_SENSOR_PERIOD report the value back to the MQTT server

 * ********************************************************************************
*/
void updateTemperature(float temp, float temp2)
{
  char str[128];
  tempAccumulator += temp;
  tempNumberOfReading++;

  if ((unsigned long)(millis() - lastTempSend) >= _SEND_ROOM_TEMP_INTERVAL_MS)
  {
    console.println("Reporting temp reading of " + String(temp));
    averageTemp = tempAccumulator / tempNumberOfReading;
    tempAccumulator = 0;
    tempNumberOfReading = 0;
    sprintf(str, "%.1f", averageTemp);
    mqtt_client.publish(mqtt_temperature_topic, str);
    lastTempSend = millis();

    tick();
  }

}

