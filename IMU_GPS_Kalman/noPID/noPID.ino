#include <MPU9250.h>
#include "eeprom_utils.h"
#include <Wire.h>
#include <Servo.h>
#include <HX711_ADC.h>
#include "HX711.h"

#define calib false
#define IMU_READ_PERIOD_MS 25
#define mag_dec -3.11

#define TARGET 145.0
#define maxPWM 1900
#define minPWM 1100
#define stallPWM 1500
#define minThrustThresh 150


#define DOUT  3
#define CLK  2
//#define commonThrustPower 75
//#define thrustTime 5000
#define loadCellFrequency 5 //in Hz
#define thresholdForceChangePerSecond 2.0 //if the rate of change in force deceted by the load cell is more than this value, motors will engage 

#define angleThresh 15

byte esc1pin = 5; //right motor
byte esc2pin = 6; //left motor
Servo rightESC;
Servo leftESC;

//const int forwardPWM = PWMstall + ((PWMmax_F - PWMstall) * commonThrustPower / 100);

HX711 scale;

boolean thrust = false;
unsigned long initT = 0;

float calibration_factor = 20000.21; //-7050 worked for my 440lb max scale setup

MPU9250 mpu;

double starting;
double targetAbs;

void setup() {
  
  Serial.begin(115200);
  Wire.begin();
  rightESC.attach(esc1pin);
  leftESC.attach(esc2pin);
  stopThrust();
  delay(5000);
  
  // Open serial communications and wait for port to open:
    
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

  unsigned long initT = millis();
  Serial.println("Waiting for PID reset");
  while (millis() < initT + 10000) {
    mpu.update();
    delay(50);
  }

  starting = mpu.getYaw();
  targetAbs = getAbs(starting, TARGET);

  Serial.println("Starting Yaw: " + String(starting));
  Serial.println("Rel Target: " + String(TARGET));
  Serial.println("Abs Target: " + String(targetAbs));

  scale.begin(DOUT, CLK);
  scale.set_scale();
  scale.tare(); //Reset the scale to 0
  long zero_factor = scale.read_average(); //Get a baseline reading

  delay(20000);
  //minThrust();
  //delay(2000);
  //motorInput(0, 0);
}

void loop() {
  
  //scale.set_scale(calibration_factor); //Adjust to this calibration factor
  //float initReading = scale.get_units();
  
  mpu.update();
  double cur = mpu.getYaw();
  double rel = getRel(cur, targetAbs);
  int pwmDiff = turnMotors(rel);
  //Serial.print(targetAbs);
  //Serial.print(",");
  //Serial.print(cur);
  //Serial.print(",");
  //Serial.print(rel);
  //Serial.print(",");
  //Serial.println(pwmDiff);
  
  delay(IMU_READ_PERIOD_MS);
  
  

  //motorInput(1500, 1500);
  //delay(100);

  
  //turnMotors(Output);

  //mpu.printRollPitchYaw();
  //Serial.print(mpu.getRoll(), 2);
  //Serial.print(",");
  //Serial.print(mpu.getPitch(), 2);
  //Serial.print(",");
  //Serial.println(mpu.getYaw(), 2);
  //Serial.println(adjustedYaw());
  
  

  /*     
  delay(1000 / loadCellFrequency); //5 Hz readings
  if (!thrust && (abs(initReading - scale.get_units()) > ((double)thresholdForceChangePerSecond / loadCellFrequency))) {
    thrust = true;
    initT = millis();
    forwardThrust();
  }

  if (thrust && (millis() > initT + thrustTime)) {
    thrust = false;
    stopThrust();
  }
  */
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



int turnMotors(double a) {
  //if (abs(a) < angleThresh) {
  //  stopThrust();
  //  return 0;
  //}
  int pwmDiff = 2 * map((int)abs(a), 0, 180, 0, 400);
  pwmDiff = clip(pwmDiff, minThrustThresh, 400);
  if (a > 0) {
    //set rightM to reverse: pwm = stallPWM - pwmDiff
    motorInput(stallPWM - pwmDiff, stallPWM + pwmDiff);
    //set leftM to forward: pwm = stallPWM + pwmDiff
  } else {
    motorInput(stallPWM + pwmDiff, stallPWM - pwmDiff);
  }
  return pwmDiff;
}

int clip(int val, int lower, int upper) {
  //if (val < lower) {
  //  return lower;
  //} else
  if (val > upper) {
    return upper;
  } else {
    return val;
  }
}

void stopThrust() {
  motorInput(stallPWM, stallPWM);
}

void maxThrust() {
  motorInput(maxPWM, maxPWM);
}

void minThrust() {
  motorInput(minPWM, minPWM);
}

void motorInput(int one, int two) {

  rightESC.writeMicroseconds(one);
  leftESC.writeMicroseconds(two);
}
