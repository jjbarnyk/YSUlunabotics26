#include <WiFi.h>  // Uses the built-in WiFi library for Uno R4 WiFi
#include <Servo.h>

// ----- WiFi Credentials -----
const char* ssid = "Lunabotics_25";
const char* password = "ysu12345";

// Static IP settings (adjust as needed)
IPAddress local_IP(192, 168, 0, 101);  
IPAddress gateway(192, 168, 0, 254);
IPAddress subnet(255, 255, 255, 0);

// Create a TCP server on port 12345
WiFiServer server(12345);

// ----- Motor Setup -----
// Here we assume your sparkmax motors are controlled by servo-like signals.
// Motor PWM pins (adjust as per your wiring on Uno R4):
Servo motor1;  // Left wheel A
Servo motor2;  // Left wheel C
Servo motor3;  // Right wheel B
Servo motor4;  // Right wheel D

// ----- Setup Function -----
void setup() {
  Serial.begin(9600);
  
  // Attach motors to digital pins
  motor1.attach(9); 
  motor2.attach(10);
  motor3.attach(11);
  motor4.attach(12);
  
  // Configure WiFi with static IP
  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid, password);
  
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected. IP Address: ");
  Serial.println(WiFi.localIP());
  
  // Start the TCP server
  server.begin();
  Serial.println("TCP server started on port 12345");
}

// ----- Loop Function -----
void loop() {
  WiFiClient client = server.available();
  if (client) {
    Serial.println("Client connected.");
    while (client.connected()) {
      if (client.available() > 0) {
        String data = client.readStringUntil('\n');
        data.trim();
        if (data.length() > 0) {
          Serial.print("Received data: ");
          Serial.println(data);
          // Expect data format: left_value,right_value
          int commaIndex = data.indexOf(',');
          if (commaIndex > 0) {
            int left_val = data.substring(0, commaIndex).toInt();
            int right_val = data.substring(commaIndex + 1).toInt();
            Serial.print("Left motor value: ");
            Serial.println(left_val);
            Serial.print("Right motor value: ");
            Serial.println(right_val);
            
            // Use the received PWM microsecond values directly
            motor1.writeMicroseconds(left_val);
            motor2.writeMicroseconds(left_val);
            motor3.writeMicroseconds(right_val);
            motor4.writeMicroseconds(right_val);
          }
        }
      }
    }
    client.stop();
    Serial.println("Client disconnected.");
  }
  
 
}
