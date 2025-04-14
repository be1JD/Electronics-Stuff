#include <WiFi.h>
#include <ESPAsyncWebServer.h>  // Build in websocket support amazing
#include <AsyncTCP.h>           // Required for AsyncWebserver library
#include <ESP32Servo.h>

const char *ssid = "ESP32-AP";
const char *password = "Jay232323";

Servo servo;
// Published values for SG90 servos; adjust if needed
int minUs = 1000;
int maxUs = 2000;

int position = 0;  // variable to store the servo position

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

#define Q1_PIN 19  // BD140 (PNP) transistor for backward direction
#define Q2_PIN 18  // BD140 (PNP) transistor for forward direction
#define Q3_PIN 5   // BD139 (NPN) transistor for forward direction
#define Q4_PIN 21  // BD139 (NPN) transistor for backward direction
// #define left_pin 25   // Turn left side
// #define right_pin 26  // Turn right side
#define servo_pin 26

//Define PWM frequency
#define PWM_FREQUENCY 50000  // 50k is best 20k frequency is noise effective & 30k frequency give low rpm tourqe
#define PWM_RESOLUTION 8

int JumpStart_Value = 130;
int JumpStart_duty = 200;  // Set at which duty cycle you want JumpStart feature
int JumpStart_delay = 50;  // At very low speeds provide delay for JumpStart & get momentum
int speedValue = 128;      //LOW signal will control speed of motor so decreasing duty cytle increase speed

// Alternate Low as High & vice versa for polarity if BJT driver in use
// Becase the Opposites Transistors are use to drive Main H-Bridge
bool H = LOW;
bool L = HIGH;


//HTML Code file here
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Remote for FPV car control</title>
    <!-- <script src="https://cdn.jsdelivr.net/npm/nipplejs@0.9.0"></script> -->
    <style>
        body {
            margin: 0;
            padding: 0px 20px;
            height: 100vh;
            width: 95vw;
            font-family: Arial, sans-serif;
            text-align: center;
        }

        /* .controls_container { */
        /* background-image: url(http://192.168.75.246:81/stream); */
        /* background-repeat: no-repeat; */
        /* background-position: center; */
        /* background-size: contain; */
        /* position: relative; */
        /* min-height: 240px; */
        /* } */

        #title {
            margin: 0px;
            padding: 0px;
        }


        .info {
            margin: 15px 0 0 0;
            display: flex;
            justify-content: space-between;
            height: 25px;

        }

        #status {
            padding: 5px;
            font-size: 16px;
            background-color: rgb(250, 33, 33);
            border-radius: 15px;
        }

        .curr_cmd_container {
            display: flex;
            justify-content: center;
            align-items: center;
            gap: 10px;
        }

        .controls_container {
            min-height: 240px;
            /* height: 50%; */
            height: 80vh;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: flex-end;
        }

        .controls_wraper {
            /* margin: vh; */
            /* display: grid; */
            /* grid-template-columns: repeat(3, 1fr); */
            /* gap: 10px; */
            width: 100%;
            display: flex;
            align-items: center;
            justify-content: space-between;
            /* gap: 100px; */
        }

        .Control_section1 {
            width: 33%;
            /* padding-right: 30px; */
            display: flex;
            justify-content: left;
            justify-content: center;
            gap: 10px;
        }

        .Control_section2 {
            width: 33%;
            position: relative;
        }

        .Control_section3 {
            width: 33%;
        }

        .video_container {
            transform: rotate(90deg);
        }

        .speed_control_container {
            /* width: 160px; */
            margin: 20px 0px 60px 0px;
        }

        .direction_control {
            margin: 0px 0px 70px 0px;
            display: flex;
            justify-content: center;
            gap: 10px;
        }

        .rear_control {
            display: flex;
            flex-direction: column;
            gap: 10px;
        }

        .controls_wraper button {
            user-select: none;
            height: 70px;
            width: 70px;
            font-size: 20px;
            padding: 15px;
            margin: 3px;
            color: white;
            border: none;
            border-radius: 10px;
        }

        #break_button {
            background-color: red;
        }

        .direction_control button {
            background-color: blue;
        }

        .break_container {
            display: flex;
            flex-direction: column;
            justify-content: center;
        }

        .rear_control button {
            background-color: blue;
        }

        .video_container img {
            height: 240px;
            width: 240px;
        }
    </style>
