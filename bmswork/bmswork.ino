#include <FastLED.h>
#define LED_PIN 10
#define NUM_LEDS 2
CRGB leds[2];
const int analogPin1 = A5;
const int analogPin2 = A4;
const int analogPin3 = A3;
const int analogPin4 = A2;
const float R1 = 33000.0;             // 33kΩ resistor
const float R2 = 12000.0;             // 12kΩ resistor
const float Vref = 5.00;              // Reference voltage of the Arduino (5V for most boards)
const int resolution = 1023;          // 10-bit ADC resolution
const float fullChargeVoltage = 4.2;  // Fully charged voltage for the battery

// Define the pins for the LEDs
const int ledPin1 = 6;
const int ledPin2 = 5;
const int ledPin3 = 8;
const int ledPin4 = 7;
const int chgPin = A0;


void setup() {
  Serial.begin(115200);  // Initialize serial communication
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  // Set LED pins as output
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);
  pinMode(ledPin4, OUTPUT);
  pinMode(chgPin, OUTPUT);
}

void loop() {
  int sensorValue2 = analogRead(analogPin1);  // Read the analog input
  int sensorValue1 = analogRead(analogPin2);  // Read the analog input
  int sensorValue3 = analogRead(analogPin3);  // Read the analog input
  int sensorValue4 = analogRead(analogPin4);  // Read the analog input

  // Convert the ADC value to voltage
  float Vout1 = (sensorValue1 * Vref) / resolution;
  float Vout2 = (sensorValue2 * Vref) / resolution;
  float Vout3 = (sensorValue3 * Vref) / resolution;
  float Vout4 = (sensorValue4 * Vref) / resolution;

  // Calculate the input voltage (battery voltage)
  float Vin4 = Vout4 * (R1 + R2) / R2;
  float Vin3 = (Vout3 * (R1 + R2) / R2);
  float Vin2 = (Vout2 * (R1 + R2) / R2);
  float Vin1 = (Vout1 * (R1 + R2) / R2);
  Vin1 = Vin1 - Vin2;
  Vin2 = Vin2 - Vin3;
  Vin3 = Vin3 - Vin4;

  if (Vin4 >= 4 && Vin3 >= 4 && Vin2 >= 4 && Vin1 >= 4) {
    digitalWrite(chgPin, LOW);
  } else {
    digitalWrite(chgPin, HIGH);
  }

  // Print the values to the serial monitor
  // Serial.print("Sensor Value1: ");
  // Serial.print(sensorValue1);

  Serial.print(" Vin1: ");
  Serial.print(Vin1);

  // Serial.print("Sensor Value2: ");
  // Serial.print(sensorValue2);

  Serial.print(" Vin2: ");
  Serial.print(Vin2);

  // Serial.print("Sensor Value3: ");
  // Serial.print(sensorValue3);

  Serial.print(" Vin3: ");
  Serial.print(Vin3);

  // Serial.print(" Sensor Value4: ");
  // Serial.print(sensorValue4);

  Serial.print(" Vin4: ");
  Serial.print(Vin4);

  // Check if each battery is fully charged
  bool allCharged = true;

  if (Vin1 > Vin2 || Vin1 > Vin3 || Vin1 > Vin4) {
    digitalWrite(ledPin2, HIGH);
    Serial.println(" vin1 is big");
  } else {
    digitalWrite(ledPin2, LOW);
    Serial.println(" vin1 is not big");
    allCharged = false;
  }

  if (Vin2 > Vin1 || Vin2 > Vin3 || Vin2 > Vin4) {
    digitalWrite(ledPin1, HIGH);
    Serial.println(" vin2 is big");
  } else {
    digitalWrite(ledPin1, LOW);
    Serial.println(" vin2 is not big");
    allCharged = false;
  }

  if (Vin3 > Vin1 || Vin3 > Vin2 || Vin3 > Vin4) {
    digitalWrite(ledPin3, HIGH);
    Serial.println(" vin3 is big");
  } else {
    digitalWrite(ledPin3, LOW);
    Serial.println(" vin3 is not big");
    allCharged = false;
  }

  if (Vin4 > Vin1 || Vin4 > Vin2 || Vin4 > Vin3) {
    digitalWrite(ledPin4, HIGH);
    Serial.println(" vin4 is big");
  } else {
    digitalWrite(ledPin4, LOW);
    Serial.println(" vin4 is not big");
    allCharged = false;
  }

  // Turn off all LEDs if all batteries are fully charged
  for (int i = 1; i <= 255; i++) {
    leds[0] = CRGB(i, i, 0);
    leds[1] = CRGB(i, 0, i);
    FastLED.show();
    delay(10);
  }
  for (int i = 255; i >= 10; i--) {
    leds[0] = CRGB(i, i, 0);
    leds[1] = CRGB(i, 0, i);
    FastLED.show();
    delay(10);
  }
  digitalWrite(ledPin1, LOW);
  digitalWrite(ledPin2, LOW);
  digitalWrite(ledPin3, LOW);
  digitalWrite(ledPin4, LOW);
  digitalWrite(chgPin, LOW);
  for (int i = 10; i >= 1; i--) {
    leds[0] = CRGB(i, i, 0);
    leds[1] = CRGB(i, 0, i);
    FastLED.show();
    delay(10);
  }
}
