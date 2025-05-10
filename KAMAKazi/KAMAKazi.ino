#include "Adafruit_VL53L0X.h"
int ARM = A3;
int FIRE = A2;
// int Trigger = 3;
int Led = 5;

int Sig_in = A0;

int Threshold = 120;
unsigned long duration;

int Det_Flag = 0;

Adafruit_VL53L0X lox = Adafruit_VL53L0X();

void setup() {
  Serial.begin(9600);

  pinMode(Sig_in, INPUT);
  pinMode(ARM, OUTPUT);
  pinMode(FIRE, OUTPUT);
  pinMode(Led, OUTPUT);

  digitalWrite(Led, LOW);
  digitalWrite(ARM, LOW);
  digitalWrite(FIRE, HIGH);

  digitalWrite(Led, HIGH);
  delay(1000);
  digitalWrite(Led, LOW);
  delay(1000);
  digitalWrite(Led, HIGH);
  delay(1000);
  digitalWrite(Led, LOW);


  Serial.println(F("Testing"));
  if (!lox.begin()) {
    Serial.println(F("Failed to BOOT"));
    // while (1) {
    digitalWrite(Led, HIGH);
    delay(500);
    digitalWrite(Led, LOW);
    delay(500);
    digitalWrite(Led, HIGH);
    delay(200);
    digitalWrite(Led, LOW);
    delay(200);
    digitalWrite(Led, HIGH);
    delay(500);
    digitalWrite(Led, LOW);
    delay(500);
    // }
  }
}

void loop() {
  duration = pulseIn(Sig_in, HIGH);
  Serial.print(duration);
  Serial.println(" ");

  if (duration > 1300 && duration < 1400) {
    Det_Flag = 1;
  }

  if (duration > 1400 && duration < 1700 && Det_Flag == 1) {
    VL53L0X_RangingMeasurementData_t measure;

    Serial.print("Reading...");
    lox.rangingTest(&measure, false);

    if (measure.RangeStatus != 4) {

      Serial.print("Distance: ");
      Serial.println(measure.RangeMilliMeter);

      if (measure.RangeMilliMeter >= Threshold) {
        digitalWrite(ARM, LOW);
        digitalWrite(FIRE, HIGH);
        digitalWrite(Led, LOW);
      } else if (measure.RangeMilliMeter < Threshold) {
        digitalWrite(ARM, HIGH);
        delay(50);
        digitalWrite(FIRE, LOW);
        digitalWrite(Led, HIGH);
         Serial.println("BOOM");
        delay(2000);
      }
    } else {
      Serial.println("Out Of Range");
    }
  } else if (duration > 1700 && Det_Flag == 1) {
    digitalWrite(ARM, HIGH);
    delay(50);
    digitalWrite(FIRE, LOW);
    digitalWrite(Led, HIGH);
    Serial.println("BOOM");
    delay(2000);
  } else if (duration < 1300) {
    digitalWrite(ARM, LOW);
    digitalWrite(FIRE, HIGH);
    digitalWrite(Led, LOW);
    Det_Flag = 0;
  }
  delay(10);
}