//Stores the values for the pulse, direction, and endstop pins in three arrays
//The first element is for the x actuator, and the second is for the z actuator
int pulsePin[] = {22, 23};
int dirPin[] = {24, 25};
int endStop[] = {26, 27};

//Variables to make referring to each index in various arrays more intuitive
int x = 0;
int z = 1;

//Defines pulses per revolution for each stepper motor
int pulsePerRev[] = {400, 8000};

//Defines the lead of the x and z screw in inches
float leadVal[] = {.1, .1};

//Defines the delay between pulses in microseconds
int pulseDelay[] = {40, 0};

//Defines variable to store the coordinates of the obstruction
float pos[] = {0, 0};

//Defines variable to store the direction of each actuator
bool dir[] = {false, true};

//Defines the step size in inches for each motor. This is the default step size for an experiment
float stepSize[] = {4, 4};

//Defines the start and end coordinates corresponding to a default experiment in which the obstruction sweeps a 64 inch by 24 inch rectangle
float startCoords[] = {0, 0};
float endCoords[] = {64, 24};

float inchDistance = 12 * 5.0;

void setup() {
  pinMode(pulsePin[x], OUTPUT);
  pinMode(dirPin[x], OUTPUT);
  pinMode(endStop[x], INPUT_PULLUP);
  digitalWrite(pulsePin[x], LOW);
  digitalWrite(dirPin[x], dir[x]);
  Serial.begin(9600);
  delay(5000);
  actuate(inchDistance);
}

void loop() {
}

void actuate(float distance) {
  int motorNum = 0;

  //If negative distance, flip direction
  if (distance < 0) {
    dir[motorNum] = !dir[motorNum];
  }

  //Loop that pulses the stepper the appropriate amount
  unsigned long totalPulses = abs(distance) / leadVal[motorNum] * pulsePerRev[motorNum];
  for (int i = 0; i < totalPulses; i++) {
    digitalWrite(pulsePin[motorNum], HIGH);
    delayMicroseconds(pulseDelay[motorNum]);
    digitalWrite(pulsePin[motorNum], LOW);
  }

  //If negative distance, flip direction back to positive
  if (distance < 0) {
    dir[motorNum] = !dir[motorNum];
  }

  //Updates the position of the obstruction
  pos[motorNum] = pos[motorNum] + distance;
}
