#include <PID_v1.h>
#include <MPU9250.h>
#include "eeprom_utils.h"
#include <Wire.h>


#define calib false
#define IMU_READ_PERIOD_MS 25
#define mag_dec -3.11

#define kP 0.04
#define kI 0
#define kD 1

#define target 45.0
#define maxPWM 1900
#define minPWM 1100
#define stallPWM 1500

double Input, Output, Setpoint;

boolean right = true;


//rightM
//leftM

PID pid(&Input, &Output, &Setpoint, kP, kI, kD, DIRECT);

MPU9250 mpu;

double starting = 0;

void setup() {
  
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  Wire.begin();
  delay(2000);
  
  if (!mpu.setup(0x68)) {
    while(1) {
      Serial.println("MPU connection failed. Please check your connection with `connection_check` example.");
      delay(5000);
    }
  }

  if (calib) {
    Serial.println("Accel & Gyro");
    mpu.calibrateAccelGyro();
    Serial.println("Mag");
    mpu.calibrateMag();
    saveCalibration();    
  }
  
  loadCalibration();
  mpu.setMagneticDeclination(mag_dec);

  while (!mpu.update()) {
    Serial.println("Init reading failed");
    delay(100);
  }

  delay(2000);

  if (target < 0) {
    right = false;
  }

  starting = mpu.getYaw() + 180;
  Input = 180;
  Setpoint = starting + target;

  Serial.println("Starting Yaw: " + String(starting));
  Serial.println("Target: " + String(target));
  
  pid.SetMode(AUTOMATIC);
  pid.SetOutputLimits(0, (maxPWM - minPWM)/2);
  pid.SetSampleTime(IMU_READ_PERIOD_MS);

  Serial.println("AdjustedYaw, OutputPWM, Error");  


}

double adjustedYaw() {
  //return mpu.getYaw() + 180;
  return mpu.getYaw() + 180 - starting;
}

void loop() {
  
  if (mpu.update()) {
    Input = adjustedYaw();
    pid.Compute();
    turnMotors(Output);

    Serial.print(Setpoint);
    Serial.print(",");
    Serial.print(Input);
    Serial.print(",");
    Serial.print(Output);
    Serial.print(",");
    Serial.println(Setpoint - Input);
    //mpu.printRollPitchYaw();
    //Serial.print(mpu.getRoll(), 2);
    //Serial.print(",");
    //Serial.print(mpu.getPitch(), 2);
    //Serial.print(",");
    //Serial.println(mpu.getYaw(), 2);
    //Serial.println(adjustedYaw());
    
    delay(IMU_READ_PERIOD_MS);
  }

}

void turnMotors(double a) {
  if (right) {
    //set rightM to forward: pwm = stallPWM + Output
    //set leftM to reverse: pwm = stallPWM - Output
  } else {
    //vice versa
  }
}
