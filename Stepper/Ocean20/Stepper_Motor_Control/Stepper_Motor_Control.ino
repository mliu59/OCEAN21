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

//Defines a "States" variable that represents the various states of the machine
enum States {STARTUP, ZERO, CALIBRATE, EXPERIMENT};

States state = STARTUP;

void setup() {

  pinMode(pulsePin[x], OUTPUT);
  pinMode(pulsePin[z], OUTPUT);
  pinMode(dirPin[x], OUTPUT);
  pinMode(dirPin[z], OUTPUT);

  pinMode(endStop[x], INPUT_PULLUP);
  pinMode(endStop[z], INPUT_PULLUP);

  //Sets the pulse pins to low
  digitalWrite(pulsePin[x], LOW);
  digitalWrite(pulsePin[z], LOW);

  //Sets the direction pins to high or low such that the motors will actuate in a positive direction
  digitalWrite(dirPin[x], dir[x]);
  digitalWrite(dirPin[z], dir[z]);

  Serial.begin(9600);
}

void loop() {
  //Switch statement that allows the user to select a program "mode"
  switch (state) {
    case STARTUP:
      state = startup();
      break;
    //State that zeros the assembly
    case ZERO:
      zeroMotors();
      state = STARTUP;
      break;
    //State that enables the calibration of the load cells
    case CALIBRATE:
      Serial.println("Calibrate");
      break;
    //State that runs an experiment
    case EXPERIMENT:
      experiment();
      break;
  }
}

//Returns 0 for x and 1 for z. This makes referring to values within arrays easier.
int chooseAxis(String motor) {
  int motorNum;
  if (motor.equals("x"))
    motorNum = 0;
  else
    motorNum = 1;

  return motorNum;
}

//Switches the direction of the specified actuator
void switchDir(String motor) {
  int motorNum = chooseAxis(motor);
  dir[motorNum] = !dir[motorNum];
}

//Sends a single pulse to the specified motor
void pulse(String motor) {
  int motorNum = chooseAxis(motor);
  digitalWrite(pulsePin[motorNum], HIGH);
  delayMicroseconds(pulseDelay[motorNum]);
  digitalWrite(pulsePin[motorNum], LOW);
}

//Actuates the specified motor. Motor rotates to move the specified distance (in inches). This is a relative move rather than an absolute move.
void actuate(String motor, float distance) {
  int motorNum = chooseAxis(motor);

  //If negative distance, flip direction
  if (distance < 0) {
    switchDir(motor);
  }

  //Loop that pulses the stepper the appropriate amount
  unsigned long totalPulses = abs(distance) / leadVal[motorNum] * pulsePerRev[motorNum];
  for (int i = 0; i < totalPulses; i++) {
    pulse(motor);
  }

  //If negative distance, flip direction back to positive
  if (distance < 0) {
    switchDir(motor);
  }

  //Updates the position of the obstruction
  pos[motorNum] = pos[motorNum] + distance;
}

//Function that zeros the actuators
void zeroMotors() {
  //Changes the direction of both motors so that they move towards the home position
  switchDir("x");
  switchDir("z");
  //Pulses the x motor until the x slider triggers the sensor
  while (digitalRead(endStop[x]) == 1) {
    pulse("x");
  }

  //Changes the direction of the x motor so that the x slider moves off of the switch
  switchDir("x");
  //Pulses the motor until the x slider is off the sensor
  while (digitalRead(endStop[x]) == 0) {
    pulse("x");
  }

  //Pulses the z motor until the z slider triggers the sensor
  while (digitalRead(endStop[z]) == 1) {
    pulse("z");
  }

  //Changes the direction of the z motor so that the z slider moves off the switch
  switchDir("z");
  //Pulses the motor until the z slider is off the switch
  while (digitalRead(endStop[z]) == 1) {
    pulse("z");
  }

  pos[x] = 0;
  pos[z] = 0;
}

