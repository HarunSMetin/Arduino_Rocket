
#include "Arduino.h"
#define VERI_SAYISI 10 

//******************************************************
//Pinler
//******************************************************
//Lora e32 Modülü   Serial3
//e32 nin pini      RX  TX   M1   M2
//Arduino Mega      14  15  GND   GND 
#include "LoRa_E32.h" 
LoRa_E32 e32ttl(&Serial3,UART_BPS_RATE_19200); 

//******************************************************
//SD KART Modülü
//              MOSI  MİSO  SCK CS
//Arduino Mega  51    50    52  43   
// CS pini değiştirilebilir. İstedğimiz herhangi bir dijital pin olur
#include <SPI.h>
#include <SD.h>
#define SD_KART_CS_PIN 43
File myFile;

//******************************************************
//Basınç  BME680_I2C 
//              SDA SCL 
//Arduino Mega  20  21
#include "Adafruit_BME680.h"
#define SEALEVELPRESSURE_HPA  1013.25 //Deniz seviyesindeki basınç değeri 
Adafruit_BME680 bme; //0x77 I2C address
//******************************************************
//GPS MODULU
//             Serial2
// GPS'in Pini  RX  TX
//Arduino Mega  16  17
#include <TinyGPSPlus.h>
static const int RXPin = 16, TXPin = 17;
static const uint32_t GPSBaud = 9600;
TinyGPSPlus gps;
 
byte packageNumber = 0;  

float  curPressure =0;  //HPA  
 
String SerialString=""; 

void setup()
{ 
  Serial.begin(19200);   
  Serial2.begin(GPSBaud);
  delay(100);
  
  if (!bme.begin())
    Serial.println("Sensor BME680 NOT detected!"); 
  if(!SD.begin(SD_KART_CS_PIN))
    Serial.println("SD Card Module NOT detected!");

  myFile = SD.open("verilerg.txt", FILE_WRITE); 
  delay(200);
  delay(200);
  e32ttl.begin();     
  delay(200);
 
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X); 
  if (myFile) {  
     myFile.println("PakatNumarasi,Irtifa_GPS,Sicaklik,Nem,Basinc,GPSEnlem,GPSBoylam;"); 
     myFile.close();
  }
}  

struct MessageYuk {  
      byte packageNum; 
      byte Irtifa_GPS[4]; 
      byte temperature[4]; 
      byte humadity[4];
      byte pressure[4];
      byte GPSe[4]; 
      byte GPSb[4];  
} message_yuk; 
 
void loop()
{  
    SerialString=""; 
  packageNumber++;  
 
  myFile = SD.open("verilerg.txt",  FILE_WRITE  ); 
  smartDelay(100);  
  curPressure =bme.readPressure();  
  
  message_yuk.packageNum =  packageNumber; 
  *(float*)(message_yuk.Irtifa_GPS) =  gps.altitude.isValid() ? gps.altitude.meters() : 0; 
  *(float*)(message_yuk.temperature) =   bme.temperature;  //Hekto pascal cinsinden
  *(float*)(message_yuk.humadity) =  bme.humidity; 
  *(float*)(message_yuk.pressure) =  curPressure; 
  *(float*)(message_yuk.GPSe) = gps.location.isValid() ? gps.location.lat() : 0;  
  *(float*)(message_yuk.GPSb) = gps.location.isValid() ? gps.location.lng() : 0;  



    
     SerialString = SerialString+(byte)(message_yuk.packageNum)+","+*(float*)(message_yuk.Irtifa_GPS)+","+
      *(float*)(message_yuk.temperature)+","+ *(float*)(message_yuk.humadity)+","+ *(float*)(message_yuk.pressure)+","+
       *(float*)(message_yuk.GPSe)+","+*(float*)(message_yuk.GPSb)+";"; 
       Serial.println(SerialString);
       
  e32ttl.sendFixedMessage (0,3,7, &message_yuk  , sizeof(MessageYuk )); 
  packageNumber = (packageNumber==255) ? 0 : packageNumber; 
    if (myFile){
          myFile.print((byte)message_yuk.packageNum);  
          myFile.print(",");  
          myFile.print(*(float*)message_yuk.Irtifa_GPS);  
          myFile.print(",");  
          myFile.print(*(float*)(message_yuk.temperature),6);
          myFile.print(","); 
          myFile.print (*(float*)(message_yuk.humadity),6);  
          myFile.print(","); 
          myFile.print (*(float*)(message_yuk.pressure),6);   
          myFile.print(","); 
          myFile.print (*(float*)(message_yuk.GPSe),6); 
          myFile.print(","); 
          myFile.print (*(float*)(message_yuk.GPSb),6);     
          myFile.println("");
      myFile.close();  
    }
}

static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (Serial2.available())
      gps.encode(Serial2.read());
  } while (millis() - start < ms);
}
