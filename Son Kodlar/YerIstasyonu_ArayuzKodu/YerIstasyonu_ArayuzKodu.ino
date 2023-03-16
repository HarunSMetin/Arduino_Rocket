 
#include "Arduino.h"
#include "LoRa_E32.h"  

#define VERISAYISI=10; 

LoRa_E32 e32ttl(&Serial3);  //mega 14 15 
void setup()
{
  Serial.begin(9600); 
  delay(100); 
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
} message;

/*
  SERIAL'A BASILAN =====>  "packageNum;explode1;explode2;pressure;X;Y;Z;GPSe;GPSb"
  BYTE BOYUTU      =====>     1BYTE   ;  1BYTE ;  1BYTE ; 4BYTE  ;4;4;4; 4  ;  4   
*/
ResponseStructContainer rsc ; 
void loop()
{
  delay(10);
  if (e32ttl.available()  > 1){ 
    rsc = e32ttl.receiveMessage(sizeof(Message)); 
    message = *(Message*) rsc.data; 

    Serial.print((byte)message.packageNum);  
    Serial.print(","); 
    Serial.print((byte)message.explode1);      
    Serial.print(",");  
    Serial.print((byte)message.explode2); 
    Serial.print(","); 
    Serial.print(*(float*)(message.pressure));
    Serial.print(","); 
    Serial.print (*(float*)(message.X),6);  
    Serial.print(","); 
    Serial.print (*(float*)(message.Y),6);  
    Serial.print(","); 
    Serial.print (*(float*)(message.Z),6);
    Serial.print(","); 
    Serial.print (*(float*)(message.GPSe),6); 
    Serial.print(","); 
    Serial.print (*(float*)(message.GPSb),6);      

  }
  else 
  { 
    for(int i = 0 ; i<VERISAYISI;i++)
    {
      Serial.print("0");
      if(i<VERISAYISI-1) 
        Serial.print(","); 
    }
  }
  Serial.println("");
}
