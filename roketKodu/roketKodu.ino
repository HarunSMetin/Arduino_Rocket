 
#include "Arduino.h"
#include "LoRa_E32.h"

#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); // Arduino RX <-- e32 TX, Arduino TX --> e32 RX
LoRa_E32 e32ttl(&mySerial);


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

void setup()
{
  mySerial.begin(9600);
  delay(100);
  if(!bno.begin()) { 
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!"); 
  } if (! mpl115a2.begin()) {
    Serial.println("Sensor not found! Check wiring"); 
  }
  bno.setExtCrystalUse(true);
  e32ttl.begin();  
  delay(100);
} 

byte i = 0;
float  curPressure =0;  //KPA  

struct Message {  
      byte packageNum ;
      byte pressure[4] ;
      byte X [4];
      byte Y [4];
      byte Z [4]; 
      byte GPSe[4]; 
      byte GPSb[4];
} message; 
void loop()
{ 
  curPressure = mpl115a2.getPressure();  //KPA 
  /*Serial.print("Pressure is : ");
  Serial.print(curPressure);
  Serial.println(" kPa");   */ 
  sensors_event_t event; 
  bno.getEvent(&event);
  
  /* Display the floating point data  
  Serial.print("Orientation: ");
  Serial.print("\tX: ");
  Serial.print(event.orientation.x, 4);
  Serial.print(" Y: ");
  Serial.print(event.orientation.y, 4);
  Serial.print(" Z: ");
  Serial.print(event.orientation.z, 4); */
  i++; 
  
  message.packageNum =  i;
  *(float*)(message.pressure) =  curPressure;
  *(float*)(message.X) =  event.orientation.x;
  *(float*)(message.Y) =  event.orientation.y;
  *(float*)(message.Z) =  event.orientation.z; 
  *(float*)(message.GPSe) =  event.orientation.z; 
  *(float*)(message.GPSb) =  event.orientation.z;

  delay(300);
  ResponseStatus rs = e32ttl.sendFixedMessage(0,3,6,&message, sizeof(Message));
  Serial.println(rs.getResponseDescription());
  if(i==255)i=0;
}
  
