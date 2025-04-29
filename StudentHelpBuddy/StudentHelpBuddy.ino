#include <LiquidCrystal.h>

// Include necessary libraries
#include <LiquidCrystal_I2C.h>  // For LCD display
#include <EEPROM.h>             // For saving settings
#include <Bounce2.h>            // For button debouncing
#include <Bridge.h>             // For Arduino Yún network communication
#include <HttpClient.h>         // For making HTTP requests

// Pushingbox configuration
#define PUSHINGBOX_API  "api.pushingbox.com"
#define DEVICEID        "vDEVICEID"  // Replace with your actual DeviceID from Pushingbox

// Pin definitions
#define BUTTON_NEXT     2
#define BUTTON_SELECT   3
#define BUTTON_BACK     4
#define BUZZER_PIN      5
#define LED_PIN         6
#define TOGGLE_SWITCH   7
#define SYSTEM_SWITCH   8       // Overall system on/off switch
#define ERROR_LED_PIN   13
#define POWER_SAVE_TIMEOUT 300000  // 5 minutes in milliseconds

// Button debouncing
Bounce nextButton = Bounce();
Bounce selectButton = Bounce();
Bounce backButton = Bounce();
Bounce toggleSwitch = Bounce();
Bounce systemSwitch = Bounce();

// Global objects
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Standard I2C address, adjust if needed
HttpClient client;                   // HTTP client for Pushingbox requests

// Navigation and Settings structure
struct Settings {
  uint8_t brightness;
  uint8_t volume;
  bool soundEnabled;
  bool dataSyncEnabled;  // Option to enable/disable data syncing
};

Settings currentSettings;

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
  bool dataLogged;  // Flag to track if timer completion was logged
};

// Checklist structures
#define MAX_ITEMS 10
#define MAX_LISTS 3
struct ChecklistItem {
  char text[16];  // 16 chars per item
  bool completed;
  bool dataLogged;  // Flag to track if completion was logged
};

struct Checklist {
  char name[16];
  ChecklistItem items[MAX_ITEMS];
  uint8_t itemCount;
  uint8_t currentItem;
};

// Toggle structures
struct ToggleTask {
  char name[16];
  bool enabled;
  bool statusLogged;  // Flag to track if status change was logged
};

#define MAX_TOGGLES 5
struct ToggleManager {
  ToggleTask tasks[MAX_TOGGLES];
  uint8_t taskCount;
  uint8_t currentTask;
};

// Navigation menu structure
enum MenuState {
  MAIN_MENU,
  TIMER_MENU,
  TIMER_SET_MENU,
  TIMER_RUNNING,
  CHECKLIST_MENU,
  CHECKLIST_VIEW,
  TOGGLE_MENU,
  SETTINGS_MENU,
  SYNC_MENU  // New menu for data sync
};

// Class definitions
class Navigation {
  private:
    MenuState currentState;
    MenuState previousState;
    static const uint8_t MAX_MENU_ITEMS = 6;  // Increased to add sync menu
    uint8_t selectedItem;
    uint8_t menuScrollPosition;
    
  public:
    Navigation() {
      currentState = MAIN_MENU;
      previousState = MAIN_MENU;
      selectedItem = 0;
      menuScrollPosition = 0;
    }
    
