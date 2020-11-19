#include <HMC5883L.h>
#include <Servo.h>

byte esc1pin = 5;
byte esc2pin = 6;

float error = 0;
int headingDegrees;

#define PWMmax_R 1100
#define PWMmax_F 1900
#define PWMstall 1500

// remember to manually define a target_bearing

#define percentPower 50
int customPWM_F = PWMstall + (abs(PWMmax_F - PWMstall) * percentPower / 100);
int customPWM_R = PWMstall + (abs(PWMmax_R - PWMstall) * percentPower / 100);

Servo ESC1;
Servo ESC2;

void setup()
{
  Serial.begin(9600);
  ss.begin(GPSBaud);
  
  while (!compass.begin())
  {
    Serial.println("Could not find a valid HMC5883L sensor, check wiring!");
    delay(500);
  }

  // Set measurement range
  compass.setRange(HMC5883L_RANGE_1_3GA);

  // Set measurement mode
  compass.setMeasurementMode(HMC5883L_CONTINOUS);

  // Set data rate
  compass.setDataRate(HMC5883L_DATARATE_30HZ);

  // Set number of samples averaged
  compass.setSamples(HMC5883L_SAMPLES_8);

  // Set calibration offset. See HMC5883L_calibration.ino
  compass.setOffset(132, 320);

  delay(2000);
 
void loop() {
  // updates compass at 10 Hz
  UpdateCompassInfo();

  
}


void turnVehicle(int target_bearing)
{
  
  // ensures that the 
  error = (540 + VTA_bearing - target_bearing) % 360 - 180

  if (abs(error) > thresh)
  {
    ESC1.writeMicroseconds(PMWstall);
    ESC2.writeMicroseconds(PWMstall);
    delay(500)
    // temporarily stall the VTA
    if (error < 0)
    {
      while (abs(error) < thresh)
      {
        ESC1.writeMicroseconds(customPWM_F);
        ESC2.writeMicroseconds(customPWM_R); 
      }
    }
    else if (error > 0)
    {
      while (abs(error) < thresh)
      {
        ESC1.writeMicroseconds(customPWM_R);
        ESC2.writeMicroseconds(customPWM_F); 
      }
    } 
  }
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

  VTA_bearing = headingDegrees

  Serial.print(" Degress = ");
  Serial.print(headingDegrees);
  Serial.println();

  delay(100);
}
