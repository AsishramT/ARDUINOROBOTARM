#include <ArduinoBLE.h>
#include <Stepper.h>

// BLE Service and Characteristic
BLEService robotService("19B10000-E8F2-537E-4F6C-D104768A1214");
BLEStringCharacteristic commandChar("19B10001-E8F2-537E-4F6C-D104768A1214",BLERead | BLEWrite | BLENotify, 20);


const int stepsPerRevolution = 200;

// Enable pins for all 4 L298N drivers
const int ENA1 = 6, ENB1 = 7;
const int ENA2 = 12, ENB2 = 13;
const int ENA3 = A4, ENB3 = A5;
const int ENA4 = 2, ENB4 = 3;

// Create stepper objects
Stepper motor1(stepsPerRevolution, A0, A1, A3, A4); //confirmed
Stepper motor2(stepsPerRevolution, 0, 1, 2, 4);
Stepper motor3(stepsPerRevolution, 4, 5, 6, 7);

// Track active motors
int activeMotorCount = 0;
int motor1Active = 0, motor2Active = 0, motor3Active = 0;

void setup() {
  Serial.begin(9600);
  delay(1000);
  
  Serial.println("\n🦾 ROBOT ARM STARTUP");
  Serial.println("===================");
  
  // Setup enable pins
  pinMode(ENA1, OUTPUT); pinMode(ENB1, OUTPUT);
  pinMode(ENA2, OUTPUT); pinMode(ENB2, OUTPUT);
  pinMode(ENA3, OUTPUT); pinMode(ENB3, OUTPUT);
  pinMode(ENA4, OUTPUT); pinMode(ENB4, OUTPUT);
  
  disableAll();
  
  // Set motor speeds
  motor1.setSpeed(60);
  motor2.setSpeed(60);
  motor3.setSpeed(60);
  
  Serial.println("✅ Motors configured");
  
  // Initialize BLE
  if (!BLE.begin()) {
    Serial.println("❌ Starting BLE failed!");
    while (1);
  }
  
  Serial.println("✅ BLE initialized");
  
  // Set device name BEFORE advertising
  BLE.setLocalName("Arduino");
  BLE.setAdvertisedService(robotService);
  
  
  // Add characteristic with proper properties
  robotService.addCharacteristic(commandChar);
  BLE.addService(robotService);
  
  // Set initial value
  commandChar.writeValue("ready");
  
  Serial.println("✅ BLE Service added");
  
  BLE.advertise();
  
  Serial.println("\n🦾 Bluetooth Robot Arm Ready!");
  Serial.println("📡 Advertising as: Arduino");
  Serial.println("Waiting for Bluetooth connections...\n");
}

void loop() {
  BLEDevice central = BLE.central();
  
  if (central) {
    Serial.print("📱 Connected to: ");
    Serial.println(central.address());
    Serial.println("✅ Ready to receive commands!\n");
    
    while (central.connected()) {
      // Check if command was written
      if (commandChar.written()) {
        String command = commandChar.value();
        
        Serial.print("📨 Received command: ");
        Serial.println(command);
        
        processCommand(command);
      }
      
      delay(10);
    }
    
    Serial.println("\n❌ Disconnected\n");
    disableAll();
    activeMotorCount = 0;
  }
}

void processCommand(String cmd) {
  if (cmd.length() < 2) {
    Serial.println("  ⚠️  Invalid command length");
    return;
  }
  
  char motorNum = cmd.charAt(0);
  char direction = cmd.charAt(1);
  
  // Validate direction
  if (direction != 'u' && direction != 'd') {
    Serial.println("  ⚠️  Invalid direction (use 'u' or 'd')");
    return;
  }
  
  // Check motor limit (max 2)
  if (!isMotorActive(motorNum) && activeMotorCount >= 2) {
    Serial.print("  ⚠️  Max 2 motors active. Currently: ");
    Serial.print(activeMotorCount);
    Serial.println(" motors");
    return;
  }
  
  // Process motor command
  processMotor(motorNum, direction);
}

int isMotorActive(char motorNum) {
  if (motorNum == '1') return motor1Active;
  if (motorNum == '2') return motor2Active;
  if (motorNum == '3') return motor3Active;
  return 0;
}

void processMotor(char motorNum, char direction) {
  int steps = 100;
  
  if (motorNum == '1') {
    if (!motor1Active) {
      enableMotor(ENA1, ENB1);
      motor1Active = 1;
      activeMotorCount++;
    }
    
    if (direction == 'u') {
      motor1.step(steps);
      Serial.println("  ⬆️  Motor 1 UP");
    } else {
      motor1.step(-steps);
      Serial.println("  ⬇️  Motor 1 DOWN");
    }
    
    motor1Active = 0;
    activeMotorCount--;
    disableMotor(ENA1, ENB1);
  }
  
  else if (motorNum == '2') {
    if (!motor2Active) {
      enableMotor(ENA2, ENB2);
      motor2Active = 1;
      activeMotorCount++;
    }
    
    if (direction == 'u') {
      motor2.step(steps);
      Serial.println("  ⬆️  Motor 2 UP");
    } else {
      motor2.step(-steps);
      Serial.println("  ⬇️  Motor 2 DOWN");
    }
    
    motor2Active = 0;
    activeMotorCount--;
    disableMotor(ENA2, ENB2);
  }
  
  else if (motorNum == '3') {
    if (!motor3Active) {
      enableMotor(ENA3, ENB3);
      motor3Active = 1;
      activeMotorCount++;
    }
    
    if (direction == 'u') {
      motor3.step(steps);
      Serial.println("  ⬆️  Motor 3 UP");
    } else {
      motor3.step(-steps);
      Serial.println("  ⬇️  Motor 3 DOWN");
    }
    
    motor3Active = 0;
    activeMotorCount--;
    disableMotor(ENA3, ENB3);
  }
}

void enableMotor(int ena, int enb) {
  digitalWrite(ena, HIGH);
  digitalWrite(enb, HIGH);
}

void disableMotor(int ena, int enb) {
  digitalWrite(ena, LOW);
  digitalWrite(enb, LOW);
}

void disableAll() {
  digitalWrite(ENA1, LOW); digitalWrite(ENB1, LOW);
  digitalWrite(ENA2, LOW); digitalWrite(ENB2, LOW);
  digitalWrite(ENA3, LOW); digitalWrite(ENB3, LOW);
  digitalWrite(ENA4, LOW); digitalWrite(ENB4, LOW);
}