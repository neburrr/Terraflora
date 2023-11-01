#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

#include <WiFi.h>

#include "secrets.h"

#define AWS_IOT_PUBLISH_TOPIC_DB   "lettuceSample/pub"
#define AWS_IOT_PUBLISH_TOPIC_REALTIME   "lettuceSample/realTimePub"
#define AWS_IOT_PUBLISH_TOPIC_CLIENT_STATE   "lettuceSample/clientState"
#define AWS_IOT_PUBLISH_TOPIC_SYSTEM_STATE   "lettuceSample/systemState"
#define AWS_IOT_PUBLISH_TOPIC_SYSTEM_STATES_DB   "lettuceSample/systemStates"
#define AWS_IOT_PUBLISH_LOG   "lettuceSample/log"
#define AWS_IOT_SUBSCRIBE_TOPIC "lettuce/order"

WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

void zAxisPos(int pos);
void zAxis(int loops, bool dir);
static void light(bool mode);
void wateringViaTime(int time);

bool scanGreenhouse();
bool scanGreenhouse1();
bool movePlaneAxis(const char *pos);

bool publishLog (const char * message);

void messageHandler(char* topic, byte* payload, unsigned int length)
{
  TelnetStream.print("incoming: ");
  TelnetStream.println(topic);
  
  StaticJsonDocument<500> doc;
  deserializeJson(doc, payload);
  const char* message = doc["message"];
  TelnetStream.println(message);

  //move 2d axis
  if(!strcmp(message, "moveAxis")){

    //publishLog("Moving 2D axis ..."); //can´t be here
    
    int x = doc["x"];
    int y = doc["y"];

    const char* pos = doc["pos"];

    if(movePlaneAxis(pos))
      publishLog("2D axis moved");
  }
  else if(!strcmp(message, "moveZAxis")){
    const int loops = doc["loops"];
    const int dir = doc["dir"];

    TelnetStream.printf("Moving Z Axis %d loops, %d direction", loops, dir);
    TelnetStream.println();
    
    zAxis(loops, dir);
  }
  else if(!strcmp(message, "moveAxisZ")){
    //publishLog("Moving Z axis ...");

    const int pos = doc["pos"];

    TelnetStream.printf("Moving Z Axis to %d position", pos);
    TelnetStream.println();
    
    zAxisPos(pos);

    publishLog("Z axis moved");
  }
  else if(!strcmp(message, "ping")){
    TelnetStream.printf("Sending Values to cloud");
    TelnetStream.println();
    
    //publishMessageDB(timeHour,timeDate, &dataPacketSensors, &systemStateData);
  }
  else if(!strcmp(message, "light")){
    publishLog("Turning ON Lights");
    AWSControl = true;
    TelnetStream.printf("Changing Light Color");
    TelnetStream.println();

    light(true);

    delay(10000);
    AWSControl = false; 
  }
  else if(!strcmp(message, "water")){
    publishLog("Watering");
    delay(1000);
    manualWater = true;
  }
  else if(!strcmp(message, "water30")){
    //publishLog("Watering");
    wateringViaTime(30);
  }
  else if(!strcmp(message, "getClients")){
    TelnetStream.printf("Printing all Clients connected to TCP Server");
    TelnetStream.println();
    //getAllClients();
  }
  else if(!strcmp(message, "greenhouseScan")){

    
    publishLog("Starting Greenhouse Scan");
    delay(1000);
    bool greenhouse = scanGreenhouse();
  }
  else if(!strcmp(message, "takephoto")){
    bool greenhouse =  scanGreenhouse1();;
  }
  else if(!strcmp(message, "waterSetPoint")){
    

    int value = doc["value"];
    PIDSETPOINTWATER = value;
    delay(1000);
    publishLog("New Water SetPoint");
  }
  else if(!strcmp(message, "tempSetPoint")){
    
    const int value = doc["value"];
    PIDSETPOINTTEMP = value;
  }
  else if(!strcmp(message, "mode")){
    
    bool value = doc["value"];
    ManualControl = value;
    delay(1000);
    TelnetStream.println("Changing Greenhouse Mode");
    publishLog("Changing Greenhouse Mode");
  }
  else if(!strcmp(message, "restart")){
    TelnetStream.println("Restart Greenhouse");
    publishLog("Restarting Greenhouse");
    delay(4000);
    
    ESP.restart();
  }
  message = "";
}

