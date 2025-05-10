#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif
#include <avr/wdt.h>
#include <math.h>

#define LED_PIN 3
#define NUM_LEDS 2
#define DEBOUNCE_DELAY 50
#define RESOLUTION 1023
#define Storage 0
#define Charging 1
#define Standby 3

int delay_time = 10;
bool fullCharge = false;
int pixelFormat = NEO_GRB + NEO_KHZ800;

// Voltage thresholds for different charge states
// uint16_t FULL_CHARGE_VOLTAGE[4] = { 817, 907, 916, 0 e fff
};  //a = 784, b = 873, c = 883, d = 519 // for new charger pcb
// uint16_t STORAGE_VOLTAGE[4] = { 731, 811, 819, 0 };
uint16_t FULL_CHARGE_VOLTAGE[4] = { 891, 998, 957, 0 };
uint16_t STORAGE_VOLTAGE[4] = { 797, 891, 856, 0 };
uint16_t variation[4] = { 4, 2, 2, 2 };

// Pin assignments
const int analogPins[] = { A5, A4, A3, A2 };
const int dumpPin[] = { 8, 7, 6, 5 };
const int chgPin = A0;
const int buttonPin = 10;

// Battery status variables
uint16_t Vin[4] = { 0 }, Vlast[4] = { 0 }, Vlast1[4] = { 0 };
bool battery_detected = true, battery_changed = false, allBatteriesFull = true, charge_done[4] = { false };
bool ready = false, change = false, almost_full = false;

Adafruit_NeoPixel* pixels;
volatile bool buttonPressed = true, toggleFlag = true;
volatile long startInterruptTime = 0, lastInterruptTime = 0;

void setup() {
  // Check if Watchdog Timer (WDT) caused a reset, and disable it if so
  if (MCUSR & _BV(WDRF)) {
    MCUSR &= ~_BV(WDRF);
    wdt_disable();
  }

  Serial.begin(115200);
  update_battery_level(Vin);  // Initialize battery voltage readings

#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif

  // Initialize NeoPixel LEDs
  pixels = new Adafruit_NeoPixel(NUM_LEDS, LED_PIN, pixelFormat);
  pixels->begin();
  pixels->setPixelColor(0, pixels->Color(100, 100, 0));
  pixels->show();

  // Set pin modes for battery discharge and charging control
  for (int i = 0; i < 4; i++) pinMode(dumpPin[i], OUTPUT);
  pinMode(chgPin, OUTPUT);
  pinMode(buttonPin, INPUT);

  digitalWrite(chgPin, LOW);
  delay(1000);

  // Enable pin change interrupt for button press detection
  PCICR |= (1 << PCIE0);
  PCMSK0 |= (1 << PCINT2);
}

void loop() {
  wdt_reset();      // Prevent watchdog timer reset
  pixels->clear();  // Clear LED display

  if (almost_full) {
    // Stop charging when batteries are nearly full
    digitalWrite(chgPin, LOW);
    for (int i = 0; i < 3; i++) digitalWrite(dumpPin[i], LOW);
    delay(50);
  }

  update_voltage(Vin);  // Read and update battery voltage levels

  if (battery_detected) {
    if (!buttonPressed) {
      handleBatteryLevels(Vin, STORAGE_VOLTAGE, Storage);
      Serial.println("Storage Mode");
    } else {
      handleBatteryLevels(Vin, FULL_CHARGE_VOLTAGE, Charging);
      Serial.println("Charging Mode");
    }
  } else {
    // If no battery is detected, go into standby mode
    digitalWrite(chgPin, LOW);
    for (int i = 0; i < 3; i++) digitalWrite(dumpPin[i], LOW);
    setLEDColor(Standby, 100);
    Serial.println("Standby Mode");
    battery_changed = false;
  }

  delay(80);
}

// Interrupt Service Routine (ISR) for button press
ISR(PCINT0_vect) {
  long currentTime = millis();
  if (toggleFlag) {
    startInterruptTime = currentTime;
    toggleFlag = false;
  } else {
    lastInterruptTime = currentTime;
    toggleFlag = true;
  }

  // Toggle mode if button is held for more than 1 second
  if ((lastInterruptTime - startInterruptTime) > 1000) {
    buttonPressed = !buttonPressed;
    change = true;
  }
}

