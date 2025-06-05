#include <WiFi.h>
#include <MobaTools.h>

const char* ssid = "Lunabotics_25";
const char* password = "ysu12345";
IPAddress local_IP(192, 168, 0, 101);
IPAddress gateway(192, 168, 0, 254);
IPAddress subnet(255, 255, 255, 0);

WiFiServer server(12345); // TCP server on port 12345

#define STEP_PIN 2
#define DIR_PIN 3
#define ENA_PIN 4

MoToStepper stepper(200, STEPDIR); // 200 steps per revolution

void setup() {
  Serial.begin(9600);
  stepper.attach(STEP_PIN, DIR_PIN);
  stepper.setRampLen(200); // Set ramp length for acceleration/deceleration
  pinMode(ENA_PIN, OUTPUT);
  digitalWrite(ENA_PIN, HIGH);

  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    String command = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        command += c;
        if (c == '\n') {
          command.trim();
          if (command == "forward") {
            stepper.rotate(1); // Rotate clockwise
            Serial.println("Motor rotating forward");
          } else if (command == "backward") {
            stepper.rotate(-1); // Rotate counterclockwise
            Serial.println("Motor rotating backward");
          } else if (command == "stop") {
            stepper.stop(); // Stop the motor
            Serial.println("Motor stopped");
          }
          command = ""; // Reset command
        }
      }
    }
    client.stop();
    Serial.println("Client disconnected");
  }

  // Print the number of steps remaining
  int remainingSteps = stepper.stepsToDo();
  if (remainingSteps != 0) {
    Serial.print("Steps remaining: ");
    Serial.println(remainingSteps);
  }
}
