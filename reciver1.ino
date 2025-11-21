/*
 * ESP32 RC Car Receiver
 * Controls 2 BTS7960 motor drivers for differential drive RC car
 * Receives commands via ESP-NOW protocol
 * Compatible with ESP32 Arduino Core v3.x
 * Uses analogWrite() instead of LEDC
 */

#include <esp_now.h>
#include <WiFi.h>

// Motor Driver 1 (Left Motor) - BTS7960 Pins
#define MOTOR1_RPWM 25  // Right PWM (Forward)
#define MOTOR1_LPWM 26  // Left PWM (Backward)
#define MOTOR1_R_EN 27  // Right Enable
#define MOTOR1_L_EN 14  // Left Enable

// Motor Driver 2 (Right Motor) - BTS7960 Pins
#define MOTOR2_RPWM 12  // Right PWM (Forward)
#define MOTOR2_LPWM 13  // Left PWM (Backward)
#define MOTOR2_R_EN 15  // Right Enable
#define MOTOR2_L_EN 2   // Left Enable

// Structure to receive data (must match transmitter)
typedef struct struct_message {
  int xAxis;      // Joystick X-axis value (-100 to 100)
  int yAxis;      // Joystick Y-axis value (-100 to 100)
  int speedLevel; // Speed control (0 to 100)
  bool button;    // Joystick button state
} struct_message;

struct_message incomingData;

// Timeout for connection loss
unsigned long lastReceiveTime = 0;
const unsigned long timeout = 1000;  // 1 second timeout

// Callback function executed when data is received
void OnDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *incomingDataByte, int len) {
  memcpy(&incomingData, incomingDataByte, sizeof(incomingData));
  lastReceiveTime = millis();
  
  // Debug output
  Serial.print("X: ");
  Serial.print(incomingData.xAxis);
  Serial.print(" | Y: ");
  Serial.print(incomingData.yAxis);
  Serial.print(" | Speed: ");
  Serial.print(incomingData.speedLevel);
  Serial.println("%");
  
  // Process motor control
  controlMotors(incomingData.xAxis, incomingData.yAxis, incomingData.speedLevel);
}

void setup() {
  Serial.begin(115200);
  
  // Configure all motor pins as outputs
  pinMode(MOTOR1_RPWM, OUTPUT);
  pinMode(MOTOR1_LPWM, OUTPUT);
  pinMode(MOTOR1_R_EN, OUTPUT);
  pinMode(MOTOR1_L_EN, OUTPUT);
  
  pinMode(MOTOR2_RPWM, OUTPUT);
  pinMode(MOTOR2_LPWM, OUTPUT);
  pinMode(MOTOR2_R_EN, OUTPUT);
  pinMode(MOTOR2_L_EN, OUTPUT);
  
  // Enable both motors
  digitalWrite(MOTOR1_R_EN, HIGH);
  digitalWrite(MOTOR1_L_EN, HIGH);
  digitalWrite(MOTOR2_R_EN, HIGH);
  digitalWrite(MOTOR2_L_EN, HIGH);
  
  // Initialize all PWM pins to 0
  analogWrite(MOTOR1_RPWM, 0);
  analogWrite(MOTOR1_LPWM, 0);
  analogWrite(MOTOR2_RPWM, 0);
  analogWrite(MOTOR2_LPWM, 0);
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  
  // Print MAC Address (IMPORTANT: Copy this to transmitter code!)
  Serial.print("Receiver MAC Address: ");
  Serial.println(WiFi.macAddress());
  Serial.println("COPY THIS MAC ADDRESS TO TRANSMITTER CODE!");
  
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Register callback function
  esp_now_register_recv_cb(OnDataRecv);
  
  Serial.println("Receiver Ready!");
}

void loop() {
  // Check for connection timeout
  if (millis() - lastReceiveTime > timeout) {
    // Stop motors if no signal received
    stopMotors();
  }
  
  delay(10);
}

// Function to control motors based on joystick input
void controlMotors(int xAxis, int yAxis, int speedLevel) {
  // Calculate base speed from potentiometer (0-255)
  int baseSpeed = map(speedLevel, 0, 100, 0, 255);
  
  // Differential drive calculation
  int leftMotorSpeed = 0;
  int rightMotorSpeed = 0;
  
  // Forward/Backward component (Y-axis)
  leftMotorSpeed = map(yAxis, -100, 100, -baseSpeed, baseSpeed);
  rightMotorSpeed = map(yAxis, -100, 100, -baseSpeed, baseSpeed);
  
  // Steering component (X-axis)
  // Negative X = Left turn, Positive X = Right turn
  int steeringAdjustment = map(abs(xAxis), 0, 100, 0, baseSpeed);
  
  if (xAxis < 0) {
    // Turn left: reduce left motor, increase right motor
    leftMotorSpeed -= steeringAdjustment;
    rightMotorSpeed += steeringAdjustment;
  } else if (xAxis > 0) {
    // Turn right: reduce right motor, increase left motor
    leftMotorSpeed += steeringAdjustment;
    rightMotorSpeed -= steeringAdjustment;
  }
  
  // Constrain values to valid range
  leftMotorSpeed = constrain(leftMotorSpeed, -255, 255);
  rightMotorSpeed = constrain(rightMotorSpeed, -255, 255);
  
  // Set motor speeds and directions
  setMotor1(leftMotorSpeed);
  setMotor2(rightMotorSpeed);
}

// Function to control Motor 1 (Left)
void setMotor1(int speed) {
  if (speed > 0) {
    // Forward
    analogWrite(MOTOR1_RPWM, abs(speed));
    analogWrite(MOTOR1_LPWM, 0);
  } else if (speed < 0) {
    // Backward
    analogWrite(MOTOR1_RPWM, 0);
    analogWrite(MOTOR1_LPWM, abs(speed));
  } else {
    // Stop
    analogWrite(MOTOR1_RPWM, 0);
    analogWrite(MOTOR1_LPWM, 0);
  }
}

// Function to control Motor 2 (Right)
void setMotor2(int speed) {
  if (speed > 0) {
    // Forward
    analogWrite(MOTOR2_RPWM, abs(speed));
    analogWrite(MOTOR2_LPWM, 0);
  } else if (speed < 0) {
    // Backward
    analogWrite(MOTOR2_RPWM, 0);
    analogWrite(MOTOR2_LPWM, abs(speed));
  } else {
    // Stop
    analogWrite(MOTOR2_RPWM, 0);
    analogWrite(MOTOR2_LPWM, 0);
  }
}

// Function to stop all motors
void stopMotors() {
  analogWrite(MOTOR1_RPWM, 0);
  analogWrite(MOTOR1_LPWM, 0);
  analogWrite(MOTOR2_RPWM, 0);
  analogWrite(MOTOR2_LPWM, 0);
}
