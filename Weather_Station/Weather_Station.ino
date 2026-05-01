/*
Weather Station
Author: yidan xu

Description: Measures temperature and light, displays with RGB LED
Board: Arduino Mega 2560

*/

// Pin Definitions
const int TEMP_PIN = A0;
const int LIGHT_PIN = A1;
const int BUTTON_PIN = 2;
const int RED_PIN = 3;
const int GREEN_PIN = 5;
const int BLUE_PIN = 6;

// Variables
int mode = 0;  // 0=Temp, 1=Light
float temperature = 0;
int lightLevel = 0;

void setup() {
  Serial.begin(9600);
  
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  
  Serial.println("Weather Station Ready!");
}

void loop() {
  // Read sensors
  readSensors();
  
  // Check button
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(200);  // Simple debounce
    mode = 1 - mode;  // Toggle mode
    Serial.print("Mode changed to: ");
    Serial.println(mode);
  }
  
  // Update display
  updateDisplay();
  
  // Print data
  Serial.print("Temp: ");
  Serial.print(temperature);
  Serial.print("C | Light: ");
  Serial.print(lightLevel);
  Serial.print(" | Mode: ");
  Serial.println(mode);
  
  delay(500);
}

void readSensors() {
  // Read temperature (simplified)
  int tempRaw = analogRead(TEMP_PIN);
  temperature = map(tempRaw, 0, 1023, 0, 40);
  
  // Read light
  lightLevel = analogRead(LIGHT_PIN);
}

void updateDisplay() {
  if (mode == 0) {
    // Temperature mode
    if (temperature > 23) {
      setColor(255, 0, 0);    // Red - Hot
    } else if (temperature > 20) {
      setColor(0, 255, 0);    // Green - Comfortable
    } else {
      setColor(0, 0, 255);    // Blue - Cold
    }
  } else {
    // Light mode
    int brightness = map(lightLevel, 0, 1023, 0, 255);
    if (lightLevel > 700) {
      setColor(255, 255, 0);  // Yellow - Bright
    } else if (lightLevel > 300) {
      setColor(brightness, brightness, brightness); // White
    } else {
      setColor(0, 0, brightness);  // Blue - Dark
    }
  }
}

void setColor(int red, int green, int blue) {
  analogWrite(RED_PIN, red);
  analogWrite(GREEN_PIN, green);
  analogWrite(BLUE_PIN, blue);
}