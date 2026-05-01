/*
  Smart Home Logic Assignment
  Author: yidan xu
  Date: 2026-03-18
* ============================================================
* Code Credits / Attribution
* ============================================================
* This code references and/or borrows from the following official examples and resources:
*
* 1. Arduino Official Example - Ping Ultrasonic Range Finder
*    - Source: https://docs.arduino.cc/built-in-examples/sensors/Ping
*    - Purpose: Implementation of the ultrasonic distance measurement function
*    - Modifications: Encapsulated into a `getDistance()` function; added distance calculation logic
*
* 2. Arduino Official Example - InputPullupSerial
*    - Source: https://docs.arduino.cc/built-in-examples/digital/InputPullupSerial
*    - Purpose: Reading button states using built-in pull-up resistors
*
* 3. Arduino Official Example - Analog Read Serial
*    - Source: https://docs.arduino.cc/built-in-examples/basics/AnalogReadSerial
*    - Purpose: Reading analog values ​​from a photoresistor (light sensor)
*
* 4. Arduino Official Example - Blink Without Delay
*    - Source: https://docs.arduino.cc/built-in-examples/digital/BlinkWithoutDelay
*    - Purpose: Implementing non-blocking timed output using `millis()`
*
* 5. Arduino Official Example - If Statement (Conditional)
*    - Source: https://docs.arduino.cc/built-in-examples/control-structures/ifStatementConditional
*    - Purpose: Logic for AND/OR/threshold-based conditional checks
*
* 6. Arduino Official Library - Servo.h
*    - Source: Arduino Standard Library
*    - Purpose: Controlling a servo motor to open/close curtains
* ============================================================
* Circuit Configuration and Component Requirements
* ============================================================
*
* [Microcontroller]
*   - Arduino Uno R3 (or other compatible boards)
*
* [Power Supply]
*   - USB Power (5V) or External Power Supply (7-12V)
*
* [Input Devices]
*
*   1. Photoresistor (Analog Sensor #1)
*      - Connection: One end to 5V, the other end to A0
*      - Voltage Divider Resistor: 10kΩ resistor connected from A0 to GND
*      - Principle: The stronger the light, the lower the resistance, and the higher the voltage at A0
*      - Reading Range: 0 (Total Darkness) ~ 1023 (Full Brightness)
*
*   2. HC-SR04 Ultrasonic Sensor (Digital Sensor, for Distance Measurement)
*      - Trig Pin: D7 (Trigger signal output)
*      - Echo Pin: D6 (Echo signal input)
*      - VCC: 5V
*      - GND: GND
*      - Principle: Transmits ultrasonic waves, calculates the echo return time, and converts it into distance
*      - Measurement Range: 2cm ~ 400cm
*
*   3. Button 1 - Manually Open Curtains (Digital Switch #1)
*      - Pin: D2
*      - Configuration: INPUT_PULLUP (Uses Arduino's internal pull-up resistor)
*      - Connection: One button terminal to GND, the other to D2
*      - Logic: Reads LOW when pressed; reads HIGH when released
*
*   4. Button 2 - Manually Close Curtains (Digital Switch #2)
*      - Pin: D3
*      - Configuration: INPUT_PULLUP
*      - Connection: One button terminal to GND, the other to D3
*      - Logic: Reads LOW when pressed; reads HIGH when released
*
* [Output Devices]
*
*   1. LED1 - Living Room Light (Output #1)
*      - Pin: D5
*      - Current-Limiting Resistor: 220Ω connected in series with either the LED's anode or cathode
*      - LED Anode (Long Leg): Connect to the resistor, then to D5; Cathode: Connect to GND
*      - Logic: HIGH = On (Bright), LOW = Off (Dim)
*
*   2. LED2 - Bedroom Light (Output #2)
*      - Pin: D4
*      - Current-limiting Resistor: 220Ω
*      - Connection method: Same as above
*
*   3. Servo Motor - Curtain Control (Output #3)
*      - Pin: D9 (PWM Pin)
*      - Power Supply: 5V (External power supply recommended to avoid overloading the Arduino)
*      - GND: GND
*      - Angle: 0° = Curtains fully open, 360° = Curtains fully closed
*/

#include <Servo.h>

Servo myServo;

// ===== Pin Definitions =====
const int LIGHT_PIN = A0;      // Photoresistor
const int TRIG_PIN = 7;        // Ultrasonic Trig
const int ECHO_PIN = 6;        // Ultrasonic Echo
const int BTN_OPEN = 2;        // Open curtain button
const int BTN_CLOSE = 3;       // Close curtain button
const int LED1 = 5;            // Living room LED
const int LED2 = 4;            // Bedroom LED
const int SERVO_PIN = 9;       // Servo motor

