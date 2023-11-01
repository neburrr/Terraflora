// IP address: 192.168.1.76
// MAC address: 8C:CE:4E:86:E7:E4

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "DHT.h"
#include "OTA.h"
#include "./credentials.h"
#include "variables.h"
#include "TCPHandler.h"
#include "AWSCloudHandler.h"
#include "pid.h"

sensorData dataPacketSensors;
systemState systemStateData;

DHT dht(DHT11PIN, DHTTYPE);


PID pidWater = PID(SAMPLINGTIME, PIDMAXWATER, PIDMINWATER, KPWATER, KDWATER, KIWATER);
PID pidWater2 = PID(SAMPLINGTIME, PIDMAXWATER, PIDMINWATER, KPWATER, KDWATER, KIWATER);
PID pidTemp = PID(SAMPLINGTIME, PIDMAXTEMP, PIDMINTEMP, KPTEMP, KDTEMP, KITEMP);

fsm_t lightFSM, waterFSM, ventilationFSM;
clientsStruct clientsState;

void set_state(fsm_t &fsm, uint8_t new_state, int mode)
{
  if (fsm.state != new_state)
  { // if the state changed tis is reseted
    fsm.state = new_state;
    fsm.tes = millis();
    fsm.tis = 0;
    if (mode != 5)
    {
      publishMessageSystemsStateDB(timeHour, timeDate, &dataPacketSensors, &systemStateData, mode);
    }
    
  }
}

void setupWIFI()
{
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

void printSensorData(){
  TelnetStream.println("-- Task Sensor Data --");

  //TelnetStream.print("soilHumidity: ");
  //TelnetStream.println(dataPacketSensors.soilHumidity);
  //TelnetStream.print("soilHumidityRaw: ");
  //TelnetStream.println(dataPacketSensors.soilHumidityRaw);
  //TelnetStream.print("airHumidityInt: ");
  //TelnetStream.println(dataPacketSensors.airHumidityInt);
  //TelnetStream.print("temperatureInt: ");
  //TelnetStream.println(dataPacketSensors.temperatureInt);
  //TelnetStream.print("LDR: ");
  //TelnetStream.println(dataPacketSensors.LDR);
  //TelnetStream.print("voltageBAT: ");
  //TelnetStream.println(dataPacketSensors.voltageBAT);
  TelnetStream.print("airHumidityExt: ");
  TelnetStream.println(dataPacketSensors.airHumidityExt);
  TelnetStream.print("TemperatureExt: ");
  TelnetStream.println(dataPacketSensors.temperatureExt);
  TelnetStream.println("");
}

bool checkSensorData(){
  if(isnan(dataPacketSensors.soilHumidity) ||
    isnan(dataPacketSensors.soilHumidityRaw) ||
    isnan(dataPacketSensors.airHumidityInt) ||
    isnan(dataPacketSensors.temperatureInt) ||
    isnan(dataPacketSensors.LDR) ||
    isnan(dataPacketSensors.voltageBAT) ||
    isnan(dataPacketSensors.airHumidityExt) ||
    isnan(dataPacketSensors.temperatureExt) || 
    dataPacketSensors.soilHumidity == 0 ||
    dataPacketSensors.soilHumidityRaw == 0 ||
    dataPacketSensors.airHumidityInt == 0 ||
    dataPacketSensors.temperatureInt == 0 ||
    dataPacketSensors.LDR == 0 ||
    dataPacketSensors.voltageBAT == 0 ||
    dataPacketSensors.airHumidityExt == 0 ||
    dataPacketSensors.temperatureExt == 0)
    return false;
  else
    return true;
}

static void parseData(char *data, sensorData *dataSensors, systemState *systemStateData){
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, data);
  int uc = doc["uc"];

  if (uc == 1){
    int mode = doc["mode"];
    if (mode == 1){
      dataSensors->airHumidityInt = doc["airhumidityint"];
      dataSensors->airHumidityRawInt = doc["airhumidityrawint"];
      dataSensors->temperatureInt = doc["temperatureint"];
      dataSensors->temperatureRawInt = doc["temperaturerawint"];
      dataSensors->soilHumidity = doc["soilHumidity"];
      dataSensors->soilHumidityRaw = doc["soilHumidityRaw"];
      dataSensors->LDR = doc["lightIntensity"];
      dataSensors->voltageBAT = doc["batVoltage"];
    }
  }

  if (uc == 2){
    int mode = doc["mode"];
    if (mode == 1){
      systemStateData->xAxis = doc["X"];
      systemStateData->yAxis = doc["Y"];
    }
  }

  if (uc == 3){
    int mode = doc["mode"];
    if (mode == 1){
      systemStateData->photo = doc["photo"];
    }
  }

  //printSensorData();
}

