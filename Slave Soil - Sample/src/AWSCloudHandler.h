#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>

#include "secrets.h"

#define AWS_IOT_PUBLISH_TOPIC   "lettuceSample/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "lettuceSample/sub"

WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

void messageHandler(char* topic, byte* payload, unsigned int length)
{
  Serial.print("incoming: ");
  Serial.println(topic);
 
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  const char* message = doc["message"];
  Serial.println(message);
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
 
void publishMessage(char* timeHour, char* timeDate, sensorData * dataPacketSensors)
{
  StaticJsonDocument<250> doc;

  doc["DateTime"] = timeDate;
  doc["HourTime"] = timeHour;
  doc["Data"]["Exterior"]["AirHumidity"] = dataPacketSensors->airHumidityExt;
  //doc["Data"]["Exterior"]["Air Humidity Raw"] = dataPacketSensors->airHumidityRawExt;
  doc["Data"]["Exterior"]["Temperature"] = dataPacketSensors->temperatureExt;
  //doc["Data"]["Exterior"]["Temperature Raw"] = dataPacketSensors->temperatureRawExt;
  doc["Data"]["Interior"]["SoilHumidity"] = dataPacketSensors->soilHumidityInt;
  doc["Data"]["Interior"]["SoilHumidityRaw"] = dataPacketSensors->soilHumidityRawInt;
  doc["Data"]["Interior"]["AirHumidity"] = dataPacketSensors->airHumidityInt;
  //doc["Data"]["Interior"]["Air Humidity Raw"] = dataPacketSensors->airHumidityRawInt;
  doc["Data"]["Interior"]["Temperature"] = dataPacketSensors->temperatureInt;
  //doc["Data"]["Interior"]["Temperature Raw"] = dataPacketSensors->temperatureRawInt;

  char jsonBuffer[250];
  serializeJson(doc, jsonBuffer);

  Serial.println("Publishing Message on AWS cloud");
 
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}