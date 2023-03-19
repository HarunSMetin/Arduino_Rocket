 
#include "Arduino.h"
#include "LoRa_E32.h"  

LoRa_E32 e32ttl(&Serial3);  //mega 14 15 

void setup()
{
	Serial.begin(9600);
	while (!Serial) {;}
	delay(100);

	e32ttl.begin();
  
	Serial.println("Start listening!"); 
	delay(1000);
}  


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


	ResponseStructContainer rsc ; 
void loop()
{
  delay(10);
	if (e32ttl.available()  > 1){ 
    rsc = e32ttl.receiveMessage(sizeof(Message)); 
    message = *(Message*) rsc.data;  
   
    Serial.print("PAKET NUMARASI: "); Serial.println((byte)message.packageNum);  
    Serial.print("1. PATLAMA DURUMU: "); Serial.println((byte)message.explode1);      
    Serial.print("2. PATLAMA DURUMU: ");  Serial.println((byte)message.explode2); 
    Serial.print("Basinç: "); 
    Serial.println(*(float*)(message.pressure));
    Serial.print("X_Jiro: "); 
    Serial.print (*(float*)(message.X_Jiro),6);  
    Serial.print("\t Y_Jiro: "); 
    Serial.print (*(float*)(message.Y_Jiro),6);  
    Serial.print("\t Z_Jiro: "); 
    Serial.println (*(float*)(message.Z_Jiro),6);   
    Serial.print("X_İvme: "); 
    Serial.print (*(float*)(message.X_Ivme),6);  
    Serial.print("\t Y_İvme: "); 
    Serial.print (*(float*)(message.Y_Ivme),6);  
    Serial.print("\t Z_İvme: "); 
    Serial.println (*(float*)(message.Z_Ivme),6);  
    Serial.print("GPS Enlem: "); 
    Serial.print (*(float*)(message.GPSe),6); 
    Serial.print("\t GPS Boylam: "); 
    Serial.println(*(float*)(message.GPSb),6); 
    Serial.println ("------------------------------------------------");
 
     
	}
}
