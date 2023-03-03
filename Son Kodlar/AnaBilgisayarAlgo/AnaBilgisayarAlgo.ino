
#include "Arduino.h"  
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
 
#define VERI_SAYISI 3
#define BASINC_OFFSET 0.3

float totalY = 0 ; 
float avarageY =85 ;  
float patlamaYDegeri  =85;


float totalBasinc = 0; 
float avarageBasinc = 91 ;   
float ilkBasincDegeri = 91;   

bool patla = true;
#define MAVI 46
#define YESIL 47
#define ROLE 26

void setup()
{  
   digitalWrite(ROLE,HIGH);
  Serial.begin(9600);  

  delay(100);

  if(!bno.begin())  Serial.println("Sensor BNO055 NOT detected!");   
  if (! mpl115a2.begin()) Serial.println("Sensor mpl115a2 not detected!"); 
  
  bno.setExtCrystalUse(true);delay(1000);   
  bno.getEvent(&event);delay(1000); 
  totalY =  event.orientation.y;
  
  for (int i =0; i < VERI_SAYISI; i++) {  
    totalBasinc += mpl115a2.getPressure();    
  }   
  avarageBasinc = totalBasinc / VERI_SAYISI;  
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
   
bool kalkti = false; 

void loop()
{    

  curPressure = mpl115a2.getPressure();  //KPA   

  bno.getEvent(&event);
  delay(100);   
  
  message.explode  = (!patla)?(byte)1:(byte)0;
  *(float*)(message.pressure) =   curPressure;
  *(float*)(message.X) =  event.orientation.x; 
  *(float*)(message.Y) =  event.orientation.y;
  *(float*)(message.Z) =  event.orientation.z; 
  *(float*)(message.GPSe) =  0; //gelecek
  *(float*)(message.GPSb) =  0;//gelecek 

  delay(20); 
 

  if(!kalkti & curPressure < (ilkBasincDegeri - BASINC_OFFSET) ){digitalWrite(MAVI,HIGH); kalkti = true;}
   totalY += ( event.orientation.y - avarageY); 
    avarageY = totalY / VERI_SAYISI ; 
    totalBasinc += (curPressure - avarageBasinc); 
    avarageBasinc = totalBasinc / VERI_SAYISI ;  
  if(kalkti){ 
    if (  patla & (avarageY < 0 )) { 
      Serial.println("PATLADI"); 
      digitalWrite(YESIL,HIGH);
      digitalWrite(ROLE,LOW);
      patla = false;
      delay(10000);
    } 
  }     

    Serial.print("Y: ");
    Serial.println(*(float*)message.Y); 
    Serial.print("Basınç: "); 
    Serial.println(*(float*)(message.pressure));
    Serial.println ("----------------------------------------------------------------------------------------------");

  delay(300);
}