static void parseDataPre(uint8_t *data){
  parseData((char *)data, &dataPacketSensors, &systemStateData);
}

void postSensorDataTCP(sensorData *dataSensors){
  jsonDocument.clear();
  jsonPostMessage("airhumidityint", dataSensors->airHumidityInt);
  jsonPostMessage("airhumidityrawint", dataSensors->airHumidityRawInt);
  jsonPostMessage("temperatureint", dataSensors->temperatureInt);
  jsonPostMessage("temperaturerawint", dataSensors->temperatureRawInt);
  jsonPostMessage("soilHumidity", dataSensors->soilHumidity);
  jsonPostMessage("soilHumidityRaw", dataSensors->soilHumidityRaw);
  jsonPostMessage("lightIntensity", dataSensors->LDR);
  jsonPostMessage("batVoltage", dataSensors->voltageBAT);

  serializeJson(jsonDocument, buffer);

  // sendMessageToClient(buffer);
}

void getSensorDataNoTask()
{
    TelnetStream.println("Reading Sensor Data");

    do
    {
      delay(1100);
      dataPacketSensors.airHumidityExt = dht.readHumidity();
      dataPacketSensors.airHumidityRawExt = 0;
      dataPacketSensors.temperatureExt = dht.readTemperature();
      dataPacketSensors.temperatureRawExt = 0;

    } while (isnan(dataPacketSensors.airHumidityExt) || isnan(dataPacketSensors.temperatureExt));

    //printSensorData();
  
}

void getSensorData(void *dataSensorsIn)
{
  sensorData *dataSensors = reinterpret_cast<sensorData *>(dataSensorsIn);

  for (;;)
  {
    TelnetStream.println("Reading Sensor Data");

    do
    {
      delay(2000);
      dataPacketSensors.airHumidityExt = dht.readHumidity();
      dataPacketSensors.airHumidityExt = dataPacketSensors.airHumidityExt * 100.0f;
      dataPacketSensors.airHumidityExt = dataPacketSensors.airHumidityExt / 100.0f;
      dataPacketSensors.airHumidityRawExt = 0;
      dataPacketSensors.temperatureExt = dht.readTemperature();
      dataPacketSensors.temperatureRawExt = 0;

    } while (isnan(dataPacketSensors.airHumidityExt) || isnan(dataPacketSensors.temperatureExt));

    printSensorData();

    vTaskDelay(10000 / portTICK_PERIOD_MS);
  }
}

void faning(bool mode)
{
  if (mode)
  {
    // TelnetStream.println("Faning");
    ledcWrite(pwmChannelFan, 255);
  }
  else
  {
    // TelnetStream.println("Not faning");
    ledcWrite(pwmChannelFan, 0);
  }
}

