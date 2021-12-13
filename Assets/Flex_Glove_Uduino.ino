#include<Uduino.h>
Uduino uduino("FlexGloveController"); // Declare and name your object

#include <Adafruit_ICM20X.h>
#include <Adafruit_ICM20948.h>
#include <Adafruit_Sensor.h>
#include <SparkFun_ADS1015_Arduino_Library.h>
#include <Wire.h>

ADS1015 pinkySensor;
ADS1015 indexSensor;
Adafruit_ICM20948 imu;

float hand[4] = {0, 0, 0, 0};
double gyr[3] = {0, 0, 0};
double acc[3] = {0, 0, 0};

uint16_t indexLow = 920;
uint16_t indexHigh = 955;
uint16_t middleLow = 950;
uint16_t middleHigh = 1140;
uint16_t ringLow = 950;
uint16_t ringHigh = 1140;
uint16_t pinkyLow = 970;
uint16_t pinkyHigh = 1100;

bool calibrating = false;

void setup()
{
  Wire.begin();
  Serial.begin(115200);
  uduino.addCommand("startCalibrate", startCalibrate);
  uduino.addCommand("endCalibrate", endCalibrate);
  uduino.addCommand("getFingers", getFingers);
  uduino.addCommand("getOrientation", getOrientation);
  
  //Begin our finger sensors, change addresses as needed.
  if (indexSensor.begin(ADS1015_ADDRESS_SCL) == false) {
     Serial.println("Index not found. Check wiring.");
     while (1);
  }  

  //Set up each sensor, change the addresses based on the location of each sensor
  if (pinkySensor.begin(ADS1015_ADDRESS_SDA) == false) {
     Serial.println("Pinky not found. Check wiring.");
     while (1);
  }

  pinkySensor.setGain(ADS1015_CONFIG_PGA_TWOTHIRDS); // Gain of 2/3 to works well with flex glove board voltage swings (default is gain of 2)
  indexSensor.setGain(ADS1015_CONFIG_PGA_TWOTHIRDS); // Gain of 2/3 to works well with flex glove board voltage swings (default is gain of 2)  

  pinkySensor.setSampleRate(ADS1015_CONFIG_RATE_3300HZ);
  indexSensor.setSampleRate(ADS1015_CONFIG_RATE_3300HZ);

  // index finger
  indexSensor.setCalibration(1, false, indexLow);
  indexSensor.setCalibration(1, true, indexHigh);

  // middle finger
  indexSensor.setCalibration(0, false, middleLow);
  indexSensor.setCalibration(0, true, middleHigh);

  // ring finger
  pinkySensor.setCalibration(1, false, ringLow);
  pinkySensor.setCalibration(1, true, ringHigh);

  // pinky
  pinkySensor.setCalibration(0, false, pinkyLow);
  pinkySensor.setCalibration(0, true, pinkyHigh);

  if (!imu.begin_I2C()) {
      Serial.println("Failed to find ICM20948 chip");
  }
}

void getFingers() {
    uduino.print("hand ");
    uduino.print(hand[0]);
    uduino.print(" ");
    uduino.print(hand[1]);
    uduino.print(" ");
    uduino.print(hand[2]);
    uduino.print(" ");
    uduino.print(hand[3]);
    uduino.println();
}

void startCalibrate() {
    calibrating = true;
    indexSensor.resetCalibration();
    pinkySensor.resetCalibration();
}

void endCalibrate() {
    calibrating = false;
}

void getOrientation() {
    uduino.printf("orientation %.3f %.3f %.3f %.3f %.3f %.3f", gyr[0], gyr[1], gyr[2], acc[0], acc[1], acc[2]);
    uduino.println();
//    uduino.printf("acc %.3f %.3f %.3f", acc[0], acc[1], acc[2]);
//    uduino.println();
}

void loop()
{
  uduino.update();

  if (uduino.isConnected()) {
      if (!calibrating) {
          // ... your own code
          hand[0] = indexSensor.getScaledAnalogData(1);
          hand[1] = indexSensor.getScaledAnalogData(0);
          hand[2] = pinkySensor.getScaledAnalogData(1);
          hand[3] = pinkySensor.getScaledAnalogData(0);

          sensors_event_t accel;
          sensors_event_t gyro;
          sensors_event_t mag;
          sensors_event_t temp;
          imu.getEvent(&accel, &gyro, &temp, &mag);

          gyr[0] = gyro.gyro.x;
          gyr[1] = gyro.gyro.y;
          gyr[2] = gyro.gyro.z;

          acc[0] = accel.acceleration.x;
          acc[1] = accel.acceleration.y;
          acc[2] = accel.acceleration.z;

          // Important: If you uduino.print values outside this loop,
          // the board will not be correctly detected on Unity !
      } else {
          indexSensor.calibrate();
          pinkySensor.calibrate();
      }

  }
}
