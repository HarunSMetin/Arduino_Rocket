 
#include "Arduino.h"
#include "LoRa_E32.h"  

LoRa_E32 e32ttl(&Serial3);  //mega 14 15 

void setup()
{
	Serial.begin(9600);
	while (!Serial) {; }
	delay(100);

	e32ttl.begin();
  
	Serial.println("Start listening!"); 
	delay(1000);,
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
    Serial.println ("----------------------------------------------------------------------------------------------");
 
     
	}
}
