/************************************************************************************

   Thermostat ESP

   Configuration parameters are found in .h file

  This code converts a dump Honneywell Thermostat to a smart one that interfaces with OpenHab through MQTT

  This code provides WIFI & MQTT configuration through the use of WIFI manager. Initial setup allow
  configuration of WIFI network and MQTT server

  It provides OTA functionality.

 *************************************************************************************/
#include <Arduino.h>
#include <RedGlobals.h>
#include <EasyButton.h>

// Sensor is being heated by the board's operation. We created a linear adjustment
// base on line-fitting a few before and after observations
float mAdjustment = 0.928;
float cAdjustment = 0.818;

float tempAccumulator;   // keeps sum of  temperature as reported by remote thermostat
int tempNumberOfReading;  // keeps # of readings
unsigned long lastTempSend;
float averageTemp;        // Average temp for the last interval -- what is displayed
Ticker ticker;            // ticker

#ifdef BUTTONS_PRESENT
EasyButton wakeButton(pgm_pin);
EasyButton upButton(up_pin);
EasyButton downButton(down_pin);
bool heatIsOn = false;
int requiredTemperature;
#endif

/*
 * ********************************************************************************

 a few routines to drive the onboard blueLED

 * ********************************************************************************
*/
void ledON()
{
  digitalWrite(blueLED, false);
}
void ledOFF()
{
  digitalWrite(blueLED, true);
}

void tick()
{
  //toggle state
  int state = digitalRead(blueLED); // get the current state of GPIO1 pin
  digitalWrite(blueLED, !state);    // set pin to the opposite state
}

/*
 * ********************************************************************************

  This routine will adjust for the temperature generated by the CPU

 * ********************************************************************************
*/

float compensateForBoardHeat(float inTemp)
{
  return inTemp * mAdjustment + cAdjustment;
}

bool changeSetTemperature(const HomieRange &range, const String &value)
{
  Homie.getLogger() << "Tide Handler got " << value << endl;
  // lightNode.setProperty("pause").send(value);

  // Homie.getMqttClient().publish("foo", 1, true,  value.c_str());

  return true;
}
/*
 * ********************************************************************************

      Homie setup & loop


 * ********************************************************************************
*/
HomieNode thermostatNode("temperature", "Temperature", "temperature");
HomieSetting<const char *> location("location", "Name or Location of the device");
bool debugMode = false;

// Code which should run AFTER the WiFi is connected.
void setupHandler()
{


  Homie.getLogger() << "Location: "
                    << location.get()
                    << endl;

  }

// Code which should run in normal loop(), after setupHandler() finished.
void loopHandler()
{
  // if (millis() - lastTemperatureSent >= TEMPERATURE_INTERVAL * 1000UL || lastTemperatureSent == 0)
  // {
  //   float temperature = 22; // Fake temperature here, for the example
  //   Homie.getLogger() << "Temperature: " << temperature << " °C" << endl;
  //   thermostatNode.setProperty("degrees").send(String(temperature));
  //   lastTemperatureSent = millis();
  // }
  // service temperature and other sensos
  serviceSensors();

#ifdef BUTTONS_PRESENT
  // service the buttons
  wakeButton.read();
  upButton.read();
  downButton.read();
#endif

#ifdef DISPLAY_PRESENT
  // service display taking care of diming it and turning it off after _DISPLAY_INTERVAL
  serviceDisplay();
#endif

  handleConsole(); // handle any commands from console
}


/*
 * ********************************************************************************

   Perform the initial hardware setup and then sequence the starting of the
   various modules

 * ********************************************************************************
*/

void setup() {
  pinMode(blueLED, OUTPUT);
  setupConsole();

  // console.enableSerial(&Serial, true);
  console.print("[RED]Thermostat ");
  console.println(VERSION);

#ifdef HEAT_POWER
  // configure the pin and make sure heat isn't turned on
  pinMode(heat_pin, OUTPUT);
  digitalWrite(heat_pin, LOW);
#endif

#ifdef BUTTONS_PRESENT
  
  wakeButton.begin();
  wakeButton.onPressed(wakeButtonPressed);
  wakeButton.onPressedFor(1500, statusButtonPressed);
  upButton.begin();
  upButton.onPressed(upButtonPressed);
  downButton.begin();
  downButton.onPressed(downButtonPressed);
#endif


  // homie setup
  Homie_setBrand("[RED]");

  Homie_setFirmware("red-tide-firmware", "1.0.0");
  Homie.setLedPin(blueLED, HIGH);
  Homie.setSetupFunction(setupHandler).setLoopFunction(loopHandler);
  // define the channels
  thermostatNode.advertise("temperature").setName("Current Temperature").setDatatype("float").setUnit("ºF");
  thermostatNode.advertise("settemperature").setName("Set Temperature").setDatatype("float").setUnit("ºF").settable(changeSetTemperature);

  Serial << "Homie Setup...";
  Homie.setup();
  Serial << "Done" << endl;

  // configure thermostat sensor
  tempAccumulator = 0;
  tempNumberOfReading = 0;
  averageTemp = -9999;
  lastTempSend = millis();
  configSensors(_TEMP_SENSOR_PERIOD, &updateTemperature);

#ifdef DISPLAY_PRESENT
  // WeMOD D1 uses Pins 4&5 for I2C
  // switch UART pins to digital mode so they can be used by the display
  // console.disableSerial();
  pinMode(SCL_pin, FUNCTION_3);
  pinMode(SDA_pin, FUNCTION_3);
  pinMode(SCL_pin, OUTPUT);
  pinMode(SDA_pin, OUTPUT);
  configureDisplay();
  displayStatus();
#endif

  ledOFF(); // turn LED off to save power
}


/*
 * ********************************************************************************

   main loop services all modules: Wifi, MQTT, HP, console and webserver

 * ********************************************************************************
*/
void loop() {

  Homie.loop();

}

/*
 * ********************************************************************************

   Button control rotines

 * ********************************************************************************
*/
#ifdef BUTTONS_PRESENT

void wakeButtonPressed()
{
  //console.println("Button: Wake");
  displayTemperature(averageTemp);
}

void statusButtonPressed()
{
  //console.println("Button: status");
  displayStatus();
}

void upButtonPressed()
{
  //console.println("Button: up");
  requiredTemperature++;
  displayRequiredTemperature(requiredTemperature);
  //publishRequiredTemp(requiredTemperature);
  thermostatNode.setProperty("settemperature").send(String(requiredTemperature));
}

void downButtonPressed()
{

  //console.println("Button: down");
  requiredTemperature--;
  displayRequiredTemperature(requiredTemperature);

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
  tempAccumulator += temp;
  tempNumberOfReading++;

  if ((unsigned long)(millis() - lastTempSend) >= _SEND_ROOM_TEMP_INTERVAL_MS)
  {
    console.println("Raw temp = " + String(temp));
    averageTemp = compensateForBoardHeat( tempAccumulator / tempNumberOfReading );
    tempAccumulator = 0;
    tempNumberOfReading = 0;
    //publishTemperature(averageTemp);
    thermostatNode.setProperty("temperature").send(String(averageTemp));
    //displayTemperature(averageTemp);
    lastTempSend = millis();

    //tick(); // comment out to save power
  }

}