    void handleNavigation() {
      if (nextButton.fell()) {
        selectedItem = (selectedItem + 1) % MAX_MENU_ITEMS;
        displayMenu();
      }
      
      if (selectButton.fell()) {
        previousState = currentState;
        switch(currentState) {
          case MAIN_MENU:
            switch(selectedItem) {
              case 0: currentState = TIMER_MENU; break;
              case 1: currentState = CHECKLIST_MENU; break;
              case 2: currentState = TOGGLE_MENU; break;
              case 3: currentState = SETTINGS_MENU; break;
              case 4: currentState = SYNC_MENU; break;  // New sync menu option
            }
            break;
          case TIMER_MENU:
            currentState = TIMER_SET_MENU;
            break;
          case TIMER_SET_MENU:
            currentState = TIMER_RUNNING;
            break;
          case CHECKLIST_MENU:
            currentState = CHECKLIST_VIEW;
            break;
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
            case 4: lcd.print("Sync Data"); break;  // New sync menu
          }
          break;
        case TIMER_MENU:
          lcd.print("Timer Mode");
          lcd.setCursor(0, 1);
          lcd.print("> ");
          switch(selectedItem % 3) {
            case 0: lcd.print("Pomodoro 25m"); break;
            case 1: lcd.print("Normal 45m"); break;
            case 2: lcd.print("Chill 15m"); break;
          }
          break;
        case TIMER_SET_MENU:
          lcd.print("Press to Start");
          lcd.setCursor(0, 1);
          lcd.print("Back to Cancel");
          break;
        case CHECKLIST_MENU:
          lcd.print("Checklists");
          lcd.setCursor(0, 1);
          lcd.print("> ");
          switch(selectedItem % MAX_LISTS) {
            case 0: lcd.print("Study Tasks"); break;
            case 1: lcd.print("Daily Tasks"); break;
            case 2: lcd.print("Custom List"); break;
          }
          break;
        case TOGGLE_MENU:
          lcd.print("Toggle Tasks");
          lcd.setCursor(0, 1);
          lcd.print("> ");
          switch(selectedItem % MAX_TOGGLES) {
            case 0: lcd.print("Focus Mode"); break;
            case 1: lcd.print("Do Not Disturb"); break;
            case 2: lcd.print("Study Reminder"); break;
            case 3: lcd.print("Break Alert"); break;
            case 4: lcd.print("Custom Toggle"); break;
          }
          break;
        case SETTINGS_MENU:
          lcd.print("Settings");
          lcd.setCursor(0, 1);
          lcd.print("> ");
          switch(selectedItem % 4) {  // Increased to add data sync setting
            case 0: lcd.print("Brightness"); break;
            case 1: lcd.print("Sound: "); lcd.print(currentSettings.soundEnabled ? "ON" : "OFF"); break;
            case 2: lcd.print("Volume"); break;
            case 3: lcd.print("Data Sync: "); lcd.print(currentSettings.dataSyncEnabled ? "ON" : "OFF"); break;
          }
          break;
        case SYNC_MENU:
          lcd.print("Data Sync");
          lcd.setCursor(0, 1);
          lcd.print("Syncing data...");
          break;
      }
    }
    
    MenuState getCurrentState() { return currentState; }
    uint8_t getSelectedItem() { return selectedItem; }
    
    void goBack() { 
      currentState = previousState;
      displayMenu();
    }
    
    void setMenuState(MenuState state) {
      previousState = currentState;
      currentState = state;
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
      
      // Get mode name
      switch(timer.mode) {
        case POMODORO: lcd.print("POMO"); break;
        case NORMAL: lcd.print("NORM"); break;
        case CHILL: lcd.print("CHILL"); break;
      }
      
      // Display time remaining
      unsigned long remaining = getTimeRemaining();
      lcd.setCursor(0, 1);
      
      // Format as MM:SS
      int minutes = remaining / 60000;
      int seconds = (remaining % 60000) / 1000;
      
      // Add leading zeros
      if (minutes < 10) lcd.print("0");
      lcd.print(minutes);
      lcd.print(":");
      if (seconds < 10) lcd.print("0");
      lcd.print(seconds);
      
      // Show status
      lcd.setCursor(8, 1);
      if (isPaused) lcd.print("[PAUSED]");
      else lcd.print("[RUNNING]");
    }
    
