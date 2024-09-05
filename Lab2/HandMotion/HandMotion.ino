#include <ESP32Servo.h>
#include <Wire.h>
#include <SparkFun_APDS9960.h>


//Servo
#define SERVO_PIN 18
Servo myservo;
int pos;


//Light sensor
SparkFun_APDS9960 apds = SparkFun_APDS9960();
volatile int isr_flag = 0;


void setup() {
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50);
  myservo.attach(SERVO_PIN, 1000, 2000);

  //start in the middle
  pos = 90;
  myservo.write(pos);

  Serial.begin(9600);
  Serial.println();
  Serial.println(F("--------------------------------"));
  Serial.println(F("SparkFun APDS-9960 - Gesture To Servo Motion"));
  Serial.println(F("--------------------------------"));


  // Initialize APDS-9960 (configure I2C and initial values)
  if (apds.init()) {
    Serial.println(F("APDS-9960 initialization complete"));
  } else {
    Serial.println(F("Something went wrong during APDS-9960 init!"));
  }

  // Start running the APDS-9960 gesture sensor engine
  if (apds.enableGestureSensor(true)) {
    Serial.println(F("Gesture sensor is now running"));
  } else {
    Serial.println(F("Something went wrong during gesture sensor init!"));
  }
}

void loop() {
  handleGesture();
}


void moveToRight() {
  for (pos; pos <= 180; pos += 2) {
    myservo.write(pos);
    delay(10);
  }
}


void moveToLeft() {
  for (pos; pos >= 0; pos -= 2) {
    myservo.write(pos);
    delay(10);
  }
}


void moveToMiddle() {
  for (pos; pos != 90; pos > 90 ? pos-- : pos++) {
    myservo.write(pos);
    delay(10);
  }
}


void handleGesture() {
  if (apds.isGestureAvailable()) {
    switch (apds.readGesture()) {
      case DIR_LEFT:
        moveToLeft();
        break;
      case DIR_RIGHT:
        moveToRight();
        break;
      default:
        moveToMiddle();
    }
  }
}