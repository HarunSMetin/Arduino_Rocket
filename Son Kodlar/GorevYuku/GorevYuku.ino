
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
static const uint32_t GPSBaud =115200;
TinyGPSPlus gps;
static const int MAX_SATELLITES = 40; 
TinyGPSCustom totalGPGSVMessages(gps, "GPGSV", 1); // $GPGSV sentence, first element
TinyGPSCustom messageNumber(gps, "GPGSV", 2);      // $GPGSV sentence, second element
TinyGPSCustom satsInView(gps, "GPGSV", 3);         // $GPGSV sentence, third element
TinyGPSCustom satNumber[4]; // to be initialized later
TinyGPSCustom elevation[4];
TinyGPSCustom azimuth[4];
TinyGPSCustom snr[4];
struct
{
  bool active;
  int elevation;
  int azimuth;
  int snr;
} sats[MAX_SATELLITES]; 

//******************************************************
//BUZZER
//              Trigger
//Arduino Mega	10 
#define BUZZER 10

#define SATSEARCHTIME 5 //5dk
//******************************************************

byte packageNumber = 0;  

float  curPressure =0;  //HPA  
 
String SerialString=""; 

struct MessageYuk {   
      char type[5] = "YUK";
      byte packageNum; 
      byte Irtifa_GPS[4]; 
      byte temperature[4]; 
      byte humadity[4];
      byte pressure[4];
      byte GPSe[4]; 
      byte GPSb[4];  
      byte GPSSatcont;  
} message_yuk;   

void Beep(int ms=400, int count = 1){
  int i =0;
  for (i=0;i<count;i++){ 
    delay(ms);
    digitalWrite(BUZZER, HIGH);
    delay(ms);
    digitalWrite(BUZZER, LOW);
  }
}
void SearchSatalite(){ 
  Serial.print("Uydu aranıyor...");
  uint32_t timer = millis();
  while (gps.satellites.value() < 8 && ( (millis() - timer) < (60000*SATSEARCHTIME)  ) )
  { 
    if (Serial2.available() > 0)
    {
      gps.encode(Serial2.read());
      if (totalGPGSVMessages.isUpdated())
      {
        for (int i=0; i<4; ++i)
        {
          int no = atoi(satNumber[i].value());
          // Serial.print(F("SatNumber is ")); Serial.println(no);
          if (no >= 1 && no <= MAX_SATELLITES)
          {
            sats[no-1].elevation = atoi(elevation[i].value());
            sats[no-1].azimuth = atoi(azimuth[i].value());
            sats[no-1].snr = atoi(snr[i].value());
            sats[no-1].active = true;
          }
        }
        
        int totalMessages = atoi(totalGPGSVMessages.value());
        int currentMessage = atoi(messageNumber.value());
        if (totalMessages == currentMessage)
        { 
          Serial.print(F("Sats= ")); 
          Serial.print(gps.satellites.value() );
          Serial.print(F(" Nums="));
          for (int i=0; i<MAX_SATELLITES; ++i)
            if (sats[i].active)
            {
              Serial.print(i+1);
              Serial.print(F(" "));
            }
          Serial.print(F(" Elevation="));
          for (int i=0; i<MAX_SATELLITES; ++i)
            if (sats[i].active)
            {
              Serial.print(sats[i].elevation);
              Serial.print(F(" "));
            }
          Serial.print(F(" Azimuth="));
          for (int i=0; i<MAX_SATELLITES; ++i)
            if (sats[i].active)
            {
              Serial.print(sats[i].azimuth);
              Serial.print(F(" "));
            }
          
          Serial.print(F(" SNR="));
          for (int i=0; i<MAX_SATELLITES; ++i)
            if (sats[i].active)
            {
              Serial.print(sats[i].snr);
              Serial.print(F(" "));
            }
          Serial.println();

          for (int i=0; i<MAX_SATELLITES; ++i)
            sats[i].active = false;
        }
      }
    }
  }
  Beep(1000);
}

void setup()
{ 
  digitalWrite(BUZZER, LOW);
  Serial.begin(19200);   
  Serial2.begin(GPSBaud);  
  for (int i=0; i<4; ++i)
  {
    satNumber[i].begin(gps, "GPGSV", 4 + 4 * i); // offsets 4, 8, 12, 16
    elevation[i].begin(gps, "GPGSV", 5 + 4 * i); // offsets 5, 9, 13, 17
    azimuth[i].begin(  gps, "GPGSV", 6 + 4 * i); // offsets 6, 10, 14, 18
    snr[i].begin(      gps, "GPGSV", 7 + 4 * i); // offsets 7, 11, 15, 19
  }
  delay(100);
  
  if (!bme.begin())
    Serial.println("Sensor BME680 NOT detected!"); 
  if(!SD.begin(SD_KART_CS_PIN))
    Serial.println("SD Card Module NOT detected!");

  myFile = SD.open("verilerg.txt", FILE_WRITE); 
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
 //SearchSatalite();
}  

 
void loop()
{  
    SerialString=""; 
  packageNumber++;  
 
  myFile = SD.open("verilerg.txt",  FILE_WRITE  ); 
  smartDelay(100);  
  curPressure =bme.readPressure()/100;   
  message_yuk.packageNum =  packageNumber; 
  *(float*)(message_yuk.Irtifa_GPS) =  gps.altitude.meters() ;  
  *(float*)(message_yuk.temperature) =   (float)bme.temperature;  //Hekto pascal cinsinden
  *(float*)(message_yuk.humadity) = (float)bme.humidity; 
  *(float*)(message_yuk.pressure) =  (float)curPressure; 
  *(float*)(message_yuk.GPSe) = (float)gps.location.lat() ;   
  *(float*)(message_yuk.GPSb) =(float)gps.location.lng() ;   
  message_yuk.GPSSatcont = (byte)gps.satellites.value() ;
       
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

     SerialString = SerialString+(byte)(message_yuk.packageNum)+","+*(float*)(message_yuk.Irtifa_GPS)+","+
      *(float*)(message_yuk.temperature)+","+ *(float*)(message_yuk.humadity)+","+ *(float*)(message_yuk.pressure)+","+
       *(float*)(message_yuk.GPSe)+","+*(float*)(message_yuk.GPSb)+";"; 
       Serial.println(SerialString);
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