</head>

<body>

    <header>
        <h2 id="title">JetRay Controller</h2>
    </header>
    <main>
        <div class="info">
            <div id="status">Not Connected !</div>
            <div class="curr_cmd_container">
                <p>Command Send :</p>
                <p id="current_command">No Command yet</p>
            </div>
        </div>

        <!-- Control Buttons -->
        <div class="controls_container">
            <div class="controls_wraper">
                <div class="Control_section1">
                    <div class="rear_control">
                        <button ontouchstart="sendCommand('forward_start')"
                            ontouchend="sendCommand('forward_stop')">⬆️</button>
                        <button ontouchstart="sendCommand('back_start')"
                            ontouchend="sendCommand('back_stop')">⬇️</button>
                    </div>
                    <div class="break_container">
                        <button id="break_button" ontouchstart="sendCommand('rear_break')"
                            ontouchend="sendCommand('break_stop')">B</button>
                    </div>
                </div>
                <div class="Control_section2">
                    <!-- FPV Camera Feed -->
                    <div class="video_container">
                        <!-- Uncomment Below code while using Video -->
                        <img id="video" src="http://192.168.75.246:81/stream"> 
                    </div>
                </div>
                <div class="Control_section3">
                    <div class="speed_control_container">
                        <label for="speed">Speed Control: <span id="speedValue">128</span></label><br>
                        <input type="range" id="speed" min="0" max="255" value="128">
                    </div>
                    <div class="direction_control">
                        <button ontouchstart="sendCommand('left_start')"
                            ontouchend="sendCommand('left_stop')">⬅️</button>
                        <button ontouchstart="sendCommand('right_start')"
                            ontouchend="sendCommand('right_stop')">➡️</button>
                    </div>
                </div>
            </div>
        </div>
    </main>

    <script>
        let ws;

        function connectWebSocket() {
            ws = new WebSocket("ws://" + location.host + "/ws");

            ws.onopen = () => {
                document.getElementById("status").innerText = "Connected!";
                document.getElementById("status").style.backgroundColor = "green";
            };

            ws.onmessage = (event) => {
                console.log("Message from ESP32:", event.data);
                document.getElementById("status").innerText = event.data;
            };

            ws.onclose = () => {
                document.getElementById("status").innerText = "Disconnected!";
                document.getElementById("status").style.backgroundColor = "yellow";
                setTimeout(connectWebSocket, 2000); // Auto-reconnect after 2 seconds
            };
        }

        window.onload = connectWebSocket;

        function sendCommand(command) {
            if (ws.readyState === WebSocket.OPEN) {
                ws.send(command);
                console.log("Sent:", command);
            }
            document.getElementById("current_command").innerText = command;
            console.log(command);
        }

        // Speed Control
        let speedSlider = document.getElementById("speed");
        let speedValueDisplay = document.getElementById("speedValue");

        speedSlider.addEventListener("input", () => {
            let speed = speedSlider.value;
            speedValueDisplay.innerText = speed;
            sendCommand("speed:" + speed);
        });

        connectWebSocket();

        let keysPressed = {}; // Track keys to prevent repeated execution

        document.addEventListener("keydown", function (event) {
            if (!keysPressed[event.key]) { // Check if key is already pressed
                keysPressed[event.key] = true; // Mark key as pressed

                switch (event.key) {
                    case "w": case "ArrowUp":
                        sendCommand("forward_start");
                        console.log("Move forward");
                        break;
                    case "s": case "ArrowDown":
                        sendCommand("back_start");
                        console.log("Move Backwards");
                        break;
                    case "a": case "ArrowLeft":
                        sendCommand("left_start");
                        console.log("Move Left");
                        break;
                    case "d": case "ArrowRight":
                        sendCommand("right_start");
                        console.log("Move Right");
                        break;
                    case " ":
                        sendCommand("rear_break");
                        console.log("Brake Started");
                        break;
                    default:
                        console.log("Other key pressed: " + event.key);
                }
            }
        });

        document.addEventListener("keyup", function (event) {
            if (keysPressed[event.key]) { // Ensure key was pressed before releasing
                delete keysPressed[event.key]; // Remove key from tracking

                switch (event.key) {
                    case "w": case "ArrowUp":
                        sendCommand("forward_stop");
                        console.log("Stop forward");
                        break;
                    case "s": case "ArrowDown":
                        sendCommand("back_stop");
                        console.log("Stop Backwards");
                        break;
                    case "a": case "ArrowLeft":
                        sendCommand("left_stop");
                        console.log("Stop Left");
                        break;
                    case "d": case "ArrowRight":
                        sendCommand("right_stop");
                        console.log("Stop Right");
                        break;
                    case " ":
                        sendCommand("break_stop");
                        console.log("Stop Breaking");
                        break;
                    default:
                        console.log("Other key Released: " + event.key);
                }
            }
        });
    </script>