void connectAWS()
{
  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
 
  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.setServer(AWS_IOT_ENDPOINT, 8883);
 
  // Create a message handler
  client.setCallback(messageHandler);
 
  Serial.println("Connecting to AWS IOT");
 
  while (!client.connect(THINGNAME))
  {
    Serial.print(".");
    delay(100);
  }
 
  if (!client.connected())
  {
    Serial.println("AWS IoT Timeout!");
    return;
  }
 
  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
 
  Serial.println("AWS IoT Connected!");
}

void reconnectAWS(){
  int reconnectTime = 0;

  TelnetStream.println("Reconnecting to AWS IOT");

  Serial.println("Connecting to Wi-Fi");
 
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    reconnectTime++;
    if(reconnectTime > 40){
      TelnetStream.println("Restarting uC, WIFI AP not found.");
      ESP.restart();
    }
  }
 
  while (!client.connect(THINGNAME))
  {
    Serial.print(".");
    delay(100);
  }

  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
 
  TelnetStream.println("AWS IoT Reconnected!");
}
 
bool publishMessageDB(char* timeHour, char* timeDate, sensorData * dataPacketSensors, systemState * systemStateData)
{
  StaticJsonDocument<1000> doc;

  doc["DT"] = timeDate;
  doc["HT"] = timeHour;
  doc["SD"]["Ext"]["AH"] = dataPacketSensors->airHumidityExt;
  doc["SD"]["Ext"]["T"] = dataPacketSensors->temperatureExt;
  
  doc["SD"]["Int"]["SH"] = dataPacketSensors->soilHumidity;
  doc["SD"]["Int"]["SHR"] = dataPacketSensors->soilHumidityRaw;
  doc["SD"]["Int"]["AH"] = dataPacketSensors->airHumidityInt;
  doc["SD"]["Int"]["T"] = dataPacketSensors->temperatureInt;
  doc["SD"]["Int"]["LDR"] = dataPacketSensors->LDR;
  doc["SD"]["Int"]["VB"] = dataPacketSensors->voltageBAT;
  doc["SS"]["W"] = systemStateData->waterState;
  doc["SS"]["L"] = systemStateData->lightState;
  doc["SS"]["V"] = systemStateData->ventilationState;

  char jsonBuffer[1000];
  serializeJson(doc, jsonBuffer);

  TelnetStream.printf("Publishing Message on AWS cloud at %s ...", AWS_IOT_PUBLISH_TOPIC_DB);
  TelnetStream.println();

  bool result = client.publish(AWS_IOT_PUBLISH_TOPIC_DB, jsonBuffer);
  if (result)
    TelnetStream.println("Published with success");
  
  return result;
}

bool publishMessageRTSensorData(sensorData * dataPacketSensors, systemState * systemStateData)
{
  StaticJsonDocument<1000> doc;

  doc["SD"]["Ext"]["AH"] = dataPacketSensors->airHumidityExt;
  doc["SD"]["Ext"]["T"] = dataPacketSensors->temperatureExt;

  doc["SD"]["Int"]["SH"] = dataPacketSensors->soilHumidity;
  doc["SD"]["Int"]["SHR"] = dataPacketSensors->soilHumidityRaw;
  doc["SD"]["Int"]["AH"] = dataPacketSensors->airHumidityInt;
  doc["SD"]["Int"]["T"] = dataPacketSensors->temperatureInt;
  doc["SD"]["Int"]["LDR"] = dataPacketSensors->LDR;
  doc["SD"]["Int"]["VB"] = dataPacketSensors->voltageBAT;
  doc["SD"]["Ext"]["LVL"] = systemStateData->waterlevel;

  char jsonBuffer[1000];
  serializeJson(doc, jsonBuffer);

  TelnetStream.printf("Publishing Real Time Message on AWS cloud at %s ...", AWS_IOT_PUBLISH_TOPIC_REALTIME);
  TelnetStream.println();

  bool result = client.publish(AWS_IOT_PUBLISH_TOPIC_REALTIME, jsonBuffer);
  if (result)
    TelnetStream.println("Published with success");
  
  return result;
}