//Actuates the motors to go to a desired position.  This is an absolute move.
void goToPos(float posx, float posz){
  float xDist = posx - pos[x];
  float zDist = posz - pos[z];
  actuate("x", xDist);
  actuate("z", zDist);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Serial communication/user input functions

//Reads an int from the serial monitor
int readInt() {
  while (Serial.available() == 0);
  int val = Serial.parseInt();
  return val;
}

//Reads a float from the serial monitor
float readFloat() {
  while (Serial.available() == 0);
  float val = Serial.parseFloat();
  return val;
}

//Prints a bunch of lines as a crude method of clearing the screen
void clearScreen() {
  for (int i = 0; i < 15; i++) {
    Serial.println();
  }
}

//Defines what happens in the startup state
States startup() {
  int desiredState;
  Serial.println("Enter the number corresponding to the desired function");
  Serial.println("Zero actuators:\t\t[1]");
  Serial.println("Calibrate load cells:\t[2]");
  Serial.println("Start an experiment:\t[3]");

  desiredState = readInt();

  if (desiredState == 1) {
    state = ZERO;
  } else if (desiredState == 2) {
    state = CALIBRATE;
  } else if (desiredState == 3) {
    state = EXPERIMENT;
  } else {
    clearScreen();
    Serial.println("Please enter 1 - 3");
  }
  clearScreen();
  return state;
}

//Defines what happens in the experiment state
States experiment() {
  int desiredState;
  //Allows user to select two experiment types. In the first type, the start and end points, along with the step sizes, are specified.
  //In the second type, the user defines each point the obstruction should pass through.
  Serial.println("Choose an experiment type");
  Serial.println("Define start/end points:\t[1]");
  Serial.println("Define a set of points:\t\t[2]");

  desiredState = readInt();

  if (desiredState == 1) {
    startEndPointsExperiment();
    state = STARTUP;
  } else if (desiredState == 2) {

  } else {
    clearScreen();
    Serial.println("Please enter 1 or 2");
  }
}

//Asks user for start/end point experiment parameters, and conducts the experiment
void startEndPointsExperiment() {
  clearScreen();
  Serial.println("Enter start x coordinate");
  startCoords[x] = readFloat();
  Serial.println("Enter start z coordinate");
  startCoords[z] = readFloat();
  Serial.println("Enter end x coordinate");
  endCoords[x] = readFloat();
  Serial.println("Enter end z coordinate");
  endCoords[z] = readFloat();

  Serial.println("Your coordinates are:");
  Serial.print("Start: ");
  Serial.print(startCoords[x]);
  Serial.print(",");
  Serial.println(startCoords[z]);
  Serial.print("End: ");
  Serial.print(endCoords[x]);
  Serial.print(",");
  Serial.println(endCoords[z]);

  Serial.println("What is the x step size?");
  stepSize[x] = readFloat();
  Serial.println("What is the z step size?");
  stepSize[z] = readFloat();

  Serial.println("Your step sizes are:");
  Serial.print("x: ");
  Serial.println(stepSize[x]);
  Serial.print("z: ");
  Serial.println(stepSize[z]);

  float deltaX = endCoords[x] - startCoords[x];
  float deltaZ = endCoords[z] - startCoords[z];

  //Calculates the total number of x and z points
  int numXPoints = deltaX / stepSize[x] + 1;
  int numZPoints = deltaZ / stepSize[z] + 1;

  Serial.println("Num Points, x, z");
  Serial.println(numXPoints);
  Serial.println(numZPoints);
  Serial.println("Beginning Experiment");

  unsigned long t1 = millis();
  for (int i = 0; i < numZPoints; i++) {
    //On the first iteration, go to the start point
    if (i == 0) {
      goToPos(startCoords[x], startCoords[z]);
    }
    //For every other iteration, index the z actuator by one step size 
    else {
      actuate("z", stepSize[z]);
    }
    Serial.println(pos[x]);
    Serial.println(pos[z]);
    Serial.println();
    //If statements alternate the direction in the x direction every other iteration to create "zigzag" type pattern
    if (i % 2 == 0) {
      for (int j = 0; j < numXPoints - 1; j++) {
        actuate("x", stepSize[x]);
        Serial.println(pos[x]);
        Serial.println(pos[z]);
        Serial.println();
      }
    } else {
      for (int j = numXPoints - 1; j > 0; j--) {
        actuate("x", -1 * stepSize[x]);
        Serial.println(pos[x]);
        Serial.println(pos[z]);
        Serial.println();
      }
    }
  }
  unsigned long t2 = millis();
  float t_sim = (float)(t2 - t1)/1000.0;
  float t_adjusted = numXPoints * numZPoints * 90 + t_sim;
  Serial.println("t simulation");
  Serial.print(t_sim);
  Serial.println(" s");
  Serial.println("t adjusted");
  Serial.print(t_adjusted);
  Serial.println(" s");
  
}
