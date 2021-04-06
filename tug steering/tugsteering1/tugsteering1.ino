#include "eeprom_utils.h"
#include <Wire.h>
#include <Servo.h> 
#include <HX711_ADC.h>
#include "HX711.h"
#include <MPU9250.h>
#include <PID_v1.h>

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
#define IMU_READ_PERIOD_MS 40
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

double coef1 = -14.3;
double coef2 = 328571.4;



double Setpoint, Input, Output;
double Kp=10, Ki=0, Kd=3;

PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);


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
  
  delay(20000);

  resetPID();


  motorInput(1900, 1900);
  delay(3000);
  
}
//need to add how to read load cell


void loop() {
  motorInput(1500, 1500);
  /*
  mpu.update();
  readLoadCell();
  
  double firstReading = lc.magnitude;
  //delay(200);
  //delay(1000/loadCellFrequency);
  //readLoadCell();
  //double secondReading = lc.magnitude;
  //float force_dt=(secondReading-firstReading)/200;
  //(1000/loadCellFrequency);

  
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

    Setpoint = 0;
    resetPID();
    
    while (millis() < startingTime + 10000) {
      mpu.update();
      double cur = mpu.getYaw();
      Input = getRel(cur, targetAbs);
      
      Serial.print(Input);
      Serial.print(", ");
      myPID.Compute();
      
      Serial.println(Output);
      motorInput(stallPWM + Output, stallPWM - Output);
      delay(IMU_READ_PERIOD_MS / 2);
    }
  } else {
    stopThrust();
  }*/
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
  //lc.angle = (int)(atan2(lc.reading2,lc.reading1)/M_PI*180);
  lc.angle = (int)(atan2(lc.reading2,lc.reading1)/M_PI*180) * 0.65;
  lc.magnitude = sqrt(lc.reading1 * lc.reading1 + lc.reading2 * lc.reading2);
}


void resetPID() {
  myPID.SetMode(AUTOMATIC);
  myPID.SetOutputLimits(-400, 400);
  myPID.SetSampleTime(IMU_READ_PERIOD_MS / 2);
}
