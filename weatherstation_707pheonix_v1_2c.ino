/* https://github.com/707pheonix/WS2812-WeatherStation by 707pgoenix
 * ver.1.1 - niq_ro changed for 8x32 matrix: https://github.com/tehniq3
 * ver.1.2 - removed the buttons + added animated mode and added sunset/sunrise (moon/sun image)
 * ver.1.2a - solved 0 seconds issues when change from 24-format to 12-h format
 * ver.1.2b - no flicker at texts + DST solved
 * ver.1.2c - added humidity indicator
 */
 
//#define FASTLED_ESP8266_NODEMCU_PIN_ORDER
#include <SolarCalculator.h> //  https://github.com/jpb10/SolarCalculator
#include "FastLED.h"
#include <ESP8266WiFi.h>
#include <time.h>
#include <ArduinoJson.h>
#define NUM_LEDS 256
// Data pin that led data will be written out over
//#define DATA_PIN 5  //5
#define DATA_PIN 14 // D5 = GPIO14 // Data pin that led data will be written out over

CRGB leds[NUM_LEDS];

//WIFI Credentials
const char* ssid = "niq_ro";
const char* password = "5coldBeers!";

int timezone = 2; // 10800s; //19080s; //9 * 3600s;
int dst = 1; //3600s;

////weather varaibles
WiFiClient client;
char servername[]="api.openweathermap.org";              // remote weather server we will connect to
String result;
String APIKEY = "ca55295c4a4b9a681dce2688e0751dde";                                 
String CityID = "680332"; // "1264527";                        
int  fetchweatherflag = 0; 
String weatherDescription ="";
String weatherLocation = "";
String Country;
float Temperature=0.00;
float Humidity;
float Pressure;

//Matrix Mapping for 8X32 matrix
int Matrix[8][32]={
                  {0,15,16,31,32,47,48,63,64,79,80,95, 96,111,112,127,128,143,144,159,160,175,176,191,192,207,208,223,224,239,240,255},
                  {1,14,17,30,33,46,49,62,65,78,81,94, 97,110,113,126,129,142,145,158,161,174,177,190,193,206,209,222,225,238,241,254},
                  {2,13,18,29,34,45,50,61,66,77,82,93, 98,109,114,125,130,141,146,157,162,173,178,189,194,205,210,221,226,237,242,253},
                  {3,12,19,28,35,44,51,60,67,76,83,92, 99,108,115,124,131,140,147,156,163,172,179,188,195,204,211,220,227,236,243,252},
                  {4,11,20,27,36,43,52,59,68,75,84,91,100,107,116,123,132,139,148,155,164,171,180,187,196,203,212,219,228,235,244,251},
                  {5,10,21,26,37,42,53,58,69,74,85,90,101,106,117,122,133,138,149,154,165,170,181,186,197,202,213,218,229,234,245,250},
                  {6, 9,22,25,38,41,54,57,70,73,86,89,102,105,118,121,134,137,150,153,166,169,182,185,198,201,214,217,230,233,246,249},
                  {7, 8,23,24,39,40,55,56,71,72,87,88,103,104,119,120,135,136,151,152,167,168,183,184,199,200,215,216,231,232,247,248},
                  };

                  
//Number mapping in 8X3 matrix
int N1 [8][3]={{0,0,0},{0,1,0},{1,1,0},{0,1,0},{0,1,0},{1,1,1},{0,0,0},{0,0,0}};        
int N2 [8][3]={{0,0,0},{1,1,1},{0,0,1},{1,1,1},{1,0,0},{1,1,1},{0,0,0},{0,0,0}};        
int N3 [8][3]={{0,0,0},{1,1,1},{0,0,1},{1,1,1},{0,0,1},{1,1,1},{0,0,0},{0,0,0}};        
int N4 [8][3]={{0,0,0},{1,0,1},{1,0,1},{1,1,1},{0,0,1},{0,0,1},{0,0,0},{0,0,0}};        
int N5 [8][3]={{0,0,0},{1,1,1},{1,0,0},{1,1,1},{0,0,1},{1,1,1},{0,0,0},{0,0,0}};        
int N6 [8][3]={{0,0,0},{1,1,1},{1,0,0},{1,1,1},{1,0,1},{1,1,1},{0,0,0},{0,0,0}};  
int N7 [8][3]={{0,0,0},{1,1,1},{0,0,1},{0,0,1},{0,0,1},{0,0,1},{0,0,0},{0,0,0}};        
int N8 [8][3]={{0,0,0},{1,1,1},{1,0,1},{1,1,1},{1,0,1},{1,1,1},{0,0,0},{0,0,0}};        
int N9 [8][3]={{0,0,0},{1,1,1},{1,0,1},{1,1,1},{0,0,1},{1,1,1},{0,0,0},{0,0,0}};        
int N0 [8][3]={{0,0,0},{1,1,1},{1,0,1},{1,0,1},{1,0,1},{1,1,1},{0,0,0},{0,0,0}};        
int Nd [8][3]={{0,0,0},{1,1,0},{1,1,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}};        
int Nc [8][3]={{0,0,0},{1,1,1},{1,0,0},{1,0,0},{1,0,0},{1,1,1},{0,0,0},{0,0,0}};  
int A  [8][3]={{0,0,0},{1,1,0},{1,0,1},{1,1,1},{1,0,1},{1,0,1},{0,0,0},{0,0,0}};
int P  [8][3]={{0,0,0},{1,1,1},{1,0,1},{1,1,0},{1,0,0},{1,0,0},{0,0,0},{0,0,0}};
int M1 [8][3]={{0,0,0},{1,0,0},{1,1,0},{1,0,1},{1,0,0},{1,0,0},{0,0,0},{0,0,0}};
int M2 [8][3]={{0,0,0},{0,1,0},{1,1,0},{0,1,0},{0,1,0},{0,1,0},{0,0,0},{0,0,0}};

// Weather mapping for first 8X8 matrix

