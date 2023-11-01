//IP: 192.168.1.81
//MAC address: F0:08:D1:C7:1F:CC

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <WiFi.h>
#include <WebServer.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <math.h>

#include "time.h"
#include "DHT.h" 
#include "OTA.h"
#include "TCPHandler.h"
#include "./credentials.h"
#include "variables.h"

#define DHTTYPE DHT11

DHT dht(DHT11PIN, DHTTYPE);

sensorData dataPacketSensors;

void setupWIFI(){
  int reconnectTime = 0;

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

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

  Serial.println("");
  Serial.println("Connected to Wi-Fi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC address: ");
  Serial.println(WiFi.macAddress());
  
  TelnetStream.begin();
  Serial.println("Telnet Started");
}

void postSensorDataTCP(sensorData* dataSensors) {  

  TelnetStream.println("Sending Data to TCP Server");

  DynamicJsonDocument doc(1024);

  doc["uc"] = 1;
  doc["mode"]  = 1;
  doc["airhumidityint"] = dataSensors->airHumidityInt;
  doc["airhumidityrawint"] = dataSensors->airHumidityRawInt;
  doc["temperatureint"] = dataSensors->temperatureInt;
  doc["temperaturerawint"]  = dataSensors->temperatureRawInt;
  doc["soilHumidity"] = dataSensors->soilHumidity;
  doc["soilHumidityRaw"] = dataSensors->soilHumidityRaw;
  doc["lightIntensity"] = dataSensors->LDR;
  doc["batVoltage"] = dataSensors->voltageBAT;

  serializeJson(doc, buffer);
  
  /*
  jsonDocument.clear();
  jsonPostMessage("uc", 1);
  jsonPostMessage("mode", 1);
  jsonPostMessage("airhumidityint", dataSensors->airHumidityInt);
  jsonPostMessage("airhumidityrawint", dataSensors->airHumidityRawInt);
  jsonPostMessage("temperatureint", dataSensors->temperatureInt);
  jsonPostMessage("temperaturerawint", dataSensors->temperatureRawInt);
  jsonPostMessage("soilHumidity", dataSensors->soilHumidity);
  jsonPostMessage("soilHumidityRaw", dataSensors->soilHumidityRaw);
  jsonPostMessage("lightIntensity", dataSensors->LDR);
  jsonPostMessage("batVoltage", dataSensors->voltageBAT);
  
  
  serializeJson(jsonDocument, buffer);*/

  sendMessageToServer(buffer);
}

void getSensorData(void *parameter){
  sensorData* dataSensors = reinterpret_cast<sensorData*>(parameter);

  for(;;){
    dataSensors->soilHumidityRaw = analogRead(soilHumidityPin);
    dataSensors->soilHumidity = map(dataSensors->soilHumidityRaw, wetValue, dryValue, 100, 0);

    dataSensors->LDR = analogRead(LDRPin) + 161;

    int batVal = analogRead(batVoltageADCPin);
    dataSensors->voltageBAT = (3.3/4095)*batVal + ((3.3/4095)*batVal)/2.6 * 9.23;
    dataSensors->voltageBAT = round(dataSensors->voltageBAT*100)/100;
    
    do{
      delay(1100);
      dataSensors->airHumidityInt = dht.readHumidity();
      dataSensors->airHumidityRawInt = 0;
      dataSensors->temperatureInt = dht.readTemperature();
      dataSensors->temperatureRawInt = 0; 
    } while (isnan(dataPacketSensors.airHumidityInt) || isnan(dataPacketSensors.temperatureInt));

    postSensorDataTCP(&dataPacketSensors);

    vTaskDelay(10000/ portTICK_RATE_MS);
  }
}

void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  Serial.print("Day of week: ");
  Serial.println(&timeinfo, "%A");
  Serial.print("Month: ");
  Serial.println(&timeinfo, "%B");
  Serial.print("Day of Month: ");
  Serial.println(&timeinfo, "%d");
  Serial.print("Year: ");
  Serial.println(&timeinfo, "%Y");
  Serial.print("Hour: ");
  Serial.println(&timeinfo, "%H");
  Serial.print("Hour (12 hour format): ");
  Serial.println(&timeinfo, "%I");
  Serial.print("Minute: ");
  Serial.println(&timeinfo, "%M");
  Serial.print("Second: ");
  Serial.println(&timeinfo, "%S");

  Serial.println("Time variables");
  char timeHour[3];
  strftime(timeHour,3, "%H", &timeinfo);
  Serial.println(timeHour);
  char timeWeekDay[10];
  strftime(timeWeekDay,10, "%A", &timeinfo);
  Serial.println(timeWeekDay);
  Serial.println();
}

void batVoltage(){  
  int batVal = analogRead(batVoltageADCPin) + 150;
  Serial.print("ANALOGGG vol: ");
  Serial.println(batVal);

  float voltageBAT = (3.3/4095)*batVal + ((3.3/4095)*batVal)/2.6 * 9.23;

  Serial.print("BATVOL: ");
  Serial.println(voltageBAT);

  int batCur = analogRead(batCurrentADCPin);
  Serial.print("ANALOGGG cur: ");
  Serial.println(batCur);

  float adctoVol = 3.3 / 4095;

  float currentBAT = (adctoVol*batCur) * 2;

  Serial.print("BATCUR: ");
  Serial.println(currentBAT);

  int LDRValue = analogRead(LDRPin);
  Serial.print("LDR: ");
  Serial.println(LDRValue);
  }

void setupTask(){
  xTaskCreate(
  getSensorData, 
  "Read sensor Data", 
  20000, 
  &dataPacketSensors, 
  1, 
  NULL);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");

  setupWIFI();

  setupOTA("SOIL", ssid, password);

  setupTCP();

  dht.begin();

  setupTask(); 

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop() {
  ArduinoOTA.handle();
  TelnetStream.println("Soil - Sample Code");

  struct tm timeinfo;
  
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
  }
  
  if(timeinfo.tm_hour == 0 && timeinfo.tm_min == 0 && timeinfo.tm_sec < 10 && timeinfo.tm_sec > 5){
    TelnetStream.println("Restarting Soil uC");
    ESP.restart();
  }

  delay(4000);
}