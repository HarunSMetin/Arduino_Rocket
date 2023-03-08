#include <TinyGPSPlus.h>
#include "Arduino.h"
#include "LoRa_E32.h" 

LoRa_E32 e32ttl(&Serial3); //mega 14 15
//for arduino nano 10 , 11 
/*
#include "SoftwareSerial.h"
SoftwareSerial mySerial(10,11);
LoRa_E32 e32ttl(&mySerial); 
*/
#include<Wire.h> 
 
//Adafruit_BNO055
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h> 
#include <Adafruit_MPL115A2.h>

Adafruit_BNO055 bno = Adafruit_BNO055(55); 
sensors_event_t event; 

Adafruit_MPL115A2 mpl115a2; 
float  curPressure =0;  //KPA  


byte packageNumber = 0; 
//GPS İÇİN
static const int RXPin = 16, TXPin = 17;
static const uint32_t GPSBaud = 9600;
TinyGPSPlus gps;
void setup()
{ 
  
  Serial.begin(9600);   
  Serial2.begin(GPSBaud);
  delay(100);

  if(!bno.begin())  Serial.println("Sensor BNO055 NOT detected!");   
  if (! mpl115a2.begin()) Serial.println("Sensor mpl115a2 not detected!"); 
  
  bno.setExtCrystalUse(true);
  e32ttl.begin();  

  delay(100);  
}  

struct Message {  
      byte packageNum ;
      byte explode1 ; 
      byte explode2 ;
      byte pressure[4] ;
      byte X [4];
      byte Y [4];
      byte Z [4]; 
      byte GPSe[4]; 
      byte GPSb[4]; 
      char time[32];
} message;

char currentTime[32];
void loop()
{    
  packageNumber++;  


  bno.getEvent(&event);
  delay(100);  
  curPressure = mpl115a2.getPressure();  //KPA   

  message.packageNum =  packageNumber;
  message.explode1  = 0; 
  message.explode2  = 0;
  *(float*)(message.pressure) =   curPressure;  //kilo pascal cinsinden
  *(float*)(message.X) =  event.orientation.x; 
  *(float*)(message.Y) =  event.orientation.y;
  *(float*)(message.Z) =  event.orientation.z; 
  *(float*)(message.GPSe) = gps.location.isValid() ? gps.location.lat() : 0;  
  *(float*)(message.GPSb) = gps.location.isValid() ? gps.location.lng() : 0;  
  sprintf(currentTime, "%02d:%02d:%02d ",  gps.time.hour()+3,  gps.time.minute(),  gps.time.second());
  *(char*)(message.time)=currentTime;
 
  delay(20);
  
  e32ttl.sendFixedMessage(0,4,6,&message, sizeof(Message));

  packageNumber = (packageNumber==255) ? 0 : packageNumber; //short if     

    Serial.print("PAKET NUMARASI: "); Serial.println((byte)message.packageNum);   
    Serial.print("1. PATLAMA DURUMU: "); Serial.println(message.explode1); 
    Serial.print("2. PATLAMA DURUMU: "); Serial.println( message.explode2);

    Serial.print("\t Basınç: "); 
    Serial.print(*(float*)(message.pressure));
    Serial.print("\t X: "); 
    Serial.print (*(float*)(message.X));  
    Serial.print(" Y: "); 
    Serial.print (*(float*)(message.Y));  
    Serial.print(" Z: "); 
    Serial.print (*(float*)(message.Z));  
    Serial.print("\t GPS Enlem: "); 
    Serial.print (*(float*)(message.GPSe),6); 
    Serial.print("\t GPS Boylam: "); 
    Serial.print (*(float*)(message.GPSb),6);     
    Serial.print("\t GPS Saat: ");   
    Serial.println(*(char*)message.time);   

    Serial.print("\t GPS Tarih: ");
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d ", gps.date.month(), gps.date.day(), gps.date.year());
    Serial.print(sz);    

    Serial.println ("\n----------------------------------------------------------------------------------------------");
 
   smartDelay(100);
}

// This custom version of delay() ensures that the gps object
// is being "fed".
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (Serial2.available())
      gps.encode(Serial2.read());
  } while (millis() - start < ms);
}