</body>

</html>
)rawliteral";


// Function for Forward Rear Action
void Forward_Rear(int duty_cycle) {
  // Forward direction: Q2 and Q3 ON
  digitalWrite(Q1_PIN, H);        // PMOS (Q1) OFF
  digitalWrite(Q2_PIN, L);        // PMOS (Q2) ON
  ledcWrite(Q3_PIN, duty_cycle);  // Control speed of NMOS (Q3) with PWM
  ledcWrite(Q4_PIN, 255);         // NMOS (Q4) OFF

  //JumpStart Feature for Very Low speeds
  if (duty_cycle >= JumpStart_duty && duty_cycle < 250) {
    ledcWrite(Q3_PIN, JumpStart_Value);
    delay(JumpStart_delay);  // Used JumpStart_delay variable to configure delay later
    ledcWrite(Q3_PIN, duty_cycle);
  }
}

// Function for Backward Rear Action
void Reverse_Rear(int duty_cycle) {
  // Backward direction: Q1 and Q4 ON
  digitalWrite(Q1_PIN, L);        // PMOS (Q1) ON
  digitalWrite(Q2_PIN, H);        // PMOS (Q2) OFF
  ledcWrite(Q3_PIN, 255);         // NMOS (Q3) OFF
  ledcWrite(Q4_PIN, duty_cycle);  // Control speed of NMOS (Q4) with PWM

  //JumpStart Feature for Very Low speeds
  if (duty_cycle >= JumpStart_duty && duty_cycle < 250) {
    ledcWrite(Q4_PIN, JumpStart_Value);
    delay(JumpStart_delay);  // Used JumpStart_delay variable to configure delay later
    ledcWrite(Q4_PIN, duty_cycle);
  }
}

// Function for Stopping Rear Action
void Rear_Stop() {
  digitalWrite(Q1_PIN, H);
  digitalWrite(Q2_PIN, H);
  ledcWrite(Q3_PIN, 255);
  ledcWrite(Q4_PIN, 255);
}

// Function for Dynamic Breaking (shorting Motor terminals via transistor to High side or Low side)
void Rear_Break() {
  // Turning ON both NPN Transistor
  digitalWrite(Q1_PIN, H);
  digitalWrite(Q2_PIN, H);
  ledcWrite(Q3_PIN, 0);  // Q3 ON
  ledcWrite(Q4_PIN, 0);  // Q4 ON
}

// Function to set steering at 0 degree
void Set_straight() {
  // digitalWrite(left_pin, LOW);
  // digitalWrite(right_pin, LOW);
  servo.write(90);
}

// Function to set steering left
void Left_Turn() {
  // digitalWrite(right_pin, LOW);
  // digitalWrite(left_pin, HIGH);
  servo.write(160);
}

