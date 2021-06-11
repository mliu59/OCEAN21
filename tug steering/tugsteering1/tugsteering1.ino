/*
OCEAN21 VTA Integrated Arduino Code
Author: Miles Liu (mliu59@jhu.edu)
Last Updated: 06/10/2021
*/
//include all necessary arduino libraries
#include "eeprom_utils.h"
#include <Wire.h>
#include <Servo.h> 
#include <HX711_ADC.h>
#include "HX711.h"
#include <MPU9250.h>
#include <PID_v1.h>
#include <SPI.h>
#include <SD.h>

const int chipSelect = 53;

//defining variables related to tug steering and motor actuation
#define theta1 20
#define theta2 75

//Basic PWM Values
#define maxPWM 1900
#define minPWM 1100
#define stallPWM 1500
#define forwardOffset 250

//variables for IMU calibration 
#define calib false
#define IMU_READ_PERIOD_MS 40
#define mag_dec -3.11

//Load Cell AMp Connection
#define DOUT2  9
#define CLK2  12
#define DOUT1 11
#define CLK1 10

//initialize placeholder variables
unsigned long tugSteeringStart = 0;
int tugSteering = 0;
int m1 = 0;
int m2 = 0;
double starting;
double targetAbs;
double angle;

//duration of tug steering before attempting acitve nav again
#define tugSteeringDuration 600000
//duration of each tug steering burst before turning off motors and awaiting tension again
#define tugSteeringBurst 10000

//keep, load cell calibration values
float calibration_factor1 = 20000.21;
float calibration_factor2 = 20000.21;
double coef1 = -14.3;
double coef2 = 328571.4;

//Library Objects
HX711 scale1;
HX711 scale2;
Servo rightESC;
Servo leftESC;
Servo midESC;
MPU9250 mpu;

//Define pins for reading the PWM from PixHawk PDB & outputting PWm values to ESC
byte esc1pin=5;
byte esc2pin=6;
byte m1PWM = (A0); //M1 PWM values
byte m2PWM = (A1);


//Initialize PID tuning K values for tug steering reorientation
double Setpoint, Input, Output;
double Kp=8, Ki=0, Kd=3;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

//threshold magnitude of force before tug steering activates (lb)
float mag = 0.75;

//data structure for storing the VTA's load cell readings
struct loadCell {
  float reading1;
  float reading2;
  int angle;
  float magnitude;
};

struct loadCell lc = {0, 0, 0, 0};

//get expected absolute IMU reading given a current reading, and target relative angle
double getAbs(double a, double rel) {
  double b = a + rel;
  if (b > 180) {
    return b - 360;
  } else if (b < -180) {
    return b + 360;
  } 
  return b;
}

//Given a current and target angles, get the relative angle needed
double getRel(double cur, double tar) {
  int c = (int)(10 * (cur + 180)) % 3600;
  int t = (int)(10 * (tar + 180)) % 3600;
  return (-1 * ((5400 + c - t) % 3600 - 1800)) / 10.0;
}

//Clip a PWM value to between thresholds
int clip(int val, int lower, int upper) {
  if (val < lower) {
    return lower;
  } else if (val > upper) {
    return upper;
  } else {
    return val;
  }
}


void setup() {
  
  Serial.begin(9600);


  //attach ESCs, allow for them to auto calibrate
  Wire.begin();
  rightESC.attach(esc1pin);
  leftESC.attach(esc2pin);
  delay(5000);

  //load cell
  scale1.begin(DOUT1, CLK1);
  scale2.begin(DOUT2, CLK2);
  
  scale1.set_scale();
  scale2.set_scale();
  scale1.tare();
  scale2.tare();//Reset the scale to 0

   // Open IMU, wait for connection
  if (!mpu.setup(0x68)) {
    while(1) {
      Serial.println("MPU connection failed. Please check your connection with `connection_check` example.");
      delay(5000);
    }
  }
  //calibrate load cell if needed
  if (calib) {
    Serial.println("Accel & Gyro");
    mpu.calibrateAccelGyro();
    Serial.println("Mag");
    mpu.calibrateMag();
    mpu.saveCalibration();    
  }
  
  loadCalibration();
  mpu.setMagneticDeclination(mag_dec);

  while (!mpu.update()) {
    Serial.println("Init reading failed");
    delay(100);
  }
  //wait for ESCs to reset
  stopThrust();
  resetPID();
  delay(5000);
}


