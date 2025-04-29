#include <LiquidCrystal.h>

#include <LiquidCrystal.h>

// Include necessary libraries
#include <LiquidCrystal_I2C.h>  // For LCD display
#include <EEPROM.h>             // For saving settings
@@ -18,21 +14,15 @@
#define POWER_SAVE_TIMEOUT 300000  // 5 minutes in milliseconds

// Button debouncing
Bounce nextButton = Bounce();
Bounce selectButton = Bounce();
Bounce backButton = Bounce();

// Global objects
LiquidCrystal_I2C lcd(0x, 16, 2);  // Adjust address as needed
Bounce2::Button nextButton = Bounce2::Button();
Bounce2::Button selectButton = Bounce2::Button();
Bounce2::Button backButton = Bounce2::Button();

// Navigation and Settings structure
struct Settings {
  uint8_t brightness;
  uint8_t volume;
  bool soundEnabled;
};
// Jacks LCD screen
#include <Wire.h>
#include "rgb_lcd.h"

Settings currentSettings;
rgb_lcd lcd;

// Timer structures
enum TimerMode {
@@ -142,11 +132,13 @@ class StudentTimer {
lcd.print((remaining % 60000) / 1000); // Seconds
}

    void playAlarm() {
      if (currentSettings.soundEnabled) {
        tone(BUZZER_PIN, 1000, 1000);
      }
    }
 
      //void playAlarm() {
      //if (currentSettings.soundEnabled) {
      //  tone(BUZZER_PIN, 1000, 1000);
     // }
   // }
  

unsigned long getTimeRemaining() {
if (!timer.isRunning) return 0;
@@ -179,7 +171,7 @@ class StudentTimer {
if (timer.isRunning && !isPaused) {
displayTimer();
if (getTimeRemaining() <= 0) {
          playAlarm();
 //         playAlarm();
timer.isRunning = false;
}
}
@@ -231,6 +223,7 @@ class ChecklistManager {
void update() {
// Check for button presses to navigate checklist
if (nextButton.fell()) {
        Serial.println("button was pressed");
nextItem();
}
// Display current item if we're in checklist mode
@@ -243,57 +236,43 @@ Navigation navigation;
StudentTimer studentTimer;
ChecklistManager checklistManager;




// Add these variables
unsigned long lastActivityTime = 0;
bool isInPowerSave = false;

void setupHardware() {
  // Initialize buttons
  nextButton.attach(BUTTON_NEXT, INPUT_PULLUP);
  selectButton.attach(BUTTON_SELECT, INPUT_PULLUP);
  backButton.attach(BUTTON_BACK, INPUT_PULLUP);
  
  nextButton.interval(25);
  selectButton.interval(25);
  backButton.interval(25);
  
  // Initialize pins
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(ERROR_LED_PIN, OUTPUT);
  pinMode(TOGGLE_SWITCH, INPUT_PULLUP);
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  
  // Load saved settings
  loadSettings();
}

void setup() {
  setupHardware();
  checklistManager.loadChecklists();
  navigation.displayMenu();
}


bool anyButtonPressed() {

return !digitalRead(BUTTON_NEXT) || 
!digitalRead(BUTTON_SELECT) || 
!digitalRead(BUTTON_BACK);
}

void updateButtons() {

nextButton.update();
selectButton.update();
backButton.update();
}

struct Settings {
  uint8_t brightness;
  uint8_t volume;
  bool soundEnabled;
};

Settings currentSettings;

void loadSettings() {
// Default settings
currentSettings.brightness = 128;
currentSettings.volume = 128;
  currentSettings.soundEnabled = true;
  //currentSettings.soundEnabled = true;
// TODO: Implement EEPROM loading
}

@@ -302,17 +281,17 @@ void saveSettings() {
}

void loop() {
    // Update button states
  updateButtons();

// Update activity timestamp on any button press
if (anyButtonPressed()) {
lastActivityTime = millis();
wakeFromPowerSave();
}

checkPowerSave();
  
  // Update button states
  updateButtons();
  

// Handle navigation
navigation.handleNavigation();

@@ -345,15 +324,58 @@ void handleError(const char* errorMessage) {
void checkPowerSave() {
if (!isInPowerSave && (millis() - lastActivityTime > POWER_SAVE_TIMEOUT)) {
// Enter power save mode
    lcd.noBacklight();
    lcd.setRGB(0,0,0);
isInPowerSave = true;
}
}

void wakeFromPowerSave() {
if (isInPowerSave) {
    lcd.backlight();
    lcd.setRGB(125,125,125);
isInPowerSave = false;
lastActivityTime = millis();
}
} 
} 

void setupHardware() {
  // Initialize buttons
  nextButton.attach(BUTTON_NEXT, INPUT_PULLUP);
  selectButton.attach(BUTTON_SELECT, INPUT_PULLUP);
  backButton.attach(BUTTON_BACK, INPUT_PULLUP);
  
  nextButton.interval(25);
  selectButton.interval(25);
  backButton.interval(25);

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  lcd.setRGB(125, 125, 125);
  
  // Initialize pins
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(ERROR_LED_PIN, OUTPUT);
  pinMode(TOGGLE_SWITCH, INPUT_PULLUP);
  
  /*
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  */
  
  // Load saved settings
  loadSettings();
}

void setup() {

setupHardware();
checklistManager.loadChecklists();
navigation.displayMenu();

Serial.begin(9600);
while (!Serial);
// Navigation and Settings structure

}
