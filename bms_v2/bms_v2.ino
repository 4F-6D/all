#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>  // Required for 16 MHz Adafruit Trinket
#endif
#include <avr/wdt.h>
#include <math.h>

#define LED_PIN 3
#define NUM_LEDS 2
#define DEBOUNCE_DELAY 50
#define RESOLUTION 1023  // 10-bit ADC resolution
#define Storage 0
#define Charging 1
#define Standby 3
int delay_time = 10;
bool fullCharge = false;
int pixelFormat = NEO_GRB + NEO_KHZ800;
// uint16_t FULL_CHARGE_VOLTAGE[4] = { 816, 904, 931, 762 };  //a = 784, b = 873, c = 883, d = 519 // for new charger pcb
// uint16_t STORAGE_VOLTAGE[4] = { 762, 816, 821, 679 };
uint16_t FULL_CHARGE_VOLTAGE[4] = { 873, 979, 1003, 143 };  //a = 784, b = 873, c = 883, d = 519 // for old charger pcb
uint16_t STORAGE_VOLTAGE[4] = { 757, 851, 946, 57 };
uint16_t variation[4] = { 4, 2, 2, 2 };
const int analogPins[] = { A5, A4, A3, A2 };
const int dumpPin[] = { 8, 7, 6, 5 };
uint16_t Vin[4] = { 0, 0, 0, 0 };
uint16_t Vlast[4] = { 0, 0, 0, 0 };
uint16_t Vlast1[4] = { 0, 0, 0, 0 };
uint16_t Vlast2[4] = { 0, 0, 0, 0 };
uint16_t Vlast3[4] = { 0, 0, 0, 0 };
const int chgPin = A0;
const int buttonPin = 10;
bool battery_detected = true;
bool battery_changed = false;
uint16_t average[4] = { 0, 0, 0, 0 };
const float strength = 0.85;
Adafruit_NeoPixel* pixels;
const float SR[4] = { 2000.0, 11000.0, 22000.0, 36000.0 };
const float PR[4] = { 12000.0, 12000.0, 12000.0, 12000.0 };
const float Vref = 4.25;  // Reference voltage of the Arduino
volatile bool buttonPressed = true;
volatile bool toggleFlag = true;
volatile long startInterruptTime = 0;
volatile long lastInterruptTime = 0;
bool allBatteriesFull = true;  // Flag to check if all batteries are full
bool charge_done[4] = { false, false, false, false };
bool ready = false;
bool change = false;
bool almost_full = false;

