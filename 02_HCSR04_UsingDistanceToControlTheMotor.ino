//TMC2208 Code and Video Ref: https://www.diyengineers.com/2024/11/19/tmc-2208-super-quiet-stepper-motor-driver/
// AccelStepper: https://www.airspayce.com/mikem/arduino/AccelStepper/classAccelStepper.html

#include <AccelStepper.h>

// Define stepper pins
#define STEP_PIN 9  // Step pin
#define DIR_PIN 10  // Direction pin

// Microstepping control pins
#define MS1_PIN 7
#define MS2_PIN 8

// Steps per revolution for the motor
const float stepsPerRevolution = 200;
// Microstepping multiplier (1, 2, 4, 8, 16, or 32)
int microstepSetting = 2;  //default = 2 但轉半圈會卡住;

// AccelStepper instance in driver mode
AccelStepper stepper(AccelStepper::DRIVER, STEP_PIN, DIR_PIN);

// Input Pins and Variables
int pinButton = 6;
int buttonPressed = 0;
int buttonPressedOld = 0;
bool toRunMotor = false;

//##For the UltraSonicSensor
int trigPin = 4;          //Trig on pin 4
int echoPin = 5;          //Echo on pin 5
long duration, distance;  //設定超音波距離感測器會用到的實數
int distanceCM = 0;
int emptyCM = 40;
int approachingCM = 20;
int closeByCM = 10;

//##For the Timmer
long unsigned currentMillis = 0;
long unsigned previousMillis = 0;
long unsigned intervalTime = 2000;

//For the LED indicators
int pinLedR = 11;
int pinLedG = 13;
int pinLedB = 12;
int pinLedOld = 0;
int brightnessMax = 100;
int fadingIntervalTime = 2;
int delayFadingTime = 30;

void setup() {
  // tm1637.init();
  // tm1637.set(BRIGHT_TYPICAL);  //BRIGHT_TYPICAL = 2,BRIGHT_DARKEST= 0, BRIGHTEST = 7;
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);  // Arduino 對外送出訊號，使超音波感測器發射超音波
  pinMode(echoPin, INPUT);   // 超音波反射後，Arduino 讀入訊號，會有些微的時間差
  pinMode(pinLedR, OUTPUT);
  pinMode(pinLedG, OUTPUT);
  pinMode(pinLedB, OUTPUT);

  //##Set microstepping pins as outputs
  pinMode(MS1_PIN, OUTPUT);
  pinMode(MS2_PIN, OUTPUT);
  pinMode(pinButton, INPUT);

  // Set microstepping mode (adjust as needed: HIGH or LOW)
  digitalWrite(MS1_PIN, HIGH);  // Set to LOW or HIGH for desired microstep setting
  digitalWrite(MS2_PIN, LOW);   // Set to LOW or HIGH for desired microstep setting

  // Set the desired RPM and the max RPM
  float desiredRPM = 120;  // Set the desired speed in rpm (revolutions per minute)
  float MaxRPM = 500;      // Set max speed in rpm (revolutions per minute)

  // Calculate and set the desired and max speed in steps per second
  float speedStepsPerSec = (microstepSetting * stepsPerRevolution * desiredRPM) / 60.0;
  float Max_Speed_StepsPerSec = microstepSetting * stepsPerRevolution * MaxRPM / 60;  // Specify max speed in steps/sec (converted from RPM)
  stepper.setMaxSpeed(Max_Speed_StepsPerSec);
  stepper.setSpeed(speedStepsPerSec);
}

void loop() {
  currentMillis = millis();

  if (abs(currentMillis - previousMillis) > intervalTime) {  // Every "intervalTime" to detect if somebody was in the front of the spot

    //#1. HCSR04感測器 發射超音波 測量距離
    //Serial.print("Detect = ");
    distanceCM = detectingSomething();
    //Serial.println(distanceCM);
    brightnessMax = 150;
    if (distanceCM > emptyCM) {  //>40
      modeIndicatorLed(pinLedB);
      stepper.stop();
      toRunMotor = false;
    } else if (distanceCM > approachingCM) {  // 40 > X > 20
      brightnessMax = 100;
      modeIndicatorLed(pinLedG);
      stepper.stop();
      toRunMotor = false;
    } else {  //distanceCM < approachingCM
      modeIndicatorLed(pinLedR);
      stepper.runSpeed();
      toRunMotor = true;
    }
    Serial.print("toRunMotor = ");
    Serial.println(toRunMotor);

    previousMillis = currentMillis;
  }

  //Run the stepper motor
  if (toRunMotor) {
    stepper.runSpeed();
  }
}

void modeIndicatorLed(int ledPin) {
  if (ledPin != pinLedOld) {
    //switch off all RGB LEDs
    analogWrite(pinLedR, 0);
    analogWrite(pinLedG, 0);
    analogWrite(pinLedB, 0);
  }
  //switch on the specific LED
  analogWrite(ledPin, 100);
  pinLedOld = ledPin;
}

int detectingSomething() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);  //Waiting for the Sensor's Data
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);  // 給 Trig 高電位，持續 10微秒
  digitalWrite(trigPin, LOW);
  pinMode(echoPin, INPUT);            // 讀取 echo 的電位
  duration = pulseIn(echoPin, HIGH);  // 收到高電位時的時間

  //#2. 將感測器訊號轉換成距離
  distance = (duration / 2) / 29.1;  //超音波發射的距離為來回，因此單程距離 = 時間差 / 2  再除以  29.1 微秒，得到 cm 公分
  return distance;
}
