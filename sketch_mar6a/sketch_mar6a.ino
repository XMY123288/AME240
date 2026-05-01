#include <Servo.h>

Servo myServo;

const int servoPin = 9;
const int ldrPin = A0;
const int btnOpen = 2;
const int btnClose = 3;

// Adjust these angles after testing
const int openAngle = 0;      // Curtain fully open
const int closeAngle = 360;    // Curtain fully closed

int currentAngle = closeAngle;
unsigned long lastDebounce = 0;
const long debounceDelay = 50;

void setup() {
  Serial.begin(9600);
  
  myServo.attach(servoPin);
  pinMode(btnOpen, INPUT_PULLUP);
  pinMode(btnClose, INPUT_PULLUP);
  
  myServo.write(closeAngle);
  currentAngle = closeAngle;
  
  Serial.println("Treehouse Curtain System Started");
  Serial.println("Commands: o=open, c=close");
}

void loop() {
  // Read sensors
  int light = analogRead(ldrPin);
  int openState = digitalRead(btnOpen);
  int closeState = digitalRead(btnClose);
  
  // Button control
  if (openState == HIGH && millis() - lastDebounce > debounceDelay) {
    smoothMove(openAngle);
    Serial.println("Manual: OPEN");
    lastDebounce = millis();
  }
  if (closeState == HIGH && millis() - lastDebounce > debounceDelay) {
    smoothMove(closeAngle);
    Serial.println("Manual: CLOSE");
    lastDebounce = millis();
  }
  
  // Auto light control (3 seconds delay after manual)
  if (millis() - lastDebounce > 3000) {
    // Adjust threshold (500) based on your light readings
    if (light > 350 && currentAngle != closeAngle) {
      smoothMove(closeAngle);
      Serial.print("Light HIGH (");
      Serial.print(light);
      Serial.println(") Auto CLOSE");
    }
    if (light <= 350 && currentAngle != openAngle) {
      smoothMove(openAngle);
      Serial.print("Light LOW (");
      Serial.print(light);
      Serial.println(") Auto OPEN");
    }
  }
  
  // Serial output every second
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 1000) {
    Serial.print("Light: ");
    Serial.print(light);
    Serial.print(" | Curtain: ");
    Serial.println(currentAngle == openAngle ? "OPEN" : "CLOSED");
    lastPrint = millis();
  }
  
  // Check serial commands
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    if (cmd == 'o' || cmd == 'O') {
      smoothMove(openAngle);
      Serial.println("Serial: OPEN");
    }
    if (cmd == 'c' || cmd == 'C') {
      smoothMove(closeAngle);
      Serial.println("Serial: CLOSE");
    }
  }
  
  delay(100);
}

// Smooth movement function
void smoothMove(int target) {
  int step = (target > currentAngle) ? 1 : -1;
  for (int a = currentAngle; a != target; a += step) {
    myServo.write(a);
    delay(10);
  }
  myServo.write(target);
  currentAngle = target;
}