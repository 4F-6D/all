#include "Adafruit_VL53L0X.h"

int Trigger = 3;
int Trigger_Led = 2;

int Sig_in = A0;

int Threshold = 120;
unsigned long duration;

int Det_Flag = 0;

Adafruit_VL53L0X lox = Adafruit_VL53L0X();

void setup() {
  Serial.begin(9600);

  pinMode(Sig_in, INPUT);

  pinMode(Trigger, OUTPUT);
  digitalWrite(Trigger, LOW);
  pinMode(Trigger_Led, OUTPUT);
  digitalWrite(Trigger_Led, LOW);

  digitalWrite(Trigger_Led, HIGH);
  delay(1000);
  digitalWrite(Trigger_Led, LOW);
  delay(1000);
  digitalWrite(Trigger_Led, HIGH);
  delay(1000);
  digitalWrite(Trigger_Led, LOW);

  while (!Serial) {
    delay(1);
  }

  Serial.println(F("Testing"));
  if (!lox.begin()) {
    Serial.println(F("Failed to BOOT"));
    while (1) {
      digitalWrite(Trigger_Led, HIGH);
      delay(500);
      digitalWrite(Trigger_Led, LOW);
      delay(500);
      digitalWrite(Trigger_Led, HIGH);
      delay(200);
      digitalWrite(Trigger_Led, LOW);
      delay(200);
      digitalWrite(Trigger_Led, HIGH);
      delay(500);
      digitalWrite(Trigger_Led, LOW);
      delay(500);
    }
  }
}

void loop() {
  duration = pulseIn(Sig_in, HIGH);
  Serial.print(duration);
  Serial.println(" ");
  VL53L0X_RangingMeasurementData_t measure;
  Serial.print("Distance: ");
  Serial.println(measure.RangeMilliMeter);

  if (duration > 1300 && duration < 1400) {
    Det_Flag = 1;
  }

  if (duration > 1400 && duration < 1700 && Det_Flag == 1) {
    

    Serial.print("Reading...");
    lox.rangingTest(&measure, false);

    if (measure.RangeStatus != 4) {

      Serial.print("Distance: ");
      Serial.println(measure.RangeMilliMeter);

      if (measure.RangeMilliMeter >= Threshold) {
        digitalWrite(Trigger, LOW);
        digitalWrite(Trigger_Led, LOW);
      } else if (measure.RangeMilliMeter < Threshold) {
        digitalWrite(Trigger, HIGH);
        digitalWrite(Trigger_Led, HIGH);
        delay(2000);
      }
    } else {
      Serial.println("Out Of Range");
    }
  } else if (duration > 1700 && Det_Flag == 1) {
    digitalWrite(Trigger, HIGH);
    digitalWrite(Trigger_Led, HIGH);
    delay(2000);
  } else if (duration < 1300) {
    digitalWrite(Trigger, LOW);
    digitalWrite(Trigger_Led, LOW);
    Det_Flag = 0;
  }
  delay(10);
}