    void playAlarm() {
      if (currentSettings.soundEnabled) {
        // Play a more noticeable alarm pattern
        for (int i = 0; i < 3; i++) {
          tone(BUZZER_PIN, 1000, 300);
          delay(400);
          tone(BUZZER_PIN, 1500, 300);
          delay(400);
        }
        noTone(BUZZER_PIN);
      }
      
      // Flash LED for visual indication
      for (int i = 0; i < 5; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(200);
        digitalWrite(LED_PIN, LOW);
        delay(200);
      }
      
      // Log timer completion to Google Sheets if enabled
      if (currentSettings.dataSyncEnabled && !timer.dataLogged) {
        logTimerCompletion();
        timer.dataLogged = true;
      }
    }
    
    unsigned long getTimeRemaining() {
      if (!timer.isRunning) return 0;
      if (isPaused) return pausedTime;
      
      unsigned long elapsed = millis() - timer.startTime;
      if (elapsed >= timer.duration) return 0;
      return timer.duration - elapsed;
    }
    
    void logTimerCompletion() {
      char modeStr[10];
      switch(timer.mode) {
        case POMODORO: strcpy(modeStr, "Pomodoro"); break;
        case NORMAL: strcpy(modeStr, "Normal"); break;
        case CHILL: strcpy(modeStr, "Chill"); break;
      }
      
      // Duration in minutes
      int durationMinutes = timer.duration / 60000;
      
      // Send data to Pushingbox
      String url = "/pushingbox?devid=";
      url += DEVICEID;
      url += "&event=timer_completed";
      url += "&mode=";
      url += modeStr;
      url += "&duration=";
      url += durationMinutes;
      
      // Make HTTP request
      sendHttpRequest(url);
    }
    
  public:
    StudentTimer() {
      timer.isRunning = false;
      isPaused = false;
      timer.dataLogged = false;
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
      timer.dataLogged = false;
      
      // Short beep to indicate timer started
      if (currentSettings.soundEnabled) {
        tone(BUZZER_PIN, 800, 200);
      }
    }
    
    void pauseTimer() {
      if (timer.isRunning && !isPaused) {
        pausedTime = getTimeRemaining();
        isPaused = true;
        
        // Quick beep to indicate pause
        if (currentSettings.soundEnabled) {
          tone(BUZZER_PIN, 400, 100);
        }
      }
    }
    
    void resumeTimer() {
      if (timer.isRunning && isPaused) {
        timer.startTime = millis() - (timer.duration - pausedTime);
        isPaused = false;
        
        // Quick beep to indicate resume
        if (currentSettings.soundEnabled) {
          tone(BUZZER_PIN, 600, 100);
        }
      }
    }
    
    void stopTimer() {
      timer.isRunning = false;
      isPaused = false;
      
      // Sound to indicate stop
      if (currentSettings.soundEnabled) {
        tone(BUZZER_PIN, 300, 300);
      }
    }
    
    void update() {
      if (timer.isRunning) {
        displayTimer();
        
        if (getTimeRemaining() <= 0) {
          playAlarm();
          timer.isRunning = false;
          
          // Show completion message
          lcd.clear();
          lcd.print("Time's up!");
          lcd.setCursor(0, 1);
          lcd.print("Press any button");
        }
      }
    }
    
    bool isRunning() const { return timer.isRunning; }
    bool getIsPaused() const { return isPaused; }
    void togglePause() { isPaused ? resumeTimer() : pauseTimer(); }
};

class ChecklistManager {
  private:
    Checklist lists[MAX_LISTS];
    uint8_t currentList;
    
    void displayList() {
      lcd.clear();
      lcd.print(lists[currentList].name);
      lcd.print(" (");
      lcd.print(getCompletedCount());
      lcd.print("/");
      lcd.print(lists[currentList].itemCount);
      lcd.print(")");
    }
    
    uint8_t getCompletedCount() {
      uint8_t count = 0;
      for (int i = 0; i < lists[currentList].itemCount; i++) {
        if (lists[currentList].items[i].completed) count++;
      }
      return count;
    }
    
