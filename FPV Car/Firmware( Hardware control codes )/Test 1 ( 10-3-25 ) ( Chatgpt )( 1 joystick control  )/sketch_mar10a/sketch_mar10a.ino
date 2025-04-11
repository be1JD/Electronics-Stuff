#include <WiFi.h>
#include <WebSocketsServer.h>

const char* ssid = "Z";
const char* password = "Jay232323";

WebSocketsServer webSocket(81);

#define MOTOR_LEFT   5
#define MOTOR_RIGHT  18
#define MOTOR_BACK   19
#define MOTOR_FRONT  21

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

    pinMode(MOTOR_LEFT, OUTPUT);
    pinMode(MOTOR_RIGHT, OUTPUT);
    pinMode(MOTOR_BACK, OUTPUT);
    pinMode(MOTOR_FRONT, OUTPUT);

    digitalWrite(MOTOR_LEFT, LOW);
    digitalWrite(MOTOR_RIGHT, LOW);
    digitalWrite(MOTOR_BACK, LOW);
    digitalWrite(MOTOR_FRONT, LOW);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
    String command = String((char *)payload);
    Serial.println("Received: " + command);

    digitalWrite(MOTOR_LEFT, LOW);
    digitalWrite(MOTOR_RIGHT, LOW);
    digitalWrite(MOTOR_BACK, LOW);
    digitalWrite(MOTOR_FRONT, LOW);

    if (command == "forward_start") digitalWrite(MOTOR_FRONT, HIGH);
    else if (command == "back_start") digitalWrite(MOTOR_BACK, HIGH);
    else if (command == "left_start") digitalWrite(MOTOR_LEFT, HIGH);
    else if (command == "right_start") digitalWrite(MOTOR_RIGHT, HIGH);
}

void loop() {
    webSocket.loop();
}
