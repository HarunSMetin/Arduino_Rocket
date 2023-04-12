
#include "Arduino.h"
#define VERI_SAYISI 5

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
#define SEALEVELPRESSURE_HPA  1013.25 //Deniz seviyesindeki basınç değeri 
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

byte packageNumber = 0; 

float totalY = 0; 
float avarageY = 0;   
 
float totalBasinc = 0; 
float avarageBasinc = 91 ;   
float ilkBasincDegeri = 91;   

float  curPressure =0;  //HPA  

float BASINC_OFFSET = 0.2;
bool patla1 = true;
bool patla2 = true;

bool kalkti = false;

String SerialString="";

imu::Vector<3> acc;

void setup()
{ 
  pinMode(ROLE1, OUTPUT);
  pinMode(ROLE1_KONTROL, OUTPUT);
  pinMode(ROLE2, OUTPUT);
  pinMode(ROLE2_KONTROL, OUTPUT);
  digitalWrite(ROLE1, LOW);
  digitalWrite(ROLE2, LOW); 
  digitalWrite(ROLE1_KONTROL, LOW);
  digitalWrite(ROLE2_KONTROL, LOW);

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
    delay(100); 
    totalBasinc += bme.readPressure();   
    totalY += (float)event.orientation.y;
  }  
  avarageY = totalY / VERI_SAYISI ; 
  avarageBasinc = totalBasinc / VERI_SAYISI;   

  ilkBasincDegeri = avarageBasinc;
  
  BASINC_OFFSET = ilkBasincDegeri/300;
  if (myFile) { 
     myFile.println("PakatNumarasi,1Patlama,2Patlama,Irtifa_basinc,Irtifa_GPS,Basinc,X_Jiro,Y_Jiro,Z_Jiro,X_Ivme,Y_Ivme,Z_Ivme,ACI_Y,GPSEnlem,GPSBoylam;"); 
     myFile.close();
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
  curPressure =bme.readPressure(); 
  acc=bno.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER );
  
//////////////////////////////////Message Package Declaration ///////////////////////////////////////

  message.packageNum =  packageNumber;
  message.explode1  = (!patla1)?(byte)1:(byte)0; 
  message.explode2  = (!patla2)?(byte)1:(byte)0; 
  *(float*)(message.Irtifa_basinc) =   bme.readAltitude(SEALEVELPRESSURE_HPA); 
  *(float*)(message.Irtifa_GPS) =  gps.altitude.isValid() ? gps.altitude.meters() : 0; 
  *(float*)(message.pressure) =   curPressure;  //Hekto pascal cinsinden
  *(float*)(message.X_Jiro) =  event.gyro.x; 
  *(float*)(message.Y_Jiro) =  event.gyro.y;
  *(float*)(message.Z_Jiro) =  event.gyro.z; 
  *(float*)(message.X_Ivme) =  acc.x(); 
  *(float*)(message.Y_Ivme) =  acc.y();
  *(float*)(message.Z_Ivme) =  acc.z();  
  *(float*)(message.Aci) =   event.orientation.y; 
  *(float*)(message.GPSe) = gps.location.isValid() ? gps.location.lat() : 0;  
  *(float*)(message.GPSb) = gps.location.isValid() ? gps.location.lng() : 0;   

 //************//

  e32ttl.sendFixedMessage(0,4,6,&message, sizeof(Message));
  packageNumber = (packageNumber==255) ? 0 : packageNumber;

//////////////////////////////////Algorithm///////////////////////////////////////
 /* if(!kalkti && (curPressure < (ilkBasincDegeri - BASINC_OFFSET)) )
  { 
    kalkti = true;
    
    totalY += ( event.orientation.y- avarageY); 
    avarageY = totalY / VERI_SAYISI ; 
    totalBasinc += (curPressure- avarageBasinc); 
    avarageBasinc = totalBasinc / VERI_SAYISI ; 

    if ( ( curPressure > avarageBasinc - (BASINC_OFFSET/3)) && patla1 && ((avarageY-0.5)< 0 )) {
      patla1 = false; 
      digitalWrite(ROLE1, HIGH); 
      digitalWrite(ROLE1_KONTROL, HIGH); 
      delay(2000);
      digitalWrite(ROLE1, LOW); 
      digitalWrite(ROLE1_KONTROL, LOW); 
    }  
    if(!patla1){ 
      if(!patla2){
        //HER ŞEY BİTTİ BURDA 2. PATLAMA OLDU VE AŞAĞI SÜZÜLÜYOR
      }
      if(patla2 && (curPressure >= ilkBasincDegeri - 5.52) ){//TODO : değişcek
        patla2 = false; 
        digitalWrite(ROLE2, HIGH); 
        digitalWrite(ROLE2_KONTROL, HIGH); 
        delay(2000);
        digitalWrite(ROLE2, LOW); 
        digitalWrite(ROLE2_KONTROL, LOW); 
      } 
    }
  } 
  */
//////////////////////////////////Serial Visual ///////////////////////////////////////
    SerialString ="PAKET NUMARASI: "+(byte)(message.packageNum);
    SerialString = SerialString +"\nKALKIŞ DURUMU: "+ kalkti ? "Kalkiş Yapildi!":"Rampada duruyor!";
    SerialString = SerialString +"\n1. PATLAMA DURUMU: "+ (byte)!patla1;  
    SerialString = SerialString +"\n2. PATLAMA DURUMU: "+ (byte)!patla2;  
    SerialString = SerialString +"\nIrtifa(Basinc): "+ *(float*)(message.Irtifa_basinc)+"\tIrtifa(GPS): "+*(float*)(message.Irtifa_GPS)+"\tBasinc: "+
      *(float*)(message.pressure)+"\nX_Jiro: "  + *(float*)(message.X_Jiro)+"\t Y_Jiro: "  + *(float*)(message.Y_Jiro)+"\t Z_Jiro: "+
      *(float*)(message.Z_Jiro)  +"\nX_Ivme: "  + *(float*)(message.X_Ivme)+"\t Y_Ivme: "  + *(float*)(message.Y_Ivme)+"\t Z_İvme: "+ 
      *(float*)(message.Z_Ivme)  + "\tY_ACI: " + *(float*)(message.Aci)  +  "\nGPS Enlem: "+ *(float*)(message.GPSe)+"\t GPS Boylam: "+ *(float*)(message.GPSb)+"\n\n";   
    Serial.print(SerialString);
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
