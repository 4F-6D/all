#include <Servo.h>

#define POWER 5
#define SIG A0
#define LED 5

#define T1 0
#define T2 1
#define T3 12
#define T4 13

#define F1 A2
#define F2 A3
#define F3 A4
#define F4 A5

Servo servo1;
Servo servo2;
int pos = 0;

unsigned long Sig = 0;
uint8_t flag = 0;

bool nd1 = false;
bool nd2 = false;
bool nd3 = false;
bool nd4 = false;

const unsigned long Std_by_on = 1200;
const unsigned long Std_by_off = 1100;
const unsigned long Drop1 = 1300;
const unsigned long Drop2 = 1500;
const unsigned long Drop3 = 1700;
const unsigned long Drop4 = 1900;

const uint8_t offset = 30;
const uint8_t offset_arm = 30;

void blinkLED(int times, int delayTime) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED, LOW);
    delay(delayTime);
    digitalWrite(LED, HIGH);
    delay(delayTime);
  }
}

void moveServo(Servo &myservo, int start_angle, int end_angle) {
  if (start_angle <= end_angle) {
    for (pos = start_angle; pos <= end_angle; pos++) {
      myservo.write(pos);
      delay(2);
    }
  } else {
    for (pos = start_angle; pos >= end_angle; pos--) {
      myservo.write(pos);
      delay(2);
    }
  }
}

void activateServo(Servo &servot, int pin, int feedbackPin) {
  digitalWrite(pin, HIGH);
  digitalWrite(POWER, LOW);
  delay(10);
  if (digitalRead(feedbackPin) == 1) {
    digitalWrite(pin, LOW);
    digitalWrite(POWER, HIGH);

    if (pin == T1 || pin == T2) {
      moveServo(servot, 90, 130);
      delay(1000);
      servot.write(90);
    }
    if (pin == T3 || pin == T4) {
      moveServo(servot, 90, 50);
      delay(1000);
      servot.write(90);
    }
  }
  digitalWrite(POWER, HIGH);
}

void R_nade(bool Pin_R) {
  if (Pin_R == 0 && nd1 == false) {
    nd1 = true;
    activateServo(servo1, T1, F1);
  }
  if (Pin_R == 1 && nd3 == false) {
    nd3 = true;
    activateServo(servo1, T3, F3);
  }
}

void L_nade(bool Pin_L) {
  if (Pin_L == 0 && nd2 == false) {
    nd2 = true;
    activateServo(servo2, T2, F2);
  }
  if (Pin_L == 1 && nd4 == false) {
    nd4 = true;
    activateServo(servo2, T4, F4);
  }
  
}

void initl() {
  digitalWrite(POWER, HIGH);
  digitalWrite(T1, LOW);
  digitalWrite(T2, LOW);
  digitalWrite(T3, LOW);
  digitalWrite(T4, LOW);
  blinkLED(1, 1000);
}

void setup() {
  servo1.attach(10);
  servo2.attach(11);

  servo1.write(90);
  servo2.write(90);

  pinMode(SIG, INPUT);
  pinMode(POWER, OUTPUT);
  pinMode(T1, OUTPUT);
  pinMode(T2, OUTPUT);
  pinMode(T3, OUTPUT);
  pinMode(T4, OUTPUT);
  pinMode(LED, OUTPUT);

  pinMode(F1, INPUT);
  pinMode(F2, INPUT);
  pinMode(F3, INPUT);
  pinMode(F4, INPUT);

  initl();
}

void loop() {
  Sig = pulseIn(SIG, HIGH);

  if (flag == 0 && Sig > Std_by_on - offset_arm && Sig < Std_by_on + offset_arm) {
    delay(1000);
    flag = 1;
  } else if (flag == 1 && Sig > Std_by_off - offset_arm && Sig < Std_by_off + offset_arm) {
    flag = 0;
    nd1 = false;
    nd2 = false;
    nd3 = false;
    nd4 = false;
  }

  if (flag == 1) {
    if (Sig > Drop1 - offset && Sig < Drop1 + offset) {
      R_nade(0);
    } else if (Sig > Drop2 - offset && Sig < Drop2 + offset) {
      L_nade(0);
    } else if (Sig > Drop3 - offset && Sig < Drop3 + offset) {
      R_nade(1);
    } else if (Sig > Drop4 - offset && Sig < Drop4 + offset) {
      L_nade(1);
    }
  }
}
