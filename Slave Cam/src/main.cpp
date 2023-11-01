
#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <TelnetStream.h>
#include <JPEGDEC.h>
#include <stdio.h>
#include <stdlib.h>
#include "esp_camera.h"
#include "esp_timer.h"
#include "esp_http_server.h"
#include "img_converters.h"
#include "Arduino.h"
#include "fb_gfx.h"
#include "soc/soc.h" //disable brownout problems
#include "soc/rtc_cntl_reg.h"  //disable brownout problems
#include <FS.h>
#include <SD_MMC.h>

#include <EEPROM.h>

#include "variables.h"
#include "TCPHandler.h"
#include "AWSCloudHandler.h"
#include "credentials.h"
#include "OTA.h"
#include "camera.h"

size_t _jpg_buf_len = 0;
uint8_t * _jpg_buf = NULL;
camera_fb_t *fb;

#define EEPROM_SIZE 1
int pictureNumber = 0;

bool getImage(void){
    fb = esp_camera_fb_get();
    if (!fb)
    {
        Serial.println("Failed to get image");
        esp_camera_fb_return(fb);
        fb = NULL;
        return false;
    }
    else
    {
      Serial.println("Image Taken Successfully");
      Serial.printf("%d bytes\n", fb->len);
      return true;
    }
}

void takeImage(){
  bool resultImage = false; 
  while(!resultImage){
    resultImage = getImage();
    if(!resultImage){
      Serial.println("Couldn't take Image");
      TelnetStream.println("Couldn't take Image");

      esp_camera_fb_return(fb);
      fb = NULL;
      delay(1000);
    }
  }
  Serial.println("Image Successfully Taken");
  TelnetStream.println("Image Successfully Taken");
}

void saveImageSDCard(String folderName, String fileName){
  EEPROM.begin(EEPROM_SIZE);
  pictureNumber = EEPROM.read(0) + 1;

  // Path where new picture will be saved in SD Card
  String path = "/" + folderName + "_" + fileName + ".jpg";

  fs::FS &fs = SD_MMC; 
  Serial.printf("Picture file name: %s\n", path.c_str());
  
  File file = fs.open(path.c_str(), FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file in writing mode");
  } 
  else {
    file.write(fb->buf, fb->len); // payload (image), payload length
    Serial.printf("Saved file to path: %s\n", path.c_str());
    EEPROM.write(0, pictureNumber);
    EEPROM.commit();
  }

  file.close();
  esp_camera_fb_return(fb); 
}

void streamMQTT(){
  bool publishImageAWS = false;

  takeImage();

  while (!publishImageAWS)
    {
      publishImageAWS = publishImage(fb->buf, fb->len);
      delay(1000);
    }
    
    esp_camera_fb_return(fb);
    fb = NULL;

}

void process(String string, String filename){
  DynamicJsonDocument doc(256);

  INPUT_STRING = string;
  INPUT_STRING.toUpperCase();

  bool publishImageAWS = false;
  int photoNr = 0;

  if (INPUT_STRING.startsWith("TAKEPHOTO "))
  {
    START = INPUT_STRING.indexOf("N");
    if (!(START < 0)){
      FINISH = START + 8;
      SUB_STRING = INPUT_STRING.substring(START + 1, FINISH + 1);
      photoNr = SUB_STRING.toInt();
    }

    Serial.println("Sending Photo to cloud");
    TelnetStream.println("Sending Photo to cloud");

    delay(1000);
    takeImage();

    while (!publishImageAWS){
      publishImageAWS = publishImage(fb->buf, fb->len);
      delay(1000);
    }
    
    //esp_camera_fb_return(fb);
    //fb = NULL;

    Serial.println("Successfull AWS Post");
    TelnetStream.println("Successfull AWS Post");

    //takeImage();
    
    Serial.println("Saving Image on SDCARD");
    TelnetStream.println("Saving Image on SDCARD");

    saveImageSDCard(filename,String(photoNr));

    esp_camera_fb_return(fb);
    fb = NULL;

    Serial.println("Sending Data to TCP Server");
    TelnetStream.println("Sending Data to TCP Server");

    doc["uc"] = 3;
    doc["mode"]  = 1;
    doc["photo"] = true;

    serializeJson(doc, buffer);
    sendMessageToServer(buffer);
    delay(1000);
    }
    INPUT_STRING = "";
}


