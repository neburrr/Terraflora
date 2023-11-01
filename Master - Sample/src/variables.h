const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;
char timeDate[20];
char timeHour[9];

char buffer[250];

char logMessage[1000];

typedef struct {
  float airHumidityExt;
  uint8_t airHumidityRawExt;
  float temperatureExt;
  uint8_t temperatureRawExt;
  uint8_t soilHumidity;
  uint16_t soilHumidityRaw;
  float airHumidityInt;
  uint8_t airHumidityRawInt;
  float temperatureInt;
  uint8_t temperatureRawInt;
  uint16_t LDR;
  float voltageBAT;
} sensorData;

typedef struct {
  uint8_t state, new_state;
  // tes - time entering state
  // tis - time in state
  unsigned long tes, tis;
} fsm_t;

typedef struct {
  bool waterState;
  float waterlevel;
  unsigned long tisWater;
  char timeHourWater[9];
  bool lightState;
  int lightConsumption;
  unsigned long tisLight;
  char timeHourLight[9];
  bool ventilationState;
  unsigned long tisVentilation;
  char timeHourVentilation[9];
  int xAxis;
  int yAxis;
  int zAxis;
  bool photo;
} systemState;

/*
struct {
  char name[20];
  const char IP[20];
  bool state;
  unsigned long tes, tis;
  char timeHour[9];
} clientStruct;
*/
const char * soilAddress = "192.168.1.110";
const char * axisAddress = "192.168.1.175";
const char * camAddress = "192.168.1.112";

typedef struct {
  int nrClients;

  const char * nameSoil = "Soil Uc";
  const char * IPSoil = soilAddress;
  bool stateSoil = false;
  unsigned long tesSoil, tisSoil;
  char timeHourSoil[9];
  char timeDateSoil[20];

  const char * nameAxis = "Axis Uc";
  const char * IPAxis = axisAddress;
  bool stateAxis = false;
  unsigned long tesAxis, tisAxis;
  char timeHourAxis[9];
  char timeDateAxis[20];

  const char * nameCam = "Cam Uc";
  const char * IPCam = camAddress;
  bool stateCam = false;
  unsigned long tesCam, tisCam;
  char timeHourCam[9];
  char timeDateCam[20];

} clientsStruct;



#define ZDirPin 25 //CW+
#define ZStepPin 33 //CLK+
#define ZStepsPerRevolution 360/1.8 * 2//1600
#define PULSEWIDTH 1000
#define DELAYMOTOR 1000

#define LIGHTPIN 26

#define REDPIN 32
#define GREENPIN 26
#define BLUEPIN 27
int pwmChannelREDLED = 1;
int pwmChannelGREENLED = 2;
int pwmChannelBLUELED = 3;
int frequencyLED = 1000;
int resolutionLED = 8;

#define faningPin 12
int pwmChannelFan = 0;
int frequencyFan = 1000;
int resolutionFan = 8;

#define wateringPin 14
int pwmChannelWater = 4;
int frequencyWater = 1000;
int resolutionWater = 8;

#define wateringTime 3000

#define DHT11PIN 4
#define DHTTYPE DHT11

#define SAMPLINGTIME 10  //10s

int PIDSETPOINTWATER = 50;
#define KPWATER 1
#define KIWATER 0.025
#define KDWATER 0.0
#define PIDMAXWATER 10
#define PIDMINWATER -10

int PIDSETPOINTTEMP = 20;
#define KPTEMP 1
#define KITEMP 0.05
#define KDTEMP 0.5
#define PIDMAXTEMP 2
#define PIDMINTEMP -2

bool publishedDB = false;
bool publishedDBstate = false;

int publishHour = 0;
int publishCnt = 0;

bool AWSControl = false;
bool ManualControl = false;

bool manualWater = false;

//sonar variables
#define ZTRIGPIN 5
#define ZECHOPIN 18
#define WATERTRIGPIN 17
#define WATERECHOPIN 16

//define sound speed in mm/uS
#define SOUND_SPEED 0.34

long duration;
int distanceMm;
float waterQtMl;

#define CURRENTMEASPIN 34
int mVperAmp = 185;
int Watt = 0;
double Voltage = 0;
double VRMS = 0;
double AmpsRMS = 0;

int nPlants = 15;
const char * scanCoordinates[15] = {"G00 X0 Y50 ", "G00 X0 Y260 ", "G00 X0 Y480 ", 
                                    "G00 X320 Y480 ", "G00 X320 Y260 ", "G00 X320 Y50 ",
                                    "G00 X590 Y50 ", "G00 X590 Y260 ", "G00 X590 Y480 ",
                                    "G00 X820 Y480 ", "G00 X820 Y260 ", "G00 X820 Y50 ",
                                    "G00 X1100 Y50 ", "G00 X1100 Y260 ", "G00 X1100 Y480 "};

int xPos [15] = {0, 0, 0,
                 320, 320, 320,
                 590, 590, 590,
                 820, 820, 820,
                 1100, 1100, 1100};

int yPos [15] = {50, 260, 480,
                 480, 260, 50,
                 50, 260, 480,
                 480, 260, 50,
                 50, 260, 480};

#define STRING_SIZE 128
char
BUFFER[STRING_SIZE + 1],
       INPUT_CHAR;
char messageLogBuffer[] = "";

String
INPUT_STRING,
SUB_STRING;

int
INDEX = 0,                        //buffer index
START,                            //used for sub_string extraction
FINISH;

int
X,                                //gcode float values held here
Y;
