
#include "Arduino.h"
#define VERI_SAYISI 3

//******************************************************
//Pinler
//******************************************************
//Lora e32 Modülü   Serial3
//e32 nin pini      RX	TX   M1   M2
//Arduino Mega      14  15  GND   GND 
#include "LoRa_E32.h" 
LoRa_E32 e32ttl(&Serial3,UART_BPS_RATE_19200); 

//******************************************************
//SD KART Modülü
//	            MOSI	MİSO	SCK	CS
//Arduino Mega	51	  50	  52	43   
// CS pini değiştirilebilir. İstedğimiz herhangi bir dijital pin olur
#include <SPI.h>
#include <SD.h>
#define SD_KART_CS_PIN 43
File myFile;

//******************************************************
//Adafruit_BNO055  I2C
//	            SDA	SCL	
//Arduino Mega	20  21
#include  <Adafruit_Sensor.h>
#include  <Adafruit_BNO055.h>
#include  <utility/imumaths.h> 
#include  <Wire.h> 
Adafruit_BNO055 bno = Adafruit_BNO055(55); 
sensors_event_t event; 

//******************************************************
//Basınç  BME680_I2C 
//	            SDA	SCL	
//Arduino Mega	20  21
#include "Adafruit_BME680.h"
#define SEALEVELPRESSURE_HPA (1013.25) //Deniz seviyesindeki basınç değeri 
Adafruit_BME680 bme; //0x77 I2C address
 
//******************************************************
//GPS MODULU
//             Serial2
// GPS'in Pini  RX	TX
//Arduino Mega	16  17
#include <TinyGPSPlus.h>
static const int RXPin = 16, TXPin = 17;
static const uint32_t GPSBaud = 115200;

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
//ROLE 1 
//              Trigger
//Arduino Mega	22
#define ROLE1 22 

//ROLE1_KONTROL 
//              Trigger
//Arduino Mega	23
#define ROLE1_KONTROL 23

//******************************************************
//ROLE 2
//              Trigger
//Arduino Mega	24
#define ROLE2 24

//ROLE2_KONTROL
//              Trigger
//Arduino Mega	25
#define ROLE2_KONTROL 25 
//******************************************************
//BUZZER
//              Trigger
//Arduino Mega	10 
#define BUZZER 10
//******************PARAMETRELER****************************
#define SATSEARCHTIME 5 //5dk 
#define KALKISYUKSEKLIGI 2 // 2 METRE
#define IKINICIPATLAMAYUKSEKLIGI 500 // 500 METRE
//******************************************************

byte packageNumber = 0;  

float totalY = 0; 
float avarageY = 0;   
 
float totalBasinc = 0; 
float avarageBasinc = 91 ;   
float ilkBasincDegeri = 91;    
float ilkAci = 85;  

float  curPressure =0;  //HPA   

float baslangicYukseklik = 0;

bool patla1 = true;
bool patla2 = true;

bool kalkti = false;

float yerdenYukseklik =0;
float hamYukseklik = 905;
String SerialString="";

