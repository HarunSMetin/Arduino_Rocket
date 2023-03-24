
#include "Arduino.h"
#define VERI_SAYISI 5 

//******************************************************
//Pinler
//******************************************************
//Lora e32 Modülü   Serial3
//e32 nin pini      RX  TX   M1   M2
//Arduino Mega      14  15  GND   GND 
#include "LoRa_E32.h" 
LoRa_E32 e32ttl(&Serial3); 

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

float totalBasinc = 0; 
float avarageBasinc = 91 ;   
float ilkBasincDegeri = 91;   

float  curPressure =0;  //HPA  

float BASINC_OFFSET = 0.2;

void setup()
{ 
  Serial.begin(9600);   
  Serial2.begin(GPSBaud);
  delay(100);
  
  if (!bme.begin())
    Serial.println("Sensor BME680 NOT detected!"); 
  if(!SD.begin(SD_KART_CS_PIN))
    Serial.println("SD Card Module NOT detected!");

  myFile = SD.open("Veriler_GorevYuku.txt", FILE_WRITE); 
  delay(200);
  e32ttl.begin();     
  delay(200);
 
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  
  for (int i =0; i < VERI_SAYISI; i++) {  
    delay(100); 
    totalBasinc += bme.readPressure();   
  }  
  avarageBasinc = totalBasinc / VERI_SAYISI;   
  ilkBasincDegeri = avarageBasinc;
  BASINC_OFFSET = ilkBasincDegeri/300;
  if (myFile) { 
     myFile.println("PakatNumarasi,1Patlama,2Patlama,Basinc,X_Jiro,Y_Jiro,Z_Jiro,GPSEnlem,GPSBoylam;"); 
      myFile.close();
  }
}  

struct Message_yuk {  
      byte packageNum ; 
      byte temperature[4]; 
      byte humadity[4];
      byte pressure[4] ;
      byte GPSe[4]; 
      byte GPSb[4];  
} message_yuk; 
 
void loop()
{  
  packageNumber++;  
 
  myFile = SD.open("Veriler_GorevYuku.txt", FILE_WRITE); 
  smartDelay(100);  
  curPressure =bme.readPressure();  
  
  message_yuk.packageNum =  packageNumber; 
  *(float*)(message_yuk.temperature) =   bme.temperature;  //Hekto pascal cinsinden
  *(float*)(message_yuk.humadity) =  bme.humidity; 
  *(float*)(message_yuk.pressure) =  curPressure; 
  *(float*)(message_yuk.GPSe) = gps.location.isValid() ? gps.location.lat() : 0;  
  *(float*)(message_yuk.GPSb) = gps.location.isValid() ? gps.location.lng() : 0;  


  ResponseStatus rs = e32ttl.sendFixedMessage(0,3,6,&message_yuk, sizeof(Message_yuk));
  Serial.println(rs.getResponseDescription());

  packageNumber = (packageNumber==255) ? 0 : packageNumber;
   
    totalBasinc += (curPressure- avarageBasinc); 
    avarageBasinc = totalBasinc / VERI_SAYISI ; 
  
    Serial.print("PAKET NUMARASI: "); Serial.println((byte)message_yuk.packageNum);   
    Serial.print("Sicaklik: "); 
    Serial.println(*(float*)(message_yuk.temperature));
    Serial.print("Nem: "); 
    Serial.println (*(float*)(message_yuk.humadity),6);  
    Serial.print("Basinç: "); 
    Serial.println (*(float*)(message_yuk.pressure),6);    
    Serial.print("GPS Enlem: "); 
    Serial.print (*(float*)(message_yuk.GPSe),6); 
    Serial.print("\t GPS Boylam: "); 
    Serial.println(*(float*)(message_yuk.GPSb),6); 
    /*    
        Serial.println(); 
        Serial.println ("***************************************************************");
        Serial.print("calibrated altitude(m) :");
        Serial.println(bme.readAltitude(SEALEVELPRESSURE_HPA)); //çok yavaş //kapat
   */
    Serial.println ("------------------------------------------------------------------");
   if (myFile) {
          myFile.print((byte)message_yuk.packageNum);  
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