// Updates battery voltage multiple times for more stable readings
void update_battery_level(uint16_t* Vin) {
  for (int i = 0; i < 20; i++) update_voltage(Vin);
}

// Reads battery voltage from analog pins
void update_voltage(uint16_t* Vin) {
  battery_detected = true;
  for (int i = 0; i < 3; i++) {
    uint16_t voltageSum = 0, validReadings = 0;
    for (int j = 0; j < 10; j++) {
      uint16_t voltage = analogRead(analogPins[i]);
      if (voltage <= 2000) {
        voltageSum += voltage;
        validReadings++;
      }
    }
    Vin[i] = (validReadings > 0) ? (voltageSum / validReadings) : 0;
    if (Vin[i] <= 2) battery_detected = false;

    // Apply simple averaging to smooth out voltage readings
    Vin[i] = (Vin[i] + Vlast[i] + Vlast1[i]) / 3;
    Vlast1[i] = Vlast[i];
    Vlast[i] = Vin[i];
  }

  Serial.print("a = ");
  Serial.print(Vin[0]);
  Serial.print(", b = ");
  Serial.print(Vin[1]);
  Serial.print(", c = ");
  Serial.print(Vin[2]);
  Serial.print(", d = ");
  Serial.println(Vin[3]);
}

// Handles battery levels and charging logic
void handleBatteryLevels(uint16_t* Vin, const uint16_t* BatteryLevel, int mode) {
  allBatteriesFull = true;
  delay_time = 10;

  if (!battery_changed) {
    update_battery_level(Vin);
    battery_changed = true;
  }

  for (int i = 0; i < 3; i++) {
    if (Vin[i] >= BatteryLevel[i] - variation[i] && Vin[i] <= BatteryLevel[i] + variation[i]) {
      digitalWrite(dumpPin[i], LOW);
      charge_done[i] = true;
    } else if (Vin[i] >= BatteryLevel[i]) {
      digitalWrite(dumpPin[i], HIGH);
      charge_done[i] = false;
    } else {
      digitalWrite(dumpPin[i], LOW);
      charge_done[i] = false;
      allBatteriesFull = false;
    }
  }

  // Enable charging only if not all batteries are full
  digitalWrite(chgPin, allBatteriesFull ? LOW : HIGH);
  ready = charge_done[0] && charge_done[1] && charge_done[2];

  // Detect "almost full" condition
  almost_full = (Vin[0] >= BatteryLevel[0] - 10 && Vin[0] <= BatteryLevel[0] + 10) || (Vin[1] >= BatteryLevel[1] - 5 && Vin[1] <= BatteryLevel[1] + 5) || (Vin[2] >= BatteryLevel[2] - 5 && Vin[2] <= BatteryLevel[2] + 5);

  handleLED(mode);
}

// Controls LED behavior based on battery state (DO NOT CHANGE BREATHING LED LOGIC)
void handleLED(int mode) {
  if (!ready) {
    for (int i = 1; i <= 200; i++) {
      uint8_t brightness = 127.5 * (1 + sin(millis() / 1000.0 * PI));
      setLEDColor(mode, brightness);
      checkdelay(10);
    }
  } else {
    setLEDColor(mode, 255);
    checkdelay(3000);
  }
}

// Handles non-blocking delays to allow for interrupts to be processed
void checkdelay(int time) {
  for (int i = 0; i < (time / 10); i++) {
    if (change) {
      change = false;
      break;
    }
    delay(delay_time);
  }
}

// Sets LED color based on current mode
void setLEDColor(int mode, int brightness) {
  uint32_t color = (mode == Charging) ? pixels->Color(0, brightness, 0) : (mode == Storage) ? pixels->Color(brightness, brightness, 0)
                                                                                            : pixels->Color(brightness, 0, 0);

  for (int i = 0; i < NUM_LEDS; i++) pixels->setPixelColor(i, color);
  pixels->show();
}
