// Include necessary libraries
#include <LiquidCrystal_I2C.h>  // For LCD display
#include <EEPROM.h>             // For saving settings
#include <Bounce2.h>            // For button debouncing

// Pin definitions
#define BUTTON_NEXT     2
#define BUTTON_SELECT   3
#define BUTTON_BACK     4
#define BUZZER_PIN      5
#define LED_PIN         6
#define TOGGLE_SWITCH   7
#define ERROR_LED_PIN   13
#define POWER_SAVE_TIMEOUT 300000  // 5 minutes in milliseconds

// Button debouncing
Bounce2::Button nextButton = Bounce2::Button();
Bounce2::Button selectButton = Bounce2::Button();
Bounce2::Button backButton = Bounce2::Button();

// Jacks LCD screen
#include <Wire.h>
#include "rgb_lcd.h"

rgb_lcd lcd;

// Timer structures
enum TimerMode {
  POMODORO,
  NORMAL,
  CHILL
};

struct Timer {
  TimerMode mode;
  unsigned long duration;
  unsigned long startTime;
  bool isRunning;
};

// Checklist structures
#define MAX_ITEMS 10
struct Checklist {
  char items[MAX_ITEMS][16];  // 16 chars per item
  uint8_t itemCount;
  uint8_t currentItem;
};

// Navigation menu structure
enum MenuState {
  MAIN_MENU,
  TIMER_MENU,
  CHECKLIST_MENU,
  TOGGLE_MENU,
  SETTINGS_MENU
};

// Class definitions
class Navigation {
  private:
    MenuState currentState;
    MenuState previousState;
    static const uint8_t MAX_MENU_ITEMS = 5;
    uint8_t selectedItem;
    
  public:
    Navigation() {
      currentState = MAIN_MENU;
      previousState = MAIN_MENU;
      selectedItem = 0;
    }
    
    void handleNavigation() {
      if (nextButton.fell()) {
        selectedItem = (selectedItem + 1) % MAX_MENU_ITEMS;
        displayMenu();
      }
      
      if (selectButton.fell()) {
        previousState = currentState;
        switch(selectedItem) {
          case 0: currentState = TIMER_MENU; break;
          case 1: currentState = CHECKLIST_MENU; break;
          case 2: currentState = TOGGLE_MENU; break;
          case 3: currentState = SETTINGS_MENU; break;
        }
        displayMenu();
      }
      
      if (backButton.fell()) {
        goBack();
      }
    }
    
    void displayMenu() {
      lcd.clear();
      switch(currentState) {
        case MAIN_MENU:
          lcd.print("Main Menu");
          lcd.setCursor(0, 1);
          lcd.print("> ");
          switch(selectedItem) {
            case 0: lcd.print("Timer"); break;
            case 1: lcd.print("Checklist"); break;
            case 2: lcd.print("Toggle"); break;
            case 3: lcd.print("Settings"); break;
          }
          break;
        // Add other menu states as needed
      }
    }
    
    MenuState getCurrentState() { return currentState; }
    void goBack() { 
      currentState = previousState;
      displayMenu();
    }
};

class StudentTimer {
  private:
    Timer timer;
    unsigned long pausedTime;
    bool isPaused;
    
    void displayTimer() {
      lcd.clear();
      lcd.print("Timer: ");
      unsigned long remaining = getTimeRemaining();
      lcd.print(remaining / 60000); // Minutes
      lcd.print(":");
      lcd.print((remaining % 60000) / 1000); // Seconds
    }
    
 
      //void playAlarm() {
      //if (currentSettings.soundEnabled) {
      //  tone(BUZZER_PIN, 1000, 1000);
     // }
   // }
  
    
    unsigned long getTimeRemaining() {
      if (!timer.isRunning) return 0;
      if (isPaused) return pausedTime;
      return timer.duration - (millis() - timer.startTime);
    }
    
  public:
    StudentTimer() {
      timer.isRunning = false;
      isPaused = false;
    }
    
    void setTimerMode(TimerMode mode) {
      timer.mode = mode;
      switch(mode) {
        case POMODORO: timer.duration = 25 * 60000; break; // 25 minutes
        case NORMAL: timer.duration = 45 * 60000; break;   // 45 minutes
        case CHILL: timer.duration = 15 * 60000; break;    // 15 minutes
      }
    }
    
    void startTimer() {
      timer.startTime = millis();
      timer.isRunning = true;
      isPaused = false;
    }
    
    void update() {
      if (timer.isRunning && !isPaused) {
        displayTimer();
        if (getTimeRemaining() <= 0) {
 //         playAlarm();
          timer.isRunning = false;
        }
      }
    }
    
    bool isRunning() const { return timer.isRunning; }
    bool getIsPaused() const { return isPaused; }
};

class ChecklistManager {
  private:
    Checklist lists[3];
    uint8_t currentList;
    bool itemCompleted[MAX_ITEMS];
    
  public:
    ChecklistManager() {
      currentList = 0;
      for (int i = 0; i < MAX_ITEMS; i++) {
        itemCompleted[i] = false;
      }
    }
    
    void loadChecklists() {
      // Demo items
      strcpy(lists[0].items[0], "Study Math");
      strcpy(lists[0].items[1], "Read Chapter 5");
      lists[0].itemCount = 2;
      lists[0].currentItem = 0;
    }
    
    void displayCurrentItem() {
      lcd.clear();
      lcd.print("Task ");
      lcd.print(lists[currentList].currentItem + 1);
      lcd.print("/");
      lcd.print(lists[currentList].itemCount);
      lcd.setCursor(0, 1);
      lcd.print(lists[currentList].items[lists[currentList].currentItem]);
    }
    
    void nextItem() {
      if (lists[currentList].currentItem < lists[currentList].itemCount - 1) {
        lists[currentList].currentItem++;
        displayCurrentItem();
      }
    }

    void update() {
      // Check for button presses to navigate checklist
      if (nextButton.fell()) {
        Serial.println("button was pressed");
        nextItem();
      }
      // Display current item if we're in checklist mode
      displayCurrentItem();
    }
};

// Global objects
Navigation navigation;
StudentTimer studentTimer;
ChecklistManager checklistManager;




// Add these variables
unsigned long lastActivityTime = 0;
bool isInPowerSave = false;




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
  //currentSettings.soundEnabled = true;
  // TODO: Implement EEPROM loading
}

void saveSettings() {
  // TODO: Implement EEPROM saving
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

  // Handle navigation
  navigation.handleNavigation();
  
  // Update active components based on current state
  switch(navigation.getCurrentState()) {
    case TIMER_MENU:
      studentTimer.update();
      break;
    case CHECKLIST_MENU:
      checklistManager.update();
      break;
    case SETTINGS_MENU:
      // Handle settings updates
      break;
    default:
      break;
  }
}

void handleError(const char* errorMessage) {
  digitalWrite(ERROR_LED_PIN, HIGH);
  lcd.clear();
  lcd.print("Error:");
  lcd.setCursor(0, 1);
  lcd.print(errorMessage);
  delay(2000);
  digitalWrite(ERROR_LED_PIN, LOW);
}

void checkPowerSave() {
  if (!isInPowerSave && (millis() - lastActivityTime > POWER_SAVE_TIMEOUT)) {
    // Enter power save mode
    lcd.setRGB(0,0,0);
    isInPowerSave = true;
  }
}

void wakeFromPowerSave() {
  if (isInPowerSave) {
    lcd.setRGB(125,125,125);
    isInPowerSave = false;
    lastActivityTime = millis();
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
