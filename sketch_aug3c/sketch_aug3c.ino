#include <FastLED.h>
#include <math.h>
#define LED_PIN 10
#define NUM_LEDS 2
CRGB leds[NUM_LEDS];
bool fullcharge = false;

const int analogPins[] = { A5, A4, A3, A2 };
const int ledPins[] = { 6, 5, 8, 7 };
float Vin[4] = { 0, 0, 0, 0 };
const int chgPin = A0;
const int buttonPin = 3;  // Interrupt button pin


const float SR[4] = { 33300.0, 33500.0, 33400.0, 32600.0 };  // Series resistors
const float PR[4] = { 11900.0, 11800.0, 12000.0, 11600.0 };  // Parallel resistors
const float Vref = 4.76;                                     // Reference voltage of the Arduino (5V for most boards)
const int resolution = 1023;                                 // 10-bit ADC resolution
const float fullChargeVoltage = 4.1;                         // Fully charged voltage for the battery
const float storageVoltage = 3.5;
volatile bool buttonPressed = true;
#define DEBOUNCE_DELAY 50
bool allBatteriesFull = true;  // Initialize a flag to check if all batteries are full
int num = 0;
bool change = false;

void setup() {
  Serial.begin(115200);  // Initialize serial communication
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);

  for (int i = 0; i < 4; i++) {
    pinMode(ledPins[i], OUTPUT);
  }
  pinMode(chgPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonISR, FALLING);
}

void loop() {
  update_voltage(Vin);
  if (buttonPressed) {
    handleBatteryLevels(Vin, storageVoltage);
    Serial.println("Storage Mode");
    handleLED(0);
  } else {
    handleBatteryLevels(Vin, fullChargeVoltage);
    Serial.println("Charging Mode");
    handleLED(1);
  }


  for (int i = 0; i < 4; i++) {
    Vin[i] = 0;  // Resetting the values
  }

  delay(80);
}

void buttonISR() {
  Serial.println("Button ISR triggered");

  static unsigned long lastInterruptTime = 0;
  unsigned long interruptTime = millis();
  if (interruptTime - lastInterruptTime > DEBOUNCE_DELAY) {
    buttonPressed = !buttonPressed;
    change = true;
  }
  lastInterruptTime = interruptTime;
  if (buttonPressed) {
    setLEDColor(0, 255);
  }
  if (!buttonPressed) {
    setLEDColor(1, 255);
  }
}

void update_voltage(float* Vin) {
  for (int j = 0; j < 100; j++) {
    for (int i = 0; i < 4; i++) {
      Vin[i] += readBatteryVoltage(analogPins[i], i);
    }
    delay(0);
  }

  for (int i = 0; i < 4; i++) {
    Vin[i] /= 100.0;
  }

  for (int i = 0; i < 3; i++) {  // for calculating individual cell voltage
    Vin[i] -= Vin[i + 1];
  }
}

float readBatteryVoltage(int pin, int index) {
  int sensorValue = analogRead(pin);
  float Vout = (sensorValue * Vref) / resolution;
  return Vout * (SR[index] + PR[index]) / PR[index];
}

void handleBatteryLevels(float* Vin, const float Battery_level) {
  allBatteriesFull = true;
  for (int i = 0; i < 4; i++) {
    if (Vin[i] >= Battery_level) {
      digitalWrite(ledPins[i], HIGH);
    } else {
      digitalWrite(ledPins[i], LOW);
      allBatteriesFull = false;  // If any battery is not full, set the flag to false
    }
  }

  for (int i = 0; i < 4; i++) {
    if (Vin[i] > Battery_level) {
      digitalWrite(chgPin, LOW);
    } else {
      digitalWrite(chgPin, HIGH);
    }
  }

  if (allBatteriesFull) {
    Serial.println("All batteries are fully charged!");
  }
}

void handleLED(bool mode) {
  if (!allBatteriesFull) {
    static uint8_t brightness = 0;  // LED brightness level (0-255)
    Serial.print("Animating LED for mode: ");
    Serial.println(mode);
    for (int i = 1; i <= 200; i++) {
      brightness = 127.5 * (1 + sin(millis() / 1000.0 * PI));
      setLEDColor(mode, brightness);
      FastLED.show();
      delay(10);
    }
  } else {
    setLEDColor(mode, 255);
    checkdelay(3000);
  }
}

void checkdelay(int time) {
  for (int i = 0; i < (time / 10); i++) {
    if (change) {
      break;
    } else {
      delay(10);
    }
  }
}

void setLEDColor(bool mode, int brightness) {
  if (!mode) {
    leds[0] = CRGB(brightness, brightness, 0);
    leds[1] = CRGB(0, 0, 0);
    FastLED.show();
  } else {
    leds[1] = CRGB(brightness, 0, brightness);
    leds[0] = CRGB(0, 0, 0);
    FastLED.show();
  }
}