boolean setupSDCard(){
  delay(500);

  Serial.println("Starting SD Card");
  if(!SD_MMC.begin()){
    Serial.println("SD Card Mount Failed");
    return false;
  }
  
  uint8_t cardType = SD_MMC.cardType();
  if(cardType == CARD_NONE){
    Serial.println("No SD Card attached");
    return false;
  }
  return true;
}

void setupWifi(){
  int reconnectTime = 0;

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    reconnectTime++;
    if (reconnectTime > 60)
    {
      Serial.println("Restarting uC, WIFI AP not found.");
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

 void pingServer() {

  TelnetStream.println("Sending Data to TCP Server");

  DynamicJsonDocument doc(1024);

  doc["uc"] = 4;
  doc["mode"]  = "Hi from CAM";

  serializeJson(doc, buffer);

  sendMessageToServer(buffer);
}


void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector

  Serial.begin(115200);

  setupWifi();

  connectAWS();
  
  startCamera();

  while(!setupSDCard()){};

  setupTCP();

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  //xTaskCreatePinnedToCore(myWiFiTask, "myWiFiTask", 8192, NULL, 3, NULL, 0);
}

void loop() {
  Serial.println("-- Cam Sample Code --");
  TelnetStream.println("-- Cam Sample Code --");
  //ArduinoOTA.handle();

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)){
    TelnetStream.println("Failed to obtain time");
  }

  strftime(timeHour, 9, "%H:%M:%S", &timeinfo);
  strftime(timeDate, 20, "%d-%B-%Y", &timeinfo);

  
  client.loop();

  if(!client.connected()){
    reconnectAWS();
  }

  //streamMQTT();

  process(INPUT_STRING, String(timeDate));

  if(TCPConnected)
    pingServer();

    /*
  
  if (timeinfo.tm_hour == 10 && timeinfo.tm_min == 15)
  {
    INPUT_STRING = "TAKEPHOTO N1";

    process(INPUT_STRING, String(timeDate));
    INPUT_STRING = "  ";
    delay(60 * 1000);
  }

  if (timeinfo.tm_hour == 12 && timeinfo.tm_min == 15)
  {
    Serial.println("taking photo");
    INPUT_STRING = "TAKEPHOTO N2";

    process(INPUT_STRING, String(timeDate));
    INPUT_STRING = "  ";
    delay(60 * 1000);
  }

  if (timeinfo.tm_hour == 18 && timeinfo.tm_min == 15)
  {
    INPUT_STRING = "TAKEPHOTO N3";

    process(INPUT_STRING, String(timeDate));
    INPUT_STRING = "  ";
    delay(60 * 1000);
  }

  if (timeinfo.tm_hour == 11 && timeinfo.tm_min == 15)
  {
    INPUT_STRING = "TAKEPHOTO N4";

    process(INPUT_STRING, String(timeDate));
    INPUT_STRING = "  ";
    delay(60 * 1000);
  }
  if (timeinfo.tm_hour == 13 && timeinfo.tm_min == 15)
  {
    INPUT_STRING = "TAKEPHOTO N5";

    process(INPUT_STRING, String(timeDate));
    INPUT_STRING = "  ";
    delay(60 * 1000);
  }
  if (timeinfo.tm_hour == 14 && timeinfo.tm_min == 15)
  {
    INPUT_STRING = "TAKEPHOTO N6";

    process(INPUT_STRING, String(timeDate));
    INPUT_STRING = "  ";
    delay(60 * 1000);
  }
  if (timeinfo.tm_hour == 15 && timeinfo.tm_min == 15)
  {
    INPUT_STRING = "TAKEPHOTO N7";

    process(INPUT_STRING, String(timeDate));
    INPUT_STRING = "  ";
    delay(60 * 1000);
  }
  if (timeinfo.tm_hour == 16 && timeinfo.tm_min == 15)
  {
    INPUT_STRING = "TAKEPHOTO N8";

    process(INPUT_STRING, String(timeDate));
    INPUT_STRING = "  ";
    delay(60 * 1000);
  }
  if (timeinfo.tm_hour == 17 && timeinfo.tm_min == 15)
  {
    INPUT_STRING = "TAKEPHOTO N9";

    process(INPUT_STRING, String(timeDate));
    INPUT_STRING = "  ";
    delay(60 * 1000);
  }
  if (timeinfo.tm_hour == 19 && timeinfo.tm_min == 15)
  {
    INPUT_STRING = "TAKEPHOTO N10";

    process(INPUT_STRING, String(timeDate));
    INPUT_STRING = "  ";
    delay(60 * 1000);
  }
  */
  
  
  delay(4000);
}

