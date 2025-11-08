#include <M5StickC.h>
#include <WiFi.h>
#include <WebServer.h>
#include "puppy.h"

// ===== Wi-Fi Config =====
const char* ssid = "PuppyC_Robot";
const char* password = "12345678";

WebServer server(80);

// ===== Movement state =====
enum Movement { STOP = 0, FORWARD, BACKWARD, LEFT, RIGHT };
volatile Movement movementState = STOP;

// timing for gait steps
const unsigned long STEP_INTERVAL_MS = 100;
unsigned long lastStepMillis = 0;
uint8_t stepIndex = 0;
const uint8_t STEPS = 6;

// ===== Original forward gait (6 steps) =====
const uint8_t gait_forward[STEPS][4] = {
  {55,  60, 130, 120},
  {55, 160, 130, 120},
  {120,130, 150, 60},
  {120,130, 60,  60},
  {55, 130, 60,  60},
  {55,  60, 130, 30}
};

// ===== Original backward gait (6 steps) =====
const uint8_t gait_backward[STEPS][4] = {
  {120,130,60, 55},
  {120,130,160,55},
  {60, 150,130,120},
  {60,  60,130,120},
  {60,  60,130,55},
  {30, 130,60, 55}
};

// ===== Turn LEFT (right legs forward, left legs backward) =====
const uint8_t gait_left[STEPS][4] = {
  // servo1 (front-left), servo2 (front-right),
  // servo3 (rear-left),  servo4 (rear-right)
  {130,  60, 130,  60},   // start neutral-ish
  {150,  50, 150,  50},   // left back, right forward
  {170,  80, 170,  80},   // exaggerate difference
  {130, 120, 130, 120},   // right legs step further forward
  {100, 150, 100, 150},   // left legs swing backward
  {115, 100, 115, 100},   // return to near-neutral
};

// ===== Turn RIGHT (left legs forward, right legs backward) =====
const uint8_t gait_right[STEPS][4] = {
  { 60, 130,  60, 130},   // start neutral-ish
  { 50, 150,  50, 150},   // left forward, right back
  { 70, 160,  70, 160},   // exaggerate difference
  {120, 130, 120, 130},   // left legs step further forward
  {150, 100, 150, 100},   // right legs swing backward
  {100, 115, 100, 115},   // return to near-neutral
};

// ===== Helper to apply a step =====
void applyStep(const uint8_t step[4]) {
  angle_all_set(step[0], step[1], step[2], step[3]);
}

// ===== Neutral / stop pose =====
void puppy_start() {
  angle_all_set(90, 90, 90, 90);
}

// ===== Web Handlers ======
void handleRoot() {
  String html = R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
  <meta name="viewport" content="width=device-width, initial-scale=1" charset="UTF-8">
  <style>
    body { text-align:center; background:#f4f4f4; font-family:sans-serif; }
    h2 { margin-top:10px; }
    button {
      width: 80px; height: 80px;
      margin: 10px; font-size: 20px;
      border-radius: 20px; background:#007aff; color:white; border:none;
      touch-action: manipulation;
    }
    .row { display: flex; justify-content: center; }
  </style>
  <script>
    function sendCmd(cmd) {
      navigator.sendBeacon('/cmd?dir=' + cmd); // non-blocking on hold events
    }
    function down(cmd) {
      fetch('/cmd?dir=' + cmd + '&action=down');
    }
    function up() {
      fetch('/cmd?dir=stop');
    }
  </script>
  </head>
  <body>
    <h2>PuppyC Controller</h2>
    <div class="row">
      <button onmousedown="down('forward')" onmouseup="up()" ontouchstart="down('forward')" ontouchend="up()">↑</button>
    </div>
    <div class="row">
      <button onmousedown="down('left')" onmouseup="up()" ontouchstart="down('left')" ontouchend="up()">←</button>
      <button onmousedown="down('backward')" onmouseup="up()" ontouchstart="down('backward')" ontouchend="up()">↓</button>
      <button onmousedown="down('right')" onmouseup="up()" ontouchstart="down('right')" ontouchend="up()">→</button>
    </div>
    <p>Hold button to move. Release to stop.</p>
  </body>
  </html>
  )rawliteral";

  server.send(200, "text/html", html);
}

void handleCmd() {
  String dir = server.arg("dir");
  String action = server.arg("action"); // optional: 'down'
  if (dir == "forward") {
    movementState = FORWARD;
  } else if (dir == "backward") {
    movementState = BACKWARD;
  } else if (dir == "left") {
    movementState = LEFT;
  } else if (dir == "right") {
    movementState = RIGHT;
  } else if (dir == "stop") {
    movementState = STOP;
    puppy_start(); // immediately set neutral pose
  }
  server.send(200, "text/plain", "OK");
}

// ====== Setup ======
void setup() {
  M5.begin();
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.setTextSize(1);

  M5.Lcd.println("🐶 PuppyC WiFi Bot");
  M5.Lcd.println("Starting...");

  IIC_Init();  // init I2C for Puppy Hat
  puppy_start();

  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();

  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.println("AP Mode:");
  M5.Lcd.println("");
  M5.Lcd.println("SSID: " + String(ssid));
  M5.Lcd.println("PWD:  " + String(password));
  M5.Lcd.println("IP:   " + myIP.toString());

  server.on("/", handleRoot);
  server.on("/cmd", handleCmd);
  server.begin();
  Serial.begin(115200);
  Serial.println("Web server started.");
}

// ====== Loop: run gait steps depending on movementState ======
void loop() {
  server.handleClient();

  unsigned long now = millis();
  if (now - lastStepMillis < STEP_INTERVAL_MS) return;
  lastStepMillis = now;

  switch (movementState) {
    case STOP:
      // stay neutral (do not advance stepIndex)
      stepIndex = 0;
      break;

    case FORWARD:
      applyStep(gait_forward[stepIndex]);
      stepIndex = (stepIndex + 1) % STEPS;
      break;

    case BACKWARD:
      applyStep(gait_backward[stepIndex]);
      stepIndex = (stepIndex + 1) % STEPS;
      break;

    case LEFT:
      applyStep(gait_left[stepIndex]);
      stepIndex = (stepIndex + 1) % STEPS;
      break;

    case RIGHT:
      applyStep(gait_right[stepIndex]);
      stepIndex = (stepIndex + 1) % STEPS;
      break;
  }
}