    void logTaskCompletion(int taskIndex, bool completed) {
      if (!currentSettings.dataSyncEnabled) return;
      
      // Send data to Pushingbox
      String url = "/pushingbox?devid=";
      url += DEVICEID;
      url += "&event=task_update";
      url += "&list=";
      url += lists[currentList].name;
      url += "&task=";
      url += lists[currentList].items[taskIndex].text;
      url += "&status=";
      url += completed ? "completed" : "pending";
      
      // Make HTTP request
      sendHttpRequest(url);
      
      // Mark as logged
      lists[currentList].items[taskIndex].dataLogged = true;
    }
    
  public:
    ChecklistManager() {
      currentList = 0;
      
      // Initialize lists
      for (int i = 0; i < MAX_LISTS; i++) {
        lists[i].itemCount = 0;
        lists[i].currentItem = 0;
      }
      
      // Set default list names
      strcpy(lists[0].name, "Study Tasks");
      strcpy(lists[1].name, "Daily Tasks");
      strcpy(lists[2].name, "Custom List");
    }
    
    void loadChecklists() {
      // Study Tasks list
      strcpy(lists[0].items[0].text, "Study Math");
      lists[0].items[0].completed = false;
      lists[0].items[0].dataLogged = false;
      
      strcpy(lists[0].items[1].text, "Read Chapter 5");
      lists[0].items[1].completed = false;
      lists[0].items[1].dataLogged = false;
      
      strcpy(lists[0].items[2].text, "Review Notes");
      lists[0].items[2].completed = false;
      lists[0].items[2].dataLogged = false;
      
      strcpy(lists[0].items[3].text, "Practice Exam");
      lists[0].items[3].completed = false;
      lists[0].items[3].dataLogged = false;
      
      lists[0].itemCount = 4;
      
      // Daily Tasks list
      strcpy(lists[1].items[0].text, "Morning Review");
      lists[1].items[0].completed = false;
      lists[1].items[0].dataLogged = false;
      
      strcpy(lists[1].items[1].text, "Attend Classes");
      lists[1].items[1].completed = false;
      lists[1].items[1].dataLogged = false;
      
      strcpy(lists[1].items[2].text, "Exercise");
      lists[1].items[2].completed = false;
      lists[1].items[2].dataLogged = false;
      
      lists[1].itemCount = 3;
      
      // Custom list (empty by default)
      lists[2].itemCount = 0;
    }
    
    void displayCurrentItem() {
      if (lists[currentList].itemCount == 0) {
        lcd.clear();
        lcd.print("No items in list");
        lcd.setCursor(0, 1);
        lcd.print("Add items in app");
        return;
      }
      
      lcd.clear();
      lcd.print("Task ");
      lcd.print(lists[currentList].currentItem + 1);
      lcd.print("/");
      lcd.print(lists[currentList].itemCount);
      
      lcd.setCursor(0, 1);
      lcd.print(lists[currentList].items[lists[currentList].currentItem].completed ? "[X] " : "[ ] ");
      lcd.print(lists[currentList].items[lists[currentList].currentItem].text);
    }
    
    void nextItem() {
      if (lists[currentList].itemCount == 0) return;
      
      if (lists[currentList].currentItem < lists[currentList].itemCount - 1) {
        lists[currentList].currentItem++;
        displayCurrentItem();
      } else {
        // Wrap around to first item
        lists[currentList].currentItem = 0;
        displayCurrentItem();
      }
    }
    
    void previousItem() {
      if (lists[currentList].itemCount == 0) return;
      
      if (lists[currentList].currentItem > 0) {
        lists[currentList].currentItem--;
        displayCurrentItem();
      } else {
        // Wrap around to last item
        lists[currentList].currentItem = lists[currentList].itemCount - 1;
        displayCurrentItem();
      }
    }
    
