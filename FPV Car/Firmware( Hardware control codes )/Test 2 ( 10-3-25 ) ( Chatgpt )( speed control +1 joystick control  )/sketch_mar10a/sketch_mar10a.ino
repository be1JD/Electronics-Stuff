#include <WiFi.h>
#include <WebSocketsServer.h>

const char* ssid = "Z";
const char* password = "Jay232323";

WebSocketsServer webSocket(81);

// Motor control pins
#define MOTOR_LEFT   5
#define MOTOR_RIGHT  18
#define MOTOR_BACK   19
#define MOTOR_FRONT  21

// PWM settings
#define PWM_FREQ   1000  
#define PWM_RES   8  // 8-bit resolution (0-255)
int speedValue = 128;  // Default speed (50%)

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
    Serial.println("\nWiFi Connected!");
    Serial.print("ESP32 IP Address: ");
    Serial.println(WiFi.localIP());

    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    Serial.println("WebSocket Server Started!");

    // Set up PWM for motor control
    ledcAttach(MOTOR_FRONT, PWM_FREQ, PWM_RES);
    ledcAttach(MOTOR_BACK, PWM_FREQ, PWM_RES);
    ledcAttach(MOTOR_LEFT, PWM_FREQ, PWM_RES);
    ledcAttach(MOTOR_RIGHT, PWM_FREQ, PWM_RES);
}

// WebSocket Event Handler
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
    String command = String((char *)payload);
    Serial.println("Received: " + command);

    // Stop all motors by default
    ledcWrite(MOTOR_FRONT, 0);
    ledcWrite(MOTOR_BACK, 0);
    ledcWrite(MOTOR_LEFT, 0);
    ledcWrite(MOTOR_RIGHT, 0);

    if (command == "forward_start") ledcWrite(MOTOR_FRONT, speedValue);
    else if (command == "back_start") ledcWrite(MOTOR_BACK, speedValue);
    else if (command == "left_start") ledcWrite(MOTOR_LEFT, speedValue);
    else if (command == "right_start") ledcWrite(MOTOR_RIGHT, speedValue);
    else if (command.startsWith("speed:")) {
        speedValue = command.substring(6).toInt();
        Serial.println("Speed set to: " + String(speedValue));
    }
}

void loop() {
    webSocket.loop();
}