imu::Vector<3> acc;

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
      byte GPSSatcont;
} message;  
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
          
          Beep(50);
        }
      }
    }
  }
  Beep(1000);
}
void setup()
{ 
  pinMode(ROLE1, OUTPUT);
  pinMode(ROLE1_KONTROL, OUTPUT);
  pinMode(ROLE2, OUTPUT);
  pinMode(ROLE2_KONTROL, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  digitalWrite(ROLE1, LOW);
  digitalWrite(ROLE2, LOW); 
  digitalWrite(ROLE1_KONTROL, LOW);
  digitalWrite(ROLE2_KONTROL, LOW);
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

  if(!bno.begin())
    Serial.println("Sensor BNO055 NOT detected!"); 
  if (!bme.begin())
    Serial.println("Sensor BME680 NOT detected!"); 
  if(!SD.begin(SD_KART_CS_PIN))
    Serial.println("SD Card Module NOT detected!");


  myFile = SD.open("verilera.txt", FILE_WRITE);
  delay(200);
  bno.setExtCrystalUse(true); 
  delay(200);  
  e32ttl.begin();     
  delay(200);
 
  bme.setPressureOversampling(BME680_OS_4X);

  acc=bno.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER );

  for (int i =0; i < VERI_SAYISI; i++) {  
    bno.getEvent(&event);   
    delay(200); 
    totalBasinc += (float)bme.readPressure()/100 ;   
    totalY += (float)event.gyro.y;
    baslangicYukseklik+=bme.readAltitude(SEALEVELPRESSURE_HPA); ;
  }  
  avarageY = totalY / VERI_SAYISI ; 
  avarageBasinc = totalBasinc / VERI_SAYISI;    
  baslangicYukseklik /=VERI_SAYISI;
  ilkAci =avarageY;
  ilkBasincDegeri = avarageBasinc;
  
  if (myFile) { 
     myFile.println("PakatNumarasi,1Patlama,2Patlama,Irtifa_basinc,Irtifa_GPS,Basinc,X_Jiro,Y_Jiro,Z_Jiro,X_Ivme,Y_Ivme,Z_Ivme,ACI_Y,GPSEnlem,GPSBoylam;"); 
     myFile.close();
     Serial.println(ilkAci);
     Beep(200,2);
  }
   SearchSatalite();  
}   
void loop(){  
  packageNumber++;  
 
  myFile = SD.open("verilera.txt", FILE_WRITE);
  bno.getEvent(&event);
  smartDelay(100);  
  curPressure =bme.readPressure()/100; 
  acc=bno.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER );

  hamYukseklik = bme.readAltitude(SEALEVELPRESSURE_HPA)  ;
  yerdenYukseklik = hamYukseklik - baslangicYukseklik;
//////////////////////////////////Message Package Declaration ///////////////////////////////////////

  message.packageNum =  packageNumber;
  message.explode1  = (byte)(!patla1);
  message.explode2  = (byte)(!patla2);
  *(float*)(message.Irtifa_basinc) =  yerdenYukseklik ; //Metre
  *(float*)(message.Irtifa_GPS) =  gps.altitude.meters(); //metre 
  *(float*)(message.pressure) =   curPressure;          //HPA Hekto pascal cinsinden
  //1 rad/s = 57.2957795 deg/s  
  *(float*)(message.X_Jiro) =   event.gyro.x; //deg
  *(float*)(message.Y_Jiro) =   event.gyro.y ; //deg
  *(float*)(message.Z_Jiro) =   event.gyro.z; //deg
  *(float*)(message.X_Ivme) =  acc.x();                 // g
  *(float*)(message.Y_Ivme) =  acc.z ();                 // g
  *(float*)(message.Z_Ivme) =  acc.y();                  // g  
  *(float*)(message.Aci) =   90 - event.orientation.y;       //degree
  *(float*)(message.GPSe) =  gps.location.lat() ;  
  *(float*)(message.GPSb) =  gps.location.lng() ;  
  message.GPSSatcont = (byte)gps.satellites.value() ; 

 //************//

  e32ttl.sendFixedMessage(0,4,6,&message, sizeof(Message));
  packageNumber = (packageNumber==255) ? 0 : packageNumber;