    void toggleCurrentItem() {
      if (lists[currentList].itemCount == 0) return;
      
      int currentItemIndex = lists[currentList].currentItem;
      
      // Toggle completion status
      lists[currentList].items[currentItemIndex].completed = 
        !lists[currentList].items[currentItemIndex].completed;
        
      // Sound feedback
      if (currentSettings.soundEnabled) {
        tone(BUZZER_PIN, lists[currentList].items[currentItemIndex].completed ? 1000 : 500, 100);
      }
      
      // Log task completion to Google Sheets
      logTaskCompletion(currentItemIndex, lists[currentList].items[currentItemIndex].completed);
      
      displayCurrentItem();
    }
    
    void setCurrentList(uint8_t list) {
      if (list < MAX_LISTS) {
        currentList = list;
        lists[currentList].currentItem = 0;
        displayList();
      }
    }
    
    void update(Navigation& nav) {
      // Only handle checklist-specific button actions when in appropriate menu
      if (nav.getCurrentState() == CHECKLIST_VIEW) {
        if (nextButton.fell()) {
          nextItem();
        }
        
        if (backButton.fell()) {
          previousItem();
        }
        
        if (selectButton.fell()) {
          toggleCurrentItem();
        }
      }
    }
    
    void syncAllChecklists() {
      if (!currentSettings.dataSyncEnabled) return;
      
      // Send all checklist data
      for (int list = 0; list < MAX_LISTS; list++) {
        for (int item = 0; item < lists[list].itemCount; item++) {
          logTaskCompletion(item, lists[list].items[item].completed);
          delay(300); // Short delay between requests
        }
      }
    }
};

class ToggleManager {
  private:
    ToggleTask tasks[MAX_TOGGLES];
    uint8_t currentTask;
    
    void displayCurrentTask() {
      lcd.clear();
      lcd.print("Toggle: ");
      lcd.print(tasks[currentTask].name);
      lcd.setCursor(0, 1);
      lcd.print("Status: ");
      lcd.print(tasks[currentTask].enabled ? "ENABLED" : "DISABLED");
      
      // Set LED based on the current toggle state
      digitalWrite(LED_PIN, tasks[currentTask].enabled ? HIGH : LOW);
    }
    
    void logToggleChange(uint8_t taskIndex) {
      if (!currentSettings.dataSyncEnabled) return;
      
      // Send data to Pushingbox
      String url = "/pushingbox?devid=";
      url += DEVICEID;
      url += "&event=toggle_change";
      url += "&toggle=";
      url += tasks[taskIndex].name;
      url += "&status=";
      url += tasks[taskIndex].enabled ? "enabled" : "disabled";
      
      // Make HTTP request
      sendHttpRequest(url);
      
      // Mark as logged
      tasks[taskIndex].statusLogged = true;
    }
    
  public:
    ToggleManager() {
      currentTask = 0;
      
      // Initialize default toggle tasks
      strcpy(tasks[0].name, "Focus Mode");
      tasks[0].enabled = false;
      tasks[0].statusLogged = false;
      
      strcpy(tasks[1].name, "Do Not Disturb");
      tasks[1].enabled = false;
      tasks[1].statusLogged = false;
      
      strcpy(tasks[2].name, "Study Reminder");
      tasks[2].enabled = true;
      tasks[2].statusLogged = false;
      
      strcpy(tasks[3].name, "Break Alert");
      tasks[3].enabled = true;
      tasks[3].statusLogged = false;
      
      strcpy(tasks[4].name, "Custom Toggle");
      tasks[4].enabled = false;
      tasks[4].statusLogged = false;
    }
    
    void setCurrentTask(uint8_t task) {
      if (task < MAX_TOGGLES) {
        currentTask = task;
        displayCurrentTask();
      }
    }
    
    void toggleCurrentTask() {
      // Toggle the state
      tasks[currentTask].enabled = !tasks[currentTask].enabled;
      
      // Sound feedback
      if (currentSettings.soundEnabled) {
        tone(BUZZER_PIN, tasks[currentTask].enabled ? 800 : 400, 100);
      }
      
      // Visual feedback with LED
      digitalWrite(LED_PIN, tasks[currentTask].enabled ? HIGH : LOW);
      
      // Log toggle change
      logToggleChange(currentTask);
      
      displayCurrentTask();
    }
    