// ===== Threshold Values =====
const int LIGHT_HIGH = 300;     // Bright light threshold(Light intensity > 300 is regarded as "bright".)
const int LIGHT_LOW = 200;      // Dark light threshold(Light intensity < 200 is considered "dark".)
const int DISTANCE_NEAR = 10;   // Near distance threshold (cm)(Proximity Threshold: Distance < 10 cm is considered "someone approaching.")

// ===== Servo Angles =====
const int ANGLE_OPEN = 0;       // Curtain open position
const int ANGLE_CLOSE = 360;     // Curtain closed position

int currentAngle = ANGLE_CLOSE;  // Current curtain position

// Button debouncing (Purpose: Prevent multiple triggers caused by button mechanical bounce.)
unsigned long lastDebounceTime = 0;// Last button response time
const unsigned long DEBOUNCE_DELAY = 50;// Debounce delay (milliseconds)

// ===== Ultrasonic distance measurement function =====
long getDistance() {
  // Step 1: Send a 10-microsecond high-level trigger pulse
  digitalWrite(TRIG_PIN, LOW);      // Ensure the Trig pin is low
  delayMicroseconds(2);             // Wait 2 microseconds for stabilization
  digitalWrite(TRIG_PIN, HIGH);     // Send a high signal
  delayMicroseconds(10);            // Maintain for 10 microseconds
  digitalWrite(TRIG_PIN, LOW);      // End the trigger pulse
  
  // Step 2: Read the duration of the HIGH pulse on the Echo pin
  // The pulseIn() function waits for the pin to go HIGH, then measures the time until it goes LOW
  long time = pulseIn(ECHO_PIN, HIGH);  // Returns time (in microseconds)

  // Step 3: Calculate the distance based on the speed of sound
  // The speed of sound is approximately 340 meters/second = 0.034 cm/microsecond
  // Distance = Time × Speed ​​of Sound ÷ 2 (for the round trip)
  long distance = time * 0.034 / 2;

  return distance;
  }

void setup() {
  // Initialize serial communication at 9600 baud rate for debugging output
  Serial.begin(9600);
  Serial.println("=== Smart Home System Started ===");

  // ----- Initialize Servo Motor -----
  myServo.attach(SERVO_PIN);      // Connect the servo motor to the specified pin
  myServo.write(ANGLE_CLOSE);     // Set the initial position to "closed"
  currentAngle = ANGLE_CLOSE;     // Record the current angle

  // ----- Initialize Buttons (using built-in pull-up resistors) -----
  // INPUT_PULLUP Mode: Pin is HIGH when not pressed; becomes LOW when pressed (connected to ground)
  pinMode(BTN_OPEN, INPUT_PULLUP);
  pinMode(BTN_CLOSE, INPUT_PULLUP);

  // ----- Initialize LED Pins as Outputs -----
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);

  // ----- Initialize Ultrasonic Sensor Pins -----
  pinMode(TRIG_PIN, OUTPUT);      // Trig: Output trigger signal
  pinMode(ECHO_PIN, INPUT);       // Echo: Input echo signal

  // ----- Set Initial State -----
  digitalWrite(LED1, LOW);        // Living room light initially OFF
  digitalWrite(LED2, LOW);        // Bedroom light initially OFF

  // Output system startup completion message
  Serial.println("System Ready");
  Serial.println("Commands: o=open curtain, c=close curtain, 1=Led ON, 2=Led OFF");
  }

