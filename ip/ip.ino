#include <WiFi.h>
#include <WebServer.h>
#include "HX711.h"
#include "html.h"

// WiFi credentials
const char* ssid = "0001";
const char* password = "12345678";

// Load cell pins
const int LOADCELL_DOUT_PIN = 16;
const int LOADCELL_SCK_PIN = 4;

// Scale calibration factor
const float CALIBRATION_FACTOR = 708.1990;  // Adjust this according to your calibration

// Web server
WebServer server(80);

// HX711 scale instance
HX711 scale;



// Function to send the main page
void MainPage() {
  server.send(200, "text/html", html_page);
}

// Function to send the weight data
void sendWeightData() {
  float weight = scale.get_units(25);  // Read and average 5 readings
  Serial.print("Weight: ");
  Serial.println(weight);
  scale.power_down();
  delay(1);
  scale.power_up();
  String data = "" + String(weight, 2) + "";  // Format as JSON array
  server.send(200, "text/plain", data);
}

void setup() {
  Serial.begin(115200);
  Serial.println("HX711 Demo");

  // Initialize the scale
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(CALIBRATION_FACTOR);
  scale.tare();

  // Print initial readings
  Serial.println("Initial scale readings:");
  Serial.print("Raw reading: ");
  Serial.println(scale.read());
  Serial.print("Average reading: ");
  Serial.println(scale.read_average(20));
  Serial.print("Value: ");
  Serial.println(scale.get_value(5));
  Serial.print("Units: ");
  Serial.println(scale.get_units(5), 1);

  // Initialize WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nConnected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Setup web server routes
  server.on("/", MainPage);
  server.on("/readweight", sendWeightData);
  server.begin();
  Serial.println("Web server started");
}

void loop() {
  // Handle client requests
  server.handleClient();

  delay(1);  // Adjust the delay as needed
}
