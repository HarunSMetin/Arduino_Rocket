
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
//Arduino Mega	51	  50	  52	4   
// CS pini değiştirilebilir. İstedğimiz herhangi bir dijital pin olur
#include <SPI.h>
#include <SD.h>
#define SD_KART_CS_PIN 4
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

float totalY = 0; 
float avarageY = 0;   

byte packageNumber = 0; 

float totalBasinc = 0; 
float avarageBasinc = 91 ;   
float ilkBasincDegeri = 91;   

float  curPressure =0;  //HPA  

float BASINC_OFFSET = 0.2;
bool patla1 = true;
bool patla2 = true;
 

void setup()
{ 
  pinMode(ROLE1, OUTPUT);
  pinMode(ROLE1_KONTROL, OUTPUT);
  pinMode(ROLE2, OUTPUT);
  pinMode(ROLE2_KONTROL, OUTPUT);
  digitalWrite(ROLE1, HIGH);
  digitalWrite(ROLE2, HIGH); 
  digitalWrite(ROLE1_KONTROL, HIGH);
  digitalWrite(ROLE2_KONTROL, HIGH);

  Serial.begin(9600);   
  Serial2.begin(GPSBaud);
  delay(100);

  if(!bno.begin())
    Serial.println("Sensor BNO055 NOT detected!"); 
  if (!bme.begin())
    Serial.println("Sensor BME680 NOT detected!"); 
  if(!SD.begin(SD_KART_CS_PIN))
    Serial.println("SD Card Module NOT detected!");

  myFile = SD.open("veriler.txt", FILE_WRITE);
  delay(200);
  bno.setExtCrystalUse(true); 
  delay(200);
  e32ttl.begin();     
  delay(200);
 
  bme.setPressureOversampling(BME680_OS_4X);

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
     myFile.println("PakatNumarasi,1Patlama,2Patlama,Basinc,X,Y,Z,GPSEnlem,GPSBoylam;"); 
      myFile.close();
  }
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

bool kalkti = false;

void loop()
{  
  packageNumber++;  
 
  myFile = SD.open("veriler.txt", FILE_WRITE);
  bno.getEvent(&event);
  smartDelay(100);  
  curPressure =bme.readPressure();
  
  message.packageNum =  packageNumber;
  message.explode1  = (!patla1)?(byte)1:(byte)0; 
  message.explode2  = (!patla2)?(byte)1:(byte)0;
  *(float*)(message.pressure) =   curPressure;  //Hekto pascal cinsinden
  *(float*)(message.X) =  event.orientation.x; 
  *(float*)(message.Y) =  event.orientation.y;
  *(float*)(message.Z) =  event.orientation.z; 
  *(float*)(message.GPSe) = gps.location.isValid() ? gps.location.lat() : 0;  
  *(float*)(message.GPSb) = gps.location.isValid() ? gps.location.lng() : 0;  


  ResponseStatus rs = e32ttl.sendFixedMessage(0,4,6,&message, sizeof(Message));
  Serial.println(rs.getResponseDescription());

  packageNumber = (packageNumber==255) ? 0 : packageNumber;

  if(!kalkti & (curPressure < (ilkBasincDegeri - BASINC_OFFSET)) ){ kalkti = true;}
  if(kalkti){
    totalY += ( event.orientation.y- avarageY); 
    avarageY = totalY / VERI_SAYISI ; 
    totalBasinc += (curPressure- avarageBasinc); 
    avarageBasinc = totalBasinc / VERI_SAYISI ; 

    if ( ( curPressure > avarageBasinc - (BASINC_OFFSET/3)) & patla1 & ((avarageY-0.5)< 0 )) {
      Serial.println("1. PATLADI");
      patla1 = false; 
      digitalWrite(ROLE1, LOW); 
      digitalWrite(ROLE1_KONTROL, LOW); 
      delay(1000);
      digitalWrite(ROLE1, HIGH); 
      digitalWrite(ROLE1_KONTROL, HIGH); 
    }  
    if(!patla1){ 
      if(!patla2){
        //HER ŞEY BİTTİ BURDA 2. PATLAMA OLDU VE AŞAĞI SÜZÜLÜYOR
      }
      if(patla2 & (curPressure >= ilkBasincDegeri - 5.52) ){
        patla2 = false; 
        digitalWrite(ROLE2, LOW); 
        digitalWrite(ROLE2_KONTROL, LOW); 
        delay(1000);
        digitalWrite(ROLE2, HIGH); 
        digitalWrite(ROLE2_KONTROL, HIGH); 
      } 
    }
  } 
  
    Serial.print("PAKET NUMARASI: "); Serial.println((byte)(message.packageNum));   
    Serial.print("KALKIŞ DURUMU: "); Serial.println(kalkti ? "Kalkiş Yapildi!":"Rampada duruyor!");  
    Serial.print("1. PATLAMA DURUMU: "); Serial.println(!patla1); 
    Serial.print("2. PATLAMA DURUMU: "); Serial.println(!patla2);

    Serial.print("\t Basinç: "); 
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

        Serial.println(); 
        Serial.println ("***************************************************************");
        Serial.print("calibrated altitude(m) :");
        Serial.println(bme.readAltitude(SEALEVELPRESSURE_HPA)); //çok yavaş //kapat
   
    Serial.println ("----------------------------------------------------------------------------------------------");
   if (myFile) {
      myFile.print((byte)message.packageNum);  
      myFile.print(","); 
      myFile.print((byte)message.explode1);      
      myFile.print(",");  
      myFile.print((byte)message.explode2); 
      myFile.print(","); 
      myFile.print(*(float*)(message.pressure));
      myFile.print(","); 
      myFile.print (*(float*)(message.X),6);  
      myFile.print(","); 
      myFile.print (*(float*)(message.Y),6);  
      myFile.print(","); 
      myFile.print (*(float*)(message.Z),6);
      myFile.print(","); 
      myFile.print (*(float*)(message.GPSe),6); 
      myFile.print(","); 
      myFile.print (*(float*)(message.GPSb),6);      
      myFile.println(";");    
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