void loop() {
  // ===== 1. Read all sensors =====
  int lightValue = analogRead(LIGHT_PIN);      // Light level 0-1023
  long distance = getDistance();                // Distance in cm
  int btnOpenState = digitalRead(BTN_OPEN);    // Open button (LOW = pressed)
  int btnCloseState = digitalRead(BTN_CLOSE);  // Close button (LOW = pressed)
  
  // ===== 2. Manual button control  =====
  if (btnOpenState == LOW) {                   // Detected that the "Open Curtain" button was pressed
  myServo.write(ANGLE_OPEN);                   // Control the servo to rotate to the open angle
  currentAngle = ANGLE_OPEN;                   // Update the current angle record
  Serial.println("Manual: Curtain OPEN");      // Output operation log to the serial monitor
  delay(200);                                  // Simple debouncing: Delay 200ms to prevent accidental double-triggering
  }
  else if (btnCloseState == LOW) {             // Detected that the "Close Curtain" button was pressed
  myServo.write(ANGLE_CLOSE);                  // Control the servo to rotate to the closed angle
  currentAngle = ANGLE_CLOSE;                  // Update the current angle record
  Serial.println("Manual: Curtain CLOSED");
  delay(200);                                  // Simple debouncing delay
  }
  
  // ===== 3. Threshold Logic =====
  // If distance is less than threshold, turn on bedroom LED
  if (distance < DISTANCE_NEAR && distance > 0) {       // Distance is below the threshold and valid
  digitalWrite(LED2, HIGH);                            // Turn on the bedroom light
  Serial.println("Threshold Logic: Object NEAR -> Bedroom LED ON");
  } else {
  digitalWrite(LED2, LOW);                            // No object detected nearby; turn off the bedroom light
  }
  
  // ===== 4. AND Logic (Bright AND Near) =====
  if (lightValue > LIGHT_HIGH && distance < DISTANCE_NEAR) {
  // Send the command only if the curtains are not already closed, to avoid redundant control actions.
  if (currentAngle != ANGLE_CLOSE) {
  myServo.write(ANGLE_CLOSE);      // Close the curtains.
  currentAngle = ANGLE_CLOSE;      // Update the state record.
  }
  digitalWrite(LED1, HIGH);          // Turn on the living room light.

  // Output detailed debugging information, including sensor values.
  Serial.print("AND Logic: Light HIGH(");
  Serial.print(lightValue);
  Serial.print(") AND Near(");
  Serial.print(distance);
  Serial.println("cm) -> Curtain CLOSED + Living LED ON");
  }
  // In the actual implementation, the living room light is turned off by other logic;
  // it is not handled here to avoid potential conflicts.
  
  // ===== 5. OR Logic (Dark OR Open Button) =====
// Send command only if the curtains are not already open
    if (currentAngle != ANGLE_OPEN) {
    myServo.write(ANGLE_OPEN);       // Open the curtains
    currentAngle = ANGLE_OPEN;       // Update status record
    }
      Serial.print("OR Logic: Light LOW(");
      Serial.print(lightValue);
      Serial.println(") -> Curtain OPEN");
  
  // ===== 6. Serial Commands =====
  if (Serial.available() > 0) {          // Check if serial data is available
  char cmd = Serial.read();            // Read a single character

  if (cmd == 'o' || cmd == 'O') {      // Command to open curtains
  myServo.write(ANGLE_OPEN);
  currentAngle = ANGLE_OPEN;
  Serial.println("Serial Command: Curtain OPEN");
  }
  if (cmd == 'c' || cmd == 'C') {      // Command to close curtains
  myServo.write(ANGLE_CLOSE);
  currentAngle = ANGLE_CLOSE;
  Serial.println("Serial Command: Curtain CLOSED");
  }
  if (cmd == '1') {                    // Command to turn on living room light
  digitalWrite(LED1, HIGH);
  Serial.println("Serial Command: Living LED ON");
  }
  if (cmd == '2') {                    // Command to turn off living room light
  digitalWrite(LED1, LOW);
  Serial.println("Serial Command: Living LED OFF");
  }
  }
    
  // ===== 7. Serial Monitor Output (every 3 seconds) =====
  // ===== Periodic Status Printing =====
  // Add this line above setup()
  unsigned long lastPrintTime = 0;

  // Add this block inside loop()
  unsigned long now = millis();  // Get current time (in milliseconds)

  if (now - lastPrintTime >= 10000) {  // If more than 10 seconds have passed since the last print
  lastPrintTime = now;  // Update the print time

  // Print separator line
  Serial.println("=== SYSTEM STATUS ===");

  // Print light value
  Serial.print("Light: ");
  Serial.println(lightValue);

  // Print distance
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // Print curtain status
  Serial.print("Curtain: ");
  if (currentAngle == ANGLE_OPEN) {
  Serial.println("OPEN");
  } else {
  Serial.println("CLOSED");
  }

  // Print living room light status
  Serial.print("Living LED: ");
  if (digitalRead(LED1) == HIGH) {
  Serial.println("ON");
  } else {
  Serial.println("OFF");
  }

  // Print bedroom light status
  Serial.print("Bedroom LED: ");
  if (digitalRead(LED2) == HIGH) {
  Serial.println("ON");
  } else {
  Serial.println("OFF");
  }

  // Print closing line
  Serial.println("=====================");

  }
    delay(1000);
  }