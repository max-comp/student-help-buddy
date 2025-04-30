#include <Wire.h>
#include "rgb_lcd.h"
#include <Bounce2.h>

#define BUTTON_SELECT   8
#define BUTTON_BACK     9

#define ENCODER         3

#define TOGGLE_LED      4
#define ALERT_LED       5

#define TOGGLE_SWITCH   6
#define ON_SWITCH       10

Bounce debouncerEncoder = Bounce(); 
Bounce debouncerToggle = Bounce();
Bounce debouncerOn = Bounce();

rgb_lcd lcd;

void setup() {
  Serial.begin(9600);
  Serial.println("Setup");
  // put your setup code here, to run once:
  lcd.begin(16, 2);

  pinMode(ENCODER,INPUT_PULLUP);
  debouncerEncoder.attach(ENCODER);
  debouncerEncoder.interval(5);

  pinMode(TOGGLE_LED,OUTPUT);
  pinMode(ALERT_LED, OUTPUT);

  debouncerToggle.attach(TOGGLE_SWITCH, INPUT_PULLUP);
  debouncerOn.attach(ON_SWITCH, INPUT_PULLUP);
  debouncerToggle.interval(5);
  debouncerOn.interval(5);

  Serial.println("Setup done");
}

void loop() {
  // put your main code here, to run repeatedly:
 InputChecks();

}

void InputChecks() {

// Update Switches
  
  

  // Read debounced states
  int toggleState = debouncerToggle.update();
  int onState = debouncerOn.update();

  // Check if TOGGLE_SWITCH is pressed (LOW because of INPUT_PULLUP)
  if (toggleState == true) {
    Serial.println("TOGGLE ON (Debounced)");
    digitalWrite(TOGGLE_LED, HIGH);
  } else {
    Serial.println("TOGGLE OFF (Debounced)");
    digitalWrite(TOGGLE_LED, LOW);
  }

  // Check if ON_SWITCH is pressed
  if (onState == true) {
    Serial.println("SCREEN ON (Debounced)");
    lcd.setRGB(250, 250, 250); // Turn backlight on
    lcd.setCursor(0, 0);
    lcd.print("Hello World!"); // Add content here
  } else {
    Serial.println("SCREEN OFF (Debounced)");
    lcd.clear();
    lcd.setRGB(0, 0, 0); // Turn backlight off
  }
  delay(1000);
}
