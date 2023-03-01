/*
 * LoRa E32-TTL-100
 * Receive fixed transmission message on channel.
 * https://www.mischianti.org/2019/12/03/lora-e32-device-for-arduino-esp32-or-esp8266-power-saving-and-sending-structured-data-part-5/
 *
 * E32-TTL-100----- Arduino UNO or esp8266
 * M0         ----- 3.3v (To config) GND (To send) 7 (To dinamically manage)
 * M1         ----- 3.3v (To config) GND (To send) 6 (To dinamically manage)
 * TX         ----- RX PIN 2 (PullUP)
 * RX         ----- TX PIN 3 (PullUP & Voltage divider)
 * AUX        ----- Not connected (5 if you connect)
 * VCC        ----- 3.3v/5v
 * GND        ----- GND
 *
 */
#include "Arduino.h"
#include "LoRa_E32.h" 

#include <SoftwareSerial.h>
 SoftwareSerial mySerial(10, 11); // Arduino RX <-- e32 TX, Arduino TX --> e32 RX
LoRa_E32 e32ttl(&mySerial); 
 
void setup()
{
	Serial.begin(9600);
	while (!Serial) {
	    ;  
    }
	delay(100);

	e32ttl.begin();
 
	Serial.println();
	Serial.println("Start listening!");
   

} 
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
	if (e32ttl.available()  > 1){
		ResponseStructContainer rsc = e32ttl.receiveMessage(sizeof(Message));
		struct Message message = *(Message*) rsc.data;
    
    Serial.print("packageNum: ");
    Serial.println(  (byte)message.packageNum);
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
