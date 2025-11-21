/*
 * ESP32 RC Car Transmitter (ESP32 Arduino Core v3.x Compatible)
 */

#include <esp_now.h>
#include <WiFi.h>

// Pin Definitions
#define JOYSTICK_X_PIN 34
#define JOYSTICK_Y_PIN 35
#define JOYSTICK_SW_PIN 32
#define POT_PIN 33

uint8_t receiverMAC[] = {0x80, 0xF3, 0xDA, 0x55, 0x8C, 0x48};

// Structure to send data
typedef struct struct_message {
  int xAxis;
  int yAxis;
  int speedLevel;
  bool button;
} struct_message;

struct_message myData;

// Deadzone settings
int deadzone = 15;

// NEW CALLBACK FORMAT for ESP32 Core v3.x
void OnDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status) {
  Serial.print("Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void setup() {
  Serial.begin(115200);

  pinMode(JOYSTICK_SW_PIN, INPUT_PULLUP);

  // WiFi must be in station mode
  WiFi.mode(WIFI_STA);

  Serial.print("Transmitter MAC: ");
  Serial.println(WiFi.macAddress());

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  // Register callback
  esp_now_register_send_cb(OnDataSent);

  // Setup peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMAC, 6);
  peerInfo.channel = 0;  // Auto / match receiver
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  Serial.println("Transmitter Ready!");
}

void loop() {
  int rawX = analogRead(JOYSTICK_X_PIN);
  int rawY = analogRead(JOYSTICK_Y_PIN);
  int rawP = analogRead(POT_PIN);

  // Map values to -100..100
  int x = map(rawX, 0, 4095, -100, 100);
  int y = map(rawY, 0, 4095, -100, 100);

  if (abs(x) < deadzone) x = 0;
  if (abs(y) < deadzone) y = 0;

  int speedLevel = map(rawP, 0, 4095, 0, 100);
  bool btn = !digitalRead(JOYSTICK_SW_PIN);

  // Fill struct
  myData.xAxis = x;
  myData.yAxis = y;
  myData.speedLevel = speedLevel;
  myData.button = btn;

  // Send
  esp_now_send(receiverMAC, (uint8_t*)&myData, sizeof(myData));

  Serial.print("X: "); Serial.print(x);
  Serial.print(" Y: "); Serial.print(y);
  Serial.print(" Speed: "); Serial.print(speedLevel);
  Serial.print(" Btn: "); Serial.println(btn);

  delay(50);
}
