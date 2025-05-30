#include <Wire.h>
#include "rgb_lcd.h"
#include <Bounce2.h>



// Pin Definitions
#define TOGGLE_SWITCH   6
#define ON_SWITCH       10
#define TOGGLE_LED      4
#define ALERT_LED       5
#define BUTTON_SELECT   9
#define BUTTON_BACK     8
#define ENCODER         3

// Debouncers
Bounce debouncerToggle = Bounce();
Bounce debouncerOn = Bounce();
Bounce debouncerSelect = Bounce();
Bounce debouncerBack = Bounce();

// LCD Display
rgb_lcd lcd;

// Encoder Tracking
volatile int encoderCount = 0;
int lastEncoderState = HIGH;

//testing code
unsigned long buttonPressTime = 0;
const int alertDuration = 200; // milliseconds
String lastButtonPressed = "None";

void setup() {
  Serial.begin(9600);
  
  // Initialize LCD
  lcd.begin(16, 2);
  lcd.setRGB(0, 0, 0); // Start with backlight off

  // Configure debouncers
  debouncerToggle.attach(TOGGLE_SWITCH, INPUT_PULLUP);
  debouncerToggle.interval(25);
  debouncerOn.attach(ON_SWITCH, INPUT_PULLUP);
  debouncerOn.interval(25);
  debouncerSelect.attach(BUTTON_SELECT, INPUT_PULLUP);
  debouncerSelect.interval(50);
  debouncerBack.attach(BUTTON_BACK, INPUT_PULLUP);
  debouncerBack.interval(25);

  // Configure LED
  pinMode(TOGGLE_LED, OUTPUT);
  digitalWrite(TOGGLE_LED, LOW);
  pinMode(ALERT_LED, OUTPUT);
  digitalWrite(ALERT_LED, LOW);
}

void loop() {
  checkAllInputs();
  updateAllOutputs();
  handleEncoderEvents();
  delay(10); // Small delay to prevent flickering
}

// Input Handling Functions
void checkAllInputs() {
  updateDebouncers();
  readSwitchStates();
  readButtonStates();
}

void updateDebouncers() {
  debouncerToggle.update();
  debouncerOn.update();
  debouncerSelect.update();
  debouncerBack.update();
}

void readSwitchStates() {
  bool screenOn = (debouncerOn.read() == LOW);
  bool ledOn = (debouncerToggle.read() == LOW);
  
  updateScreen(screenOn);
  updateLED(ledOn);
}

void readButtonStates() {
  bool selectPressed = (debouncerSelect.read() == LOW);
  bool backPressed = (debouncerBack.read() == LOW);
  
  if (selectPressed) handleSelectPress();
  if (backPressed) handleBackPress();
}

void updateScreen(bool state) {
  if (state) {
    lcd.setRGB(255, 255, 255); // White backlight
    lcd.setCursor(0, 0);
    lcd.print("System Ready");
  } else {
    lcd.setRGB(0, 0, 0); // Backlight off
    lcd.clear();
  }
}

void updateLED(bool state) {
  digitalWrite(TOGGLE_LED, state ? HIGH : LOW);
}

// Button Handler Functions
void handleSelectPress() {
  digitalWrite(ALERT_LED, HIGH);
  buttonPressTime = millis();
  lastButtonPressed = "SELECT";
  Serial.println("SELECT PRESSED");
  // Add any additional select functionality
}

void handleBackPress() {
  digitalWrite(ALERT_LED, HIGH);
  buttonPressTime = millis();
  lastButtonPressed = "BACK";
  Serial.println("BACK PRESSED");
  // Add any additional back functionality
}

// Updated Output Handling Functions
void updateAllOutputs() {
  handleAlertLED();
  updateButtonDisplay();
}

void handleAlertLED() {
  if (millis() - buttonPressTime > alertDuration) {
    digitalWrite(ALERT_LED, LOW);
  }
}

void updateButtonDisplay() {
  if (debouncerOn.read() == LOW) { // Only update if screen is on
    lcd.setCursor(0, 1);
    lcd.print("Last btn: ");
    lcd.print(lastButtonPressed);
  }
}

// Updated Encoder Handler Functions
void handleEncoderEvents() {
  if (encoderCount != 0) {
    int change = encoderCount;
    encoderCount = 0;
    
    if (change > 0) {
      Serial.println("Encoder CW");
      lastButtonPressed = "Encoder+";
    } else {
      Serial.println("Encoder CCW");
      lastButtonPressed = "Encoder-";
    }
    
    // Update LCD display if screen is on
    if (debouncerOn.read() == LOW) {
      lcd.setCursor(13, 1);
      lcd.print(change > 0 ? "+" : "-");
    }
  }
}