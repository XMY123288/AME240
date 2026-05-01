/*
* AME 240 - Kinetic Light Sculpture
* Author: Yidan Xu
* Date: April 16, 2026
*
* Project Description:
* The cat sculpture rotates left and right (90 degrees in each direction). The LED strip changes its "breathing" speed and color based on the direction the cat is facing:
* - Facing Left: Blue, slow breathing (Calm)
* - Facing Forward: Pink, medium-speed breathing (Alert)
* - Facing Right: Orange, fast breathing (Excited)
*
* Circuit Connections:
* - Stepper Motor Driver Board (ULN2003):
*   IN1 → Arduino Pin 8
*   IN2 → Arduino Pin 9
*   IN3 → Arduino Pin 10
*   IN4 → Arduino Pin 11
*   +   → External 5V Power Supply
*   -   → External GND
* - DotStar LED Strip (APA102):
*   DATA → Arduino Pin 51
*   CLOCK → Arduino Pin 52
*   VCC → External 5V Power Supply
*   GND → External GND
* - Common Ground: Connect the external power supply's GND to the Arduino's GND.
*
* Hardware Requirements:
* - Arduino Mega 2560
* - 28BYJ-48 Stepper Motor + ULN2003 Driver Board
* - DotStar LED Strip (14 LEDs)
* - External 5V Power Supply
*
* References:
* - FastLED Library Examples (LED Control)
* - Arduino Stepper Library Examples (Motor Control)
* - Referenced the official examples provided by the libraries listed above. 
*/
#include <FastLED.h>
#include <Stepper.h>

// LED Strip Settings
#define DATA_PIN 51
#define CLOCK_PIN 52
#define NUM_LEDS 14

// Stepper Motor Settings (2048 steps = 1 full revolution)
Stepper motor(2048, 8, 9, 10, 11);

CRGB leds[NUM_LEDS];

int stepCount = 0;      // Tracks the total number of steps taken
int direction = 1;      // 1 = Right turn, -1 = Left turn

// "Breathing" Light Effect Variables
float bright = 0;
int fade = 5;
int zone = 1;           // 0 = Left, 1 = Center, 2 = Right

void setup() {
FastLED.addLeds<DOTSTAR, 51, 52, BGR>(leds, NUM_LEDS);
motor.setSpeed(15);
Serial.begin(9600);
}

void loop() {
// Move the motor one step
motor.step(direction);
stepCount = stepCount + direction;

// Reverse direction once 90 degrees of rotation is reached
if (stepCount >= 512) {
direction = -1;
Serial.println("Turning Left");
} else if (stepCount <= -512) {
direction = 1;
Serial.println("Turning Right");
}

// Determine the current facing direction/zone
if (stepCount < -256) {
zone = 0;  // Facing Left
} else if (stepCount > 256) {
zone = 2;  // Facing Right
} else {
zone = 1;  // Facing Forward (Center)
}

// Breathing effect logic
bright = bright + fade;
if (bright >= 255 || bright <= 0) {
fade = -fade;
}

// Select color based on direction/zone
int r, g, b;
if (zone == 0) {      // Blue
r = 50 * (bright / 255);
g = 100 * (bright / 255);
b = 200 * (bright / 255);
} else if (zone == 2) { // Orange
r = 255 * (bright / 255);
g = 100 * (bright / 255);
b = 50 * (bright / 255);
} else {              // Pink
r = 255 * (bright / 255);
g = 180 * (bright / 255);
b = 200 * (bright / 255);
}

for (int i = 0; i < NUM_LEDS; i++) {
leds[i] = CRGB(r, g, b);
}
FastLED.show();

delay(10);  // Advance one step every 10ms
}