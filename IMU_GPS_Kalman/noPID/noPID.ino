#include <PID_v1.h>
#include <MPU9250.h>
#include "eeprom_utils.h"
#include <Wire.h>


#define calib false
#define IMU_READ_PERIOD_MS 25
#define mag_dec -3.11

#define TARGET 45.0
#define maxPWM 1900
#define minPWM 1100
#define stallPWM 1500


//rightM
//leftM

MPU9250 mpu;

double starting;
double targetAbs;

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

  starting = mpu.getYaw();
  targetAbs = getAbs(starting, TARGET);

  Serial.println("Starting Yaw: " + String(starting));
  Serial.println("Rel Target: " + String(TARGET));
  Serial.println("Abs Target: " + String(targetAbs));


}

double getAbs(double a, double rel) {
  double b = a + rel;
  if (b > 180) {
    return b - 360;
  } else if (b < -180) {
    return b + 360;
  } 
  return b;
}

double getRel(double cur, double tar) {
  double c, t;
  c = cur;
  if (cur > tar) {
    t = tar + 360;
  } else {
    t = tar;
  }
  double r = t - c;
  double l = 360 - r;
  return (r < l) ? r : -l;
}

void loop() {
  
  mpu.update();
  double cur = mpu.getYaw();
  double rel = getRel(cur, targetAbs);
  int pwmDiff = turnMotors(rel);
  Serial.print(targetAbs);
  Serial.print(",");
  Serial.print(cur);
  Serial.print(",");
  Serial.print(rel);
  Serial.print(",");
  Serial.println(pwmDiff);
  
  

  
  //turnMotors(Output);

  //mpu.printRollPitchYaw();
  //Serial.print(mpu.getRoll(), 2);
  //Serial.print(",");
  //Serial.print(mpu.getPitch(), 2);
  //Serial.print(",");
  //Serial.println(mpu.getYaw(), 2);
  //Serial.println(adjustedYaw());
  
  delay(IMU_READ_PERIOD_MS);

}

int turnMotors(double a) {
  int pwmDiff = map((int)abs(a), 0, 180, 0, 400);
  if (a > 0) {
    //set rightM to forward: pwm = stallPWM + pwmDiff
    //set leftM to reverse: pwm = stallPWM - pwmDiff
  } else {
    //vice versa
  }
  return pwmDiff;
}
