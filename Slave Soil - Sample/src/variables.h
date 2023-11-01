#define soilHumidityPin 33
#define batVoltageADCPin 34
#define batCurrentADCPin 35
#define LDRPin 32
#define DHT11PIN 14

const int dryValue = 3500; 
const int wetValue = 1500;

char buffer[250];

float currentSome = 0;
int8_t nrSamplesCur = 100;

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

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;
char timeDate[20];
char timeHour[9];