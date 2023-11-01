#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
//#include <iot.h>
//#include <MQTTClient.h>

#include "secrets.h"

#define AWS_IOT_PUBLISH_TOPIC "lettuce/images"
#define AWS_IOT_SUBSCRIBE_TOPIC "lettuce/images"

WiFiClientSecure net = WiFiClientSecure();
//MQTTClient client = MQTTClient(65000);
PubSubClient client(net);
//IOT iotclient(net, client);

void messageHandler(char* topic, byte* payload, unsigned int length)
{
  TelnetStream.print("incoming: ");
  TelnetStream.println(topic);
  
  StaticJsonDocument<500> doc;
  deserializeJson(doc, payload);
  const char* message = doc["message"];
  TelnetStream.println(message);
}

/*
void connectAWS()
{
  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.begin(AWS_IOT_ENDPOINT, 8883, net);

  // Create a message handler
  // client.onMessage(messageHandler);

  Serial.print("Connecting to AWS IOT");

  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(100);
  }

  if(!client.connected()){
    Serial.println("AWS IoT Timeout!");
    return;
  }

  // Subscribe to a topic
  // client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);

  Serial.println("AWS IoT Connected!");
}*/



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
  
  net.setHandshakeTimeout(200);
  net.setTimeout(200);
  

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

  Serial.println("Reconnecting to AWS IOT");

  Serial.println("Connecting to Wi-Fi");
  ESP.restart();
 
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    reconnectTime++;
    if(reconnectTime > 40){
      Serial.println("Restarting uC, WIFI AP not found.");
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
 
bool publishImage2(uint8_t *data, uint32_t len) {
  Serial.print("Setup publishImage2: Executing on core ");
  Serial.println(xPortGetCoreID());
  unsigned long start_ts = millis();

  client.beginPublish(AWS_IOT_PUBLISH_TOPIC, len, false);
    
  size_t res;
  uint32_t offset = 0;
  uint32_t to_write = len;
  uint32_t buf_len;

  do {
    buf_len = to_write;
    if (buf_len > 64000)
      buf_len = 64000;
    
    res = client.write(data+offset, buf_len);
    
    offset += buf_len;
    to_write -= buf_len;

  } while (res == buf_len && to_write > 0);

  bool result = client.endPublish();

  if (result){
    Serial.printf("Published in MQTT channel %s: (binary data of length %d bytes, written in %ld ms)\n", AWS_IOT_PUBLISH_TOPIC, len, millis()-start_ts);
    TelnetStream.printf("Published in MQTT channel %s: (binary data of length %d bytes, written in %ld ms)", AWS_IOT_PUBLISH_TOPIC, len, millis()-start_ts);
    TelnetStream.println();
  }
  else{
    Serial.println("Failed publishing image to MQTT");
    TelnetStream.println("Failed publishing image to MQTT");
  }

  return result;
}

bool publishImage (uint8_t * imgBuffer, uint32_t imgLenght)
{ 
  Serial.print("Setup publishImage: Executing on core ");
  Serial.println(xPortGetCoreID());
  unsigned long start_ts = millis();
  Serial.println("Publishing Image");
  TelnetStream.println("Publishing Image");

  while (!client.connected()){
    reconnectAWS();
    }

  bool result = client.publish(AWS_IOT_PUBLISH_TOPIC, imgBuffer, imgLenght);

  /*
  client.beginPublish(AWS_IOT_PUBLISH_TOPIC, imgLenght, false);
  for (size_t i = 0; i < imgLenght; i++)
  {
    client.write(&imgBuffer[i], 1);
  }
  bool result = client.endPublish();*/

  if (result){
    Serial.printf("Published in MQTT channel %s: (binary data of length %d bytes, written in %ld ms)\n", AWS_IOT_PUBLISH_TOPIC, imgLenght, millis()-start_ts);
    TelnetStream.printf("Published in MQTT channel %s: (binary data of length %d bytes, written in %ld ms)", AWS_IOT_PUBLISH_TOPIC, imgLenght, millis()-start_ts);
    TelnetStream.println();
  }
  else{
    Serial.println("Failed publishing image to MQTT");
    TelnetStream.println("Failed publishing image to MQTT");
  }

  return result;
}