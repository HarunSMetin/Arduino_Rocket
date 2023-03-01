/*
 * LoRa E32-TTL-100
 * Send fixed transmission structured message to a specified point.
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

// ---------- esp8266 pins --------------
//LoRa_E32 e32ttl(D2, D3, D5, D7, D6);
//LoRa_E32 e32ttl(D2, D3, D5, D7, D6); // Config without connect AUX and M0 M1
#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); // Arduino RX <-- e32 TX, Arduino TX --> e32 RX
LoRa_E32 e32ttl(&mySerial);

//#include <SoftwareSerial.h>
//SoftwareSerial mySerial(D2, D3); // Arduino RX <-- e32 TX, Arduino TX --> e32 RX
//LoRa_E32 e32ttl(&mySerial, D5, D7, D6);
// -------------------------------------

// ---------- Arduino pins --------------
//LoRa_E32 e32ttl(2, 3, 5, 7, 6);
//LoRa_E32 e32ttl(2, 3); // Config without connect AUX and M0 M1

//#include <SoftwareSerial.h>
//SoftwareSerial mySerial(2, 3); // Arduino RX <-- e32 TX, Arduino TX --> e32 RX
//LoRa_E32 e32ttl(&mySerial, 5, 7, 6);
// -------------------------------------

// ---------------- STM32 --------------------
// HardwareSerial Serial2(USART2);   // PA3  (RX)  PA2  (TX)
// LoRa_E32 e32ttl(&Serial2, PA0, PB0, PB10); //  RX AUX M0 M1
// -------------------------------------------------

// ---------- Raspberry PI Pico pins --------------
// LoRa_E32 e32ttl100(&Serial2, 2, 10, 11); //  RX AUX M0 M1
// -------------------------------------

void printParameters(struct Configuration configuration);
void printModuleInformation(struct ModuleInformation moduleInformation);
//The setup function is called once at startup of the sketch
void setup()
{
	Serial.begin(9600);
	while (!Serial) {
	    ; // wait for serial port to connect. Needed for native USB
    }
	delay(100);

	e32ttl.begin();

	// ---------------------------
}
struct Message {
    char type[5];
    char message[8];
    int temperature;
} message;

int i = 0;
// The loop function is called in an endless loop
void loop()
{
	delay(2500);
	i++;
	struct Message {
	    char type[5] = "TEMP";
	    char message[8] = "Kitchen";
	    byte temperature[4];
	} message;

	*(float*)(message.temperature) = 19.2;

	ResponseStatus rs = e32ttl.sendFixedMessage(0,2,6,&message, sizeof(Message));
	Serial.println(rs.getResponseDescription());
}
