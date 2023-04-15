
#include "Arduino.h"
#define VERI_SAYISI 3

//******************************************************
//Pinler
//******************************************************
//Lora e32 Modülü   Serial3
//e32 nin pini      RX	TX   M1   M2
//Arduino Mega      14  15  GND   GND 
#include "LoRa_E32.h" 
LoRa_E32 e32ttl(&Serial3); 

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
static const uint32_t GPSBaud = 9600;
TinyGPSPlus gps;

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

byte packageNumber = 0; 

float totalY = 0; 
float avarageY = 0;   
 
float totalBasinc = 0; 
float avarageBasinc = 91 ;   
float ilkBasincDegeri = 91;    
float ilkAci = 85;  

float  curPressure =0;  //HPA   

float baslanicYukseklik = 0;

bool patla1 = true;
bool patla2 = true;

bool kalkti = false;

 float yerdenYukseklik =0;
 float hamYukseklik = 905;
String SerialString="";

imu::Vector<3> acc;

void Beep(int ms=400){
  delay(ms);
  digitalWrite(BUZZER, HIGH);
  delay(ms);
  digitalWrite(BUZZER, LOW);
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
   Serial.begin(9600);
  Serial2.begin(GPSBaud);
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
    totalBasinc += (float)bme.readPressure()/100;   
    totalY += (float)event.gyro.z;
    baslanicYukseklik+=bme.readAltitude(SEALEVELPRESSURE_HPA); ;
  }  
  avarageY = totalY / VERI_SAYISI ; 
  avarageBasinc = totalBasinc / VERI_SAYISI;    
  baslanicYukseklik /=VERI_SAYISI;
  ilkAci =avarageY;
  ilkBasincDegeri = avarageBasinc;
  
  if (myFile) { 
     myFile.println("PakatNumarasi,1Patlama,2Patlama,Irtifa_basinc,Irtifa_GPS,Basinc,X_Jiro,Y_Jiro,Z_Jiro,X_Ivme,Y_Ivme,Z_Ivme,ACI_Y,GPSEnlem,GPSBoylam;"); 
     myFile.close();
     Serial.println(ilkAci);
     Beep();
     Beep();
  }
}  
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
} message;   
void loop(){  
  packageNumber++;  
 
  myFile = SD.open("verilera.txt", FILE_WRITE);
  bno.getEvent(&event);
  smartDelay(100);  
  curPressure =bme.readPressure()/100; 
  acc=bno.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER );

  hamYukseklik = bme.readAltitude(SEALEVELPRESSURE_HPA)  ;
  yerdenYukseklik = hamYukseklik - baslanicYukseklik;
//////////////////////////////////Message Package Declaration ///////////////////////////////////////

  message.packageNum =  packageNumber;
  message.explode1  = (byte)(!patla1);
  message.explode2  = (byte)(!patla2);
  *(float*)(message.Irtifa_basinc) =  hamYukseklik ; //Metre
  *(float*)(message.Irtifa_GPS) =  gps.altitude.meters(); //metre 
  *(float*)(message.pressure) =   curPressure;          //HPA Hekto pascal cinsinden
  //1 rad/s = 57.2957795 deg/s
  *(float*)(message.X_Jiro) =   (event.gyro.x ); //deg/s
  *(float*)(message.Y_Jiro) =   (event.gyro.z) ; //deg/s
  *(float*)(message.Z_Jiro) =   (event.gyro.y); //deg/s 
  *(float*)(message.X_Ivme) =  acc.x();                 // g
  *(float*)(message.Y_Ivme) =  acc.z ();                 // g
  *(float*)(message.Z_Ivme) =  acc.y();                  // g  
  *(float*)(message.Aci) =   event.orientation.z;       //degree
  *(float*)(message.GPSe) =  gps.location.lat() ;  
  *(float*)(message.GPSb) =  gps.location.lng() ;   

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
  if(!kalkti && ( yerdenYukseklik> 0.4) )  { 
    kalkti = true; 
    Beep(100);
  }
  if(kalkti) {
    totalY += ( event.gyro.z- avarageY); 
    avarageY = totalY / VERI_SAYISI ; 
    totalBasinc += (curPressure- avarageBasinc); 
    avarageBasinc = totalBasinc / VERI_SAYISI ; 
Serial.println(avarageY);
    if (  patla1 && (avarageY > ilkAci+80 )) {
      patla1 = false;  
      Beep(200);
      Beep(200);  
      digitalWrite(ROLE1, HIGH); 
      digitalWrite(ROLE1_KONTROL, HIGH); 
      delay(2000);
      digitalWrite(ROLE1, LOW); 
      digitalWrite(ROLE1_KONTROL, LOW); 
    }  
    if(!patla1){ 
      if(!patla2){
        //HER ŞEY BİTTİ BURDA 2. PATLAMA OLDU VE AŞAĞI SÜZÜLÜYOR
        while(true)
          Beep(1000); 
      }
      if(patla2 && yerdenYukseklik<=0.1){//TODO : değişcek
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
  SerialString = SerialString +"PAKET NUMARASI: "+(byte)(message.packageNum)+"Yerden yukseklik: "+yerdenYukseklik+"\n1. PATLAMA DURUMU: "+ (byte)!patla1 +"\n2. PATLAMA DURUMU: "+ (byte)!patla2+"\nIrtifa(Basinc): "+ *(float*)(message.Irtifa_basinc)+"\tIrtifa(GPS): "+*(float*)(message.Irtifa_GPS)+"\tBasinc: "+
      *(float*)(message.pressure)+"\nX_Jiro: "  + *(float*)(message.X_Jiro)+"\t Y_Jiro: "  + *(float*)(message.Y_Jiro)+"\t Z_Jiro: "+
      *(float*)(message.Z_Jiro)  +
      *(float*)(message.Z_Ivme) + "\n\n";   
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