    void nextTask() {
      currentTask = (currentTask + 1) % MAX_TOGGLES;
      displayCurrentTask();
    }
    
    void previousTask() {
      currentTask = (currentTask > 0) ? (currentTask - 1) : (MAX_TOGGLES - 1);
      displayCurrentTask();
    }
    
    void update(Navigation& nav) {
      if (nav.getCurrentState() == TOGGLE_MENU) {
        if (nextButton.fell()) {
          nextTask();
        }
        
        if (backButton.fell()) {
          previousTask();
        }
        
        if (selectButton.fell()) {
          toggleCurrentTask();
        }
        
        // External hardware toggle switch
        if (toggleSwitch.fell() || toggleSwitch.rose()) {
          // Use the physical switch to toggle current task
          toggleCurrentTask();
        }
      }
    }
    
    void syncAllToggles() {
      if (!currentSettings.dataSyncEnabled) return;
      
      // Send all toggle statuses
      for (int i = 0; i < MAX_TOGGLES; i++) {
        logToggleChange(i);
        delay(300); // Short delay between requests
      }
    }
};

// Global objects
Navigation navigation;
StudentTimer studentTimer;
ChecklistManager checklistManager;
ToggleManager toggleManager;

// Add these variables
unsigned long lastActivityTime = 0;
bool isInPowerSave = false;
bool systemEnabled = true;

// Function to send HTTP requests to Pushingbox
void sendHttpRequest(String url) {
  // Create a client connection
  client.get(PUSHINGBOX_API, url);
  
  // Get the response status
  int statusCode = client.responseStatusCode();
  
  // Check if request was successful
  if (statusCode != 200) {
    handleError("Data sync failed");
  }
  
  // Read the response
  client.skipResponseHeaders();
  
  // Clean up
  client.stop();
}

// Function to sync all data to Google Sheets
void syncAllData() {
  if (!currentSettings.dataSyncEnabled) {
    lcd.clear();
    lcd.print("Data Sync");
    lcd.setCursor(0, 1);
    lcd.print("Sync is disabled");
    delay(2000);
    return;
  }
  
  lcd.clear();
  lcd.print("Data Sync");
  lcd.setCursor(0, 1);
  lcd.print("Syncing...");
  
  // Sync all data
  checklistManager.syncAllChecklists();
  toggleManager.syncAllToggles();
  
  // Show success message
  lcd.clear();
  lcd.print("Data Sync");
  lcd.setCursor(0, 1);
  lcd.print("Sync completed");
  delay(2000);
}

void setupHardware() {
  // Initialize buttons with debouncing
  nextButton.attach(BUTTON_NEXT, INPUT_PULLUP);
  selectButton.attach(BUTTON_SELECT, INPUT_PULLUP);
  backButton.attach(BUTTON_BACK, INPUT_PULLUP);
  toggleSwitch.attach(TOGGLE_SWITCH, INPUT_PULLUP);
  systemSwitch.attach(SYSTEM_SWITCH, INPUT_PULLUP);
  
  nextButton.interval(25);
  selectButton.interval(25);
  backButton.interval(25);
  toggleSwitch.interval(25);
  systemSwitch.interval(25);
  
  // Initialize pins
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(ERROR_LED_PIN, OUTPUT);
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  
  // Welcome message
  lcd.clear();
  lcd.print("Student Help");
  lcd.setCursor(0, 1);
  lcd.print("Buddy v1.0");
  delay(1500);
  
  // Load saved settings
  loadSettings();
}

void setup() {
  // Initialize serial for debugging
  Serial.begin(9600);
  
  setupHardware();
  
  // Initialize Bridge for network communication
  lcd.clear();
  lcd.print("Starting Bridge");
  lcd.setCursor(0, 1);
  lcd.print("Please wait...");
  
  Bridge.begin();
  
  checklistManager.loadChecklists();
  navigation.displayMenu();
  
  // Play startup sound
  if (currentSettings.soundEnabled) {
    tone(BUZZER_PIN, 600, 200);
    delay(250);
    tone(BUZZER_PIN, 800, 200);
  }
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
  toggleSwitch.update();
  systemSwitch.update();
}

