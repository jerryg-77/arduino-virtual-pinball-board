#include "Accelerometer.h"
#include <Arduino.h>
#include <Joystick.h>
#include <Adafruit_MPU6050.h>
#include "Enums.h"


Adafruit_MPU6050 mpu;
Adafruit_Sensor *mpu_accel;
Accelerometer::Accelerometer() {
}

void Accelerometer::init(Joystick_* joystick, Config* config) {
  _joystick = joystick;
  _config = config;

  byte count = 0;
  if (!mpu.begin()) {
    delay(1000);
  }
  while (!mpu.begin()) {
    //if (DEBUG) {Serial.print(F("DEBUG,Failed to find MPU6050 chip\r\n"));}
    delay(100);
    if (count > 10) {
      _config->accelerometer = 0;
      _config->accelerometerEprom = 0;
      break;
    }
    count++;
  }
  if (count > 10) {
    return;
  } else {
    count = 0;
  }
  
  //setup motion detection
  // MPU6050_BAND_5_HZ 
  // MPU6050_BAND_10_HZ 
  // MPU6050_BAND_21_HZ 
  // MPU6050_BAND_44_HZ 
  // MPU6050_BAND_94_HZ 
  // MPU6050_BAND_184_HZ 
  // MPU6050_BAND_260_HZ 
  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);

  // MPU6050_HIGHPASS_DISABLE
  // MPU6050_HIGHPASS_5_HZ
  // MPU6050_HIGHPASS_2_5_HZ
  // MPU6050_HIGHPASS_1_25_HZ
  // MPU6050_HIGHPASS_0_63_HZ
  // MPU6050_HIGHPASS_UNUSED
  // MPU6050_HIGHPASS_HOLD
  mpu.setHighPassFilter(MPU6050_HIGHPASS_0_63_HZ);

  mpu.setMotionDetectionThreshold(1);
  mpu.setMotionDetectionDuration(20);
  mpu.setInterruptPinLatch(true);  // Keep it latched.  Will turn off when reinitialized.
  mpu.setInterruptPinPolarity(true);
  mpu.setMotionInterrupt(true);
  resetAccelerometer();
  //assign accelerometer
  mpu_accel = mpu.getAccelerometerSensor();
  delay(100);
  sensors_event_t a;
  while (count < 10) {
    mpu_accel->getEvent(&a);
    xValueOffset += a.acceleration.x;
    yValueOffset += a.acceleration.y;
    count++;
  }
  xValueOffset = xValueOffset/10;
  yValueOffset = yValueOffset/10;

  
  
  //if (DEBUG) {Serial.print(F("DEBUG,MPU6050 Found!\r\n"));}
}

void Accelerometer::resetAccelerometer() {
  _joystick->setXAxisRange(-_config->accelerometerMax, _config->accelerometerMax);
  _joystick->setYAxisRange(-_config->accelerometerMax, _config->accelerometerMax);

  mpu.setAccelerometerRange(_config->accelerometerSensitivity);
}

void Accelerometer::accelerometerRead() {
  //xValue = 0;
  //yValue = 0;
  //if(mpu.getMotionInterruptStatus()) {
  /* Get new sensor events with the readings */
  sensors_event_t a;
  mpu_accel->getEvent(&a);

  xValue = floor((a.acceleration.x - xValueOffset)*100);
  yValue = floor((a.acceleration.y - yValueOffset)*100);
  if (abs(xValue) < _config->accelerometerDeadZone) {
    xValue = 0;
  }
  if (abs(yValue) < _config->accelerometerDeadZone) {
    yValue = 0;
  }
  int temp = xValue;
  if (_config->orientation == RIGHT) {
    xValue = -yValue;
    yValue = temp;
  } else if (_config->orientation == FORWARD) {
    xValue = -xValue;
    yValue = -yValue;
  } else if (_config->orientation == LEFT) {
    xValue = yValue;
    yValue = -temp;
  } else if (_config->orientation == UP_BACK) {
    xValue = -xValue;
    yValue = yValue;
  } else if (_config->orientation == UP_RIGHT) {
    xValue = -yValue;
    yValue = -temp;
  } else if (_config->orientation == UP_FORWARD) {
    xValue = xValue;
    yValue = -yValue;
  } else if (_config->orientation == UP_LEFT) {
    xValue = yValue;
    yValue = temp;
  }
  if(buttonState == 0 && (abs(xValue) > _config->accelerometerTilt || abs(yValue) > _config->accelerometerTilt)) {
    _joystick->setButton(_config->tiltButton, 1);
    buttonState = 1;
  } else if (buttonState == 1 && (abs(xValue) < _config->accelerometerTilt && abs(yValue) < _config->accelerometerTilt)) {
    _joystick->setButton(_config->tiltButton, 0);
    buttonState = 0;
  }
  _joystick->setXAxis(xValue);
  _joystick->setYAxis(yValue);
  //if (DEBUG) {Serial.print(F("DEBUG,AccelX:"));}
  //if (DEBUG) {Serial.print(xValue);}
  //if (DEBUG) {Serial.print(F(","));}
  //if (DEBUG) {Serial.print(F("AccelY:"));}
  //if (DEBUG) {Serial.print(yValue);}
  //if (DEBUG) {Serial.print(F("\r\n"));}
}

void Accelerometer::sendAccelerometerState() {
  sensors_event_t a;
  mpu_accel->getEvent(&a);
  Serial.print(F("A,"));
  Serial.print((a.acceleration.x - xValueOffset));
  Serial.print(F(","));
  Serial.print((a.acceleration.y - yValueOffset));
  Serial.print(F(","));
  Serial.print(xValue);
  Serial.print(F(","));
  Serial.print(yValue);
  Serial.print(F("\r\n"));
  
}
