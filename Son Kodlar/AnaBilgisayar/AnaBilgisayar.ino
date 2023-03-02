 
#include "Arduino.h"
#include "LoRa_E32.h" 

LoRa_E32 e32ttl(&Serial3); //mega 14 15


#include<Wire.h> 
// LPS25HB I2C address is 0x5C(92)
#define Addr 0x5C

//Adafruit_BNO055
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h> 
#include <Adafruit_MPL115A2.h>

Adafruit_BNO055 bno = Adafruit_BNO055(55); 
Adafruit_MPL115A2 mpl115a2; 

byte packageNumber = 0;
float  curPressure =0;  //KPA  
 
#define veriSayisi 5
 
 
float total= -1; 
float avarage = -1 ;  
float patlamaNok = -10;
sensors_event_t event; 

void setup()
{ 
  Serial.begin(9600); 
  delay(100);
  if(!bno.begin()) { 
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!"); 
  } if (! mpl115a2.begin()) {
    Serial.println("Sensor not found! Check wiring"); 
  }
  bno.setExtCrystalUse(true);
  e32ttl.begin();  
  delay(100); 
 

  for (int i =0; i<veriSayisi;i++) {  
    bno.getEvent(&event);  
    delay(100);  
    total += (float)event.orientation.y;
  }
  avarage = total / veriSayisi ; 
  patlamaNok=avarage-45;
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
bool patla = true;
void loop()
{  
  packageNumber++;  

  curPressure = mpl115a2.getPressure();  //KPA   

  bno.getEvent(&event);  
  total += (event.orientation.y - avarage); 
  avarage = total / veriSayisi ;
  
 // Serial.println();
 // Serial.print ("Avarage : " );Serial.print(avarage); Serial.print (" Y : "); Serial.print ((float)event.orientation.y);

  message.packageNum =  packageNumber;
  message.explode  = !patla;
  *(float*)(message.pressure) =   curPressure;
  *(float*)(message.X) =  event.orientation.x; 
  *(float*)(message.Y) =  event.orientation.y;
  *(float*)(message.Z) =  event.orientation.z; 
  *(float*)(message.GPSe) =  0; 
  *(float*)(message.GPSb) =  0;

  delay(100);
  
  ResponseStatus rs = e32ttl.sendFixedMessage(0,4,6,&message, sizeof(Message));
  Serial.println(rs.getResponseDescription());
  packageNumber = (packageNumber==255) ? 0 : packageNumber; //short if    


  if (patla & (avarage < patlamaNok)) {
    Serial.println("PATLADI");
    patla = false;
  }
}
  
