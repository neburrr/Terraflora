// file that handles coreXY movement
// at first homes axis
// receives next command to move camera
// sends message when camera is in position

#include <Arduino.h>
#include <ArduinoJson.h>
#include "OTA.h"
#include "variables.h"
#include "./credentials.h"
#include "TCPHandler.h"

bool endstopx = false;
bool endstopy = false;

void postCoordinatesTCP(int xPosition, int yPosition) {  

  TelnetStream.println("Sending Data to TCP Server");

  DynamicJsonDocument doc(1024);

  doc["uc"] = 2;
  doc["mode"]  = 1;
  doc["X"] = xPosition;
  doc["Y"] = yPosition;
  
  serializeJson(doc, buffer);

  sendMessageToServer(buffer);
}

void pingServer() {

  TelnetStream.println("Sending Data to TCP Server");

  DynamicJsonDocument doc(1024);

  doc["uc"] = 4;
  doc["mode"]  = "Hi from AXIS";

  serializeJson(doc, buffer);

  sendMessageToServer(buffer);
}


void step_motors()
{
  digitalWrite(DIRX, !DIRECTIONX);
  digitalWrite(DIRY, !DIRECTIONY);

  delayMicroseconds(PULSE_WIDTH);

  digitalWrite(STEPX, HIGH);
  digitalWrite(STEPY, HIGH);

  delayMicroseconds(PULSE_WIDTH);

  digitalWrite(STEPX, LOW);
  digitalWrite(STEPY, LOW);

  delayMicroseconds(DELAY);
}

void YPositive()
{
  DIRECTIONX = CW;
  DIRECTIONY = CW;
  step_motors();
}

void YNegative()
{
  DIRECTIONX = CCW;
  DIRECTIONY = CCW;
  step_motors();
}

void XPositive()
{
  DIRECTIONX = CW;
  DIRECTIONY = CCW;
  step_motors();
}

void XNegative()
{
  DIRECTIONX = CCW;
  DIRECTIONY = CW;
  step_motors();
}

// ------------------------------------------------------------------------
// DRAW LINE
// ------------------------------------------------------------------------
/*
  This routine assumes that motor1 controls the x-axis and that motor2 controls
  the y-axis.

  The algorithm automatically maps all "octants" to "octant 0" and
  automatically swaps the XY coordinates if dY is greater than dX. A swap
  flag determines which motor moves for any combination X,Y inputs. The swap
  algorithm is further optimised by realising that dY is always positive
  in quadrants 0,1 and that dX is always positive in "quadrants" 0,3.

  Each intermediate XY co-ordinate is plotted which results in a straight line
*/
void draw_line(int x1, int y1, int x2, int y2)
{ // these are "scaled" co-ordinates
  int dy, dx;

  dy = y2 - y1;
  dx = x2 - x1;

  // move x direction
  for (int i = 0; i < abs(dx); i++)
  {
    if (dx < 0)
      XNegative();
    if (dx > 0)
      XPositive();
  }

  for (int i = 0; i < abs(dy); i++)
  {
    if (dy < 0)
      YNegative();
    if (dy > 0)
      YPositive();
  }
}

// -------------------------------
// MOVE_TO
// -------------------------------
void move_to(float x, float y)
{ // x,y are absolute co-ordinates

  // ----- apply scale factor
  THIS_X = round(x * STEPS_PER_MM * SCALE_FACTOR); // scale x and y
  THIS_Y = round(y * STEPS_PER_MM * SCALE_FACTOR);

  // ----- draw a line between these "scaled" co-ordinates
  draw_line(LAST_X, LAST_Y, THIS_X, THIS_Y);

  // ----- remember last "scaled" co-ordinate
  LAST_X = THIS_X;
  LAST_Y = THIS_Y;

  postCoordinatesTCP(x, y);
}

void homeSequence(){
  while (!digitalRead(ENDSTOPYPIN)){
    YNegative();
  }
  delay(200);

  while (!digitalRead(ENDSTOPXPIN)){
    XNegative();
  }

  LAST_X = 0;
  LAST_Y = 0;

  postCoordinatesTCP(0, 0);
}


//--------------------------------------------------------------------------
// PROCESS
//--------------------------------------------------------------------------
void process(String string)
{

  delay(100);
  // turn servos on
  

  // ----- convert string to upper case
  INPUT_STRING = string;
  INPUT_STRING.toUpperCase();

  if (INPUT_STRING.startsWith("G00"))
  {

    // ----- extract X
    START = INPUT_STRING.indexOf('X');
    if (!(START < 0))
    {
      FINISH = START + 8;
      SUB_STRING = INPUT_STRING.substring(START + 1, FINISH + 1);
      X = SUB_STRING.toFloat();
    }

    // ----- extract Y
    START = INPUT_STRING.indexOf('Y');
    if (!(START < 0))
    {
      FINISH = START + 8;
      SUB_STRING = INPUT_STRING.substring(START + 1, FINISH + 1);
      Y = SUB_STRING.toFloat();
    }
    
    if(X > 1100.0){
      X = 1100;
      Serial.println("X out of boundaries.");
    }

    if(Y > 490.0){
      Serial.printf("Y: %f", Y);
      Y = 490;
      Serial.println("Y out of boundaries.");
    }

    Serial.printf("moving to x: %f - y: %f\n", X, Y);
    digitalWrite(ENABLEPIN, LOW);
    move_to(X, Y);
    digitalWrite(ENABLEPIN, HIGH);

    INPUT_STRING = "";
  }

  if (INPUT_STRING.startsWith("G28"))
  {
    Serial.println("Performing Home Sequence");
    digitalWrite(ENABLEPIN, LOW);
    homeSequence();
    digitalWrite(ENABLEPIN, HIGH);
  }

  // turn servos off
  
}

