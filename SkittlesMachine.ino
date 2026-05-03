/*
  Skittles Color Sorting Machine
  Author: Rexo Ludus

  This project was created as part of an educational experience for a group of 10-year-old students 
  in primary school. The goal was to spark their curiosity about technology and show how electronics, 
  coding, and 3D design can be combined into a fun and working machine.

  The idea was to solve a 'real-world problem': sorting Skittles candy by color. Of course, the kids 
  were quick to point out that they could easily sort the candy by hand — and much faster!
  But once they saw the motors, the sensors, and the mechanical arms in action, they were hooked.

  This project includes:
  - Arduino Uno R3 control code
  - 3D printed mechanical parts
  - A PCA9685 servo driver
  - A TCS3200 color sensor
  - Stepper motor turntable
  - Educational value: priceless

  I hope this inspires others to build, share, and promote technical education —
  especially for young minds who are just discovering how awesome engineering can be.

  Feel free to build upon this, remix it, and improve it — but most of all: share it with the next generation.
  De link to the 3D design: https://makerworld.com/en/models/1481101-skittlers-sorter#profileId-1546502

  #MoreKidsInTech 
*/

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

#define stepSize 10
#define ServoBuffer 0
#define Servo1 3
#define Servo2 5
#define Servo3 6
#define Servo4 2
#define Servo5 4
#define Servo6 1
#define StepDirPin 2
#define StepStepPin 3
#define StepEnablePin 4

#define CamSwitchPin 5
#define BOUNCE_CHECK_MS 5
#define STEP_DELAY_US 500
#define IGNORE_STEPS 150

#define S0 8
#define S1 9
#define S2 10
#define S3 11
#define sensorOut 12
#define ledPin 13
#define BUTTONStop 6
#define BUTTONStart 7

bool isRunning = false;
bool previousButtonState = HIGH;
unsigned long lastToggleTime = 0;
const unsigned long debounceDelay = 200;
int red = 0;
int green = 0;
int blue = 0;
int turnCount = 0;
String colorRegister[7] = { "", "", "", "", "", "", "" };

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

int servoMin[7] = {1100, 830, 750, 850, 900, 900, 850};
int servoMax[7] = {1600, 1500, 1500, 1500, 1500, 1500, 1500};
int servos[7] = { 0, 3, 5, 6, 2, 4, 1 };

String colorNames[7] = { "", "Purple", "Red", "Green", "Yellow", "Orange", "Black" };
int colorToPosition(String color) {
  if (color == "Purple") return 1;
  if (color == "Red") return 2;
  if (color == "Green") return 3;
  if (color == "Yellow") return 4;
  if (color == "Orange") return 5;
  if (color == "Black" || color == "-") return 6;
  return -1;
}

void setup() {
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(BUTTONStart, INPUT_PULLUP);
  pinMode(BUTTONStop, INPUT_PULLUP);

  digitalWrite(ledPin, LOW);
  digitalWrite(S0, HIGH);
  digitalWrite(S1, HIGH);

  pinMode(CamSwitchPin, INPUT_PULLUP);
  Serial.begin(9600);
  pwm.begin();
  pwm.setPWMFreq(60);
  TurnOffServos();

  pinMode(StepDirPin, OUTPUT);
  pinMode(StepStepPin, OUTPUT);
  pinMode(StepEnablePin, OUTPUT);
  digitalWrite(StepEnablePin, HIGH);

  for (int i = 0; i < 7; i++) {
    colorRegister[i] = "";
  }
  Serial.println("Color register cleared.");
  Serial.println("Setup finished, start resetting table...");
  //reset();
}

void loop() {
  bool buttonState = digitalRead(BUTTONStart);

  if (buttonState == LOW && previousButtonState == HIGH && millis() - lastToggleTime > debounceDelay) {
    isRunning = !isRunning;
    lastToggleTime = millis();
    Serial.print("Sorting machine status: ");
    Serial.println(isRunning ? "STARTED" : "STOPPED");
  }

  previousButtonState = buttonState;

  if (isRunning) {
    Turntable();
    ColorCheck();
    ColorServo();
  }

  if (digitalRead(BUTTONStop) == LOW) {
    reset();
    isRunning = false;
    Serial.println("Machine reset and stopped.");
  }

  SerialCommands();
  delay(10);
}

void SerialCommands() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.toLowerCase();
    command.trim();

    if (command.startsWith("test")) {
      int firstSpaceIndex = command.indexOf(' ');
      int secondSpaceIndex = command.indexOf(' ', firstSpaceIndex + 1);
      int servoNum, testCount = 1;

      if (firstSpaceIndex != -1) {
        servoNum = command.substring(firstSpaceIndex + 1, secondSpaceIndex != -1 ? secondSpaceIndex : command.length()).toInt();
        if (secondSpaceIndex != -1) {
          testCount = command.substring(secondSpaceIndex + 1).toInt();
          testCount = constrain(testCount, 1, 100);
        }

        if (servoNum >= 0 && servoNum < 7) {
          Serial.print("Starting test for servo ");
          Serial.print(servoNum);
          Serial.print(" for ");
          Serial.print(testCount);
          Serial.println(" times.");
          for (int i = 0; i < testCount; i++) {
            RunServo(servoNum);
          }
        } else {
          Serial.println("Servo number out of range.");
        }
      }
    } else if (command.startsWith("validate")) {
      Serial.println("Validating color 25 times...");
      for (int steps = 0; steps < 25; steps++) {
        Turntable();
        delay(100);
        ColorCheck();
      }
    } else if (command.startsWith("turn")) {
      Turntable();
    } else {
      Serial.println("Invalid command.");
    }
  }
}

