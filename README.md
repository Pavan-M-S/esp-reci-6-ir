# ESP32 4-Way Traffic Density Receiver & Cloud Emergency Dispatcher

[![Platform](https://img.shields.io/badge/Platform-PlatformIO-orange.svg)](https://platformio.org/)
[![Hardware](https://img.shields.io/badge/Hardware-ESP32--DevKit-blue.svg)](https://www.espressif.com/)
[![Database](https://img.shields.io/badge/Database-Firebase%20RTDB-yellow.svg)](https://firebase.google.com/)

A real-time traffic monitoring and cloud gateway firmware running on the ESP32. It ingests 4-way intersection congestion and dynamic timer telemetry from an upstream controller (e.g. Arduino Nano) via hardware UART, batches the metrics into structured JSON for Google Firebase Realtime Database, and monitors the cloud for emergency vehicle preemption commands.

---

## Architecture & Data Flow

```
+--------------------+        UART Serial1 (38400 baud)        +-------------------+        Wi-Fi / JSON Batch        +------------------------+
| 4-Road IR Sensor   | --------------------------------------> |   ESP32 Gateway   | -------------------------------> | Google Firebase RTDB   |
| Controller (Nano)  |   R1, R1T, R2, R2T, R3, R3T, R4, R4T    |   (This Device)   |                                  | (Root JSON Tree)       |
+--------------------+ <-------------------------------------- +-------------------+ <------------------------------- +------------------------+
                               35x "Emergency" Bursts                                      Polls "/Set" == 2
```

---

## Features

- **4-Way Junction Telemetry Parsing**: Ingests vehicle counts (`R1` through `R4`) and calculated timer durations (`R1T` through `R4T`) along with system `Mode` over hardware UART `Serial1` (GPIO 21 RX / GPIO 22 TX at 38400 baud).
- **Atomic Batch JSON Cloud Upload**: Utilizes `FirebaseJson` to atomically write all 9 traffic metrics to the Firebase database in a single HTTP transaction, drastically reducing cloud round-trips.
- **Bi-Directional Cloud Emergency Override**:
  - Continuously polls `/Set` in Firebase Realtime Database.
  - When an emergency vehicle is detected (`/Set == 2`), the ESP32 transmits a high-priority burst of 35 `"Emergency"` signals to the traffic light controller to preempt the signal timing and grant green corridors.
  - Automatically clears the flag (`/Set = 0`) upon successful dispatch.
- **Automatic Fallback Mode**: When the upstream sensor controller is silent, automatically defaults all road counts to zero and switches to fallback mode (`Mode = 1`).

---

## Hardware & Pinout

| ESP32 Pin | Signal | Target Device | Notes |
| :--- | :--- | :--- | :--- |
| `GPIO 21` | `RX1` | Arduino Nano (TX) | UART input from sensor controller |
| `GPIO 22` | `TX1` | Arduino Nano (RX) | UART output for emergency dispatch |
| `GND` | Ground | Common Ground | Common ground between boards |
| `USB` | Debug | Host PC | Serial debug console at 115200 baud |

---

## Software Dependencies & Configuration

From [`platformio.ini`](platformio.ini):

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
lib_deps = 
    mobizt/Firebase ESP32 Client@^4.4.17
```

### Credentials Setup
Update network credentials and Firebase configuration in [`src/main.cpp`](src/main.cpp):

```cpp
#define FIREBASE_PROJECT_API_KEY "YOUR_FIREBASE_API_KEY"
#define FIREBASE_DATABASE_URL    "https://your-project.firebaseio.com/"
#define FIREBASE_EMAIL           "your-email@example.com"
#define FIREBASE_PASSWORD        "your-password"

#define WIFI_SSID                "YOUR_WIFI_SSID"
#define WIFI_PASSWORD            "YOUR_WIFI_PASSWORD"
```

---

## How to Build & Flash

```bash
# Compile source code
pio run

# Upload firmware
pio run --target upload

# View live telemetry in Serial Monitor
pio device monitor -b 115200
```

---

## Project Structure

```
esp reci 6 ir/
├── src/
│   └── main.cpp         # 4-road telemetry parser, Firebase batch JSON sync & emergency handler
└── platformio.ini       # PlatformIO environment & Firebase library configuration
```

---

## License

MIT License.
