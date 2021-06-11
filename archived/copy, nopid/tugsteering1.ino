#include "eeprom_utils.h"
#include <Wire.h>
#include <Servo.h> 
#include <HX711_ADC.h>
#include "HX711.h"
#include <MPU9250.h>

#define theta1 30
#define theta2 75
#define f_turn 1650   //turning forward thrust pwm
#define f_thrust_time 5000 //forward thrust time
#define f_thrust 1900
#define loadCellFrequency 5 //in Hz
#define thresholdForceChangePerSecond 0.5 

#define maxPWM 1900
#define minPWM 1100
#define stallPWM 1500

#define calib false
#define IMU_READ_PERIOD_MS 25
#define mag_dec -3.11

#define DOUT2  9
#define CLK2  8
#define DOUT1 11
#define CLK1 10

float calibration_factor1 = 20000.21;
float calibration_factor2 = 20000.21;//-7050 worked for my 440lb max scale setup

HX711 scale1;
HX711 scale2;
Servo rightESC;
Servo leftESC;

byte esc1pin=5;
byte esc2pin=6;

MPU9250 mpu;

double starting;
double targetAbs;
double angle;
double Kp = 15;

double coef1 = -14.3;
double coef2 = 328571.4;

struct loadCell {
  float reading1;
  float reading2;
  int angle;
  float magnitude;
};

struct loadCell lc = {0, 0, 0, 0};

double getAbs(double a, double rel) {
  double b = a + rel;
  if (b > 180) {
    return b - 360;
  } else if (b < -180) {
    return b + 360;
  } 
  return b;
}
/*
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
}*/

double getRel(double cur, double tar) {
  int c = (int)(10 * (cur + 180)) % 3600;
  int t = (int)(10 * (tar + 180)) % 3600;
  return (-1 * ((5400 + c - t) % 3600 - 1800)) / 10.0;
}

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
  
  Serial.begin(115200);
  Wire.begin();
  rightESC.attach(esc1pin);
  leftESC.attach(esc2pin);
  delay(5000);

  //load cell
  Serial.begin(9600);
  scale1.begin(DOUT1, CLK1);
  scale2.begin(DOUT2, CLK2);
  
  scale1.set_scale();
  scale2.set_scale();
  scale1.tare();
  scale2.tare();//Reset the scale to 0

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
    //mpu.saveCalibration();    
  }
  
  //loadCalibration();
  mpu.setMagneticDeclination(mag_dec);

  while (!mpu.update()) {
    Serial.println("Init reading failed");
    delay(100);
  }

  /*
  unsigned long initT = millis();
  Serial.println("Waiting for PID reset");
  while (millis() < initT + 10000) {
    mpu.update();
    delay(50);
  }
  */

  //long zero_factor1 = scale1.read_average(); //Get a baseline reading
  //long zero_factor2 = scale2.read_average();
  
  //delay(10000);
}
//need to add how to read load cell


void loop() {
  
  mpu.update();
  readLoadCell();

  /*
  Serial.print(String(mpu.getYaw(), 4));
  Serial.print(",");
  Serial.print(lc.reading1);
  Serial.print(",");
  Serial.print(lc.reading2);
  Serial.print(",");
  Serial.print(lc.angle);
  Serial.print(",");
  Serial.println(lc.magnitude); 
  */
  
  double firstReading = lc.magnitude;
  //delay(200);
  //delay(1000/loadCellFrequency);
  //readLoadCell();
  //double secondReading = lc.magnitude;
  //float force_dt=(secondReading-firstReading)/200;
  //(1000/loadCellFrequency);
  
  //Serial.println(firstReading);
  //Serial.print(", ");
  //Serial.print(secondReading);
  //Serial.print(", ");
  //Serial.println(force_dt);

  
  float mag = 0.5;
  if (abs(firstReading) > mag) {
  //if (abs(force_dt) > thresholdForceChangePerSecond){

    Serial.println("Detected force! Starting angle");
    
    starting = mpu.getYaw();
    Serial.println(String(starting));
    targetAbs = getAbs(starting, lc.angle);
    Serial.println(String(targetAbs));
    Serial.println(getRel(starting, targetAbs));
    unsigned long startingTime = millis();
    while (millis() < startingTime + 10000) {
      mpu.update();
      double cur = mpu.getYaw();
      double rel = getRel(cur, targetAbs);
      //Serial.println(rel);
      int PWM_diff = Kp * rel;
      //Serial.println(PWM_diff);
      //motorInput(stallPWM - PWM_diff, stallPWM + PWM_diff);
      delay(IMU_READ_PERIOD_MS);
    }
    
    //targetAbs = getAbs(starting, lc.angle);
    //boolean turning = true;
    //unsigned long startingTime = millis();
    /*
    while (turning) {
      mpu.update();
      readLoadCell();
      targetAbs = getAbs(starting, lc.magnitude - starting); //TODO
      
      if (millis() > startingTime + f_thrust_time) {
        Serial.println("Stopping turn");
        turning = false;
      }

      
      if (abs(targetAbs) > theta2) {
        Serial.println("above theta2");
        fullDiffThrust(); //need to verify direction
      } else if (abs(targetAbs) > theta1) {
        int PWM_diff = Kd * targetAbs;
        motorInput(f_turn + PWM_diff, f_turn - PWM_diff); //need to verify direction
        Serial.println("above theta1");
      } else {
        motorInput(f_thrust, f_thrust);
        Serial.println("straight");
      }
      
      delay(IMU_READ_PERIOD_MS);*/
  } else {
    stopThrust();
  }
delay(IMU_READ_PERIOD_MS);
}

void stopThrust() {
  motorInput(stallPWM, stallPWM);
}

void motorInput(int one, int two) {
  int r = clip(one, minPWM, maxPWM);
  int l = clip(two, minPWM, maxPWM);
  rightESC.writeMicroseconds(map(r, 1100, 1900, 1900, 1100));
  leftESC.writeMicroseconds(map(l, 1100, 1900, 1900, 1100));
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

void readLoadCell() { //some how extrapolate angle from this
  lc.reading1 = scale1.get_units() / coef1 / 21176.4;
  lc.reading2 = scale2.get_units() / coef2 * -1;
  lc.angle = (int)(atan2(lc.reading2,lc.reading1)/M_PI*180);
  lc.magnitude = sqrt(lc.reading1 * lc.reading1 + lc.reading2 * lc.reading2);
}
