
#define PI 3.1415926535897932384626433832795
#define HALF_PI 1.5707963267948966192313216916398
#define TWO_PI 6.283185307179586476925286766559
#define DEG_TO_RAD 0.017453292519943295769236907684886
#define RAD_TO_DEG 57.295779513082320876798154814105

// ----- Bit set/clear/check/toggle macros
#define SET(x,y) (x |=(1<<y)) 
#define CLR(x,y) (x &= (~(1<<y)))
#define CHK(x,y) (x & (1<<y))
#define TOG(x,y) (x^=(1<<y))

// ----- motor definitions 
#define STEPS_PER_MM 200*8/40      //200steps/rev; 16 x microstepping; 40mm/rev

#define DIRX 16                      //arduino ports
#define DIRY 27
#define STEPX 26
#define STEPY 25
#define ENDSTOPXPIN 19
#define ENDSTOPYPIN 5
#define ENABLEPIN 12


bool
CW = true,                          //flag ... does not affect motor direction
CCW = false,                        //flag ... does not affect motor direction
DIRECTIONX,                         //motor directions can be changed in step_motors()
DIRECTIONY;

long
PULSE_WIDTH = 2,                    //easydriver step pulse-width (uS)
DELAY = 250;                        //delay (uS) between motor steps (controls speed)

// ----- plotter definitions
#define BAUD 9600
#define XOFF 0x13                   //pause transmission (19 decimal)
#define XON 0x11                    //resume transmission (17 decimal)
#define PEN 3

float
SCALE_FACTOR = 1.0;

int
/*
   XY plotters only deal in integer steps.
*/
THIS_X = 0,                         //"scaled" x co-ordinate (rounded)
THIS_Y = 0,                         //"scaled" y co-ordinate (rounded)
LAST_X = 0,                         //"scaled" x co-ordinate (rounded)
LAST_Y = 0;                         //"scaled" y co-ordinate (rounded)

// ----- gcode definitions
#define STRING_SIZE 128             //string size

char
BUFFER[STRING_SIZE + 1],
       INPUT_CHAR;

String
INPUT_STRING,
SUB_STRING;

int
INDEX = 0,                        //buffer index
START,                            //used for sub_string extraction
FINISH;

float
X,                                //gcode float values held here
Y,
I,
J;

//sonar variables

#define trigPin 5
#define echoPin 18

//define sound speed in cm/uS
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

long duration;
float distanceCm;
float distanceInch;

//TCP BUFFER
char buffer[250];


const char * scanCoordinates[15] = {"G00 X0 Y50", "G00 X0 Y260", "G00 X0 Y480", 
                                    "G00 X320 Y480", "G00 X320 Y260", "G00 X320 Y50",
                                    "G00 X590 Y50", "G00 X590 Y260", "G00 X590 Y480",
                                    "G00 X820 Y480", "G00 X820 Y260", "G00 X820 Y50",
                                    "G00 X1100 Y50", "G00 X1100 Y260", "G00 X1100 Y480"};