void loadSettings() {
  // Default settings
  currentSettings.brightness = 128;
  currentSettings.volume = 128;
  currentSettings.soundEnabled = true;
  currentSettings.dataSyncEnabled = true;  // Enable data sync by default
  // TODO: Implement EEPROM loading
}

void saveSettings() {
  // TODO: Implement EEPROM saving
}

void handleSystemSwitch() {
  if (systemSwitch.fell()) {
    // System switch turned off
    systemEnabled = false;
    lcd.clear();
    lcd.print("System Disabled");
    lcd.noBacklight();
    digitalWrite(LED_PIN, LOW);
    noTone(BUZZER_PIN);
  } else if (systemSwitch.rose()) {
    // System switch turned on
    systemEnabled = true;
    lcd.backlight();
    lcd.clear();
    lcd.print("System Enabled");
    delay(1000);
    navigation.displayMenu();
    
    // Play startup sound
    if (currentSettings.soundEnabled) {
      tone(BUZZER_PIN, 800, 100);
    }
  }
}

void loop() {
  // Update all button states
  updateButtons();
  
  // Check system on/off switch first
  handleSystemSwitch();
  
  // Only proceed if system is enabled
  if (!systemEnabled) {
    delay(100); // Small delay to reduce CPU usage when disabled
    return;
  }
  
  // Update activity timestamp on any button press
  if (anyButtonPressed()) {
    lastActivityTime = millis();
    wakeFromPowerSave();
  }
  
  checkPowerSave();
  
  // Handle global navigation
  navigation.handleNavigation();
  
  // Update active components based on current state
  switch(navigation.getCurrentState()) {
    case TIMER_MENU:
      // Set timer mode based on selection
      if (selectButton.fell()) {
        studentTimer.setTimerMode((TimerMode)(navigation.getSelectedItem() % 3));
      }
      break;
      
    case TIMER_SET_MENU:
      if (selectButton.fell()) {
        studentTimer.startTimer();
        navigation.setMenuState(TIMER_RUNNING);
      }
      break;
      
    case TIMER_RUNNING:
      studentTimer.update();
      
      // Handle timer controls
      if (selectButton.fell()) {
        studentTimer.togglePause();
      }
      
      if (backButton.fell() && !studentTimer.isRunning()) {
        navigation.setMenuState(TIMER_MENU);
      }
      break;
      
    case CHECKLIST_MENU:
      if (selectButton.fell()) {
        checklistManager.setCurrentList(navigation.getSelectedItem() % MAX_LISTS);
      }
      break;
      
    case CHECKLIST_VIEW:
      checklistManager.update(navigation);
      break;
      
    case TOGGLE_MENU:
      toggleManager.update(navigation);
      break;
      
    case SETTINGS_MENU:
      if (selectButton.fell()) {
        // Handle settings changes
        switch(navigation.getSelectedItem() % 4) {
          case 1: // Sound toggle
            currentSettings.soundEnabled = !currentSettings.soundEnabled;
            // Play tone to indicate new state
            if (currentSettings.soundEnabled) {
              tone(BUZZER_PIN, 800, 200);
            }
            break;
          case 3: // Data sync toggle
            currentSettings.dataSyncEnabled = !currentSettings.dataSyncEnabled;
            break;
        }
        saveSettings();
        navigation.displayMenu();
      }
      break;
      
    case SYNC_MENU:
      // Start data sync
      syncAllData();
      navigation.setMenuState(MAIN_MENU);
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
    lcd.noBacklight();
    isInPowerSave = true;
  }
}

void wakeFromPowerSave() {
  if (isInPowerSave) {
    lcd.backlight();
    isInPowerSave = false;
    lastActivityTime = millis();
  }
} 