bool publishMessageRTSystemState (clientsStruct * clientsState, systemState * systemStateData)
{
  StaticJsonDocument<1000> doc;

  doc["SS"]["W"] = systemStateData->waterState;
  doc["SS"]["L"] = systemStateData->lightState;
  doc["SS"]["V"] = systemStateData->ventilationState;

  doc["SS"]["X"] = systemStateData->xAxis;
  doc["SS"]["Y"] = systemStateData->yAxis;
  doc["SS"]["Z"] = systemStateData->zAxis;

  doc["SS"]["WSP"] = PIDSETPOINTWATER;
  doc["SS"]["M"] = ManualControl;

  char jsonBuffer[1000];
  serializeJson(doc, jsonBuffer);

  TelnetStream.printf("Publishing Real Time Message on AWS cloud at %s ...", AWS_IOT_PUBLISH_TOPIC_SYSTEM_STATE);
  TelnetStream.println();

  bool result = client.publish(AWS_IOT_PUBLISH_TOPIC_SYSTEM_STATE, jsonBuffer);
  if (result)
    TelnetStream.println("Published on AWS with success");
  
  return result;
}

bool publishMessageRTClientState (clientsStruct * clientsState, systemState * systemStateData)
{
  StaticJsonDocument<1000> doc;

  doc["CS"]["S"]["S"] = clientsState->stateSoil;
  doc["CS"]["S"]["TH"] = clientsState->timeHourSoil;
  doc["CS"]["S"]["TD"] = clientsState->timeHourSoil;
  doc["CS"]["S"]["IP"] = clientsState->IPSoil;

  doc["CS"]["A"]["S"] = clientsState->stateAxis;
  doc["CS"]["A"]["TH"] = clientsState->timeHourAxis;
  doc["CS"]["A"]["TD"] = clientsState->timeHourAxis;
  doc["CS"]["A"]["IP"] = clientsState->IPAxis;

  doc["CS"]["C"]["S"] = clientsState->stateCam;
  doc["CS"]["C"]["TH"] = clientsState->timeHourCam;
  doc["CS"]["C"]["TD"] = clientsState->timeHourCam;
  doc["CS"]["C"]["IP"] = clientsState->IPCam;

  char jsonBuffer[1000];
  serializeJson(doc, jsonBuffer);

  TelnetStream.printf("Publishing Real Time Message on AWS cloud at %s ...", AWS_IOT_PUBLISH_TOPIC_CLIENT_STATE);
  TelnetStream.println();

  bool result = client.publish(AWS_IOT_PUBLISH_TOPIC_CLIENT_STATE, jsonBuffer);
  if (result)
    TelnetStream.println("Published on AWS with success");
  
  return result;
}


//when mode:
// 0 - new Axis Position
// 1 - new Light State
// 2 - new Water state
// 3 - new Ventilation State
bool publishMessageSystemsStateDB (char* timeHour, char* timeDate, sensorData * dataPacketSensors, systemState * systemStateData, int mode)
{
  StaticJsonDocument<1000> doc;

  doc["M"] = mode;

  doc["DT"] = timeDate;
  doc["HT"] = timeHour;

  doc["W"]["S"] = systemStateData->waterState;
  doc["W"]["LVL"] = systemStateData->waterlevel;
  doc["W"]["TES"] = systemStateData->tisWater;

  doc["L"]["S"] = systemStateData->lightState;
  doc["L"]["CONS"] =systemStateData->lightConsumption;
  doc["L"]["TES"] = systemStateData->tisLight;

  doc["V"]["S"] = systemStateData->ventilationState;
  doc["V"]["TES"] = systemStateData->tisVentilation;

  doc["X"] = systemStateData->xAxis;
  doc["Y"] = systemStateData->yAxis;
  doc["Z"] = systemStateData->zAxis;

  char jsonBuffer[1000];
  serializeJson(doc, jsonBuffer);

  TelnetStream.printf("Publishing Real Time Message on AWS cloud at %s ...", AWS_IOT_PUBLISH_TOPIC_SYSTEM_STATES_DB);
  TelnetStream.println();

  bool result = client.publish(AWS_IOT_PUBLISH_TOPIC_SYSTEM_STATES_DB, jsonBuffer);
  if (result)
    TelnetStream.println("Published with success");
  
  return result;
}


bool publishLog (const char * message)
{
  StaticJsonDocument<1000> doc;

  doc["message"] = message;

  char jsonBuffer[1000];
  serializeJson(doc, jsonBuffer);

  TelnetStream.printf("Publishing Real Time Message on AWS cloud at %s ...", AWS_IOT_PUBLISH_LOG);
  TelnetStream.println();

  bool result = client.publish(AWS_IOT_PUBLISH_LOG, jsonBuffer);
  if (result)
    TelnetStream.println("Published with success");
  
  return result;
}
