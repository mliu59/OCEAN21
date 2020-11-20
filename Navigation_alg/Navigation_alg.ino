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
#define thersholdHeading 20

#define maxTime 5000

byte esc1pin = 5;
byte esc2pin = 6;
Servo ESC1;
Servo ESC2;
MPU9250 mpu;

int steerPWM_F = PWMstall + ((PWMmax_F - PWMstall) * steerPercentPower / 100);
int steerPWM_R = PWMstall + ((PWMmax_R - PWMstall) * steerPercentPower / 100);
int forwardPWM = PWMstall + ((PWMmax_F - PWMstall) * commonThrustPower / 100);

double commandHeading = 0;
double commandDistance = 0;

double vta[4] = {0, 0, 0, 0}; //vehicle states: {lat, long, depth, heading (deg)}
double rov[4] = {0, 0, 0, 0); //VTA depth will always be 0
                                         //otherwise, data will be fed via radio
double absoluteDistance = 0;

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  ESC1.attach(esc1pin);
  ESC2.attach(esc2pin);
  
  while (!mpu.setup())
  {
    Serial.println("Could not find a valid MPU9250 sensor, check wiring!");
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
  newCommand = true;
  delay(2000);
}

void loop() {
  UpdateCompassInfo(); // updates compass at 10 Hz
  updateState();
  calculateHeadingDistance();
  if (absoluteDistance > thresholdDistance) {
    if (commandHeading > thresholdHeading) {
      turnVTA();
    }
    runVTA();
  }
}



void turnVTA() {
  // ensures that the 
  error = (540 + VTA_bearing - target_bearing) % 360 - 180

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
}

void runVTA() {
  ESC1.writeMicroseconds(forwardPWM);
  ESC2.writeMicroseconds(forwardPWM);
  int runTime = commandDistance / VTA_EXPECTED_SPEED * 1000;
  delay(runTime > maxTime ? maxTime : runTime);
  stopThrust();
}

void stopThrust() {
  ESC1.writeMicroseconds(PMWstall);
  ESC2.writeMicroseconds(PMWstall);
}

void UpdateCompassInfo()
{
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
}

void updateState(String command) {  
  //update VTA Vehicle State Array with GPS, run Kalman Filter if applicable
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
  return (heading - vta[3]);
}

double measureDistance(double lat1, double lon1, double lat2, double lon2) {  // generally used geo measurement function
  double R = 6378.137; // Radius of earth in KM
  double dLat = lat2 * M_PI / 180 - lat1 * M_PI / 180;
  double dLon = lon2 * M_PI / 180 - lon1 * M_PI / 180;
  double a = sin(dLat/2) * sin(dLat/2) +
    cos(lat1 * M_PI / 180) * cos(lat2 * M_PI / 180) *
    sin(dLon/2) * sin(dLon/2);
  return 1000 * R * 2 * atan2(sqrt(a), sqrt(1-a)); //meters
}