void TurnOffServos() {
  for (int servoNum = 0; servoNum <= 7; servoNum++) {
    pwm.setPWM(servoNum, 0, 0);
  }
}

void reset() {
  for (int i = 0; i < 7; i++) {
    colorRegister[i] = "";
  }

  Serial.println("Open trapdoor 6...");
  OpenServo(servos[6]);
  Serial.println("Resetting table: ");
  for (int steps = 0; steps < 9; steps++) {
    Turntable();
  }
  Serial.println("Resetting Servos: ");
  for (int i = 1; i < 7; i++) {
    int servoChannel = servos[i];
    RunServo(servoChannel);
  }
  Serial.println("Testing Buffer: ");
  buffer(true);
  buffer(false);
  turnCount = 0;
}

void buffer(bool left) {
  int direction = left ? 1800 : 1200;
  pwm.writeMicroseconds(ServoBuffer, direction);
  delay(1000);
  TurnOffServos();
}

void RunServo(int servoNum) {
  int minPulse = servoMin[servoNum];
  int maxPulse = servoMax[servoNum];
  for (uint16_t microsec = minPulse; microsec < maxPulse; microsec += stepSize) {
    pwm.writeMicroseconds(servoNum, microsec);
  }
  delay(150);
  for (uint16_t microsec = maxPulse; microsec > minPulse; microsec -= stepSize) {
    pwm.writeMicroseconds(servoNum, microsec);
  }
  delay(10);
  TurnOffServos();
}

void OpenServo(int servoNum) {
  int minPulse = servoMin[servoNum];
  int maxPulse = servoMax[servoNum];
  for (uint16_t microsec = minPulse; microsec < maxPulse; microsec += stepSize) {
    pwm.writeMicroseconds(servoNum, microsec);
  }
  TurnOffServos();
}

void Turntable() {
  delay(100);
  digitalWrite(StepEnablePin, LOW);
  digitalWrite(StepDirPin, false);

  int bufferPulse = (turnCount / 2) % 2 == 0 ? 1800 : 1200;
  turnCount++;

  for (int stepCount = 0; stepCount < IGNORE_STEPS; stepCount++) {
    digitalWrite(StepStepPin, HIGH);
    pwm.writeMicroseconds(ServoBuffer, bufferPulse);
    delayMicroseconds(STEP_DELAY_US);
    digitalWrite(StepStepPin, LOW);
    delayMicroseconds(STEP_DELAY_US);
  }

  while (digitalRead(CamSwitchPin) == HIGH) {
    digitalWrite(StepStepPin, HIGH);
    pwm.writeMicroseconds(ServoBuffer, bufferPulse);
    delayMicroseconds(STEP_DELAY_US);
    digitalWrite(StepStepPin, LOW);
    delayMicroseconds(STEP_DELAY_US);
  }

  digitalWrite(StepEnablePin, HIGH);
  pwm.setPWM(ServoBuffer, 0, 0);

  for (int i = 6; i > 0; i--) {
    colorRegister[i] = colorRegister[i - 1];
  }
}

void ColorServo() {
  for (int i = 1; i < 7; i++) {
    String color = colorRegister[i];
    int desiredPosition = colorToPosition(color);

    if (desiredPosition == i) {
      RunServo(servos[i]);
      Serial.print("\u2192 Color "); Serial.print(color);
      Serial.print(" activated at position "); Serial.println(i);
    }
  }

  Serial.print("Color Register: ");
  for (int i = 0; i < 7; i++) {
    Serial.print(colorRegister[i]); Serial.print(" ");
  }
  Serial.println();
}

void ColorCheck() {
  digitalWrite(ledPin, HIGH);
  delay(250);
  digitalWrite(S2, LOW); digitalWrite(S3, LOW);
  int r = pulseIn(sensorOut, LOW);
  digitalWrite(S2, HIGH); digitalWrite(S3, HIGH);
  int g = pulseIn(sensorOut, LOW);
  digitalWrite(S2, LOW); digitalWrite(S3, HIGH);
  int b = pulseIn(sensorOut, LOW);
  digitalWrite(ledPin, LOW);

  colorRegister[0] = detectColor(r, g, b);
  Serial.print("Red: "); Serial.print(r);
  Serial.print("  Green: "); Serial.print(g);
  Serial.print("  Blue: "); Serial.print(b);
  Serial.print("  --> Detected as: ");
  Serial.println(colorRegister[0]);
}

String detectColor(int r, int g, int b) {
  if (r >= 18 && r <= 22 && g >= 25 && g <= 35 && b >= 25 && b <= 28) return "Red";
  if (r >= 10 && r <= 14 && g >= 9 && g <= 17 && b >= 17 && b <= 20) return "Yellow";
  if (r >= 15 && r <= 23 && g >= 14 && g <= 22 && b >= 15 && b <= 24) return "Green";
  if (r >= 13 && r <= 16 && g >= 23 && g <= 27 && b >= 18 && b <= 25) return "Orange";
  if (r >= 21 && r <= 32 && g >= 25 && g <= 37 && b >= 20 && b <= 30) return "Purple";
  if (r >= 29 && r <= 31 && g >= 30 && g <= 31 && b >= 25 && b <= 26) return "-";
  return "Unknown";
}
