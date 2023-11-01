const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;
char timeDate[20];
char timeHour[9];

String INPUT_STRING,
SUB_STRING;

int
INDEX = 0,                        //buffer index
START,                            //used for sub_string extraction
FINISH;

bool TCPConnected = false;

//TCP BUFFER
char buffer[250];

bool myWiFiFirstConnect = true;

#define LED_GPIO 4