//Function to set steering right
void Right_Turn() {
  // digitalWrite(left_pin, LOW);
  // digitalWrite(right_pin, HIGH);
  servo.write(20);  // Turn left
}

// Function to check Motor connection and working
void Hardware_check() {

  int delay_time = 10;
  pinMode(Q1_PIN, OUTPUT);
  pinMode(Q2_PIN, OUTPUT);
  // pinMode(left_pin, OUTPUT);
  // pinMode(right_pin, OUTPUT);

  digitalWrite(Q2_PIN, HIGH);
  for (int i = 10; i <= 200; i++) {
    ledcAttach(Q3_PIN, 100 * i, PWM_RESOLUTION);
    ledcWrite(Q3_PIN, 240);
    delay(delay_time);
  }
  // for (int i; i <= 100; i++) {
  //   ledcAttach(Q4_PIN, 1000 * i, PWM_RESOLUTION);
  //   digitalWrite(Q1_PIN, HIGH);
  //   ledcWrite(Q4_PIN, 230);
  //   delay(delay_time);
  //   digitalWrite(Q1_PIN, LOW);
  //   ledcWrite(Q4_PIN, 255);
  // }
  Left_Turn();
  delay(100);
  Right_Turn();
  delay(10);
}

// Function to send messages back to the user
void notifyClients(String message) {
  ws.textAll(message);  // This method is used to send message to client
}

// WebSocket Event Handler // Process Commands
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    String command = (char *)data;

    // Rear Control Commands
    if (command == "forward_start") Forward_Rear(speedValue);
    else if (command == "back_start") Reverse_Rear(speedValue);
    else if (command == "forward_stop") Rear_Stop();
    else if (command == "back_stop") Rear_Stop();
    else if (command == "rear_break") Rear_Break();
    else if (command == "break_stop") Rear_Stop();  // You can use Rear Stop which will turn off all transistor

    // Direction Control Commands
    else if (command == "left_start") Left_Turn();
    else if (command == "right_start") Right_Turn();
    else if (command == "right_stop") Set_straight();
    else if (command == "left_stop") Set_straight();
    else if (command.startsWith("speed:")) {
      speedValue = command.substring(6).toInt();
      Serial.println("Speed set to: " + String(speedValue));
    }
  }
}

// Function to handle user HTTP requests & Websocket request
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
             AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.println("WebSocket client connected");
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.println("WebSocket client disconnected");
  } else if (type == WS_EVT_DATA) {
    handleWebSocketMessage(arg, data, len);
  }
}

void setup() {
  Serial.begin(115200);

  Hardware_check();  // run hardware check for hardware working

  // Set up Access Point
  WiFi.softAP(ssid, password);
  Serial.println("AP started");
  Serial.println(WiFi.softAPIP());

  // Set up websocket
  ws.onEvent(onEvent);
  server.addHandler(&ws);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });

  server.begin();

  // Set motor control pins as output
  pinMode(Q1_PIN, OUTPUT);
  pinMode(Q2_PIN, OUTPUT);
  ledcAttach(Q3_PIN, PWM_FREQUENCY, PWM_RESOLUTION);
  ledcAttach(Q4_PIN, PWM_FREQUENCY, PWM_RESOLUTION);
  // pinMode(left_pin, OUTPUT);
  // pinMode(right_pin, OUTPUT);
  servo.setPeriodHertz(50);  // Standard 50hz servo
  servo.attach(servo_pin, minUs, maxUs);

  // Initially, turn off all transistors (ensures the motor is stopped)
  digitalWrite(Q1_PIN, H);
  digitalWrite(Q2_PIN, H);
  // digitalWrite(Q3_PIN, H);
  // digitalWrite(Q4_PIN, H);
  ledcWrite(Q3_PIN, 255);
  ledcWrite(Q4_PIN, 255);
  // digitalWrite(left_pin, LOW);
  // digitalWrite(right_pin, LOW);
  servo.write(90);
}


// Loop function
void loop() {
  // Nothing here! WebSocket is event-driven.
}
