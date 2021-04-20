#include "eeprom_utils.h"
#include <Wire.h>
#include <Servo.h> 
#include <HX711_ADC.h>
#include "HX711.h"
#include <MPU9250.h>
#include <PID_v1.h>
#include <SPI.h>
//#include <nRF24L01.h>
//#include <RF24.h>
#include <SD.h>

const int chipSelect = 53;

#define theta1 20
#define theta2 75
#define f_turn 1650   //turning forward thrust pwm
#define f_thrust_time 5000 //forward thrust time
#define f_thrust 1900
#define loadCellFrequency 5 //in Hz
#define thresholdForceChangePerSecond 0.5 

#define maxPWM 1900
#define minPWM 1100
#define stallPWM 1500
#define forwardOffset 250

#define calib false
#define IMU_READ_PERIOD_MS 40
#define mag_dec -3.11

#define DOUT2  9
#define CLK2  12
#define DOUT1 11
#define CLK1 10

unsigned long tugSteeringStart = 0;
int tugSteering = 0;
int m1 = 0;
int m2 = 0;

#define tugSteeringDuration 600000
#define tugSteeringBurst 10000

float calibration_factor1 = 20000.21;
float calibration_factor2 = 20000.21;//-7050 worked for my 440lb max scale setup

HX711 scale1;
HX711 scale2;
Servo rightESC;
Servo leftESC;
Servo midESC;

//RF24 radio(7, 8); // CE, CSN
const byte address[6] = "00001";

byte esc1pin=5;
byte esc2pin=6;
byte esc3pin=3;

MPU9250 mpu;

double starting;
double targetAbs;
double angle;

double coef1 = -14.3;
double coef2 = 328571.4;

byte m1PWM = (A0); //M1 PWM values
byte m2PWM = (A1);

double Setpoint, Input, Output;
double Kp=8, Ki=0, Kd=3;


float mag = 0.75;




PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

char radioText[32] = "";


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

  //radio.begin();
  //radio.openWritingPipe(address);
  //radio.setPALevel(RF24_PA_MIN);
  //radio.stopListening();
  
  Serial.begin(9600);

  radioWrite("Serial setup");

  
  Wire.begin();
  rightESC.attach(esc1pin);
  leftESC.attach(esc2pin);
  midESC.attach(esc3pin);
  delay(5000);

  //load cell
  scale1.begin(DOUT1, CLK1);
  scale2.begin(DOUT2, CLK2);
  
  scale1.set_scale();
  scale2.set_scale();
  scale1.tare();
  scale2.tare();//Reset the scale to 0

   // Open serial communications and wait for port to open:
  //Serial.println("lmao");  
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
  //Serial.println("lmao");
  //loadCalibration();
  mpu.setMagneticDeclination(mag_dec);

  while (!mpu.update()) {
    Serial.println("Init reading failed");
    delay(100);
  }
  //Serial.println("lmao");

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
  //radioWrite("Stopping motors");
  
  stopThrust();
  //Serial.println("waiting 20");
  //delay(10000);

  resetPID();


  //motorInput(1900, 1900, 1500);
  delay(5000);
  //radioWrite("setup done");

  /*
  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  Serial.println("card initialized.");
  */
}
//need to add how to read load cell


void loop() {
  
  mpu.update();
  readLoadCell();

  
  double firstReading = lc.magnitude;
  //delay(200);
  //delay(1000/loadCellFrequency);
  //readLoadCell();
  //double secondReading = lc.magnitude;
  //float force_dt=(secondReading-firstReading)/200;
  //(1000/loadCellFrequency);
  //radioWrite(String(firstReading));
  
  if (abs(firstReading) > mag) {
    tugSteering = 1;
    tugSteeringStart = millis();
    //radioWrite("Detected force! Starting tug steering");
    //radioWrite(String(firstReading));
    //datalog();
    //while (millis() < tugSteeringStart + tugSteeringDuration) {
      //mpu.update();
      //readLoadCell();
      //firstReading = lc.magnitude;
      //datalog();
      //if (abs(firstReading) > mag) {

        //tugSteeringStart = millis();
        radioWrite("Detected force!");
        radioWrite(String(firstReading));
        starting = mpu.getYaw();
        targetAbs = getAbs(starting, lc.angle);
        radioWrite(String(getRel(starting, targetAbs)));
        unsigned long startingTime = millis();
    
        Setpoint = 0;
        resetPID();
        tugSteering = 2;
        
        while (millis() < startingTime + tugSteeringBurst) {
          //datalog();
          mpu.update();
          double cur = mpu.getYaw();
          Input = getRel(cur, targetAbs);
          
          //Serial.print(Input);
          //Serial.print(", ");
          myPID.Compute();
          
          //Serial.println(Output);
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
  //} else {
  //  int motor1=pulseIn(m1PWM,HIGH);
  //  int motor2=pulseIn(m2PWM,HIGH);
  //  motorInput(motor1,motor2);
  //  datalog();
  //}
  delay(IMU_READ_PERIOD_MS);

}

void radioWrite(String a) {
  //Serial.println(a);
  //for (int i = 0; i < a.length(); i++) {
  //  radioText[i] = a[i];
  //} 
  //radio.write(&radioText, sizeof(radioText)); 
}

void datalog() {
  /*
  String dataString = String(millis()) + "," + String(tugSteering) + "," + String(lc.magnitude) + "," + String(lc.angle) + "," + String(m1) + "," + String(m2) + "," + String(mpu.getYaw());
  
  File dataFile = SD.open("datalog.csv", FILE_WRITE);
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    Serial.println(dataString);
  }
  dataFile.close();*/
}

void stopThrust() {
  motorInput(stallPWM, stallPWM);
}

void motorInput(int one, int two) {
  int r = clip(one, minPWM, maxPWM);
  int l = clip(two, minPWM, maxPWM);
  rightESC.writeMicroseconds(map(r, 1100, 1900, 1900, 1100));
  leftESC.writeMicroseconds(map(l, 1100, 1900, 1900, 1100));
  m1 = r;
  m2 = l;
  //radioWrite(String(r) +","+ String(l)+"," + String(m));
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
  //lc.angle = (int)(atan2(lc.reading2,lc.reading1)/M_PI*180);
  lc.angle = (int)(atan2(lc.reading2,lc.reading1)/M_PI*180) * 0.65;
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
