#include <MPU9250.h>
#include <Servo.h>
#include <Wire.h>

#define PWMmax_R 1100
#define PWMmax_F 1900
#define PWMstall 1500

#define CALIBRATE true

#define steerPercentPower 25
#define commonThrustPower 75

#define targetHeading 30 //degrees
#define targetDistance 50 //meters
#define VTA_EXPECTED_SPEED 1.5 //in meters per second

#define thresholdDistance 100
#define thresholdHeading 20
#define maxTime 5000
#define turnInterval 100

byte esc1pin = 5;
byte esc2pin = 6;
Servo ESC1;
Servo ESC2;
MPU9250 mpu;

const int steerPWM_F = PWMstall + ((PWMmax_F - PWMstall) * steerPercentPower / 100);
const int steerPWM_R = PWMstall + ((PWMmax_R - PWMstall) * steerPercentPower / 100);
const int forwardPWM = PWMstall + ((PWMmax_F - PWMstall) * commonThrustPower / 100);

double commandHeading = 0;
double commandDistance = 0;

double vta[4] = {0, 0, 0, 0}; //vehicle states: {lat, long, depth, heading (deg)}
double rov[4] = {0, 0, 0, 0}; //VTA depth will always be 0
                                         //otherwise, data will be fed via radio
double absoluteDistance = 0;

boolean temp = true;


void setup()
{
  Serial.begin(9600);
  Wire.begin();
  ESC1.attach(esc1pin);
  ESC2.attach(esc2pin);

  mpu.setMagneticDeclination(-11); //Assuming the VTA is used in Baltimore, which has a magnetic declination of -11 deg 4'
  //https://www.magnetic-declination.com/
  
  while (!mpu.setup())
  {
    Serial.println("Could not find a valid MPU9250 sensor");
    delay(1000);
  }

  //add lines to cold start satelite signal
  
  if (CALIBRATE) {
    Serial.println("Calibrating");
    mpu.calibrateAccelGyro();
    Serial.println("AccelGyro Calibrated");
    delay(5000);
    mpu.calibrateMag();
  }

  stopThrust();

  /*
  compass.setRange(HMC5883L_RANGE_1_3GA);   // Set measurement range
  compass.setMeasurementMode(HMC5883L_CONTINOUS);   // Set measurement mode
  compass.setDataRate(HMC5883L_DATARATE_30HZ);  // Set data rate
  compass.setSamples(HMC5883L_SAMPLES_8);  // Set number of samples averaged
  compass.setOffset(132, 320);  // Set calibration offset. See HMC5883L_calibration.ino
  */
  delay(2000);
  Serial.println("Setup complete");
}

void loop() {
  if (temp) {
    updateState();
    calculateHeadingDistance();
    if (absoluteDistance > thresholdDistance) {
      Serial.println("distance > thresh, nav");
      if (commandHeading > thresholdHeading) {
        Serial.println("heading > thresh, turning");
        turnVTA();
      }
      runVTA();
    }
    temp = !temp;
  }
}

void updateHeading() {
  Serial.println("Mag: " + String(mpu.getYaw()));
  vta[3] = mpu.getYaw();
}

void turnVTA() {

  double diff = calcDiff(commandHeading, vta[3]);

  while (abs(diff) > thresholdHeading) {
    diff = calcDiff(commandHeading, vta[3]);
    if (diff > 0) {
      turnRight();
    } else {
      turnLeft();
    }
    delay(turnInterval);
    updateHeading();
  }
  /*
  if (abs(error) > thresh)
  {
    stopThrust();
    delay(500)
    // temporarily stall the VTA
    if (error < 0)
    {
      while (abs(error) < thresh)
      {
        ESC1.writeMicroseconds(steerPWM_F);
        ESC2.writeMicroseconds(steerPWM_R); 
      }
    }
    else if (error > 0)
    {
      while (abs(error) < thresh)
      {
        ESC1.writeMicroseconds(steerPWM_R);
        ESC2.writeMicroseconds(steerPWM_F); 
      }
    } 
  }
  */
}