int ClearDay[]  = {2,5,16,18,19,20,21,23,26,27,28,29,34,35,36,37,40,42,43,44,45,47,58,61};
int ClearNight[]  = {18,19,20,21,25,26,27,28,29,30,32,39};  // adde by niq_ro
int sunsmall[]  = {28,31,33,34,45,46,48,51}; 
int moonsmall[]  = {33,34,44,47};   // added by niq_ro
int Cloud1[]    = {3,4,5,10,11,12,13,19,20,21,27};
int Cloud2[]    = {3,4,5,10,11,19,20,21,26,27,28,29,35,36,37,43};
int Cloud3[]    = {4,5,10,11,19,20,21,26,27,28,35,36,37,43,45};
int Cloud4[]    = {3,4,10,11,21,26,36,37,42,43,44,52,53,58,59,60};
int windy11[][8]={{0,0,2,0,0,5,0,0},{0,14,0,0,0,10,0,0},{0,0,18,0,20,0,0,0},{0,0,0,28,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0}};
int windy12[][8]={{0,0,2,0,0,5,0,0},{0,0,0,12,0,10,0,0},{0,0,18,0,0,21,0,0},{0,30,0,0,0,26,0,0},{0,0,34,0,36,0,0,0},{0,0,0,44,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0}};
int windy13[][8]={{0,0,0,0,0,0,0,0},{0,0,0,12,11,0,0,0},{0,0,18,0,0,21,0,0},{0,0,0,28,0,26,0,0},{0,0,34,0,0,37,0,0},{0,46,0,0,0,42,0,0},{0,0,50,0,52,0,0,0},{0,0,0,60,0,0,0,0}};
int windy14[][8]={{0,0,2,0,4,0,0,0},{0,0,0,12,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0},{0,0,0,35,36,0,0,0},{0,0,45,0,0,42,0,0},{0,0,0,51,0,53,0,0},{0,0,62,0,0,58,0,0}};
int StaticCloud[] = {13,17,18,19,20,27,28,29,33,34,35,36,43,44,45,46,47,49,50,51,61};
int Raindrop1[] = {8,10,25,40,42,57};
int Raindrop2[] = {9,24,26,41,56,58};
int Raindrop3[] = {5,7,22,37,39,54};
int Raindrop4[] = {6,21,23,38,53,55};
int thunder[]   = {19,20,23,25,27,29,33,36,37};
int mist1[]     = {1,4,7,10,13,18,21,24,27,30,33,36,39,40,43,46,50,53,56,59,62};
int mist2[]     = {2,5,8,11,14,17,20,23,26,29,34,37,42,45,49,52,55,61,58};

// alphabets in 8X3 matrix format
int a  [8][3]={{0,0,0},{1,1,0},{1,0,1},{1,1,1},{1,0,1},{1,0,1},{0,0,0},{0,0,0}};
int b  [8][3]={{0,0,0},{1,1,0},{1,0,1},{1,1,0},{1,0,1},{1,1,0},{0,0,0},{0,0,0}};
int c  [8][3]={{0,0,0},{1,1,1},{1,0,0},{1,0,0},{1,0,0},{1,1,1},{0,0,0},{0,0,0}};
int d  [8][3]={{0,0,0},{1,1,0},{1,0,1},{1,0,1},{1,0,1},{1,1,0},{0,0,0},{0,0,0}};
int e  [8][3]={{0,0,0},{1,1,1},{1,0,0},{1,1,1},{1,0,0},{1,1,1},{0,0,0},{0,0,0}};
int f  [8][3]={{0,0,0},{1,1,1},{1,0,0},{1,1,0},{1,0,0},{1,0,0},{0,0,0},{0,0,0}};
int g  [8][3]={{0,0,0},{1,1,0},{1,0,0},{1,0,0},{1,0,1},{1,1,0},{0,0,0},{0,0,0}};
int h  [8][3]={{0,0,0},{1,0,1},{1,0,1},{1,1,1},{1,0,1},{1,0,1},{0,0,0},{0,0,0}};
int i  [8][3]={{0,0,0},{1,1,1},{0,1,0},{0,1,0},{0,1,0},{1,1,1},{0,0,0},{0,0,0}};
int j  [8][3]={{0,0,0},{1,1,1},{0,1,0},{0,0,0},{0,1,0},{1,0,0},{0,0,0},{0,0,0}};
int k  [8][3]={{0,0,0},{1,0,1},{1,0,1},{1,1,0},{1,0,1},{1,0,0},{0,0,0},{0,0,0}};
int l  [8][3]={{0,0,0},{1,0,0},{1,0,0},{1,0,0},{1,0,0},{1,1,1},{0,0,0},{0,0,0}};
int mm1[8][3]={{0,0,0},{1,0,0},{1,1,0},{1,0,1},{1,0,0},{1,0,0},{0,0,0},{0,0,0}};
int mm2[8][3]={{0,0,0},{0,1,0},{1,1,0},{0,1,0},{0,1,0},{0,1,0},{0,0,0},{0,0,0}};
int nn1[8][3]={{0,0,0},{1,0,0},{1,1,0},{1,0,1},{1,0,0},{1,0,0},{0,0,0},{0,0,0}};
int nn2[8][3]={{0,0,0},{1,0,0},{1,0,0},{1,0,0},{1,0,0},{1,0,0},{0,0,0},{0,0,0}};
int o  [8][3]={{0,0,0},{1,1,1},{1,0,1},{1,0,1},{1,0,1},{1,1,1},{0,0,0},{0,0,0}};
int p  [8][3]={{0,0,0},{1,1,1},{1,0,1},{1,1,0},{1,0,0},{1,0,0},{0,0,0},{0,0,0}};
int Q  [8][3]={{0,0,0},{1,1,1},{1,0,1},{0,1,1},{0,0,1},{0,0,1},{0,0,0},{0,0,0}};
int r  [8][3]={{0,0,0},{1,1,1},{1,0,1},{1,1,0},{1,0,1},{1,0,1},{0,0,0},{0,0,0}};
int s  [8][3]={{0,0,0},{1,1,1},{1,0,0},{1,1,1},{0,0,1},{1,1,1},{0,0,0},{0,0,0}};
int t  [8][3]={{0,0,0},{1,1,1},{0,1,0},{0,1,0},{0,1,0},{0,1,0},{0,0,0},{0,0,0}};
int u  [8][3]={{0,0,0},{1,0,1},{1,0,1},{1,0,1},{1,0,1},{1,1,1},{0,0,0},{0,0,0}};
int v  [8][3]={{0,0,0},{1,0,1},{1,0,1},{1,0,1},{1,0,1},{0,1,0},{0,0,0},{0,0,0}};
int w1 [8][3]={{0,0,0},{1,0,0},{1,0,0},{1,0,1},{1,1,0},{1,0,0},{0,0,0},{0,0,0}};
int w2 [8][3]={{0,0,0},{0,1,0},{0,1,0},{0,1,0},{1,1,0},{0,1,0},{0,0,0},{0,0,0}};
int x  [8][3]={{0,0,0},{1,0,1},{1,0,1},{0,1,0},{1,0,1},{1,0,0},{0,0,0},{0,0,0}};
int y  [8][3]={{0,0,0},{1,0,1},{1,0,1},{0,1,0},{0,1,0},{0,0,0},{0,0,0},{0,0,0}};
int z  [8][3]={{0,0,0},{1,1,1},{0,0,1},{0,1,0},{1,0,0},{1,1,1},{0,0,0},{0,0,0}};

