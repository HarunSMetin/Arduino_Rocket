 
#include <Wire.h>
#include <Adafruit_MPL115A2.h>

Adafruit_MPL115A2 mpl115a2;

void setup(void) 
{
  Serial.begin(9600);  
  if (! mpl115a2.begin()) {
    Serial.println("Sensor not found! Check wiring"); 
  }
}

void loop(void) 
{
  float pressureKPA = 0, temperatureC = 0;    
  Serial.println("MPL11A2");
  mpl115a2.getPT(&pressureKPA,&temperatureC);
  Serial.println("Pressure (kPa): "); Serial.print(pressureKPA, 4); Serial.print(" kPa  ");
  Serial.println("Temp (*C): "); Serial.print(temperatureC, 1); 
  Serial.println(" *C both measured together");
  
  pressureKPA = mpl115a2.getPressure();  
  Serial.println("Pressure (kPa): "); Serial.print(pressureKPA, 4); Serial.print(" kPa");

  temperatureC = mpl115a2.getTemperature();  
  Serial.println("Temp (*C): "); Serial.print(temperatureC, 1); Serial.print (" *C");
  delay(500);
} 
