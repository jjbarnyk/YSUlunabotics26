#include <WiFi.h>
#include <MobaTools.h>
#include <Servo.h>

// ----- Wi-Fi Setup -----
const char* ssid     = "Lunabotics_25";
const char* password = "ysu12345";
IPAddress local_IP(192,168,0,101), gateway(192,168,0,254), subnet(255,255,255,0);
WiFiServer server(12345);

// ----- Stepper Setup -----
#define STEP_PIN   2
#define DIR_PIN    3
#define ENA_PIN    4
#define STEP_SPEED 5000
#define RAMP_LEN   20
MoToStepper stepper(200, STEPDIR);
int currentDirection = 0;  // 1 = forward, -1 = backward, 0 = stop

// ----- Limit switches -----
#define TOP_LIMIT_PIN     8
#define BOTTOM_LIMIT_PIN  7
#define LIMIT_ACTIVE      LOW  // NC type: LOW = triggered

// ----- Bucket-servo -----
Servo rotServo1, rotServo2;
const int bucketPinL = 6, bucketPinR = 5;

// ----- Wheel-motor -----
Servo motorA, motorC, motorB, motorD;
const int pinA = 9, pinC = 10, pinB = 11, pinD = 12;

void setup() {
  Serial.begin(115200);

  // Stepper
  pinMode(ENA_PIN, OUTPUT);
  digitalWrite(ENA_PIN, HIGH);
  stepper.attach(STEP_PIN, DIR_PIN);
  stepper.setSpeed(STEP_SPEED);
  stepper.setRampLen(RAMP_LEN);

  // Limit switches
  pinMode(TOP_LIMIT_PIN, INPUT_PULLUP);
  pinMode(BOTTOM_LIMIT_PIN, INPUT_PULLUP);

  // Bucket
  rotServo1.attach(bucketPinL);
  rotServo2.attach(bucketPinR);
  rotServo1.writeMicroseconds(1500);
  rotServo2.writeMicroseconds(1500);

  // Wheels
  motorA.attach(pinA); motorC.attach(pinC);
  motorB.attach(pinB); motorD.attach(pinD);

  // Wi-Fi
  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi IP: " + WiFi.localIP().toString());
  server.begin();
  Serial.println("Server started on port 12345");
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    Serial.println("Client connected");

    while (client.connected()) {
      if (client.available()) {
        String cmd = client.readStringUntil('\n');
        cmd.trim();

        bool topHit    = (digitalRead(TOP_LIMIT_PIN)    == LIMIT_ACTIVE);
        bool bottomHit = (digitalRead(BOTTOM_LIMIT_PIN) == LIMIT_ACTIVE);

        Serial.print("Cmd: "); Serial.println(cmd);
        Serial.print("Top raw: "); Serial.print(topHit);
        Serial.print("  Bottom raw: "); Serial.println(bottomHit);

        // Stepper control
        if (cmd == "forward") {
          if (!topHit) {
            stepper.rotate(1);
            currentDirection = 1;
            Serial.println("↑ forward start");
          } else {
            stepper.stop();
            currentDirection = 0;
            Serial.println("↑ forward blocked by top limit");
          }
        } else if (cmd == "backward") {
          if (!bottomHit) {
            stepper.rotate(-1);
            currentDirection = -1;
            Serial.println("↓ backward start");
          } else {
            stepper.stop();
            currentDirection = 0;
            Serial.println("↓ backward blocked by bottom limit");
          }
        } else if (cmd == "stop") {
          stepper.stop();
          currentDirection = 0;
          Serial.println("⏹️ stepper stop");
        }

        // Bucket PWM
        else if (cmd.startsWith("SERVO:")) {
          int pwm = constrain(cmd.substring(6).toInt(), 1000, 2000);
          rotServo1.writeMicroseconds(pwm);
          rotServo2.writeMicroseconds(3000 - pwm);
          Serial.print("→ buckets PWM "); Serial.println(pwm);
        }

        // Wheels
        else {
          int comma = cmd.indexOf(',');
          if (comma > 0) {
            int lv = cmd.substring(0, comma).toInt();
            int rv = cmd.substring(comma + 1).toInt();
            motorA.writeMicroseconds(lv);
            motorC.writeMicroseconds(lv);
            motorB.writeMicroseconds(rv);
            motorD.writeMicroseconds(rv);
            Serial.print("→ wheels L="); Serial.print(lv);
            Serial.print(" R="); Serial.println(rv);
          }
        }
      }

      // Realtime stop if limit hit mid-motion
      if (currentDirection == 1 && digitalRead(TOP_LIMIT_PIN) == LIMIT_ACTIVE) {
        stepper.stop();
        currentDirection = 0;
        Serial.println("‼️ Stopped: hit top during motion");
      }
      if (currentDirection == -1 && digitalRead(BOTTOM_LIMIT_PIN) == LIMIT_ACTIVE) {
        stepper.stop();
        currentDirection = 0;
        Serial.println("‼️ Stopped: hit bottom during motion");
      }
    }

    client.stop();
    Serial.println("Client disconnected");
  }
}
