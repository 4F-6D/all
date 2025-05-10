/*
  Pin Change Interrupt Test
  pin-change-test.ino
  Demonstrates use of Pin Change Interrupt
  Input on D7, LED on D13

  DroneBot Workshop 2022
  https://dronebotworkshop.com
*/

// LED and switch
const byte buttonPin = 10;

// Boolean to represent toggle state
volatile bool togglestate = false;

void setup() {

  // Set LED as output
  pinMode(ledPin, OUTPUT);

  // Set switch as input with pullup
  pinMode(buttonPin, INPUT);

Serial.begin(115200);
  while (!Serial) {
    ;  // Wait for serial port to connect.
  }

  // Enable PCIE2 Bit3 = 1 (Port D)
  PCICR |= B00000001;
  // Select PCINT23 Bit7 = 1 (Pin D7)
  PCMSK2 |= B00000100;

}

void loop() {
  // No code in Loop
}

ISR (PCINT2_vect)
{
  // Interrupt for Port D
  // Invert toggle state
  togglestate = !togglestate;
  // Indicate state on LED
  digitalWrite(ledPin, togglestate);
  Serial.println("start ");

}