// varaible to store time value
int h1=0;
int h2=0;
int m1=0;
int m2=0;
int s1=0;
int s2=0;
int dat=0;
int mon=0;
int yr=0;
// varaible to store last time value 
int temph1=11;
int temph2=11;
int tempm1=11;
int tempm2=11;
int temps1=11;
int temps2=11;
int tempdat=11;
int tempmon=11;
int tempyr=11;
//other control varaible
int offset =3;             // to control reset of clock digits
float tempTEMP=0.00;       // last vale of temp to prevent unwanted number refresh
float tempHUMY=0.00;  
int secondreference=0;     // to use second value to control Animations
int minutereference=0;     // to use minute value to control Animations
int hourreference=0;     // to use hour value to control Animations
int stringoffset=9; // offset for controlling text with animation
int displayflag=0;
int screenid=1;
unsigned long tpschimbare;
byte noapte = 1;

// Location - Craiova: 44.317452,23.811336
double latitude = 44.31;
double longitude = 23.81;
double transit, sunrise, sunset;
int ora1, ora2, oraactuala;
int r1, hr1, mn1, r2, hr2, mn2; 

byte intensitate = 5;
byte intensitate1 = 10;  // day
byte intensitate0 = 3;  // night
byte intensitate3 = 8;
byte schimbare = 0;

void setup() {
  // sanity check delay - allows reprogramming if accidently blowing power w/leds
  delay(2000);
  
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(8);
  FastLED.clear();
  displaytext(offset,N7,Matrix);
  delay(2000);
      // TIME setup
  Serial.begin(115200);
  Serial.println();
  Serial.print("Wifi connecting to ");
  Serial.println( ssid );
  WiFi.begin(ssid,password);
  Serial.println();
  Serial.print("Connecting");
  
  

  while( WiFi.status() != WL_CONNECTED ){
      delay(500);
      Serial.print(".");        
  }

  Serial.println();
  Serial.println("Wifi Connected Success!");
  Serial.print("NodeMCU IP Address : ");
  Serial.println(WiFi.localIP() );
  configTime(timezone*3600, dst*3600, "pool.ntp.org","time.nist.gov");
  Serial.println("\nWaiting for Internet time");

  while(!time(nullptr)){
     Serial.print("*");
     delay(1000);
  }
  Serial.println("\nTime response....OK");   
  getWeatherData();
  FastLED.clear();

//timeformat1();// display time in HH:MM:SS format
FastLED.show();
delay(2100);
}

void loop() {
  // put your main code here, to run repeatedly:
    time_t now = time(nullptr);
    struct tm* p_tm = localtime(&now);
    //Serial.println(p_tm->tm_hour);
    h1=p_tm->tm_hour/10;  //1st digit of hour
    h2=p_tm->tm_hour%10;  //2st digit of hour
    m1=p_tm->tm_min/10;   //1st digit of minute
    m2=p_tm->tm_min%10;   //2st digit of minute
    s1=p_tm->tm_sec/10;   //1st digit of sec
    s2=p_tm->tm_sec%10;   //2st digit of sec
    dat=p_tm->tm_mday;    //Day of the month
    mon=p_tm->tm_mon + 1; // month of the year
    yr=p_tm->tm_year + 1900; //year
    secondreference= p_tm->tm_sec;                     // to be used as update animation based on second value
    hourreference=p_tm->tm_hour;
    minutereference=p_tm->tm_min;
    if(p_tm->tm_min%10==0 && fetchweatherflag ==0)    //Get new data every 10 minutes
    {
     fetchweatherflag = 1;
      getWeatherData();
    }
    else if (p_tm->tm_min%10!=0)
    {
     fetchweatherflag = 0;
    }

//timeformat1();// display time in HH:MM:SS format
//timeformat2();// display time in HH:MM PM/AM format
//showtemperature(Temperature);// display temperate with thermometer animation 
//showweather(weatherDescription);
//cleardigit(0,Matrix);
//cleardigit(4,Matrix);
//ClearDayAnime(ClearDay,(sizeof(ClearDay) / sizeof(int)));
//ClearDayAnime(sunsmall,(sizeof(sunsmall) / sizeof(int)));
//CloudyAnime();
//RainAnime();
//StormAnime();
//MistAnime();
//delay(120);

if (millis() - tpschimbare > 10000)
{
screenid=screenid+1;
if(screenid>4) screenid=1;
tpschimbare = millis();
schimbare = 0;
Soare();
night();
}
//mode1();
mode2();

FastLED.show();
delay(200);

///void loop ends
}