//Tuz gölü rakımı yaklaşık 905 m yani 2969,16 feet 
//14616 feete çıkılacak 
//https://www.google.com/url?sa=i&rct=j&q=&esrc=s&source=web&cd=&cad=rja&uact=8&ved=0CAQQw7AJahcKEwiw3uys4JH-AhUAAAAAHQAAAAAQBw&url=https%3A%2F%2Fwww.ngdc.noaa.gov%2Fstp%2Fspace-weather%2Fonline-publications%2Fmiscellaneous%2Fus-standard-atmosphere-1976%2Fus-standard-atmosphere_st76-1562_noaa.pdf&psig=AOvVaw34JdSmuRhWO3bbcUJQLBqC&ust=1680750139172882
//NOAA
//911.48- 908.13 hpa (Tuz gölü basıncı yaklaşık bu olmalı),
//581.33 hpa -579.00 hpa (tepede olacak yaklasik basinc)
//////////////////////////////////Algorithm///////////////////////////////////////
  if(!kalkti && ( ( yerdenYukseklik > KALKISYUKSEKLIGI) || ((*(float*)message.X_Jiro) != 0 && (*(float*)message.X_Jiro)  < (ilkAci - 10)) ) )  
  { 
    kalkti = true; 
    Beep(10,4);
  }
  if(kalkti) {
    totalY += ( event.gyro.y - avarageY); 
    avarageY = totalY / VERI_SAYISI ; 
    totalBasinc += (curPressure- avarageBasinc); 
    avarageBasinc = totalBasinc / VERI_SAYISI ; 
    if (  patla1 && (avarageY < 0 )) { // TODO : basincla alakali kontrol
      patla1 = false;  
      Beep(100,2);
      digitalWrite(ROLE1, HIGH); 
      digitalWrite(ROLE1_KONTROL, HIGH); 
      delay(2000);
      digitalWrite(ROLE1, LOW); 
      digitalWrite(ROLE1_KONTROL, LOW); 
    }  
    if(!patla1){ 
      if(!patla2){
        //HER ŞEY BİTTİ BURDA 2. PATLAMA OLDU VE AŞAĞI SÜZÜLÜYOR
        Beep(200); 
      }
      if(patla2 && yerdenYukseklik<=IKINICIPATLAMAYUKSEKLIGI){ //TODO : değişcek
        patla2 = false; 
          Beep(300); 
          Beep(300); 
          Beep(300); 
        digitalWrite(ROLE2, HIGH); 
        digitalWrite(ROLE2_KONTROL, HIGH); 
        delay(2000);
        digitalWrite(ROLE2, LOW); 
        digitalWrite(ROLE2_KONTROL, LOW); 
      } 
    }
  }

  
//////////////////////////////////Serial Visual ///////////////////////////////////////
  SerialString = SerialString +"PAKET NUMARASI: "+(byte)(message.packageNum)+"\nYerden yukseklik: "+yerdenYukseklik+"\n1. PATLAMA DURUMU: "+ (byte)!patla1 +"\n2. PATLAMA DURUMU: "+ (byte)!patla2+"\nIrtifa(Basinc): "+ *(float*)(message.Irtifa_basinc)+"\tIrtifa(GPS): "+*(float*)(message.Irtifa_GPS)+"\tBasinc: "+
      *(float*)(message.pressure)+"\nX_Jiro: "  + *(float*)(message.X_Jiro)+"\t Y_Jiro: "  + *(float*)(message.Y_Jiro)+"\t Z_Jiro: "+
      *(float*)(message.Z_Jiro)  +"\n X_Ivme: "+
      *(float*)(message.X_Ivme) +"\t Y_Ivme: "+
      *(float*)(message.Y_Ivme) +"\t Z_Ivme: "+
      *(float*)(message.Z_Ivme) + "\n";   
    Serial.println(SerialString);
    SerialString="";
    
//////////////////////////////////File Write ///////////////////////////////////////
 if (myFile) {
      myFile.print((byte)message.packageNum);  
      myFile.print(","); 
      myFile.print((byte)message.explode1);      
      myFile.print(",");  
      myFile.print((byte)message.explode2); 
      myFile.print(","); 
      myFile.print(*(float*)(message.Irtifa_basinc));
      myFile.print(","); 
      myFile.print(*(float*)(message.Irtifa_GPS));
      myFile.print(","); 
      myFile.print(*(float*)(message.pressure));
      myFile.print(","); 
      myFile.print (*(float*)(message.X_Jiro),6);  
      myFile.print(","); 
      myFile.print (*(float*)(message.Y_Jiro),6);  
      myFile.print(","); 
      myFile.print (*(float*)(message.Z_Jiro),6); 
      myFile.print(","); 
      myFile.print (*(float*)(message.X_Ivme),6);  
      myFile.print(","); 
      myFile.print (*(float*)(message.Y_Ivme),6);  
      myFile.print(","); 
      myFile.print (*(float*)(message.Z_Ivme),6);
      myFile.print(","); 
      myFile.print (*(float*)(message.Aci),6);
      myFile.print(","); 
      myFile.print (*(float*)(message.GPSe),6); 
      myFile.print(","); 
      myFile.print (*(float*)(message.GPSb),6);      
      myFile.println(";");  

      myFile.close();
    }
}

static void smartDelay(unsigned long ms) {
  unsigned long start = millis();
  do 
  {
    while (Serial2.available())
      gps.encode(Serial2.read());
  } while (millis() - start < ms);
}
