const int analogPin1 = A2;
const int analogPin2 = A3;
const int analogPin3 = A4;
const int analogPin4 = A5;    
const float R1 = 33000.0;     // 33kΩ resistor
const float R2 = 12000.0;     // 12kΩ resistor
const float Vref = 5.0;       // Reference voltage of the Arduino (5V for most boards)
const int resolution = 1023;  // 10-bit ADC resolution
const float fullChargeVoltage = 4.2;  // Fully charged voltage for the battery

// Define the pins for the LEDs
const int ledPin1 = 8;
const int ledPin2 = 9;
const int ledPin3 = 10;
const int ledPin4 = 11;

void setup() {
  Serial.begin(115200);  // Initialize serial communication
  
  // Set LED pins as output
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);
  pinMode(ledPin4, OUTPUT);
}

void loop() {
  int sensorValue1 = analogRead(analogPin1);  // Read the analog input
  int sensorValue2 = analogRead(analogPin2);  // Read the analog input
  int sensorValue3 = analogRead(analogPin3);  // Read the analog input
  int sensorValue4 = analogRead(analogPin4);  // Read the analog input
  
  // Convert the ADC value to voltage
  float Vout1 = (sensorValue1 * Vref) / resolution;
  float Vout2 = (sensorValue2 * Vref) / resolution;
  float Vout3 = (sensorValue3 * Vref) / resolution;
  float Vout4 = (sensorValue4 * Vref) / resolution;

  // Calculate the input voltage (battery voltage)
  float Vin1 = Vout1 * (R1 + R2) / R2;
  float Vin2 = Vout2 * (R1 + R2) / R2;
  float Vin3 = Vout3 * (R1 + R2) / R2;
  float Vin4 = Vout4 * (R1 + R2) / R2;
  
  // Print the values to the serial monitor
  Serial.print("Sensor Value1: ");
  Serial.print(sensorValue1);
  Serial.print(" Vout1: ");
  Serial.print(Vout1);
  Serial.print(" Vin1: ");
  Serial.println(Vin1);

  Serial.print("Sensor Value2: ");
  Serial.print(sensorValue2);
  Serial.print(" Vout2: ");
  Serial.print(Vout2);
  Serial.print(" Vin2: ");
  Serial.println(Vin2);

  Serial.print("Sensor Value3: ");
  Serial.print(sensorValue3);
  Serial.print(" Vout3: ");
  Serial.print(Vout3);
  Serial.print(" Vin3: ");
  Serial.println(Vin3);

  Serial.print("Sensor Value4: ");
  Serial.print(sensorValue4);
  Serial.print(" Vout4: ");
  Serial.print(Vout4);
  Serial.print(" Vin4: ");
  Serial.println(Vin4);

  // Check if each battery is fully charged
  bool allCharged = true;

  if (Vin1 >= fullChargeVoltage) {
    digitalWrite(ledPin1, HIGH);
  } else {
    digitalWrite(ledPin1, LOW);
    allCharged = false;
  }

  if (Vin2 >= fullChargeVoltage) {
    digitalWrite(ledPin2, HIGH);
  } else {
    digitalWrite(ledPin2, LOW);
    allCharged = false;
  }

  if (Vin3 >= fullChargeVoltage) {
    digitalWrite(ledPin3, HIGH);
  } else {
    digitalWrite(ledPin3, LOW);
    allCharged = false;
  }

  if (Vin4 >= fullChargeVoltage) {
    digitalWrite(ledPin4, HIGH);
  } else {
    digitalWrite(ledPin4, LOW);
    allCharged = false;
  }

  // Turn off all LEDs if all batteries are fully charged
  if (allCharged) {
    digitalWrite(ledPin1, LOW);
    digitalWrite(ledPin2, LOW);
    digitalWrite(ledPin3, LOW);
    digitalWrite(ledPin4, LOW);
  }

  delay(100);  // Wait for 100 milliseconds before next measurement
}