float getWaterReservoir()
{
  digitalWrite(WATERTRIGPIN, LOW);
  delayMicroseconds(2);
  digitalWrite(WATERTRIGPIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(WATERTRIGPIN, LOW);

  duration = pulseIn(WATERECHOPIN, HIGH);

  distanceMm = duration * SOUND_SPEED / 2;

  waterQtMl = -30.871 * distanceMm + 5768.9;

  return waterQtMl;
}

void wateringViaTime(int time)
{
  ledcWrite(pwmChannelWater, 255);
  delay(time * 1000);
  ledcWrite(pwmChannelWater, 0);
  systemStateData.waterlevel = getWaterReservoir();
}

void watering(bool mode)
{
  if (mode)
  {
    // TelnetStream.println("Watering");
    // digitalWrite(wateringPin, LOW);
    ledcWrite(pwmChannelWater, 255);
  }
  else
  {
    // digitalWrite(wateringPin, HIGH);
    ledcWrite(pwmChannelWater, 0);
    // TelnetStream.println("Watering Stoped");
  }
  systemStateData.waterlevel = getWaterReservoir();
}

void wateringDC(int time)
{ 
  ledcWrite(pwmChannelWater, 255);
  delay(time * 1000);
  ledcWrite(pwmChannelWater, 0);
  delay(10000 - (time * 1000));
  
  systemStateData.waterlevel = getWaterReservoir();
}

float getVPP()
{
  float result;
  int readValue;       // value read from the sensor
  int maxValue = 0;    // store max value here
  int minValue = 4096; // store min value here ESP32 ADC resolution

  uint32_t start_time = millis();
  while ((millis() - start_time) < 1000) // sample for 1 Sec
  {
    readValue = analogRead(CURRENTMEASPIN);
    // see if you have a new maxValue
    if (readValue > maxValue)
    {
      /*record the maximum sensor value*/
      maxValue = readValue;
    }
    if (readValue < minValue)
    {
      /*record the minimum sensor value*/
      minValue = readValue;
    }
  }

  // Subtract min from max
  result = ((maxValue - minValue) * 3.3) / 4096.0; // ESP32 ADC resolution 4096

  return result;
}

int getLightCurrent()
{
  Voltage = getVPP();
  /*
  TelnetStream.print("getVPP: ");
  TelnetStream.println(Voltage);
  TelnetStream.print("analogread: ");
  TelnetStream.println(analogRead(CURRENTMEASPIN));*/

  VRMS = (Voltage / 2.0) * 0.707;             // root 2 is 0.707
  AmpsRMS = ((VRMS * 1000) / mVperAmp) - 0.3; // 0.3 is the error I got for my sensor
  return AmpsRMS;
}

void light(bool mode)
{
  if (mode)
  {
    ledcWrite(pwmChannelREDLED, 255);
  }
  else
  {
    ledcWrite(pwmChannelREDLED, 0);
  }

  systemStateData.lightConsumption = getLightCurrent();
}

void setupZMotor()
{
  pinMode(ZStepPin, OUTPUT);
  pinMode(ZDirPin, OUTPUT);
}

void stepMotor()
{
  delayMicroseconds(PULSEWIDTH);
  digitalWrite(ZStepPin, HIGH);
  delayMicroseconds(PULSEWIDTH);
  digitalWrite(ZStepPin, LOW);

  delayMicroseconds(DELAYMOTOR);
}

int getZPos()
{
  digitalWrite(ZTRIGPIN, LOW);
  delayMicroseconds(2);
  digitalWrite(ZTRIGPIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(ZTRIGPIN, LOW);

  duration = pulseIn(ZECHOPIN, HIGH);

  distanceMm = duration * SOUND_SPEED / 2;

  TelnetStream.print("height: ");
  TelnetStream.println(distanceMm);

  return distanceMm;
}

void zAxisPos(int pos){

  /*
  int value = systemStateData.zAxis - pos;

  TelnetStream.print("pos = ");
  TelnetStream.println(pos);

  TelnetStream.print("zaxis = ");
  TelnetStream.println(systemStateData.zAxis);

  TelnetStream.print("value = ");
  TelnetStream.println(value);
  TelnetStream.print("value/20 = ");

  TelnetStream.println(value/2.7);


  if(value < 0){
    digitalWrite(ZDirPin, LOW);

    for(int i = 0; i < abs(value/2.7); i++){
      for (int i = 0; i < ZStepsPerRevolution; i++){
        stepMotor();
      }
    }
  }
  else{
    digitalWrite(ZDirPin, HIGH);

    for(int i = 0; i < abs(value/2.7); i++){
      for (int i = 0; i < ZStepsPerRevolution; i++){
        stepMotor();
      }
    }

  }
  */

  
  if (pos < systemStateData.zAxis){
    // Set the spinning direction clockwise:
    digitalWrite(ZDirPin, HIGH);

    while (pos < getZPos()){
      for (int i = 0; i < ZStepsPerRevolution; i++){
        stepMotor();
      }
      delay(500);
    }
  }
  else{
    digitalWrite(ZDirPin, LOW);
    while (pos > getZPos()){
      for (int i = 0; i < ZStepsPerRevolution; i++){
        stepMotor();
      }
      delay(500);
    }
  }
  
  delay(1000);
  systemStateData.zAxis = getZPos();
}

// true lows platform
void zAxis(int loops, bool dir)
{
  if (dir)
  {
    // Set the spinning direction clockwise:
    digitalWrite(ZDirPin, HIGH);
    for (int j = 0; j < loops; j++)
    {
      for (int i = 0; i < ZStepsPerRevolution; i++)
      {
        stepMotor();
      }
    }
  }

  else
  {
    digitalWrite(ZDirPin, LOW);
    for (int j = 0; j < loops; j++)
    {
      for (int i = 0; i < ZStepsPerRevolution; i++)
      {
        stepMotor();
      }
    }
  }
}

int getXY(String string){

  INPUT_STRING = string;
  INPUT_STRING.toUpperCase();

  if (INPUT_STRING.startsWith("G00")){
    START = INPUT_STRING.indexOf('X');
    if (!(START < 0))
    {
      TelnetStream.println("got here");
      FINISH = START + 8;
      SUB_STRING = INPUT_STRING.substring(START + 1, FINISH + 1);
      X = SUB_STRING.toFloat();
      
    }
    START = INPUT_STRING.indexOf('Y');
    if (!(START < 0))
    {
      FINISH = START + 8;
      SUB_STRING = INPUT_STRING.substring(START + 1, FINISH + 1);
      Y = SUB_STRING.toFloat();
    }
  }
    TelnetStream.println(X);
    return X, Y;
}

bool movePlaneAxis(const char * pos){
  TelnetStream.println("Moving axis ...");
  //publishLog("Moving axis ...");
  delay(1000);

  X, Y = getXY(String(pos));
  TelnetStream.printf("Moving axis to x:%d y:%d", X, Y);
  TelnetStream.println("");
  
  bool result = (getClient(axisAddress) != 0);
  if(!result){
    TelnetStream.println("Axis Not Connected to the system");
    publishLog("Axis Not Connected to the system");
    return false;
  }
    
  if(result){
    if(X > 1100.0){
      X = 1100;
      TelnetStream.println("X out of boundaries.");
      //publishLog("X out of boundaries. Moving to X1100");
    }

    if(Y > 490.0){
      Y = 490;
      TelnetStream.println("Y out of boundaries.");
      //publishLog("Y out of boundaries. Moving to Y490");
    }

    while (!(systemStateData.xAxis == X && systemStateData.yAxis == Y)){
      sendMessageToClient(pos, axisAddress);
      TelnetStream.println("Positioning Axis...");
      //publishLog("Positioning Axis...");
      delay(4000);
    }

    TelnetStream.println("On Position");
  }
  return  true;
}

bool scanGreenhouse1(){
  TelnetStream.println("Taking photo...");

  publishLog("Taking photo...");

  delay(2000);
  
  bool result = (getClient(camAddress) != 0);
  if(!result){
    TelnetStream.println("Cam Not Connected to the system");
    delay(1000);
    publishLog("Cam Not Connected to the system");
    return false;
  }
    
  if(result){
    //zAxisPos(1000);

    String message = "";
    while (!systemStateData.photo){
      TelnetStream.println("Taking Photo...");
      systemStateData.photo =  false;

      String message = "TAKEPHOTO N1";
      result = (getClient(camAddress) != 0);
      if(result)
        sendMessageToClient(message.c_str(), camAddress);
      else
        break;
      delay(5000);
      TelnetStream.print("photooooooooo: ");
      TelnetStream.println(systemStateData.photo);
    }
    TelnetStream.print("photooooooooo: ");
    TelnetStream.println(systemStateData.photo);
    
    delay(2000);
    publishLog("Photo Taken");
    systemStateData.photo =  false;
  }
  
  return  true;
}

bool scanGreenhouse(){
  TelnetStream.println("Start Greenhouse Scan...");
  
  
  bool result = (getClient(axisAddress) != 0);
  if(!result){
    TelnetStream.println("Axis Not Connected to the system");
    return false;
  }
  
  /*
  bool result = (getClient(camAddress) != 0);
  if(!result){
    TelnetStream.println("Cam Not Connected to the system");
    return false;
  }*/
    
  if(result){
    
    const char* message = "G28";
    bool result = sendMessageToClient(message, axisAddress);
    if (result){
      TelnetStream.println("Homing Axis....");
    }
    else{
      TelnetStream.println("Moving Axis Aborted, couldn't send TCP message");
    }
    delay(1000);
    while (systemStateData.xAxis != 0 && systemStateData.yAxis != 0){
      TelnetStream.println(systemStateData.xAxis);
      TelnetStream.println("Homing....");
    }
    TelnetStream.println("Axis Homed!");

    for (int8_t i = 0; i < nPlants; i++){
      
      while (!(systemStateData.xAxis == xPos[i] && systemStateData.yAxis == yPos[i])){
        sendMessageToClient(scanCoordinates[i], axisAddress);
        TelnetStream.println("Positioning Axis...");
        delay(5000);
      }
      delay(5000);
      /*
      systemStateData.photo = false;
      while (!systemStateData.photo){
        TelnetStream.println("Taking Photo...");
        systemStateData.photo =  false;

        String message = "TAKEPHOTO N";
        message = message + i;
        sendMessageToClient(message.c_str(), camAddress);

        delay(5000);
      }
      */
    }

    delay(1000);
    result = sendMessageToClient(message, axisAddress);
    if (result){
      TelnetStream.println("Homing Axis....");
    }
    else{
      TelnetStream.println("Moving Axis Aborted, couldn't send TCP message");
    }

    while (systemStateData.xAxis != 0 && systemStateData.yAxis != 0){
      //TelnetStream.println(systemStateData.xAxis);
      TelnetStream.println("Homing....");
    }
  }
  return  true;
}

void controlActuatorsManual(systemState *systemStateData){
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
  }

  strftime(timeHour, 9, "%H:%M:%S", &timeinfo);
  strftime(timeDate, 20, "%d/%B/%Y", &timeinfo);

  TelnetStream.print("MANUAL CONTROL");
  TelnetStream.print(" - ");
  TelnetStream.print(timeDate);
  TelnetStream.print(" - ");
  TelnetStream.println(timeHour);

  unsigned long cur_time = millis(); // Just one call to millis()
  lightFSM.tis = cur_time - lightFSM.tes;
  ventilationFSM.tis = cur_time - ventilationFSM.tes;
  waterFSM.tis = cur_time - waterFSM.tes;

  // lightFSM;
  if (timeinfo.tm_hour >= 8 && timeinfo.tm_hour < 20)
    lightFSM.new_state = 1;
  else
    lightFSM.new_state = 0;

  // ventilationFSM
  if (timeinfo.tm_hour >= 8 && timeinfo.tm_hour < 20)
    ventilationFSM.new_state = 1;
  else
    ventilationFSM.new_state = 0;

  // waterFSM
  if ((timeinfo.tm_hour == 8 && timeinfo.tm_min == 0) || (timeinfo.tm_hour == 20 && timeinfo.tm_min == 0))
    waterFSM.new_state = 1;
  else
    waterFSM.new_state = 0;

  set_state(lightFSM, lightFSM.new_state, 1);
  set_state(ventilationFSM, ventilationFSM.new_state, 3);
  set_state(waterFSM, waterFSM.new_state, 2);

  // update systemStateData to db
  systemStateData->lightState = lightFSM.state;
  systemStateData->tisLight = lightFSM.tis;
  systemStateData->ventilationState = ventilationFSM.state;
  systemStateData->tisVentilation = ventilationFSM.tis;
  systemStateData->waterState = waterFSM.state;
  systemStateData->tisWater = waterFSM.tis;

  // lightFSM
  if (lightFSM.state == 0)
    light(false);
  else if (lightFSM.state == 1)
    light(true);

  // ventilationFSM
  if (ventilationFSM.state == 0)
    faning(false);
  else if (ventilationFSM.state == 1)
    faning(true);

  // waterFSM
  if (waterFSM.state == 0)
    watering(false);
  else if (lightFSM.state == 1)
    watering(true);

  //TelnetStream.println("-- System States --");
  //TelnetStream.print("lightFSM.state: ");
  //TelnetStream.println(lightFSM.state);
  //TelnetStream.print("ventilationFSM.state: ");
  //TelnetStream.println(ventilationFSM.state);
  //TelnetStream.print("waterFSM.state: ");
  //TelnetStream.println(waterFSM.state);
}

void controlActuatorsAuto(systemState *systemStateData){
  int pidValueWater = pidWater.calculate((double)PIDSETPOINTWATER, (double)dataPacketSensors.soilHumidity);
  int pidValueWater2 = pidWater2.calculate((double)PIDSETPOINTWATER, (double)dataPacketSensors.soilHumidity);
  int pidValueTemp = pidTemp.calculate(PIDSETPOINTTEMP, dataPacketSensors.temperatureInt);

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
  }

  strftime(timeHour, 9, "%H:%M:%S", &timeinfo);
  strftime(timeDate, 20, "%d/%B/%Y", &timeinfo);

  TelnetStream.print("AUTONOMOUS CONTROL");
  TelnetStream.print(" - ");
  TelnetStream.print(timeDate);
  TelnetStream.print(" - ");
  TelnetStream.println(timeHour);

  unsigned long cur_time = millis(); // Just one call to millis()
  lightFSM.tis = cur_time - lightFSM.tes;
  ventilationFSM.tis = cur_time - ventilationFSM.tes;
  waterFSM.tis = cur_time - waterFSM.tes;

  // lightFSM;
  if (timeinfo.tm_hour >= 8 && timeinfo.tm_hour < 16)
    lightFSM.new_state = 1;
  else
    lightFSM.new_state = 0;

  // ventilationFSM
  if (timeinfo.tm_hour >= 8 && timeinfo.tm_hour < 20)
    ventilationFSM.new_state = 1;
  else
    ventilationFSM.new_state = 0;

  //waterFSM
  if(pidValueWater > 0 || manualWater)
    waterFSM.new_state = 1;
  else
    waterFSM.new_state = 0;

  // update systemStateData to db
  systemStateData->lightState = lightFSM.new_state;
  systemStateData->tisLight = lightFSM.tis;
  systemStateData->ventilationState = ventilationFSM.new_state;
  systemStateData->tisVentilation = ventilationFSM.tis;
  systemStateData->waterState = waterFSM.new_state;
  systemStateData->tisWater = waterFSM.tis;

  //if new state
  if (waterFSM.new_state!= waterFSM.state || lightFSM.new_state!= lightFSM.state || ventilationFSM.new_state!= ventilationFSM.state || !publishedDBstate)
  {
    if(checkSensorData()){
      if (publishMessageDB(timeHour, timeDate, &dataPacketSensors, systemStateData)){
      publishedDBstate = true;
      }
      else
        publishedDBstate = false;
    }
    else
        publishedDBstate = false;
  }


  set_state(lightFSM, lightFSM.new_state, 1);
  set_state(ventilationFSM, ventilationFSM.new_state, 3);
  set_state(waterFSM, waterFSM.new_state, 2);

  // lightFSM
  if (lightFSM.state == 0)
    light(false);
  else if (lightFSM.state == 1)
    light(true);

  // ventilationFSM
  if (ventilationFSM.state == 0)
    faning(false);
  else if (ventilationFSM.state == 1)
    faning(true);

  // waterFSM
  if (waterFSM.state == 0)
    watering(false);
  else if(manualWater)
    watering(true);
  else if (lightFSM.state == 1)
    wateringDC(pidValueWater2);

  if(manualWater)
    manualWater = false;
  //TelnetStream.println("-- System States --");
  //TelnetStream.print("lightFSM.state: ");
  //TelnetStream.println(lightFSM.state);
  //TelnetStream.print("ventilationFSM.state: ");
  //TelnetStream.println(ventilationFSM.state);
  //TelnetStream.print("waterFSM.state: ");
  //TelnetStream.println(waterFSM.state);

  TelnetStream.print("pidValueWater: ");
  TelnetStream.println(pidValueWater);
  TelnetStream.print("soilHumidity: ");
  TelnetStream.println(dataPacketSensors.soilHumidity);
  TelnetStream.print("PIDSETPOINT: ");
  TelnetStream.println(PIDSETPOINTWATER);
}