void digit(int offset, int digpoint [8][3], int matrix [8][32] )
{ cleardigit(offset,matrix);
  for (int i=0 ; i<8; i++)
  { for (int k=0 ; k<=i; k++)
    { for(int j=0;j<3; j++)
      { if (digpoint[k][j]==0)
        leds[matrix [k][j+offset]] = CRGB(0,0,0);
        else
        leds[matrix [k][j+offset]] = CRGB(200,200,255);
        }
        //delay(5);
        FastLED.show(); 
    }
    //delay(30);
   }
 }

  void displaytext(int offset, int digpoint [8][3], int matrix [8][32] )
{ cleardigit(offset,matrix);
  for (int i=0 ; i<8; i++)
  { for (int k=0 ; k<=i; k++)
    { for(int j=0;j<3; j++)
      { if (digpoint[k][j]==0)
        leds[matrix [k][j+offset]] = CRGB(0,0,0);
        else
        leds[matrix [k][j+offset]] = CRGB(200,200,255);
        }
        //delay(10);
        }
    //delay(20);
   }
   FastLED.show(); 
  }

void cleardigit(int offset,int matrix [8][32])
{ for (int k=0 ; k<8; k++)
    { for(int j=0;j<4; j++)
      { leds[matrix [k][j+offset]] = CRGB(0,0,0);
} }}


void timeformat1 ()
{   
     if (temps2 != s2)
    { offset=27;
      temps2=s2;
      updatedigit(offset,s2);
     // if(s2==8) delay(500);
      }  
      if (temps1 != s1)
    { offset=23;
      temps1=s1;
      updatedigit(offset,s1);
      }
     if (tempm2 != m2)
    { offset=17;
      tempm2=m2;
      updatedigit(offset,m2);
      } 
     if (tempm1 != m1)
    { offset=13;
      tempm1=m1;
      updatedigit(offset,m1);      
      }
     if (temph2 != h2)
    { offset=7;
      temph2=h2;
      updatedigit(offset,h2);
      }
      if (temph1 != h1)
    { offset=3;
      temph1=h1;
      updatedigit(offset,h1);
      }

// 24-hour format - flashing seconds
      if((s1*10+s2)%2==0)
      {leds[91] = CRGB(0,200,255);;
      leds[93] = CRGB(0,200,255);
      leds[171] = CRGB(0,200,255);
      leds[173]= CRGB(0,200,255);
      }
      else
      {leds[91] = CRGB(200,200,255);;
      leds[93] = CRGB(200,200,255);
      leds[171] = CRGB(200,200,255);
      leds[173]= CRGB(200,200,255);
      }
  }


void timeformat2 ()
{    //FastLED.clear();
  if((h1*10+h2)>12)
  {
    int temp = h1*10+h2-12;
    
    h1=temp/10;
    h2=temp%10;
    if (schimbare == 0)
    {
    updatedigit(22,12);
    updatedigit(26,13);
    updatedigit(29,14);
    schimbare = 1;
    }
  }
  else
  {
    if (schimbare == 0)
    {
    updatedigit(22,11);
    updatedigit(26,13);
    updatedigit(29,14);
    schimbare = 1;
    }
  }
  if (temph1 != h1  )
    { offset=3;
      temph1=h1;
      if (h1 == 0) 
      cleardigit(offset,Matrix);
      else
      updatedigit(offset,h1);
      }
   if (temph2 != h2  )
    { offset=7;
      temph2=h2;
      updatedigit(offset,h2);
      }
   if (tempm1 != m1 ) 
    { offset=13;
      tempm1=m1;
      updatedigit(offset,m1);      
      }
   if (tempm2 != m2 )
    { offset=17;
      tempm2=m2;
      updatedigit(offset,m2);
      } 

// 12-hour format - flashing seconds
      if((s1*10+s2)%2==0)
      {leds[91] = CRGB(0,200,255);;
      leds[93] = CRGB(0,200,255);
      }
      else
      {leds[91] = CRGB(200,200,255);;
      leds[93] = CRGB(200,200,255);
      }
  }


void updatedigit (int offset, int num)
{
  switch (num) {
  case 1: 
    digit(offset,N1,Matrix);
    break;
  case 2:
    digit(offset,N2,Matrix);
    break;
  case 3:
    digit(offset,N3,Matrix);
    break;
  case 4:
   digit(offset,N4,Matrix);
    break;
  case 5:
    digit(offset,N5,Matrix);
    break;
  case 6:
    digit(offset,N6,Matrix);
    break;
  case 7:
    digit(offset,N7,Matrix);
    break;
  case 8:
    digit(offset,N8,Matrix);
    break;
   case 9:
    digit(offset,N9,Matrix);
    break;
  case 0:
    digit(offset,N0,Matrix);
    break;
  case 11:
    displaytext(offset,A,Matrix);
    break;
  case 12:
    displaytext(offset,P,Matrix);
    break;
  case 13:
    displaytext(offset,M1,Matrix);
    break;
  case 14:
    displaytext(offset,M2,Matrix);
    break;
  default:
    // if nothing else matches, do the default
    // default is optional
    break;
} 
  }


  void getWeatherData()     //client function to send/receive GET request data.
{
  if (client.connect(servername, 80))   
          {                         //starts client connection, checks for connection
          client.println("GET /data/2.5/weather?id="+CityID+"&units=metric&APPID="+APIKEY);
          client.println("Host: api.openweathermap.org");
          client.println("User-Agent: ArduinoWiFi/1.1");
          client.println("Connection: close");
          client.println();
          } 
  else {
         Serial.println("connection failed");        //error message if no client connect
          Serial.println();
       }

  while(client.connected() && !client.available()) 
  delay(1);                                          //waits for data
  while (client.connected() || client.available())    
       {                                             //connected or data available
         char c = client.read();                     //gets byte from ethernet buffer
         result = result+c;
       }

client.stop();                                      //stop client
result.replace('[', ' ');
result.replace(']', ' ');
Serial.println(result);
char jsonArray [result.length()+1];
result.toCharArray(jsonArray,sizeof(jsonArray));
jsonArray[result.length() + 1] = '\0';
StaticJsonBuffer<1024> json_buf;
JsonObject &root = json_buf.parseObject(jsonArray);

if (!root.success())
  {
    Serial.println("parseObject() failed");
  }

String location = root["name"];
String country = root["sys"]["country"];
float temperature = root["main"]["temp"];
float humidity = root["main"]["humidity"];
String weather = root["weather"]["main"];
String description = root["weather"]["description"];
float pressure = root["main"]["pressure"];
//weatherDescription = description;
weatherDescription = weather;
weatherLocation = location;
Country = country;
Temperature = temperature;
Humidity = humidity;
Pressure = pressure;

Serial.println(weather);
Serial.println(temperature);

}


