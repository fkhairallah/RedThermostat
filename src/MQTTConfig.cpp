/**********************************************************************************
 *
 * Configure the MQTT server by:
 *     - create all the topic using prefix/location/subtopic
 *     - configure MQTT server and port and setup callback routine
 *     - attempt a connection and log to debug topic if success
 * 
 *********************************************************************************/
#include <RedGlobals.h>


WiFiClient espClient;
PubSubClient mqtt_client(espClient);

// mqtt client settings
char mqtt_topic[64] = "thermostat/default";                                         //contains current settings
char mqtt_temperature_topic[64] = "thermostat/default/temperature";                 //temperature
#ifdef DISPLAY_PRESENT
char mqtt_requiredTemperature_topic[64] = "thermostat/default/requiredTemperature"; //listens for commands
#endif
char mqtt_debug_topic[64] = "thermostat/default/debug";                             //debug messages
char mqtt_debug_set_topic[64] = "thermostat/default/debug/set";                     //enable/disable debug messages


int secondsWithoutMQTT;

// MQTT Settings
// debug mode, when true, will send all packets received from the heatpump to topic mqtt_debug_topic
// this can also be set by sending "on" to mqtt_debug_set_topic
bool debugMode = false;
bool retain = true; //change to false to disable mqtt retain


/*
 * ********************************************************************************

  ********************  CUSTOMIZABLE SECTION  ***************************

 * ********************************************************************************
*/

// configure all topics based on function & location
void configureTopics() 
{
    // heatpump/location/...
  sprintf(mqtt_topic, "%s/%s", MQTT_TOPIC_PREFIX, deviceLocation);
  sprintf(mqtt_temperature_topic, "%s/%s/temperature", MQTT_TOPIC_PREFIX, deviceLocation);
  sprintf(mqtt_debug_topic, "%s/%s/debug", MQTT_TOPIC_PREFIX, deviceLocation);
  sprintf(mqtt_debug_set_topic, "%s/%s/debug/set", MQTT_TOPIC_PREFIX, deviceLocation);
#ifdef DISPLAY_PRESENT
  sprintf(mqtt_requiredTemperature_topic, "%s/%s/requiredTemperature", MQTT_TOPIC_PREFIX, deviceLocation);
#endif

}


// this is called when a connection is established with the server
// it subscribe to all define and needed topics
void subscribeToTopics()
{
  
#ifdef DISPLAY_PRESENT
      mqtt_client.subscribe(mqtt_requiredTemperature_topic);
#endif
      mqtt_client.subscribe(mqtt_debug_set_topic);
}

// This routine is called when an MQTT message is received 
// it handles any needed functionality and return TRUE
// returns FALSE if the topic is outside the scope of this function
bool processMQTTcommand(char* topic, char* message)
{
#ifdef DISPLAY_PRESENT
  if (strcmp(topic, mqtt_requiredTemperature_topic) == 0)
  {
    int temp = atoi(message);
    if ((temp > 45) && (temp < 100))
    {
      requiredTemperature = temp;
      displayRequiredTemperature(temp);
      writeConfigToDisk(); // save it
    }
    return true;
  }
  else
  
#endif
  return false;
}

#ifdef TEMP_SENSOR_PRESENT
// Publish the temperature to the MQTT server
void mqttPublishTemperature(char* tempStr)
{
  mqtt_client.publish(mqtt_temperature_topic, tempStr);
}
#endif



/*
 * ********************************************************************************

   This routine handles all MQTT callbacks and processes the commands sent to hp
   1. it extracts the topic & message
   2. it routes it to processMQTTcommand to handle app-specific actions
   3. Handles house keeping command such as turning debug on/off

 * ********************************************************************************
*/
void mqttCallback(char *topic, byte *payload, unsigned int length)
{

  // Copy payload into message buffer
  char message[length + 1];
  for (unsigned int i = 0; i < length; i++)
  {
    message[i] = (char)payload[i];
  }
  message[length] = '\0';

  // try processing main commands
  if (!processMQTTcommand(topic, message))
  {
    if (strcmp(topic, mqtt_debug_set_topic) == 0)
    {
      if (strcmp(message, "on") == 0)
      {
        debugMode = true;
        mqtt_client.publish(mqtt_debug_topic, "debug mode enabled");
      }
      else if (strcmp(message, "off") == 0)
      {
        debugMode = false;
        mqtt_client.publish(mqtt_debug_topic, "debug mode disabled");
      }
    }
    else
    {
      mqtt_client.publish(mqtt_debug_topic, strcat((char*)"wrong mqtt topic: ", topic));
    }
  }
}


/*
 * ********************************************************************************

   Configure the MQTT server by:
    - create all the topic using prefix/location/subtopic
    - configure MQTT server and port and setup callback routine
    - attempt a connection and log to debug topic if success

 * ********************************************************************************
*/

void configureMQTT()
{

  // configure the topics using location
  configureTopics();

  // configure mqtt connection
  mqtt_client.setServer(mqttServer, atoi(mqttPort));
  mqtt_client.setCallback(mqttCallback);

  console.print("MQTT Server :'");
  console.print(mqttServer);
  console.print("' Port: ");
  console.print(String(atoi(mqttPort)));
  console.print(" Topic set to: '");
  console.print(mqtt_topic);
  console.println("'");

}

/*********************************************************************************

   attemps a connection to the MQTT server. if it fails increment secondsWithoutMQTT
   and return.

   This code relies on an existing Wifi connection which checked and dealt with
   elsewhere in the code

 **********************************************************************************/

bool checkMQTTConnection() {

  if (mqtt_client.connected()) 
  {
     // loop through the client
     mqtt_client.loop();
  }
  else 
  {
    // set server name
    mqtt_client.setServer(mqttServer, atoi(mqttPort) );

    // Attempt to connect
    if (mqtt_client.connect(myHostName))
    {
      subscribeToTopics();
      console.println("Connected to MQTT");
      char str[128];
      sprintf(str, "%s %s [%s] MQTT{%s,%s}  IP:%i.%i.%i.%i", MQTT_TOPIC_PREFIX, VERSION, deviceLocation, mqttServer, mqttPort, WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
      mqtt_client.publish(mqtt_debug_topic, str, true);
      secondsWithoutMQTT = 0;
      return true;
    }
    else
    {
      delay(500);
      secondsWithoutMQTT++;
      return false;
    }
  }
  return true;
}

/*********************************************************************************
 * 
 * Disconnect from the MQTT server. This is done when server name or port have
 * changed and we need to reconnect
 * 
 *********************************************************************************/

void mqttDisconnect()
{
  if (mqtt_client.connected()) mqtt_client.disconnect();
}