void setupTask(){
  xTaskCreate(
      getSensorData,
      "Read sensor data",
      10000,
      &dataPacketSensors,
      1,
      NULL);
}

void setup(){
  Serial.begin(115200);
  Serial.println("Booting");

  setupWIFI();

  setupTCP();

  dht.begin();

  connectAWS();

  setupOTA("MASTER", ssid, password);

  setupTask();

  setupZMotor();

  // LIGHT SETUP
  pinMode(LIGHTPIN, OUTPUT);
  digitalWrite(LIGHTPIN, HIGH);

  // SONAR SETUP
  pinMode(ZTRIGPIN, OUTPUT);
  pinMode(ZECHOPIN, INPUT);
  pinMode(WATERTRIGPIN, OUTPUT);
  pinMode(WATERECHOPIN, INPUT);

  // LED LIGHTS SETUP
  ledcSetup(pwmChannelREDLED, frequencyLED, resolutionLED);
  ledcAttachPin(REDPIN, pwmChannelREDLED);

  // FAN SETUP
  ledcSetup(pwmChannelFan, frequencyFan, resolutionFan);
  ledcAttachPin(faningPin, pwmChannelFan);

  // WATER PUMP SETUP
  ledcSetup(pwmChannelWater, frequencyWater, resolutionWater);
  ledcAttachPin(wateringPin, pwmChannelWater);

  // SET STATES
  set_state(lightFSM, 0, 5);
  set_state(ventilationFSM, 0, 5);
  set_state(waterFSM, 0, 5);

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)){
    TelnetStream.println("Failed to obtain time");
  }

  strftime(timeHour, 9, "%H:%M:%S", &timeinfo);
  strftime(timeDate, 20, "%d/%B/%Y", &timeinfo);

  // GET Z POS 
  dataPacketSensors.soilHumidity = PIDSETPOINTWATER + 5;
  systemStateData.xAxis = 1;
  systemStateData.yAxis = 1;
  systemStateData.zAxis = getZPos();

  publishMessageSystemsStateDB(timeHour, timeDate, &dataPacketSensors, &systemStateData, 0);

  systemStateData.waterlevel = getWaterReservoir();
  publishMessageSystemsStateDB(timeHour, timeDate, &dataPacketSensors, &systemStateData, 2);
}