void showtemperature(float temp)
{
  AnimeThermo (temp,secondreference);
  if (schimbare == 0)
  {
  if(tempTEMP != temp)
  {
  tempTEMP = temp;
  int bd= (int) temp; 
  float adtemp = temp - bd;
  adtemp=adtemp*10;
  int ad = (int) adtemp;
  if (bd/10 == 0) 
    cleardigit(10,Matrix);
  else
    updatedigit(10,bd/10);
  updatedigit(14,bd%10);
  updatedigit(20,ad);
  Serial.println(bd);
  Serial.println(bd%10);
  Serial.println(adtemp);

 //to display degeree and celcius symbol
  leds[150]   = CRGB(200,200,235);
  leds[193]   = CRGB(200,200,235);
  leds[206]   = CRGB(200,200,235);
  leds[194]   = CRGB(200,200,235);
  leds[205]   = CRGB(200,200,235);
  leds[222]   = CRGB(200,200,235);
  leds[225]   = CRGB(200,200,235);
  leds[238]   = CRGB(200,200,255);
  leds[221]   = CRGB(200,200,255);
  leds[220]   = CRGB(200,200,255);
  leds[219]   = CRGB(200,200,255);
  leds[218]   = CRGB(200,200,255);
  leds[229]   = CRGB(200,200,255);
  leds[234]   = CRGB(200,200,255);
  }
  schimbare = 1;
  }
  //Function End
  }

  void AnimeThermo(float temp, int sec)
{
  leds[9  ]   = CRGB(200,200,235);
  leds[10 ]   = CRGB(200,200,235);
  leds[16 ]   = CRGB(200,200,235);
  leds[17 ]   = CRGB(200,200,235);
  leds[18 ]   = CRGB(200,200,235);
  leds[19 ]   = CRGB(200,200,235);
  leds[20 ]   = CRGB(200,200,235);
  leds[23 ]   = CRGB(200,200,235);
  leds[24 ]   = CRGB(200,200,235);
  leds[31 ]   = CRGB(200,200,235);
  leds[32 ]   = CRGB(200,200,235);
  leds[33 ]   = CRGB(200,200,235);
  leds[34 ]   = CRGB(200,200,235);
  leds[35 ]   = CRGB(200,200,235);
  leds[36 ]   = CRGB(200,200,255);
  leds[39 ]   = CRGB(200,200,255);
  leds[41 ]   = CRGB(200,200,255);
  leds[42 ]   = CRGB(200,200,255);  
  leds[60 ]   = CRGB(200,200,255);
  leds[62 ]   = CRGB(200,200,255);
  leds[65 ]   = CRGB(200,200,255);
  leds[67 ]   = CRGB(200,200,255);
  leds[69 ]   = CRGB(200,200,255);
// leds[8]   = CRGB(200,200,255);
// leds[240]   = CRGB(200,200,255);

if (temp<=10)
{ leds[30 ]   = CRGB(0,0,0);
  leds[29 ]   = CRGB(0,0,0);
  leds[28 ]   = CRGB(0,0,0);
  leds[27 ]   = CRGB(0,0,0);
  leds[21 ]   = CRGB(0,0,0);
  leds[26 ]   = CRGB(0,0,0);
  leds[37 ]   = CRGB(0,0,0);
  leds[22 ]   = CRGB(0,0,240);
  leds[25 ]   = CRGB(0,0,240);
  leds[38 ]   = CRGB(0,0,240);
  }
else if (temp>10 && temp <=20)
{ leds[30 ]   = CRGB(0,0,0);
  leds[29 ]   = CRGB(0,0,0);
  leds[28 ]   = CRGB(0,0,0);
  leds[27 ]   = CRGB(0,0,0);
  leds[21 ]   = CRGB(0,240,100);
  leds[26 ]   = CRGB(0,240,100);
  leds[37 ]   = CRGB(0,240,100);
  leds[22 ]   = CRGB(0,0,240);
  leds[25 ]   = CRGB(0,0,240);
  leds[38 ]   = CRGB(0,0,240);
  }

else if (temp>20 && temp <=30)
{ leds[30 ]   = CRGB(0,0,0);
  leds[29 ]   = CRGB(0,0,0);
  leds[28 ]   = CRGB(0,0,0);
  leds[27 ]   = CRGB(255,165,0);
  leds[21 ]   = CRGB(255,164,0);
  leds[26 ]   = CRGB(0,240,100);
  leds[37 ]   = CRGB(0,240,100);
  leds[22 ]   = CRGB(0,240,100);
  leds[25 ]   = CRGB(0,240,100);
  leds[38 ]   = CRGB(0,240,100);
  }
else if (temp>30 && temp <=40)
{ leds[30 ]   = CRGB(0,0,0);
  leds[29 ]   = CRGB(0,0,0);
  leds[28 ]   = CRGB(0,0,0);
  leds[27 ]   = CRGB(255,0,0);
  leds[21 ]   = CRGB(255,0,0);
  leds[26 ]   = CRGB(255,165,0);
  leds[37 ]   = CRGB(255,165,0);
  leds[22 ]   = CRGB(255,165,0);
  leds[25 ]   = CRGB(255,165,0);
  leds[38 ]   = CRGB(255,165,0);
  }
else if (temp>40 )
{ leds[30 ]   = CRGB(0,0,0);
  leds[29 ]   = CRGB(255,0,0);
  leds[28 ]   = CRGB(255,0,0);
  leds[27 ]   = CRGB(255,0,0);
  leds[21 ]   = CRGB(255,0,0);
  leds[26 ]   = CRGB(255,0,0);
  leds[37 ]   = CRGB(255,0,0);
  leds[22 ]   = CRGB(255,0,0);
  leds[25 ]   = CRGB(255,0,0);
  leds[38 ]   = CRGB(255,0,0);
  }

// sign animated
if (sec%2 == 0)
{ leds[58 ]   = CRGB(200,200,255);
  leds[51 ]   = CRGB(200,200,255);
  leds[49 ]   = CRGB(0,0,0);
} 
else
{ leds[58 ]   = CRGB(0,0,0);
  leds[51 ]   = CRGB(0,0,0);
  leds[49 ]   = CRGB(200,200,255);;
} 
 FastLED.show(); 
  }

