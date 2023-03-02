 
#include "Arduino.h"
#include "LoRa_E32.h"  

LoRa_E32 e32ttl(&Serial3);  //mega 14 15
 
void setup()
{
	Serial.begin(9600);
	while (!Serial) {Serial.println("Serial is not open") ;   }
	delay(100);

	e32ttl.begin();
  
	Serial.println("Start listening!");
   
	delay(100);

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

	ResponseStructContainer rsc ; 
void loop()
{
	if (e32ttl.available()  > 1){ 
    rsc = e32ttl.receiveMessage(sizeof(Message)); 
    message = *(Message*) rsc.data; 
    Serial.print("packageNum: ");
    Serial.println(  (byte)message.packageNum); 
    Serial.print("Patladı mı : ");
    Serial.println(  (byte)message.explode);
    Serial.print("Pressure: "); 
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
	}
}