void loop() {
  

  //at each loop, update load cell
  mpu.update();
  readLoadCell();

  double firstReading = lc.magnitude;
  
  //if load cell reading greater than threshold, styart tug steering
  if (abs(firstReading) > mag) {
    tugSteeringStart = millis();

    //while tug steering loop
    while (millis() < tugSteeringStart + tugSteeringDuration) {
      mpu.update();
      readLoadCell();
      firstReading = lc.magnitude;
      if (abs(firstReading) > mag) {

        starting = mpu.getYaw();
        targetAbs = getAbs(starting, lc.angle);
        unsigned long startingTime = millis();
    
        Setpoint = 0;
        resetPID();
        tugSteering = 2;
        
        while (millis() < startingTime + tugSteeringBurst) {
          mpu.update();
          double cur = mpu.getYaw();
          Input = getRel(cur, targetAbs);
          
          myPID.Compute();
          
          if (abs(Input) > theta2) {
            motorInput(stallPWM + Output, stallPWM - Output);
          } else if (abs(Input) > theta1) {
            motorInput(stallPWM + Output + forwardOffset, stallPWM - Output + forwardOffset);
          } else {
            
            motorInput(1900, 1900);
          }
          delay(IMU_READ_PERIOD_MS / 2);
        }
      }
      delay(IMU_READ_PERIOD_MS);
    }
    //if tug steering not active, or has passed, read PWM from PixHawk and relay to ESCs. (Active Nav)
  } else {
    int motor1=pulseIn(m1PWM,HIGH);
    int motor2=pulseIn(m2PWM,HIGH);
    motorInput(motor1,motor2);
  }
}

void stopThrust() {
  motorInput(stallPWM, stallPWM);
}

//relay motor commands to ESCs
void motorInput(int one, int two) {
  int r = clip(one, minPWM, maxPWM);
  int l = clip(two, minPWM, maxPWM);
  rightESC.writeMicroseconds(map(r, 1100, 1900, 1900, 1100));
  leftESC.writeMicroseconds(map(l, 1100, 1900, 1900, 1100));
  m1 = r;
  m2 = l;
}

int getLoadCellAngle() {
  return lc.angle;
}

void fullDiffThrust() {
  if (targetAbs > 0) {
    motorInput(minPWM,maxPWM);
  } else {
    motorInput(maxPWM,minPWM);
  }
}

void readLoadCell() {
  double r11 = scale1.get_units() / coef1 / 21176.4;
  double r21 = scale2.get_units() / coef2 * -1;
  delay(10);
  double r12 = scale1.get_units() / coef1 / 21176.4;
  double r22 = scale2.get_units() / coef2 * -1;
  delay(10);
  double r13 = scale1.get_units() / coef1 / 21176.4;
  double r23 = scale2.get_units() / coef2 * -1;
  
  lc.reading1 = (r11 + r12 + r13)/3;
  lc.reading2 = (r21 + r22 + r23)/3;
  lc.angle = (int)(atan2(lc.reading2,lc.reading1)/M_PI*180);
  lc.magnitude = sqrt(lc.reading1 * lc.reading1 + lc.reading2 * lc.reading2);
}

void fullForward() {
  motorInput(1900, 1900);
}


void resetPID() {
  myPID.SetMode(AUTOMATIC);
  myPID.SetOutputLimits(-400, 400);
  myPID.SetSampleTime(IMU_READ_PERIOD_MS / 2);
}