//iterator = (sizeof(Raindrop1) / sizeof(int))
void ClearDayAnime(int *Num,int iterator)
{ for (int i=0; i< iterator;i++)
  { leds[*(Num+i)] = CRGB(255,int (random(140, 255)),0);
    }
  }

void ClearNightAnime(int *Num,int iterator)
{ for (int i=0; i< iterator;i++)
  { leds[*(Num+i)] = CRGB(255,int (random(140, 255)),0);
    }
  }


void AnimeXX (int *Num,int iterator, int r , int g, int b)
{ for (int i=0; i< iterator;i++)
  { leds[*(Num+i)] = CRGB(r,g,b);
    }
  }

//int StaticCloud[] = {5,61,59,58,57,65,66,67,68,69,70,71,126,125,124,123,122,121,130,131,132,133};
//int Raindrop1[] = {190,186,195,199,254,250};

void RainAnime()
{   
  AnimeXX(StaticCloud,sizeof(StaticCloud) / sizeof(int),220,220,240);
  if(secondreference%4==0)
  AnimeXX(Raindrop1,sizeof(Raindrop1) / sizeof(int),0,0,220);
    if(secondreference%4==1)
  AnimeXX(Raindrop2,sizeof(Raindrop2) / sizeof(int),0,0,220);
    if(secondreference%4==2)
  AnimeXX(Raindrop3,sizeof(Raindrop3) / sizeof(int),0,0,220);
    if(secondreference%4==3)
  AnimeXX(Raindrop4,sizeof(Raindrop4) / sizeof(int),0,0,220);
}

void StormAnime()
{   
  AnimeXX(StaticCloud,sizeof(StaticCloud) / sizeof(int),220,220,240);
  if(secondreference%2==1)
  AnimeXX(thunder,sizeof(thunder) / sizeof(int),255,140,0);
 }

void MistAnime()
{   
  //AnimeXX(StaticCloud,sizeof(StaticCloud) / sizeof(int),220,220,240);
  if(secondreference%2==0)
  AnimeXX(mist1,sizeof(mist1) / sizeof(int),135,206,250);
  if(secondreference%2==1)
  AnimeXX(mist2,sizeof(mist2) / sizeof(int),135,206,250);
 }

void CloudyAnime()
{   if(secondreference%4==0)
  windy1();
    if(secondreference%4==1)
  windy2();
    if(secondreference%4==2)
  windy3();
    if(secondreference%4==3)
  windy4();
}


void windy1()
{  for (int i=0;i<8;i++)
  { for (int j=0;j<8;j++)
        { if (windy11[i][j]!=0)
          { leds[windy11[i][j]] = CRGB(135,206,235);
            }
            }  }}

void windy2()
{ for (int i=0;i<8;i++)
  { for (int j=0;j<8;j++)
        {  if (windy12[i][j]!=0)
          {  leds[windy12[i][j]] = CRGB(135,206,235);
            }
             }   }  }

void windy3()
{ leds[45] = CRGB(0,0,0);
  leds[51] = CRGB(0,0,0);
  for (int i=0;i<8;i++)
  {for (int j=0;j<8;j++)
        { if (windy13[i][j]!=0)
          { leds[windy13[i][j]] = CRGB(135,206,235);
            }
            }  } }

void windy4( )
{  for (int i=0;i<8;i++)
  { for (int j=0;j<8;j++)
        {  if (windy14[i][j]!=0)
          {  leds[windy14[i][j]] = CRGB(135,206,235);
            }
         }   }  }

void ScrollingText(String disp)
{
  int offsetval[]   = {9,13,17,21,25,29};
  int tempcheck[]  =  {11,12,13,14,11,12};
  int strLen= disp.length();
  if (strLen>0 && strLen<7)
  {
    //function goes here
    for (int i=0 ; i<strLen; i++)
    {
      updatedigit (offsetval[i], tempcheck[i]);
      }
   
    // function ends here
    }
  }

