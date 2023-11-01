#ifndef HTTPSCLIENT
#define HTTPSCLIENT

#include <Arduino.h>
#include <HTTPClient.h>
#include <TelnetStream.h>
#include <ArduinoJson.h>

WiFiClient client;
HTTPClient http;

StaticJsonDocument<250> jsonDocument;
char buffer[250];

typedef struct sensorData {
  float airHumidity;
  uint8_t airHumidityRaw;
  float temperature;
  uint8_t temperatureRaw;
} sensorData;

extern sensorData dataPacketSensors;


const char* serverName = "http://192.168.1.130:80/datasensorext";

void setupHTTPS(){
  http.begin(client, serverName);
}

void add_json_object(const char *tag, float value) {
  JsonObject obj = jsonDocument.createNestedObject();
  obj["type"] = tag;
  obj["value"] = value;
}

void jsonPostMessage(const char *tag, float value) {
  JsonObject obj = jsonDocument.createNestedObject();
  obj[tag] = value;
}

void jsonPostMessage1(const char *tag, char *value) {
  JsonObject obj = jsonDocument.createNestedObject();
  obj[tag] = value;
}

void postSensorData(sensorData* dataSensors){
  TelnetStream.println("Sending message to server");
  http.addHeader("Content-Type", "application/json");

  jsonDocument.clear();
  jsonPostMessage("airhumidity", dataSensors->airHumidity);
  jsonPostMessage("airhumidityraw", 5.5);
  jsonPostMessage("temperature", dataSensors->temperature);
  jsonPostMessage("temperatureraw", dataSensors->temperatureRaw);
  
  serializeJson(jsonDocument, buffer);

  //Serial.println(buffer);

  //http.end();

}

void getDataPacket(){


}

#endif