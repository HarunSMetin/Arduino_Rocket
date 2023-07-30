
#include "Arduino.h"
#include "LoRa_E32.h"   

LoRa_E32 e32ttl(&Serial3,UART_BPS_RATE_19200);  //mega 14 15 
LoRa_E32 e32ttl_yuk(&Serial2,UART_BPS_RATE_19200);  //mega 16 17 

void setup()
{
  Serial.begin(19200); 
  delay(200); 
  e32ttl.begin();
  delay(200); 
  e32ttl_yuk.begin(); 
  delay(200);  

}   

struct Message {     
      byte packageNum ;
      byte status; // exp1, exp2, gpsA , BME, BNO ,SdCard
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
      byte GPSSatcont;
} message;     
struct MessageYuk {    
      byte status; // exp1, exp2, gpsA , BME, BNO ,SdCard
      byte packageNum; 
      byte Irtifa_GPS[4]; 
      byte temperature[4]; 
      byte humadity[4];
      byte pressure[4];
      byte GPSe[4]; 
      byte GPSb[4];  
      byte GPSSatcont;  
} message_yuk;    

char type[5];
int AnaAvailable = 0;
int YukAvailable = 0;
void loop()
{ 

  ResponseContainer       rs1 ;    
  ResponseContainer       rs2 ;   
  AnaAvailable=e32ttl.available() ;
  YukAvailable=e32ttl_yuk.available();
  if(AnaAvailable > 1 || YukAvailable > 1  )
  {  
      if (AnaAvailable > 1 ){   
		    rs2 = e32ttl.receiveInitialMessage(sizeof(type));  
        if (rs2.data=="ANA"){ 
          ResponseStructContainer rsc2 = e32ttl.receiveMessage(sizeof(Message)); 
          message = *(Message*) rsc2.data; 
		    	rsc2.close(); 
        }
      }  
     if (YukAvailable > 1 ){ 		 
		    rs1 = e32ttl_yuk.receiveInitialMessage(sizeof(type));  
        if (rs1.data =="YUK"){
          ResponseStructContainer rsc = e32ttl_yuk.receiveMessage(sizeof(MessageYuk)); 
          message_yuk = *(MessageYuk*) rsc.data;   
			    rsc.close();
        } 
      } 
      Serial.print((byte)message.packageNum ); 
      Serial.print(",");
      Serial.print(message.status ); 
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
      Serial.print(*(float*)(message.Aci          ),6); 
      Serial.print(",");
      Serial.print(*(float*)(message.GPSe         ),6); 
      Serial.print(",");
      Serial.print(*(float*)(message.GPSb         ),6); 
      Serial.print(",");
      Serial.print(message.GPSSatcont);  
      Serial.print(",");
      Serial.print(message_yuk.status );   
      Serial.print(",");
      Serial.print((byte)(message_yuk.packageNum) ); 
      Serial.print(",");
      Serial.print(*(float*)(message_yuk.Irtifa_GPS ) ,6); 
      Serial.print(",");
      Serial.print(*(float*)(message_yuk.temperature) ,6); 
      Serial.print(",");
      Serial.print(*(float*)(message_yuk.humadity   ) ,6); 
      Serial.print(",");
      Serial.print(*(float*)(message_yuk.pressure   ),6); 
      Serial.print(",");
      Serial.print(*(float*)(message_yuk.GPSe       ),6 ); 
      Serial.print(",");
      Serial.print(*(float*)(message_yuk.GPSb       ),6 );    
      Serial.print(",");
      Serial.print(message_yuk.GPSSatcont);    
      Serial.println();
  } 
  delay(50); 
}