void showweather (String wDe)
{
  //Serial.println(wDe);
  cleardigit(0,Matrix);
  cleardigit(4,Matrix);
  if (wDe.equalsIgnoreCase("Thunderstorm"))
  {
    StormAnime();
    if (schimbare == 0)
      {
    displaytext(9 ,s ,Matrix);
    displaytext(13,t ,Matrix);
    displaytext(17,o ,Matrix);
    displaytext(21,r ,Matrix);
    displaytext(25,mm1 ,Matrix);
    displaytext(28,mm2 ,Matrix);
    schimbare = 1;
      }
    }

    if (wDe.equalsIgnoreCase("Drizzle"))
  {
      if (minutereference%2==0)
      RainAnime();
      else
      CloudyAnime();
    if (schimbare == 0)
      {
    displaytext(9 ,d ,Matrix);
    displaytext(13,r ,Matrix);
    displaytext(17,i ,Matrix);
    displaytext(21,z ,Matrix);
    displaytext(25,l ,Matrix);
    displaytext(29,e ,Matrix);
    schimbare = 1;
      }
    }
  if (wDe.equalsIgnoreCase("Rain"))
    {
      RainAnime();
      if (schimbare == 0)
      {
      //displaytext(9 ,s ,Matrix);
      displaytext(12,r ,Matrix);
      displaytext(16,a ,Matrix);
      displaytext(20,i ,Matrix);
      displaytext(24,nn1 ,Matrix);
      displaytext(27,nn2 ,Matrix);
      schimbare = 1;
      }
      }
    if (wDe.equalsIgnoreCase("Snow"))
    {
      RainAnime();
      if (schimbare == 0)
      {
      //displaytext(9 ,s ,Matrix);
      displaytext(10 ,s ,Matrix);
      displaytext(14,nn1 ,Matrix);
      displaytext(17,nn2 ,Matrix);
      displaytext(19,o ,Matrix);
      displaytext(23,w1 ,Matrix);
      displaytext(26,w2 ,Matrix);
      schimbare = 1;
      }
      }
    if (wDe.equalsIgnoreCase("Clear"))
    {
      //if(hourreference >5 &&  hourreference<18)
      if (noapte == 0)
      ClearDayAnime(ClearDay,(sizeof(ClearDay) / sizeof(int)));
      else
      ClearNightAnime(ClearNight,(sizeof(ClearNight) / sizeof(int)));
      // add animation for clear night sky with moon
      if (schimbare == 0)
      {
      //displaytext(9 ,s ,Matrix);
      displaytext(10,c ,Matrix);
      displaytext(14,l ,Matrix);
      displaytext(18,e ,Matrix);
      displaytext(22,a ,Matrix);
      displaytext(26,r ,Matrix);
      schimbare = 1;
      }
      }
    if (wDe.equalsIgnoreCase("Clouds"))
    {
      //if(hourreference >5 &&  hourreference<18)
      if (noapte == 0)
      {
      ClearDayAnime(sunsmall,(sizeof(sunsmall) / sizeof(int)));
      CloudyAnime();
      }
      else
      {
      ClearNightAnime(moonsmall,(sizeof(moonsmall) / sizeof(int)));
      CloudyAnime();
      }
      if (schimbare == 0)
      {
      //Serial.println("reached clouds");
      displaytext(9 ,c ,Matrix);
      displaytext(13,l ,Matrix);
      displaytext(17,o ,Matrix);
      displaytext(21,u ,Matrix);
      displaytext(25,d ,Matrix);
      displaytext(29,s ,Matrix);
      schimbare = 1;
      }
      }
      if (wDe.equalsIgnoreCase("Mist"))
    {
      MistAnime();
      if (schimbare == 0)
      {
      //displaytext(9 ,s ,Matrix);
      displaytext(12,mm1 ,Matrix);
      displaytext(15,mm2 ,Matrix);
      displaytext(19,i ,Matrix);
      displaytext(23,s ,Matrix);
      displaytext(27,t ,Matrix);
      //leds[14] = CRGB(200,200,255);
      //FastLED.show();
      schimbare = 1; 
      }
      }
      if (wDe.equalsIgnoreCase("Fog"))
    {
      MistAnime();
      if (schimbare == 0)
      {
      //displaytext(9 ,s ,Matrix);
      //displaytext(9 ,c ,Matrix);
      displaytext(13,f ,Matrix);
      displaytext(17,o ,Matrix);
      displaytext(21,g ,Matrix);
      //displaytext(25,d ,Matrix);
      //displaytext(29,s ,Matrix);
      leds[103] = CRGB(200,200,255);
      leds[104] = CRGB(200,200,255);
      FastLED.show(); 
      schimbare = 1;
      }
      }
    //function ends
  }

void resetTimeDisplay()
{
    FastLED.clear();
    temph1=11;
    temph2=11;
    tempm1=11;
    tempm2=11;
    temps1=11;
    temps2=11;
    tempdat=33;
    tempmon=13;
    tempyr=0;
/*
    displaytext(3,N0,Matrix);
    displaytext(7,N0,Matrix);
    displaytext(13,N0,Matrix);
    displaytext(17,N0,Matrix);
    displaytext(23,N0,Matrix);
    displaytext(27,N0,Matrix);
*/
  } 

void mode1()
{
  if (screenid==1)
{  if (displayflag!=1)
  {    displayflag=1;
        FastLED.clear();
        resetTimeDisplay(); }
     timeformat2();}
if (screenid==2)
{  if (displayflag!=2)
  {    displayflag=2;
    FastLED.clear();
    tempTEMP=0.0;  }
    showtemperature(Temperature);}
if (screenid==3)
{ if (displayflag!=3)
  {    displayflag=3;
    FastLED.clear();  }
showweather(weatherDescription);}
  }

void mode2()
{
  if (screenid==1)
{  if (displayflag!=1)
  {    displayflag=1;
        FastLED.clear();
        resetTimeDisplay(); }
   if ((s1+s2) == 0)
   {
   for (int i=170 ; i<239; i++)
   {
   leds[i] = CRGB(0,0,0);
   }
    temps1=11;
    temps2=11;     
   }  
   if (m2%2 == 0) 
      timeformat2();
   else
      timeformat1();
   }
if (screenid==2)
{  if (displayflag!=2)
  {    displayflag=2;
    FastLED.clear();
    tempTEMP=0.0;  }
    showtemperature(Temperature);}
if (screenid==3)
{  if (displayflag!=3)
  {    displayflag=3;
    FastLED.clear();
    tempHUMY=0.0;  }
    showhumidity(Humidity);}
if (screenid==4)
{ if (displayflag!=4)
  {    displayflag=4;
    FastLED.clear();  }
showweather(weatherDescription);}
  }

void Soare()
{
   // Calculate the times of sunrise, transit, and sunset, in hours (UTC)
  calcSunriseSunset(yr, mon, dat, latitude, longitude, transit, sunrise, sunset);

r1 = int(round((sunrise + timezone + dst) * 60));
hr1 = (r1 / 60) % 24;
mn1 = r1 % 60;

r2 = int(round((sunset + timezone + dst) * 60));
hr2 = (r2 / 60) % 24;
mn2 = r2 % 60;

  Serial.print("Sunrise = ");
  Serial.print(sunrise+60*(timezone+dst));
  Serial.print(" = ");
  Serial.print(hr1);
  Serial.print(":");
  Serial.print(mn1);
  Serial.print(" !  / ");
  Serial.print("Sunset = ");
  Serial.print(sunset+60*(timezone+dst));
  Serial.print(" = ");
  Serial.print(hr2);
  Serial.print(":");
  Serial.print(mn2);
  Serial.print(" ! ");
}

