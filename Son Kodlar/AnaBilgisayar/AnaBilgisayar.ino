
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

#define VERI_SAYISI 5 
#define BASINC_OFFSET 10

float totalY = -1; 
float avarageY = -1 ;  
float patlamaYDegeri  = -1;


float totalBasinc = -1; 
float avarageBasinc = -1 ;   
float ilkBasincDegeri = -1 ;   

bool patla = true;

void setup()
{ 
  Serial.begin(9600);  

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
} message;  


void loop()
{  
  packageNumber++;  

  curPressure = mpl115a2.getPressure();  //KPA   

  bno.getEvent(&event);
  delay(100);  

  message.packageNum =  packageNumber;
  message.explode  = (!patla)?(byte)1:(byte)0;
  *(float*)(message.pressure) =   curPressure;
  *(float*)(message.X) =  event.orientation.x; 
  *(float*)(message.Y) =  event.orientation.y;
  *(float*)(message.Z) =  event.orientation.z; 
  *(float*)(message.GPSe) =  0; //gelecek
  *(float*)(message.GPSb) =  0;//gelecek 

  delay(20);
  
  ResponseStatus rs = e32ttl.sendFixedMessage(0,4,6,&message, sizeof(Message));
  Serial.println(rs.getResponseDescription());

  packageNumber = (packageNumber==255) ? 0 : packageNumber; //short if     

  if(curPressure < (ilkBasincDegeri - BASINC_OFFSET) ){ 
    totalY += ( event.orientation.y- avarageY); 
    avarageY = totalY / VERI_SAYISI ; 
    if ( (curPressure > avarageBasinc) & patla & (avarageY < patlamaYDegeri )) {
      Serial.println("PATLADI");
      patla = false;
      delay(100);
    } 
    totalBasinc += (curPressure - avarageBasinc); 
    avarageBasinc = totalBasinc / VERI_SAYISI ; 
  }

    Serial.print("PAKET NUMARASI: ");
    Serial.println((byte)message.packageNum); 
    Serial.print("Patladı mı : ");
    Serial.println((byte)message.explode);
    Serial.print("Basınç: "); 
    Serial.println(*(float*)(message.pressure));
    Serial.print("X: "); 
    Serial.print (*(float*)(message.X));  
    Serial.print(" Y: "); 
    Serial.print (*(float*)(message.Y));  
    Serial.print(" Z: "); 
    Serial.println (*(float*)(message.Z));  
    Serial.print(" GPS Enlem: "); 
    Serial.println (*(float*)(message.GPSe)); 
    Serial.print(" GPS Boylam: "); 
    Serial.println (*(float*)(message.GPSb));  
    Serial.print("**** Y Avarage : ");Serial.println(avarageY);  
    Serial.println ("----------------------------------------------------------------------------------------------");

  delay(100);
}
