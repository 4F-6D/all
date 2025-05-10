

// constants won't change. They're used here to set pin numbers:
const int F1 = 16;  // A2 FIRE   the number of the pushbutton pin
const int F2 = 17; // A3 ARM
const int F3 = 18;  // A4 SDA
const int F4 = 19;  // A5 SCL
const int tr1 = 13; // SCK
const int tr2 = 12; // MISO
const int tr3 = 0; // RX
const int tr4 = 1; // TX
const int s1 = 10; // RST
const int s2 = 11; // MOSI
const int pw = 5; // LED
const int SIG = 14;  // A0


// variables will change:
int buttonState = 0;  // variable for reading the pushbutton status

void setup() {
  // initialize the LED pin as an output:
  pinMode(tr1, OUTPUT);
  pinMode(tr2, OUTPUT);
  pinMode(tr3, OUTPUT);
  pinMode(tr4, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(pw, OUTPUT);
  // initialize the pushbutton pin as an input:
  pinMode(F1, INPUT_PULLUP);
  pinMode(F2, INPUT_PULLUP);
  pinMode(F3, INPUT_PULLUP);
  pinMode(F4, INPUT_PULLUP);
  pinMode(SIG, INPUT_PULLUP);
  // // Serial.begin(9600);
}

void loop() {
  digitalWrite(tr1, LOW);
  digitalWrite(tr2, HIGH);
  digitalWrite(tr3, HIGH);
  digitalWrite(tr4, HIGH);
  digitalWrite(s1, HIGH);
  digitalWrite(s2, HIGH);
  digitalWrite(pw, HIGH);
  // read the state of the pushbutton value:
  int d1 = digitalRead(F1);
  int d2 = digitalRead(F2);
  int d3 = digitalRead(F3);
  int d4 = digitalRead(F4);
  int si = digitalRead(SIG);

  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (d1 == HIGH) {
    // Serial.print("d1");
    // Serial.println(d1);
  }
  if (d2 == HIGH) {
    // Serial.print("d2");
    // Serial.println(d2);
  }
  if (d3 == HIGH) {
    // Serial.print("d3");
    // Serial.println(d3);
  }
  if (d4 == HIGH) {
    // Serial.print("d4");
    // Serial.println(d4);
  }
   if (si == HIGH) {
    // Serial.print("si");
    // Serial.println(si);
  }
  delay(1000);
   // Serial.println("hi om");
}