void night() { 
  ora1 = 100*hr1 + mn1;  // rasarit 
  ora2 = 100*hr2 + mn2;  // apus
  oraactuala = 1000*h1 + 100*h2 + 10*m1 + m2;  // ora actuala

  Serial.print(ora1);
  Serial.print(" ? ");
  Serial.print(oraactuala);
  Serial.print(" ? ");
  Serial.print(ora2);  

  if ((oraactuala > ora2) or (oraactuala < ora1))  // night 
  {
    noapte = 1;
    intensitate = intensitate0;
  }
  else
  {
    noapte = 0;
    intensitate = intensitate1;
  }
  if (intensitate3 != intensitate)
  {
    temph1=11;
    temph2=11;
    tempm1=11;
    tempm2=11;
    temps1=11;
    temps2=11;
    intensitate3 = intensitate;;  
  }
    FastLED.setBrightness(intensitate);
    FastLED.show();
  if (noapte == 0) 
  Serial.println(" -> daytime ! ");
  else
  Serial.println(" -> nighttime ! ");
}


void showhumidity(float humy)
{
 AnimeHygro (humy,secondreference);
 if (schimbare == 0)  
  {
  if(tempHUMY != humy)
  {
  tempHUMY = humy;
  int bd = (int) humy; 
  if (bd/10 == 0) 
    cleardigit(12,Matrix);
  else
    updatedigit(12,bd/10);
  updatedigit(16,bd%10);
//  Serial.println(bd);
//  Serial.println(bd%10);
//  Serial.println(adtemp);

  //to display percent symbol + RH
  leds[161]   = CRGB(200,200,235);
  leds[164]   = CRGB(200,200,235);
  leds[172]   = CRGB(200,200,235);
  leds[178]   = CRGB(200,200,235);
  leds[181]   = CRGB(200,200,235);     
  //  displaytext(9 ,s ,Matrix);
    displaytext(24,r ,Matrix);
    displaytext(28,h ,Matrix);
  }
  schimbare = 1;
  }
  //Function End
  }

  void AnimeHygro(float humy, int sec)
{
  leds[3  ]   = CRGB(200,200,235);
  leds[4  ]   = CRGB(200,200,235);
  leds[5  ]   = CRGB(200,200,235);
  leds[6  ]   = CRGB(200,200,235);
  leds[8  ]   = CRGB(200,200,235);
  leds[13 ]   = CRGB(200,200,235);
  leds[17 ]   = CRGB(200,200,235);
  leds[18 ]   = CRGB(200,200,235);
  leds[23 ]   = CRGB(200,200,235);
  leds[24 ]   = CRGB(200,200,235);
  leds[27 ]   = CRGB(200,200,235);
  leds[28 ]   = CRGB(200,200,235);
  leds[37 ]   = CRGB(200,200,255);
  leds[38 ]   = CRGB(200,200,255);  
  leds[60 ]   = CRGB(200,200,255);  
  leds[62 ]   = CRGB(200,200,255);
  leds[65 ]   = CRGB(200,200,255);
  leds[67 ]   = CRGB(200,200,255);
  leds[69 ]   = CRGB(200,200,255);

if (humy<30)
{ leds[9  ]   = CRGB(0,0,255);
  leds[22 ]   = CRGB(0,0,255);
  leds[25 ]   = CRGB(0,0,255);
  leds[10 ]   = CRGB(0,0,0);
  leds[21 ]   = CRGB(0,0,0);
  leds[26 ]   = CRGB(0,0,0);
  leds[11 ]   = CRGB(0,0,0);
  leds[20 ]   = CRGB(0,0,0);
  leds[12 ]   = CRGB(0,0,0);
  leds[19 ]   = CRGB(0,0,0);
  }
else if (humy>=30 && humy <50)
{ leds[9  ]   = CRGB(0,0,255);
  leds[22 ]   = CRGB(0,0,255);
  leds[25 ]   = CRGB(0,0,255);
  leds[10 ]   = CRGB(0,2555,0);
  leds[21 ]   = CRGB(0,255,0);
  leds[26 ]   = CRGB(0,255,0);
  leds[11 ]   = CRGB(0,0,0);
  leds[20 ]   = CRGB(0,0,0);
  leds[12 ]   = CRGB(0,0,0);
  leds[19 ]   = CRGB(0,0,0);
  }
else if (humy>=50 && humy <75)
{ leds[9  ]   = CRGB(0,0,255);
  leds[22 ]   = CRGB(0,0,255);
  leds[25 ]   = CRGB(0,0,255);
  leds[10 ]   = CRGB(0,255,0);
  leds[21 ]   = CRGB(0,255,0);
  leds[26 ]   = CRGB(0,255,0);
  leds[11 ]   = CRGB(255,255,0);
  leds[20 ]   = CRGB(255,255,0);
  leds[12 ]   = CRGB(0,0,0);
  leds[19 ]   = CRGB(0,0,0);
  }
else if (humy>=75 )
{ leds[9  ]   = CRGB(0,0,255);
  leds[22 ]   = CRGB(0,0,255);
  leds[25 ]   = CRGB(0,0,255);
  leds[10 ]   = CRGB(0,255,0);
  leds[21 ]   = CRGB(0,255,0);
  leds[26 ]   = CRGB(0,255,0);
  leds[11 ]   = CRGB(255,255,0);
  leds[20 ]   = CRGB(255,255,0);
  leds[12 ]   = CRGB(255,0,0);
  leds[19 ]   = CRGB(255,0,0);
  }

// sign animated
if (sec%2 == 0)
{ leds[58 ]   = CRGB(200,200,255);
  leds[51 ]   = CRGB(200,200,255);
  leds[49 ]   = CRGB(0,0,0);
} 
else
{ leds[58 ]   = CRGB(0,0,0);
  leds[51 ]   = CRGB(0,0,0);
  leds[49 ]   = CRGB(200,200,255);;
} 
 FastLED.show(); 
  } 