double calcDiff(double target, double cur) {
  double diff = target - cur;
  if (diff > 0) {
    diff = (diff > 180 ? (diff - 360) : diff);
  } else {
    diff = (diff < -180 ? (diff + 360) : diff);
  }
  return diff;
}


void runVTA() {
  forwardThrust();
  int runTime = commandDistance / VTA_EXPECTED_SPEED * 1000;
  int Time = runTime > maxTime ? maxTime : runTime;
  Serial.println("VTA " + String(commandDistance) + " away, running motors for " + String(Time));
  delay(Time);
  stopThrust();
}

void forwardThrust() {
  Serial.println("Going straight");
  motorInput(forwardPWM, forwardPWM);
}

void stopThrust() {
  Serial.println("Stopping VTA");
  motorInput(PWMstall, PWMstall);
}

void turnRight() {
  Serial.println("Turning Right");
  motorInput(steerPWM_F, steerPWM_R);
}

void turnLeft() {
  Serial.println("Turning Left");
  motorInput(steerPWM_R, steerPWM_F);
}

void motorInput(int one, int two) {
  ESC1.writeMicroseconds(one);
  ESC2.writeMicroseconds(two);
}

void updateState() {
  Serial.println("Updating vehicle states");  
  //update VTA Vehicle State Array with GPS, run Kalman Filter if applicable
    /*
 Vector norm = compass.readNormalize();

  // Calculate heading
  float newHeading = atan2(norm.YAxis, norm.XAxis);

  float declinationAngle = -0.349;
  newHeading += declinationAngle;

  // Correct for heading < 0deg and heading > 360deg

  // Convert to degrees
  headingDegrees = newHeading * 180/M_PI;

  VTA_bearing = headingDegrees;

  Serial.print(" Degress = ");
  Serial.print(headingDegrees);
  Serial.println();
  */
  //update ROV Vehicle State Array with radio
  //update absoluteDistance
}

void calculateHeadingDistance() {
  //commandDistance = measureDistance(vta[0], vta[1], rov[0], rov[1]);
  //commandHeading = measureHeading(vta[0], vta[1], rov[0], rov[1], vta[3]);
  commandHeading = targetHeading;
  commandDistance = targetDistance;
}

double measureHeading(double lat1, double lon1, double lat2, double lon2, double curHeading) {
  //Assuming we are operating around the US, lat lon magnitudes
  //not directly using atan because we need to use the great-circle distance function (measureDistance) 
  double EWDistance = measureDistance(lat1, lon1, lat1, lon2);
  double NSDistance = measureDistance(lat1, lon1, lat2, lon1);
  double heading = atan2(NSDistance, EWDistance);
  if (lat1 > lat2) {
    if (lon1 > lon2) {
      //vta is NW of rov, head SE
      heading += 90;
    } else {
      //vta is NE of rov, head SW
      heading = 270 - heading;
    }
  } else {
    if (lon1 > lon2) {
      //vta is SW of rov, head NE
      heading = 90 - heading;
    } else {
      //vta is SE of rov, head NW
      heading += 270;
    }
  }
  Serial.println("Target heading -  " + String(heading));
  Serial.println("Current heading - " + String(vta[3]));
  double relative = (heading - vta[3]);
  Serial.println("Command heading - " + String(relative));
  return relative;
}

double measureDistance(double lat1, double lon1, double lat2, double lon2) {  // generally used geo measurement function
  double R = 6378.137; // Radius of earth in KM
  double dLat = lat2 * M_PI / 180 - lat1 * M_PI / 180;
  double dLon = lon2 * M_PI / 180 - lon1 * M_PI / 180;
  double a = sin(dLat/2) * sin(dLat/2) +
    cos(lat1 * M_PI / 180) * cos(lat2 * M_PI / 180) *
    sin(dLon/2) * sin(dLon/2);
  double distance = 1000 * R * 2 * atan2(sqrt(a), sqrt(1-a));
  Serial.println("Measured distance - " + String(distance));
  return distance; //meters
}
