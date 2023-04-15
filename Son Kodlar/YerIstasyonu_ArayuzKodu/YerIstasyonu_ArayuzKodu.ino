
#include "Arduino.h"
#include "LoRa_E32.h"   

LoRa_E32 e32ttl(&Serial3);  //mega 14 15 
LoRa_E32 e32ttl_yuk(&Serial2);  //mega 16 17 
void setup()
{
  Serial.begin(9600); 
  delay(200); 
  e32ttl.begin();
  delay(200);  
  e32ttl_yuk.begin(); 
  delay(200);

}  
//static byte VERISAYISI = 22;  

struct Message {   
      byte packageNum ;
      byte explode1 ; 
      byte explode2 ;
      byte Irtifa_basinc[4] ;
      byte Irtifa_GPS[4] ;
      byte pressure[4] ;
      byte X_Jiro [4];
      byte Y_Jiro [4];
      byte Z_Jiro [4]; 
      byte X_Ivme [4];
      byte Y_Ivme [4];
      byte Z_Ivme [4];  
      byte Aci [4];
      byte GPSe[4]; 
      byte GPSb[4];  
}message ;   


struct MessageYuk {  
      byte packageNum; 
      byte Irtifa_GPS[4]; 
      byte temperature[4]; 
      byte humadity[4];
      byte pressure[4];
      byte GPSe[4]; 
      byte GPSb[4];  
} messageYuk; 

ResponseStructContainer rsc ;  
 ResponseStructContainer rsc2 ; 
void loop()
{
  delay(10); 
  if( e32ttl.available()  > 1 ||e32ttl_yuk.available()  > 1  ){
    if (e32ttl.available()  > 1 ){ 
        rsc = e32ttl.receiveMessage(sizeof(Message)); 
        message = *(Message*) rsc.data;
      }
     if (e32ttl_yuk.available()  > 1 ){ 
        rsc2 = e32ttl_yuk.receiveMessage(sizeof(MessageYuk)); 
        messageYuk = *(MessageYuk*) rsc2.data;  
    } 
      Serial.print((byte)message.packageNum ); 
      Serial.print(",");
      Serial.print((byte) message.explode1 );
      Serial.print(",");
      Serial.print((byte)message.explode2   ,6);  
      Serial.print(",");
      Serial.print(*(float*)(message.Irtifa_basinc),6);
      Serial.print(",");
      Serial.print(*(float*)(message.Irtifa_GPS   ),6);
      Serial.print(",");
      Serial.print(*(float*)(message.pressure     ),6);
      Serial.print(",");
      Serial.print(*(float*)(message.X_Jiro       ),6);
      Serial.print(",");
      Serial.print(*(float*)(message.Y_Jiro       ),6);
      Serial.print(",");
      Serial.print(*(float*)(message.Z_Jiro       ),6); 
      Serial.print(",");
      Serial.print(*(float*)(message.X_Ivme       ),6);
      Serial.print(",");
      Serial.print(*(float*)(message.Y_Ivme       ),6);
      Serial.print(",");
      Serial.print(*(float*)(message.Z_Ivme       ),6);
      Serial.print(",");
      Serial.print(*(float*)(message.Aci),6); 
      Serial.print(",");
      Serial.print(*(float*)(message.GPSe),6); 
      Serial.print(",");
      Serial.print(*(float*)(message.GPSb),6); 
      Serial.print(",");
      Serial.print((byte)(messageYuk.packageNum) ); 
      Serial.print(",");
      Serial.print(*(float*)(messageYuk.Irtifa_GPS),6); 
      Serial.print(",");
      Serial.print(*(float*)(messageYuk.temperature),6); 
      Serial.print(",");
      Serial.print(*(float*)(messageYuk.humadity),6); 
      Serial.print(",");
      Serial.print(*(float*)(messageYuk.pressure),6); 
      Serial.print(",");
      Serial.print(*(float*)(messageYuk.GPSe),6); 
      Serial.print(",");
      Serial.print(*(float*)(messageYuk.GPSb) );    
      Serial.println();
      }
  }