void getSonarData(void *dataSensorsIn)
{
  for (;;)
  {
    // Clears the trigPin
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // Reads the echoPin, returns the sound wave travel time in microseconds
    duration = pulseIn(echoPin, HIGH);

    // Calculate the distance
    distanceCm = duration * SOUND_SPEED / 2;

    // Prints the distance in the Serial Monitor
    Serial.print("Distance (cm): ");
    Serial.println(distanceCm);

    vTaskDelay(10000 / portTICK_PERIOD_MS);
  }
}

void setupSonarTask()
{
  xTaskCreate(
      getSonarData,
      "Sonar Task",
      1024,
      NULL,
      1,
      NULL);
}

void setupMotorsAxis()
{ 
  //disable motors
  //pinMode(ENABLEPIN, OUTPUT);
  //digitalWrite(ENABLEPIN, HIGH);

  // ----- initialise motorX
  pinMode(DIRX, OUTPUT);
  pinMode(STEPX, OUTPUT);
  digitalWrite(DIRX, CW);
  delayMicroseconds(PULSE_WIDTH);
  digitalWrite(STEPX, LOW);

  // ----- initialise motory
  pinMode(DIRY, OUTPUT);
  pinMode(STEPY, OUTPUT);
  digitalWrite(DIRY, CW);
  delayMicroseconds(PULSE_WIDTH);
  digitalWrite(STEPY, LOW);

  // ----- plotter setup
  memset(BUFFER, '\0', sizeof(BUFFER)); // fill with string terminators
  INPUT_STRING.reserve(STRING_SIZE);
  INPUT_STRING = "";
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

void IRAM_ATTR ISRENDSTOPX()
{
  endstopx = !endstopx;
}

void IRAM_ATTR ISRENDSTOPY()
{
  endstopy = !endstopy;
}

int getXY(String string){

  INPUT_STRING = string;
  INPUT_STRING.toUpperCase();

  // ----------------------------------
  // G00   linear move with pen_up
  // ----------------------------------
  if (INPUT_STRING.startsWith("G00")){

    // ----- extract X
    START = INPUT_STRING.indexOf('X');
    if (!(START < 0))
    {
      FINISH = START + 8;
      SUB_STRING = INPUT_STRING.substring(START + 1, FINISH + 1);
      X = SUB_STRING.toFloat();
    }

    // ----- extract Y
    START = INPUT_STRING.indexOf('Y');
    if (!(START < 0))
    {
      FINISH = START + 8;
      SUB_STRING = INPUT_STRING.substring(START + 1, FINISH + 1);
      Y = SUB_STRING.toFloat();
    }
    
  }

    return X, Y;
}


void setup()
{
  Serial.begin(115200);
  Serial.println("Booting");

  setupWIFI();

  //setupOTA("AXIS", ssid, password);

  setupTCP();

  setupMotorsAxis();

  // endstopx = digitalRead(ENDSTOPX);
  // endstopy = digitalRead(ENDSTOPY);

  // attachInterrupt(ENDSTOPX, ISRENDSTOPX, CHANGE);
  // attachInterrupt(ENDSTOPY, ISRENDSTOPY, CHANGE);

  // setupSonarTask();

  pinMode(ENDSTOPXPIN, INPUT_PULLUP);
  pinMode(ENDSTOPYPIN, INPUT_PULLUP);

  process("G28");  
  delay(1000);
}

void loop()
{
  ArduinoOTA.handle();

  TelnetStream.println("AXIS Loop");
  //Serial.println("AXIS Loop");

  process(INPUT_STRING);

  INPUT_STRING = "";

  while (Serial.available())
  {
    INPUT_CHAR = (char)Serial.read(); 
    Serial.write(INPUT_CHAR);         
    BUFFER[INDEX++] = INPUT_CHAR;     
    if (INPUT_CHAR == '\n')
    {                                      
      Serial.flush();                      
      Serial.write(XOFF);                   
      INPUT_STRING = BUFFER;                
      process(INPUT_STRING);                
      memset(BUFFER, '\0', sizeof(BUFFER)); 
      INDEX = 0;                            
      INPUT_STRING = "";                    
      Serial.flush();                       
      Serial.write(XON);                    
    }
  }

  /*
    Serial.println("Xpositive");
    for (size_t i = 0; i < 200; i++)
    {
      XPositive();
    }
    delay(1000);
    Serial.println("XNegative");
    for (size_t i = 0; i < 200; i++)
    {
      XNegative();
    }
    delay(1000);*/
    
  postCoordinatesTCP(X, Y);

  //Serial.printf("y: %d , x: %d\n", digitalRead(ENDSTOPYPIN), digitalRead(ENDSTOPXPIN));

  //pingServer();
  delay(4000);
  
}
