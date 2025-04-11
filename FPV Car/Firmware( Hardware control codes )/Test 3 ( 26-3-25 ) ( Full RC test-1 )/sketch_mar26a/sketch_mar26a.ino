#include <WiFi.h>
#include <WebSocketsServer.h>

const char* ssid = "Z";
const char* password = "Jay232323";

WebSocketsServer webSocket(81);

#define Q1_PIN 19     // BD140 (PNP) transistor for backward direction
#define Q2_PIN 18     // BD140 (PNP) transistor for forward direction
#define Q3_PIN 5      // BD139 (NPN) transistor for forward direction
#define Q4_PIN 21     // BD139 (NPN) transistor for backward direction
#define left_pin 25   // Turn left side
#define right_pin 26  // Turn right side

//Define PWM frequency
#define PWM_FREQUENCY 50000  // 50k is best 20k frequency is noise effective & 30k frequency give low rpm tourqe
#define PWM_RESOLUTION 8

int speedValue = 128;  //LOW signal will control speed of motor so decreasing duty cytle increase speed

// Alternate Low as High & vice versa for polarity if BJT driver in use
bool H = LOW;
bool L = HIGH;

// Function for Forward Rear Action
void Forward_Rear(int duty_cycle) {
  // Forward direction: Q2 and Q3 ON
  digitalWrite(Q1_PIN, H);        // PMOS (Q1) OFF
  digitalWrite(Q2_PIN, L);        // PMOS (Q2) ON
  ledcWrite(Q3_PIN, duty_cycle);  // Control speed of NMOS (Q3) with PWM
  ledcWrite(Q4_PIN, 255);         // NMOS (Q4) OFF
}

// Function for Backward Rear Action
void Reverse_Rear(int duty_cycle) {
  // Backward direction: Q1 and Q4 ON
  digitalWrite(Q1_PIN, L);        // PMOS (Q1) ON
  digitalWrite(Q2_PIN, H);        // PMOS (Q2) OFF
  ledcWrite(Q3_PIN, 255);         // NMOS (Q3) OFF
  ledcWrite(Q4_PIN, duty_cycle);  // Control speed of NMOS (Q4) with PWM
}

// Function for Stopping Rear Action
void Rear_Stop() {
  digitalWrite(Q1_PIN, H);
  digitalWrite(Q2_PIN, H);
  ledcWrite(Q3_PIN, 255);
  ledcWrite(Q4_PIN, 255);
}

// Function to set steering at 0 degree
void Set_straight() {
  digitalWrite(left_pin, LOW);
  digitalWrite(right_pin, LOW);
}

// Function to set steering left
void Left_Turn() {
  digitalWrite(left_pin, HIGH);
}

//Function to set steering right
void Right_Turn() {
  digitalWrite(right_pin, HIGH);
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println("WebSocket Server Started!");

  // Set motor control pins as output
  pinMode(Q1_PIN, OUTPUT);
  pinMode(Q2_PIN, OUTPUT);
  ledcAttach(Q3_PIN, PWM_FREQUENCY, PWM_RESOLUTION);
  ledcAttach(Q4_PIN, PWM_FREQUENCY, PWM_RESOLUTION);
  pinMode(left_pin, OUTPUT);
  pinMode(right_pin, OUTPUT);

  // Initially, turn off all transistors (ensures the motor is stopped)
  digitalWrite(Q1_PIN, H);
  digitalWrite(Q2_PIN, H);
  // digitalWrite(Q3_PIN, H);
  // digitalWrite(Q4_PIN, H);
  ledcWrite(Q3_PIN, 255);
  ledcWrite(Q4_PIN, 255);
}

// WebSocket Event Handler
void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
  String command = String((char*)payload);
  Serial.println("Received: " + command);


  if (command == "forward_start") Forward_Rear(speedValue);
  else if (command == "back_start") Reverse_Rear(speedValue);
  else if (command == "forward_stop") Rear_Stop();
  else if (command == "back_stop") Rear_Stop();
  else if (command == "left_start") Left_Turn();
  else if (command == "right_start") Right_Turn();
  else if (command == "right_stop") Set_straight();
  else if (command == "left_stop") Set_straight();
  else if (command.startsWith("speed:")) {
    speedValue = command.substring(6).toInt();
    Serial.println("Speed set to: " + String(speedValue));
  }
}

void loop() {
  webSocket.loop();
}
