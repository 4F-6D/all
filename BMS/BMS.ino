const int analogPin1 = A2;
const int analogPin2 = A3;
const int analogPin3 = A4;
const int analogPin4 = A5;    // Pin connected to the voltage divider
const float R1 = 33000.0;     // 33kΩ resistor
const float R2 = 12000.0;     // 12kΩ resistor
const float Vref = 5.0;       // Reference voltage of the Arduino (5V for most boards)
const int resolution = 1023;  // 10-bit ADC resolution

void setup() {
  Serial.begin(9600);  // Initialize serial communication
}

void loop() {
  int sensorValue1 = analogRead(analogPin1);        // Read the analog input
  int sensorValue2 = analogRead(analogPin2);        // Read the analog input
  int sensorValue3 = analogRead(analogPin3);        // Read the analog input
  int sensorValue4 = analogRead(analogPin4);        // Read the analog input
  
  float Vout1 = (sensorValue1 * Vref) / resolution;  // Convert the ADC value to voltage
  float Vin1 = Vout1 * (R1 + R2) / R2;               // Calculate the input voltage (battery voltage)
 
  float Vout2 = (sensorValue2 * Vref) / resolution;  // Convert the ADC value to voltage
  float Vin2 = Vout2 * (R1 + R2) / R2;
 
  float Vout3 = (sensorValue3 * Vref) / resolution;  // Convert the ADC value to voltage
  float Vin3 = Vout3 * (R1 + R2) / R2;
 
  float Vout4 = (sensorValue4 * Vref) / resolution;  // Convert the ADC value to voltage
  float Vin4 = Vout4 * (R1 + R2) / R2;
  // Print the values to the serial monitor
  Serial.print("Sensor Value: ");
  Serial.print(sensorValue);
  Serial.print(" Vout1: ");
  Serial.print(Vout);
  Serial.print(" Vin1: ");
  Serial.printl(Vin);

  Serial.print(" Vout2: ");
  Serial.print(Vout);
  Serial.print(" Vin2: ");
  Serial.println(Vin);

 Serial.print(" Vout3: ");
  Serial.print(Vout);
  Serial.print(" Vin3: ");
  Serial.println(Vin);

 Serial.print(" Vout4: ");
  Serial.print(Vout);
  Serial.print(" Vin4: ");
  Serial.println(Vin);


  delay(100);  // Wait for a second before next measurement
}
