# PuppyBot Web Controller 🐶

This project implements a **web-based controller for the PuppyGo quadruped robot**, allowing you to control walking, turning, and standing behaviors directly from a browser.  
It runs on an **M5StickC / M5StickC Plus** (ESP32-based) and uses a simple HTML interface served over Wi-Fi.

---

##  Features

-  Four basic gaits:
  - **Go (Forward)** – Walk forward smoothly  
  - **Back (Backward)** – Reverse gait  
  - **Left / Right** – Turn in place with coordinated leg motion  
-  **Web UI** with D-pad controls (works on desktop and mobile)
-  Displays the device **IP address** on screen
-  Simple HTML + WebSocket interface for real-time control
-  Adjustable gait speed and angle amplitudes

---

##  Hardware Requirements

| Component | Description |
|------------|-------------|
| [M5StickC / M5StickC Plus](https://shop.m5stack.com/) | Main controller |
| PuppyGo / compatible quadruped frame | 6–8 DOF mini robot |
| Wi-Fi connection | For the web interface |

---

##  Software Setup

1. **Install the Arduino IDE**
   - Version 2.0+ recommended  
   - Add the ESP32 board package via Board Manager

2. **Install Required Libraries**
   - `M5StickC` or `M5StickCPlus`
   - `WiFi.h`
   - `WebServer.h` or `ESPAsyncWebServer.h`
   - `WebSocketsServer.h`

3. **Open the Sketch**
   ```bash
   PuppyCWeb/PuppyCWeb.ino
   ```

4. **Adjust Your Wi-Fi Credentials to whatever you want**
   ```cpp
   const char* ssid = "YOUR_WIFI";
   const char* password = "YOUR_PASSWORD";
   ```

5. **Upload to the M5StickC**

---

##  Usage

1. After booting, check the **display for the IP address**.
2. Open a browser and navigate to `http://<device_ip>`.
3. Use the **D-pad interface** to control the robot:
   - ⬆️ Forward  
   - ⬇️ Backward  
   - ⬅️ Turn Left  
   - ➡️ Turn Right  

---

##  Gait Sequences

The walking and turning motions are defined by 6-step keyframes specifying the **servo angles** for each leg.  
You can fine-tune gait smoothness or speed by adjusting these arrays in the sketch:

```cpp
// Example: stronger left turn gait
float puppyLeft[6][4] = {
  {20, 10, 0, 0},
  {10, 20, 0, 0},
  {0, 30, 0, 0},
  {0, 0, -30, 0},
  {0, 0, -20, -10},
  {0, 0, -10, -20}
};
```

For finer control, you can modify gait amplitude, phase, or delay timing between legs.

---

## Author

Developed by **Jose Victorio Salazar**  
Smart Robots Design Lab — Tohoku University  
Contributions, issues, and pull requests are welcome!

---

## License

This project is licensed under the **MIT License**.  
See [LICENSE](LICENSE) for details.
