#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
 
#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10
 
#define SEALEVELPRESSURE_HPA (1013.25)
 
Adafruit_BME680 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK);
 
void setup() {
  Serial.begin(115200);
  if (Serial)
    Serial.println(F("BME680 async test"));
 
  if (!bme.begin()) {
    Serial.println(F("Could not find a valid BME680 sensor, check wiring!"));
    while (1);
  }
 
  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms
}
 
void loop() {
  // Tell BME680 to begin measurement.
  unsigned long endTime = bme.beginReading();
  if (endTime == 0) {
    Serial.println(F("Failed to begin reading :("));
    return;
  }
  
  Serial.print(F("Reading started at "));
  Serial.print(millis());
  Serial.print(F(" and will finish at "));
  Serial.println(endTime);
 
  Serial.println(F("You can do other work during BME680 measurement."));
  delay(50); // This represents parallel work.
  // There's no need to delay() until millis() >= endTime: bme.endReading()
  // takes care of that. It's okay for parallel work to take longer than
  // BME680's measurement time.
 
  // Obtain measurement results from BME680. Note that this operation isn't
  // instantaneous even if milli() >= endTime due to I2C/SPI latency.
  if (!bme.endReading()) {
    Serial.println(F("Failed to complete reading :("));
    return;
  }
  Serial.print(F("Reading completed at "));
  Serial.println(millis());
 
  float temperature = bme.temperature;
  float pressure = bme.pressure / 100.0;
  float altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
  float humidity = bme.humidity;
  float gas = bme.gas_resistance / 1000.0;
  double dewPoint = dewPointFast(temperature, humidity);
  
  Serial.print("Temperature = ");
  Serial.print(temperature);
  Serial.println(" *C");
 
  Serial.print("Pressure = ");
  Serial.print(pressure);
  Serial.println(" hPa");
 
  Serial.print("Humidity = ");
  Serial.print(humidity);
  Serial.println(" %");
 
  Serial.print("Dew Point = ");
  Serial.print(dewPoint);
  Serial.println(" *C");
 
  Serial.print("Approx. Altitude = ");
  Serial.print(altitude);
  Serial.println(" m");
 
  Serial.print("Gas = ");
  Serial.print(gas);
  Serial.println(" KOhms");
 
  Serial.println();
  delay(2000);
}
 
double dewPointFast(double celsius, double humidity)
{
  double a = 17.271;
  double b = 237.7;
  double temp = (a * celsius) / (b + celsius) + log(humidity * 0.01);
  double Td = (b * temp) / (a - temp);
  return Td;
}
