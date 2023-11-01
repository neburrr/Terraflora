#include <Arduino.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WebServer.h>
#include <WiFi.h>
#include <TelnetStream.h>

typedef struct struct_message {
  uint8_t ID;
  uint8_t Type;
  uint8_t Length;
  uint16_t Checksum;
  int Info;
} struct_message;

struct_message dataPacket;

WebServer server(80);

StaticJsonDocument<250> jsonDocument;
char buffer[250];

void handlePost() {
  if (server.hasArg("plain") == false) {
  }
  String body = server.arg("plain");
  
  TelnetStream.print(body);

  deserializeJson(jsonDocument, body);

  dataPacketSensors->airHumidityExt = jsonDocument["airhumidity"];
  dataPacketSensors->airHumidityRawExt = jsonDocument["airhumidityraw"];
  dataPacketSensors->temperatureExt = jsonDocument["temperature"];
  dataPacketSensors->temperatureRawExt = jsonDocument["temperatureraw"];
  
  float a = jsonDocument["airhumidityraw"];
  TelnetStream.print("hum: ");
  TelnetStream.println(a);

  server.send(200, "application/json", "{}");
}

void add_json_object(char *tag, float value, char *unit) {
  JsonObject obj = jsonDocument.createNestedObject();
  obj["type"] = tag;
  obj["value"] = value;
  obj["unit"] = unit; 
}

void getData() {
  jsonDocument.clear();

  add_json_object("soil humidity", dataPacketSensors->soilHumidityInt, "%");

  serializeJson(jsonDocument, buffer);
  server.send(200, "application/json", buffer);
}

void create_json(char *tag, float value, char *unit) {  
  jsonDocument.clear();  
  jsonDocument["type"] = tag;
  jsonDocument["value"] = value;
  jsonDocument["unit"] = unit;
  serializeJson(jsonDocument, buffer);
}

void getSoilHumidity(){
  Serial.println("Get humidity");
  create_json("humidity", dataPacketSensors->soilHumidityInt, "%");
  server.send(200, "application/json", buffer);
}


void setupRouting() {     
  server.on("/soilhumidity", getSoilHumidity);     
  server.on("/data", getData);     
  server.on("/datasensorext", HTTP_POST, handlePost);    
          
  server.begin();    
}