void loop()
{
  TelnetStream.println("-- Master Sample Code --");
  Serial.println("-- Master Sample Code --");

  ArduinoOTA.handle();

  bool AWSConnected = client.loop();

  if (!client.connected()){
    reconnectAWS();
  }

  //getSensorDataNoTask();

  if (ManualControl){
    controlActuatorsManual(&systemStateData);
  }

  if (!ManualControl){
    controlActuatorsAuto(&systemStateData);
  }

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    TelnetStream.println("Failed to obtain time");
  }

  strftime(timeHour, 9, "%H:%M:%S", &timeinfo);
  strftime(timeDate, 20, "%d/%B/%Y", &timeinfo);

  // uploads every hour to lettuceSample/pub db
  if (timeinfo.tm_hour != publishHour || !publishedDB)
  {
    if(checkSensorData()){
      if (publishMessageDB(timeHour, timeDate, &dataPacketSensors, &systemStateData)){
      publishHour = timeinfo.tm_hour;
      publishedDB = true;
      }
      else
        publishedDB = false;
    }
    else
        publishedDB = false;
  }

  if (timeinfo.tm_hour == 0 && timeinfo.tm_min == 0 && timeinfo.tm_sec < 5)
  {
    TelnetStream.println("Restarting Master uC");
    ESP.restart();
  }

  systemStateData.zAxis = getZPos();

  getAllClients(timeHour, timeDate, &clientsState);

  publishMessageRTSystemState(&clientsState, &systemStateData);
  publishMessageRTClientState(&clientsState, &systemStateData);

  //publishMessageRealTimeMonitor(&dataPacketSensors, &systemStateData);
  if(checkSensorData())
    publishMessageRTSensorData(&dataPacketSensors, &systemStateData);

  delay(4000);
}
 