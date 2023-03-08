
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
Adafruit_BNO055 bno = Adafruit_BNO055(55); 
sensors_event_t event; 

//Basınç

#include <Adafruit_MPL115A2.h> 
Adafruit_MPL115A2 mpl115a2; 
float  curPressure =0;  //KPA  

 //GPS İÇİN
static const int RXPin = 16, TXPin = 17;
static const uint32_t GPSBaud = 9600;
TinyGPSPlus gps;

byte packageNumber = 0; 

#define VERI_SAYISI 5
#define BASINC_OFFSET 0.3

float totalY = 0 ; 
float avarageY = 0 ;  
float patlamaYDegeri  = 0;


float totalBasinc = 0; 
float avarageBasinc = 91 ;   
float ilkBasincDegeri = 91;   

bool patla = true;
char currentTime[32];


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
  
  for (int i =0; i < VERI_SAYISI; i++) {  
    bno.getEvent(&event);   
    delay(100); 
    totalBasinc += mpl115a2.getPressure();   
    totalY += (float)event.orientation.y;
  }  
  avarageY = totalY / VERI_SAYISI ; 
  avarageBasinc = totalBasinc / VERI_SAYISI;  

  patlamaYDegeri = avarageY - 45;  //DERECEYE BAK

  ilkBasincDegeri = avarageBasinc;
}  

struct Message {  
      byte packageNum ;
      byte explode ;
      byte pressure[4] ;
      byte X [4];
      byte Y [4];
      byte Z [4]; 
      byte GPSe[4]; 
      byte GPSb[4]; 
      char time[32];
} message;   

bool kalkti = false;
void loop()
{  
  packageNumber++;  
 
  bno.getEvent(&event);
  smartDelay(100);  
  curPressure =mpl115a2.getPressure();
  message.packageNum =  packageNumber;
  message.explode  = (!patla)?(byte)1:(byte)0;
  *(float*)(message.pressure) =   curPressure;  //kilo pascal cinsinden
  *(float*)(message.X) =  event.orientation.x; 
  *(float*)(message.Y) =  event.orientation.y;
  *(float*)(message.Z) =  event.orientation.z; 
  *(float*)(message.GPSe) = gps.location.isValid() ? gps.location.lat() : 0;  
  *(float*)(message.GPSb) = gps.location.isValid() ? gps.location.lng() : 0;  
  sprintf(currentTime, "%02d:%02d:%02d ",  gps.time.hour()+3,  gps.time.minute(),  gps.time.second());
  *(char*)(message.time))=currentTime;


  ResponseStatus rs = e32ttl.sendFixedMessage(0,4,6,&message, sizeof(Message));
  Serial.println(rs.getResponseDescription());

  packageNumber = (packageNumber==255) ? 0 : packageNumber; //short if     

  if(!kalkti & curPressure < (ilkBasincDegeri - BASINC_OFFSET) ){ kalkti = true;}
  if(kalkti){
    totalY += ( event.orientation.y- avarageY); 
    avarageY = totalY / VERI_SAYISI ; 
    totalBasinc += (curPressure- avarageBasinc); 
    avarageBasinc = totalBasinc / VERI_SAYISI ; 

    if ( ( curPressure > avarageBasinc - BASINC_OFFSET) & patla & (avarageY < patlamaYDegeri )) {
      Serial.println("PATLADI");
      patla = false; 
      delay(1000);
    } 
  } 
  
    Serial.print("PAKET NUMARASI: "); Serial.println((byte)message.packageNum);  
    Serial.print("PATLAMA DURUMU: "); Serial.println(!patla);  
    Serial.print("KALKIŞ DURUMU: "); Serial.println(kalkti ? "Kalış Yapıldı!":"Rampada duruyor!"); 

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
    Serial.println ("----------------------------------------------------------------------------------------------");
 
}

static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (Serial2.available())
      gps.encode(Serial2.read());
  } while (millis() - start < ms);
}