void setup() {
  if (MCUSR & _BV(WDRF)) {
    MCUSR &= ~_BV(WDRF);  // Clear the WDT reset flag
    wdt_disable();        // Disable the WDT
  }

  Serial.begin(115200);  // Initialize Serial Monitor for debugging
  // while (!Serial) { ; }  // Wait for serial port to connect
  // Serial.println("System Start");
  update_battery_level(Vin);


#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif

  pixels = new Adafruit_NeoPixel(NUM_LEDS, LED_PIN, pixelFormat);
  pixels->begin();

  for (int i = 0; i < 4; i++) {
    pinMode(dumpPin[i], OUTPUT);
    digitalWrite(dumpPin[i], LOW);
  }
  pinMode(chgPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  digitalWrite(chgPin, LOW);


  // Set up pin change interrupt for the button pin
  PCICR |= (1 << PCIE0);    // Enable pin change interrupt for group 0
  PCMSK0 |= (1 << PCINT2);  // Enable pin change interrupt for PCINT2 (digital pin 10)
}

void loop() {
  wdt_reset();      // Reset watchdog timer to prevent system reset
  pixels->clear();  // Clear LED pixels
  if (almost_full) {
    digitalWrite(chgPin, LOW);
    for (int i = 0; i < 3; i++) {
      digitalWrite(dumpPin[i], LOW);
    }
    delay(50);
  }
  update_voltage(Vin);  // Update voltage readings

  if (battery_detected) {
    if (!buttonPressed) {
      handleBatteryLevels(Vin, STORAGE_VOLTAGE, Storage);
      Serial.println("Storage Mode");
    } else {
      handleBatteryLevels(Vin, FULL_CHARGE_VOLTAGE, Charging);
      Serial.println("Charging Mode");
    }
  } else {
    digitalWrite(chgPin, LOW);
    for (int i = 0; i < 3; i++) {
      digitalWrite(dumpPin[i], LOW);
    }
    setLEDColor(Standby, 100);
    Serial.println("stand by");
    battery_changed = false;
    // if (!buttonPressed) {
    //   handleBatteryLevels(Vin, STORAGE_VOLTAGE, Storage);
    //   Serial.println("Storage Mode");
    // } else {
    //   handleBatteryLevels(Vin, FULL_CHARGE_VOLTAGE, Charging);
    //   Serial.println("Charging Mode");
    // }
  }

  delay(80);  // Small delay to prevent rapid cycling
}

ISR(PCINT0_vect) {
  if (toggleFlag) {
    startInterruptTime = millis();  // Record start time
    Serial.println("Button Pressed");
    lastInterruptTime = 0;  // Reset last interrupt time
    toggleFlag = false;
  } else {
    lastInterruptTime = millis();  // Record end time
    Serial.println("Button Released");
    toggleFlag = true;
  }

  if ((lastInterruptTime - startInterruptTime) > 1000) {  // Check if button was held for 5 seconds
    Serial.println("Mode Toggle Triggered");
    buttonPressed = !buttonPressed;  // Toggle button state
    change = true;
    startInterruptTime = lastInterruptTime = 0;  // Reset timing
  }
}

void update_battery_level(uint16_t* Vin) {
  for (int i = 0; i < 85; i++) {
    update_voltage(Vin);
    Serial.println(i);
  }
}

void update_voltage(uint16_t* Vin) {
  for (int i = 0; i < 3; i++) {
    uint16_t voltageSum = 0;
    int validReadings = 0;
    for (int j = 0; j < 10; j++) {  // Reduce the number of readings to save stack space
      uint16_t voltage = readBatteryVoltage(analogPins[i], i);
      if (voltage >= 0 && voltage <= 2000) {  // Check if the voltage is finite
        voltageSum += voltage;
        validReadings++;
      } else {
        Serial.print("Invalid voltage reading for pin ");
        Serial.println(i);
        j--;  // Retry the reading
        delay(1);
      }
      delay(1);
    }
    if (validReadings > 1) {
      Vin[i] = voltageSum / validReadings;  // Average the valid readings
    } else {
      Vin[i] = 0;  // Default to 0 if no valid readings
    }
  }

  if (Vin[0] < 2 || Vin[1] - Vin[0] <= 2 || Vin[2] - Vin[1] <= 2) {
    battery_detected = false;
  } else {
    battery_detected = true;
  }


  for (int i = 0; i < 3; i++) {
    if (!isinf(Vin[i])) {
      Vin[i] = (Vin[i] + Vlast[i] + Vlast1[i]) / 3;
      // average[i] = (strength * average[i]) + ((1.0 - strength) * Vin[i]);
      // Vin[i] = average[i];
      Vlast1[i] = Vlast[i];
      Vlast[i] = Vin[i];
    } else {
      Serial.println("vin is inf");
    }
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

uint16_t readBatteryVoltage(int pin, int index) {
  int sensorValue = analogRead(pin);
  // uint16_t Vout = (sensorValue * Vref) / RESOLUTION;
  // return Vout * (SR[index] + PR[index]) / PR[index];
  return sensorValue;
}

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
      Serial.print(dumpPin[i]);
      Serial.println(" is charged");
      charge_done[i] = true;
      delay_time = 3;
    } else if (Vin[i] >= BatteryLevel[i]) {
      digitalWrite(dumpPin[i], HIGH);
      Serial.print(dumpPin[i]);
      Serial.println(" is high");
      charge_done[i] = false;
      delay_time = 3;
    } else {
      digitalWrite(dumpPin[i], LOW);
      Serial.print(dumpPin[i]);
      Serial.println(" is low");
      charge_done[i] = false;
      allBatteriesFull = false;  // Set flag if any battery is not full
    }
  }
  digitalWrite(chgPin, allBatteriesFull ? LOW : HIGH);  // Control charging
  if (charge_done[0] && charge_done[1] && charge_done[2] && charge_done[3]) {
    ready = true;
  } else {
    ready = false;
  }
  if ((Vin[0] >= (BatteryLevel[0] - 10) && Vin[0] <= (BatteryLevel[0] + 10)) || (Vin[1] >= (BatteryLevel[1] - 5) && Vin[1] <= (BatteryLevel[1] + 5)) || (Vin[2] >= (BatteryLevel[2] - 5) && Vin[2] <= (BatteryLevel[2] + 5))) {
    almost_full = true;
  } else {
    almost_full = false;
  }
  handleLED(mode);
}

void handleLED(int mode) {
  if (!ready) {
    static uint8_t brightness = 0;  // LED brightness level (0-255)
    for (int i = 1; i <= 200; i++) {
      brightness = 127.5 * (1 + sin(millis() / 1000.0 * PI));
      setLEDColor(mode, brightness);
      // FastLED.show();
      checkdelay(10);
    }
  } else {
    setLEDColor(mode, 255);
    checkdelay(3000);
  }
}

void checkdelay(int time) {
  for (int i = 0; i < (time / 10); i++) {
    if (change) {
      change = false;
      break;
    }
    delay(delay_time);
  }
}

void setLEDColor(int mode, int brightness) {
  if (ready) {
    if (mode == Charging) {
      pixels->setPixelColor(0, pixels->Color(0, 250, 0));
      pixels->setPixelColor(1, pixels->Color(0, 0, 0));
    } else if (mode == Storage) {
      pixels->setPixelColor(0, pixels->Color(0, 0, 0));
      pixels->setPixelColor(1, pixels->Color(0, 250, 0));
    }
    if (mode == Standby) {
      pixels->setPixelColor(0, pixels->Color(brightness, 0, 0));
      pixels->setPixelColor(1, pixels->Color(brightness, 0, 0));
    }

  } else {
    if (mode == Charging) {
      pixels->setPixelColor(0, pixels->Color(brightness, 0, brightness));
      pixels->setPixelColor(1, pixels->Color(0, 0, 0));
    }
    if (mode == Storage) {
      pixels->setPixelColor(0, pixels->Color(0, 0, 0));
      pixels->setPixelColor(1, pixels->Color(brightness, brightness, 0));
    }
    if (mode == Standby) {
      pixels->setPixelColor(0, pixels->Color(brightness, 0, 0));
      pixels->setPixelColor(1, pixels->Color(brightness, 0, 0));
    }
  }
  pixels->show();
}
