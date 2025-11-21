
# 🚗 **ESP32 RC Car using ESP-NOW (Transmitter + Receiver)**

A fully wireless RC-car system built using **two ESP32 boards**, **two BTS7960 high-current motor drivers**, and **ESP-NOW** for ultra-fast and low-latency communication.

This project supports:

✔ Analog joystick steering
✔ Potentiometer-based speed control
✔ Button input
✔ Differential motor control
✔ Automatic failsafe (motors stop if signal is lost)
✔ Expandable multi-channel design

---

# 🧩 **Project Overview**

This system has **two ESP32 boards**:

### **1️⃣ Transmitter ESP32**

* Reads **joystick X/Y**
* Reads **potentiometer** (speed control)
* Reads **joystick switch button**
* Sends the values wirelessly to the receiver using **ESP-NOW**

### **2️⃣ Receiver ESP32**

* Receives joystick data
* Processes differential drive (left/right motors)
* Controls **two BTS7960 motor drivers**
* Automatically stops motors if transmitter signal is lost

---

# 📦 Hardware Required

### 🔹 **Transmitter Side**

| Component                       | Qty     |
| ------------------------------- | ------- |
| ESP32 Dev Board                 | 1       |
| 2-axis Joystick Module          | 1/2     |
| Potentiometer (10k recommended) | 1       |
| Jumper wires                    | Several |
| 5V power bank / Li-ion battery  | 1       |

---

### 🔹 **Receiver / RC Car Side**

| Component                             | Qty                    |
| ------------------------------------- | ---------------------- |
| ESP32 Dev Board                       | 1                      |
| BTS7960 Motor Driver                  | 2 (Left + Right motor) |
| 12V–24V battery for motors            | 1                      |
| DC motors (high torque)               | 2                      |
| Chassis + wheels                      | 1 set                  |
| Jumper wires (thick wires for motors) | Several                |

---

# 🕹️ **Transmitter Wiring**

### **Joystick**

| Joystick Pin | ESP32 Pin |
| ------------ | --------- |
| VRX          | GPIO34    |
| VRY          | GPIO35    |
| SW           | GPIO32    |
| +5V          | 3.3V/5V   |
| GND          | GND       |

### **Potentiometer**

| Pot Pin       | ESP32 Pin |
| ------------- | --------- |
| Middle Output | GPIO33    |
| One Side      | 3.3V/5V   |
| Other Side    | GND       |

---

# 🚗 **Receiver Wiring (BTS7960 + Motors)**

## **Motor Driver 1 – LEFT Motor**

| BTS7960 Pin | ESP32 Pin |
| ----------- | --------- |
| RPWM        | GPIO25    |
| LPWM        | GPIO26    |
| R_EN        | GPIO27    |
| L_EN        | GPIO14    |

## **Motor Driver 2 – RIGHT Motor**

| BTS7960 Pin | ESP32 Pin |
| ----------- | --------- |
| RPWM        | GPIO12    |
| LPWM        | GPIO13    |
| R_EN        | GPIO15    |
| L_EN        | GPIO2     |

### **BTS7960 Power**

* **B+ → Motor Battery +**
* **B− → Motor Battery −**
* **Motor terminals → actual motor**

⚠️ **Never power motors from ESP32 5V — BTS takes a separate supply.**

---

# 🛰️ **ESP-NOW Pairing (IMPORTANT)**

###  🟦 Step 1 — Get the Receiver’s MAC Address

Upload the *receiver code* first.

Open Serial Monitor → it prints:

```
Receiver MAC Address: 80:F3:DA:55:8C:48
COPY THIS MAC ADDRESS TO TRANSMITTER CODE!
```

### 🟦 Step 2 — Add MAC to Transmitter Code

In the transmitter code, replace:

```cpp
uint8_t receiverMAC[] = {0x80, 0xF3, 0xDA, 0x55, 0x8C, 0x48};
```

with your own receiver’s MAC.

If this is wrong → transmitter will NOT connect.

---

# 📡 **How the System Works**

### 🕹️ Transmitter

* Reads joystick X/Y (0–4095)
* Converts to −100 … +100 range
* Reads potentiometer → 0–100 speedLevel
* Packs into a struct:

```cpp
struct_message {
  int xAxis;
  int yAxis;
  int speedLevel;
  bool button;
};
```

* Sends to receiver via ESP-NOW every 50ms

### 🚗 Receiver

* Gets the struct and extracts X/Y/speed values
* Calculates:

  * Forward/backward speed (Y axis)
  * Left/right steering adjustment (X axis)
* Uses differential drive:

  * Left motor speed adjusted opposite of right motor
* Speed converted to PWM (0–255)
* Controls motors through BTS7960 PWMs
* Timeout protection stops motors if no signal for 1 sec


---

# 📡 **Transmitter Code Explanation**

### **1. Reads Joystick + Pot**

```cpp
int rawX = analogRead(JOYSTICK_X_PIN);
int rawY = analogRead(JOYSTICK_Y_PIN);
int rawP = analogRead(POT_PIN);
```

### **2. Convert joystick raw values to a usable −100 to +100 range**

```cpp
x = map(rawX, 0, 4095, -100, 100);
```

### **3. Apply deadzone**

Removes jitter around the center.

### **4. Send Data via ESP-NOW**

```cpp
esp_now_send(receiverMAC, (uint8_t*)&myData, sizeof(myData));
```

---

# 🚗 **Receiver Code Explanation**

### **1. ESP-NOW receive callback**

Triggered every time transmitter sends data.

```cpp
void OnDataRecv(...)  
{
  memcpy(&incomingData, incomingDataByte, sizeof(incomingData));
  controlMotors(...);
}
```

### **2. Differential drive calculation**

```cpp
leftMotorSpeed = map(yAxis, -100, 100, -baseSpeed, baseSpeed);
rightMotorSpeed = map(yAxis, -100, 100, -baseSpeed, baseSpeed);
```

### **3. Steering adjustments**

```cpp
if (xAxis < 0) { turn left }
if (xAxis > 0) { turn right }
```

### **4. Apply to BTS7960**

Forward:

```cpp
analogWrite(RPWM, speed);
analogWrite(LPWM, 0);
```

Backward:

```cpp
analogWrite(LPWM, speed);
analogWrite(RPWM, 0);
```

### **5. Signal loss protection**

If no packet is received for 1 second → motors stop.

---

# 🚀 **Future Improvements**

Here are things you can add later (easy upgrades):

### **🔹 1. Extra Channels**

* Add headlights using LED
* Horn/buzzer
* Servo for camera pan/tilt
* Extra buttons for custom features

Just expand the struct:

```cpp
int extra1;
int extra2;
bool sw2;
```

### **🔹 2. FPV Camera System**

Add ESP32-CAM or analog FPV system.

### **🔹 3. OLED Display on Transmitter**

* Show speed
* Battery status
* Signal strength

### **🔹 4. Telemetry Return Channel**

Receiver → Transmitter
Send:

* Battery voltage
* Motor temperature
* Distance sensor values

### **🔹 5. Add PID Motor Control**

Smoothen movement and avoid jerkiness.

---

# 🧪 **Testing Tips**

* Always test motors with wheels lifted first
* Expect the joystick Y-axis to control forward/back
* X-axis controls left/right turning
* Potentiometer smoothens speed


