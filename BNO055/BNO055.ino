#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
  
Adafruit_BNO055 bno = Adafruit_BNO055(55);

void setup(void) 
{
  Serial.begin(9600);
  Serial.println("Orientation Sensor Test"); Serial.println("");
  
  /* Initialise the sensor */
  if(!bno.begin())
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
  delay(1000);
  bno.setExtCrystalUse(true);
}

void loop(void) 
{
  // Possible vector values can be:
  // - VECTOR_ACCELEROMETER - m/s^2
  // - VECTOR_MAGNETOMETER  - uT
  // - VECTOR_GYROSCOPE     - rad/s
  // - VECTOR_EULER         - degrees
  // - VECTOR_LINEARACCEL   - m/s^2
  // - VECTOR_GRAVITY       - m/s^2
  
  /* Get a new sensor event */ 
  sensors_event_t event; 
  bno.getEvent(&event);
  
  /* Display the floating point data */
  Serial.print("Orientation: ");
  Serial.print("\tX: ");
  Serial.print(event.orientation.x, 4);
  Serial.print(" Y: ");
  Serial.print(event.orientation.y, 4);
  Serial.print(" Z: ");
  Serial.print(event.orientation.z, 4);
  Serial.println("");
  
  delay(1000);
/*
  imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
  
  //Display the floating point data 
  Serial.print("Euler: ");
  Serial.print("\tX: ");
  Serial.print(euler.x());
  Serial.print(" Y: ");
  Serial.print(euler.y());
  Serial.print(" Z: ");
  Serial.print(euler.z());
  Serial.println("");
  
  delay(100);
  imu::Quaternion quat = bno.getQuat();

  //Display the quat data   
  Serial.print("Quaternion: ");
  Serial.print("\tqW: ");
  Serial.print(quat.w(), 4);
  Serial.print(" qX: ");
  Serial.print(quat.y(), 4);
  Serial.print(" qY: ");
  Serial.print(quat.x(), 4);
  Serial.print(" qZ: ");
  Serial.print(quat.z(), 4);
  Serial.println("");

  delay(100);
  //Display the current temperature 
  int8_t temp = bno.getTemp();
  
  Serial.println("Current Temperature: ");
  Serial.print(temp);
  Serial.print(" *C");
  Serial.println("");
  */
}
