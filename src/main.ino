#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

// Pin Configuration
SoftwareSerial mySerial(2, 3); // RX, TX for fingerprint sensor
LiquidCrystal_I2C lcd(0x27, 16, 2); // Set the LCD I2C address
Adafruit_Fingerprint finger(&mySerial); // Initialize fingerprint sensor with SoftwareSerial

const int buzzerPin = 8; // Buzzer pin
const int relayPin = 7; // Relay pin
const int greenLEDPin = 9; // Green LED pin
const int redLEDPin = 10; // Red LED pin

// Define custom characters for the stop sign and arrow animations
byte stopSign1[8] = {
  0b00000,
  0b01110,
  0b11111,
  0b11111,
  0b11111,
  0b01110,
  0b00000,
  0b00000
};

byte stopSign2[8] = {
  0b00000,
  0b00000,
  0b01110,
  0b11111,
  0b11111,
  0b01110,
  0b00000,
  0b00000
};

byte arrow1[8] = {
  0b00000,
  0b00000,
  0b00000,
  0b11111,
  0b01110,
  0b00100,
  0b00000,
  0b00000
};

byte arrow2[8] = {
  0b00000,
  0b00000,
  0b11111,
  0b11111,
  0b01110,
  0b00100,
  0b00000,
  0b00000
};

void setup() {
  // Initialize Serial Monitor for debugging
  Serial.begin(9600);
  
  // Initialize the LCD
  lcd.init();                      
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("VideotronicMaker"); // Display welcome message on the LCD
  lcd.setCursor(0, 1);
  lcd.print("Touch Sensor");

  // Set pin modes for LED, buzzer, and relay
  pinMode(buzzerPin, OUTPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(greenLEDPin, OUTPUT);
  pinMode(redLEDPin, OUTPUT);

  // Initialize the fingerprint sensor
  mySerial.begin(57600); // Start SoftwareSerial for fingerprint sensor        
  finger.begin(57600); // Start communication with the fingerprint sensor

  // Check if the fingerprint sensor is connected properly
  if (finger.verifyPassword()) {
    lcd.setCursor(0, 1);
    lcd.print("                "); // Clear the line
    lcd.setCursor(0, 1);
    lcd.print("Sensor ready"); // Display sensor ready message
    Serial.println("Fingerprint sensor is ready."); // Output to Serial Monitor
  } else {
    lcd.setCursor(0, 1);
    lcd.print("                "); // Clear the line
    lcd.setCursor(0, 1);
    lcd.print("Init error"); // Display initialization error
    Serial.println("Fingerprint sensor initialization failed."); // Output to Serial Monitor
  }

  // Create custom characters for animations
  lcd.createChar(0, stopSign1);
  lcd.createChar(1, stopSign2);
  lcd.createChar(2, arrow1);
  lcd.createChar(3, arrow2);

  delay(2000); // Display initial message for 2 seconds
  resetState();
}

void loop() {
  // Default state: Show "VideotronicMaker" on the LCD
  lcd.setCursor(0, 0);
  lcd.print("VideotronicMaker");
  lcd.setCursor(0, 1);
  lcd.print("Touch Sensor");

  // Check if a finger is placed on the sensor
  if (finger.getImage() == FINGERPRINT_OK) {
    int id = getFingerprintIDez();
    if (id == -1) {
      handleAccessDenied(); // Unauthorized access
    } else {
      handleAccessGranted(); // Authorized access
    }

    // Reset state after handling access
    resetState();
  }

  delay(100); // Small delay to avoid overwhelming the sensor
}

void handleAccessDenied() {
  // Handle unauthorized access
  digitalWrite(relayPin, LOW); // Turn off relay
  digitalWrite(redLEDPin, HIGH); // Turn on red LED
  digitalWrite(greenLEDPin, LOW); // Turn off green LED
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Access Denied");
  Serial.println("Access Denied - Unauthorized fingerprint."); // Output to Serial Monitor

  // Animate stop sign
  for (int i = 0; i < 16; i++) {
    lcd.setCursor(i, 1);
    lcd.write(byte(i % 2)); // Cycle through stop sign characters
    delay(150);
  }

  // Sound buzzer for denied access
  for (int i = 0; i < 3; i++) {
    tone(buzzerPin, 1000, 200); // Beep three times quickly
    delay(300); // Wait for 300ms between beeps
  }
  delay(2000); // Display "Access Denied" for 2 seconds
}

void handleAccessGranted() {
  // Handle authorized access
  digitalWrite(relayPin, HIGH); // Turn on relay
  digitalWrite(redLEDPin, LOW); // Turn off red LED
  digitalWrite(greenLEDPin, HIGH); // Turn on green LED
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Access Granted");
  Serial.println("Access Granted - Authorized fingerprint."); // Output to Serial Monitor
  tone(buzzerPin, 1000, 3000); // Beep for 3 seconds straight

  // Animate progress arrows
  for (int i = 0; i < 16; i++) {
    lcd.setCursor(i, 1);
    lcd.write(byte(2 + (i % 2))); // Cycle through arrow characters
    delay(150);
  }

  delay(3000); // Display "Access Granted" for 3 seconds
}

void resetState() {
  // Reset to default state
  digitalWrite(relayPin, LOW); // Turn off relay
  digitalWrite(redLEDPin, LOW); // Turn off red LED
  digitalWrite(greenLEDPin, LOW); // Turn off green LED
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("VideotronicMaker");
  lcd.setCursor(0, 1);
  lcd.print("Touch Sensor");
  delay(2000); // Display the default message for 2 seconds before next scan
}

// Helper function to get a fingerprint ID
int getFingerprintIDez() {
  uint8_t p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK) return -1;

  // Found a match!
  Serial.print("Fingerprint ID: "); // Output to Serial Monitor
  Serial.println(finger.fingerID); // Output to Serial Monitor
  return finger.fingerID;
}