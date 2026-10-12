#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"

Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 

Adafruit_DCMotor *myMotor = AFMS.getMotor(1);

void setup() {
  Serial.begin(9600);
  AFMS.begin();
  // put your setup code here, to run once:
  Adafruit_DCMotor *DCMotor = AFMS.getMotor(1);
  Adafruit_DCMotor *frontMotor = AFMS.getMotor(2);
  DCMotor->setSpeed(200);
  DCMotor->run(RELEASE);
}

void loop() {
  // Spin forward
  // myMotor->run(FORWARD);
  // delay(100);
  // myMotor->run(RELEASE);
  // delay(1000);
}