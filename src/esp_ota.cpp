/**********************************************************************************
 * 
 * 
 **********************************************************************************
 *
 *********************************************************************************/
#include <RedGlobals.h>
#include <ArduinoOTA.h>

bool otaInProgress; // flags if OTA is in progress

/*
 * ********************************************************************************
 * 
 * ********************************************************************************
 */
void configureOTA(const char *hostName)
{
  // configure OTA
  otaInProgress = false; // used to stop other activities during OTA update

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(hostName);

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    otaInProgress = true;
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
    {
      type = "sketch";
    }
    else
    { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    console.println("Start updating " + type);
  });

  ArduinoOTA.onEnd([]() {
    otaInProgress = false;
    console.println("\nEnd");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    char buffer[100];
    sprintf(buffer, "Progress: %u%%\r", (progress / (total / 100)));
    console.println(buffer);
  });

  ArduinoOTA.onError([](ota_error_t error) {
    otaInProgress = false;
    char buffer[100];
    sprintf(buffer, "Error[%u]: ", error);
    console.println(buffer);
    if (error == OTA_AUTH_ERROR)
    {
      console.println("Auth Failed");
    }
    else if (error == OTA_BEGIN_ERROR)
    {
      console.println("Begin Failed");
    }
    else if (error == OTA_CONNECT_ERROR)
    {
      console.println("Connect Failed");
    }
    else if (error == OTA_RECEIVE_ERROR)
    {
      console.println("Receive Failed");
    }
    else if (error == OTA_END_ERROR)
    {
      console.println("End Failed");
    }
  });

  ArduinoOTA.begin();
}

void handleOTA()
{
  ArduinoOTA.handle();
}
