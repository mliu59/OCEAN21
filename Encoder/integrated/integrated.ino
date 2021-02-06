#include <Encoder.h>
#define MAX_POS_ANGLE 150
//max angle on one side of the midpoint (total range of motion is 2 * MAX_POS_ANGLE)
#define PULSE_PER_REV 1024
//^^ this could be wrong, might be 2048 instead. Amazon page has conflicting information

Encoder enc(2, 3);
long maxPos = -PULSE_PER_REV;
long minPos = PULSE_PER_REV;
long newPos = -PULSE_PER_REV;
long midPos = 0;
long angle = 0; 

void setup() {
  Serial.begin(9600);
  Serial.println("Basic Encoder Test:");
}

void loop() {
  newPos = enc.read();
  if (newPos > maxPos) {
    maxPos = newPos;
  }
  if (newPos < minPos) {
    minPos = newPos;
  }
  calcMid();
  angle = map(newPos, minPos, maxPos, MAX_POS_ANGLE, -MAX_POS_ANGLE);
  Serial.println(angle);
}

void calcMid() {
  midPos = (maxPos + minPos) / 2;
}
