 
#include "Arduino.h"
#include "LoRa_E32.h"   

LoRa_E32 e32ttl(&Serial3);  //mega 14 15 
void setup()
{
  Serial.begin(9600); 
  delay(100); 
  e32ttl.begin();  
  delay(100);

}  
static byte VERISAYISI = 12; 
struct Message {  
      byte packageNum ;
      byte explode1 ; 
      byte explode2 ;
      byte pressure[4] ;
      byte X_Jiro [4];
      byte Y_Jiro [4];
      byte Z_Jiro [4]; 
      byte X_Ivme [4];
      byte Y_Ivme [4];
      byte Z_Ivme [4]; 
      byte GPSe[4]; 
      byte GPSb[4];  
} message;   

/*
  SERIAL'A BASILAN      =====>  "packageNum,explode1,explode2,pressure,X_Jiro,Y_Jiro,Z_Jiro,X_Ivme,Y_Ivme ,Z_Ivme,GPSe ,GPSb"
  BYTE BOYUTU (39Byte)  =====>     1BYTE   ,  1BYTE ,  1BYTE , 4BYTE  ,4BYTE ,4BYTE ,4BYTE ,4BYTE , 4BYTE ,4BYTE ,4BYTE,4BYTE  
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
    Serial.print (*(float*)(message.X_Jiro),6);  
    Serial.print(","); 
    Serial.print (*(float*)(message.Y_Jiro),6);  
    Serial.print(","); 
    Serial.print (*(float*)(message.Z_Jiro),6);   
    Serial.print(","); 
    Serial.print (*(float*)(message.X_Ivme),6);  
    Serial.print(","); 
    Serial.print (*(float*)(message.Y_Ivme),6);  
    Serial.print(","); 
    Serial.print (*(float*)(message.Z_Ivme),6);  
    Serial.print(","); 
    Serial.print (*(float*)(message.GPSe),6); 
    Serial.print(","); 
    Serial.print (*(float*)(message.GPSb),6);      
    Serial.println("");
  }/*
  else 
  { 
    for(int i = 0 ; i < VERISAYISI ; i++)
    {
      Serial.print("0");
      if(i < VERISAYISI-1) 
        Serial.print(","); 
    }
  }*/
}
