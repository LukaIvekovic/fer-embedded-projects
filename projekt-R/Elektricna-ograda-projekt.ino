#include <TimeLib.h>

#define BUTTON_DEVICE_PIN 7

#define LED_GREEN_PIN 8
#define LED_YELLOW_OPEN_PIN 9
#define LED_YELLOW_CLOSE_PIN 10
#define LED_RED_PIN 11

#define BUTTON_OBSTACLE_PIN 12


void setup() {
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_YELLOW_CLOSE_PIN, OUTPUT);
  pinMode(LED_YELLOW_OPEN_PIN, OUTPUT);

  pinMode(BUTTON_DEVICE_PIN, INPUT);
  pinMode(BUTTON_OBSTACLE_PIN, INPUT);

  setTime(21, 24, 0, 6, 1, 2024);

  Serial.begin(9600);
  while (!Serial) {
    ;
  }
  Serial.println("Setup completed");
  Serial.println("Current time set to: " 
  + String(hour()) + ":" + String(minute()) + ":" + String(second()) + " " 
  + String(day()) + "." + String(month()) + "." + String(year()));
}

int delayMilliseconds = 100;
int delayBetweenButtonPress = 2000;  // button press doesn't have effect for 2 seconds after the first press
int openingTime = 7000;              // fixed 7 seconds gate opening time until we introduce sensors for closed/opened state

int blinkOnDuration = 500;
int blinkOffDuration = 300;
int blinkCount = 0;
int blinkingTime = 0;

int gateDirection = 0;       // 1 -> gate opening, 0 -> gate not moving, -1 -> gate closing
int gatePreviousDirection = -1;  // 1 -> gate opening, 0 -> gate not moving, -1 -> gate closing

int buttonDeviceState;
int buttonObstacleState;

boolean gateStoppedOnObstacle = false;


void loop() {
  buttonDeviceState = digitalRead(BUTTON_DEVICE_PIN);
  buttonObstacleState = digitalRead(BUTTON_OBSTACLE_PIN);

  if (buttonDeviceState == LOW) {
    gateStoppedOnObstacle = false;
    digitalWrite(LED_RED_PIN, LOW);
    moveGate();

  } else {
    if (gateDirection != 0) {
      int blinkLedPin = (gateDirection == 1) ? LED_YELLOW_OPEN_PIN : LED_YELLOW_CLOSE_PIN;

      if (blinkingTime < openingTime) {
        blinkLed(blinkLedPin);
        blinkingTime += delayMilliseconds;

      } else {
        gateSuccessfulStop(blinkLedPin);
      }
    } else if (gateStoppedOnObstacle == true) {
      blinkLed(LED_RED_PIN);

    } else {
      turnOffAllLeds();
    }
  }

  if (buttonObstacleState == LOW) {
    stopGateObstacle();
  }

  int currentHour = hour();
  int currentMinute = minute();

  if (((currentHour == 17 && currentMinute == 0) || (currentHour == 9 && currentMinute == 0)) 
        && gateDirection == 0) { // if it's 9 or 17 oclock open the gate if it's not already moving, otherwise ignore it (Uredaj has priority)
    
    gateStoppedOnObstacle = false;
    digitalWrite(LED_RED_PIN, LOW);
    openGate();

  } else if (((currentHour == 17 && currentMinute == 15) || (currentHour == 9 && currentMinute == 15)) 
            && gateDirection == 0) { // if it's 9:15 or 17:15 close the gate if it's not already moving, otherwise ignore it (Uredaj has priority)

    gateStoppedOnObstacle = false;
    digitalWrite(LED_RED_PIN, LOW);
    closeGate();

  }

  delay(delayMilliseconds);
}

void moveGate() {
  int previousState = gateDirection;  // variable to remember gateDirection

  if (gateDirection == 0) {

    if (gatePreviousDirection == -1) {  // if gate was previously closed, open it

      digitalWrite(LED_YELLOW_OPEN_PIN, HIGH);
      gateDirection = 1;
      Serial.println("Opening the gate...");

    } else if (gatePreviousDirection == 1) {  // if gate was previously opened, close it

      digitalWrite(LED_YELLOW_CLOSE_PIN, HIGH);
      gateDirection = -1;
      Serial.println("Closing the gate...");
    }

  } else {

    if (gateDirection == 1) {
      digitalWrite(LED_YELLOW_OPEN_PIN, LOW);
    } else {
      digitalWrite(LED_YELLOW_CLOSE_PIN, LOW);
    }

    gateDirection = 0;
    digitalWrite(LED_RED_PIN, HIGH);
    Serial.println("Stopping the gate...");

    blinkingTime = openingTime - blinkingTime;  // simulate the place where the gate stopped (and time needed to open/close it from that point)
  }

  gatePreviousDirection = previousState;
}

void stopGateObstacle(){
  if (gateDirection != 0) {
    gatePreviousDirection = gateDirection;
    gateDirection = 0;

    Serial.println("Stopping the gate due to an obstacle!");

    blinkingTime = openingTime - blinkingTime;
    gateStoppedOnObstacle = true;

    turnOffAllLeds();
  }
}

void openGate(){
  if (gatePreviousDirection == -1 || blinkingTime != 0) {
    digitalWrite(LED_YELLOW_OPEN_PIN, HIGH);
    gatePreviousDirection = gateDirection;
    gateDirection = 1;
    Serial.println("Opening the gate...");
  } else {
    Serial.println("Gate already open!");
  }
}

void closeGate(){
  if (gatePreviousDirection == 1 || blinkingTime != 0) {
    digitalWrite(LED_YELLOW_CLOSE_PIN, HIGH);
    gatePreviousDirection = gateDirection;
    gateDirection = -1;
    Serial.println("Closing the gate...");
  } else {
    Serial.println("Gate already closed!");
  }
}

void blinkLed(int blinkLedPin) {
  if (blinkCount >= blinkOnDuration + blinkOffDuration) {
    blinkCount = 0;
  }

  if (blinkCount < blinkOnDuration) {
    digitalWrite(blinkLedPin, HIGH);
  } else {
    digitalWrite(blinkLedPin, LOW);
  }

  blinkCount += delayMilliseconds;
}

void gateSuccessfulStop(int blinkLed) {
  String gateState = (gateDirection == 1) ? "opened" : "closed";

  digitalWrite(blinkLed, LOW);

  digitalWrite(LED_GREEN_PIN, HIGH);
  blinkingTime = 0;
  gatePreviousDirection = gateDirection;
  gateDirection = 0;
  blinkCount = 0;

  Serial.println("Gate successfully " + gateState + "!");
}

void turnOffAllLeds() {
  digitalWrite(LED_YELLOW_CLOSE_PIN, LOW);
  digitalWrite(LED_YELLOW_OPEN_PIN, LOW);
  digitalWrite(LED_GREEN_PIN, LOW);
  digitalWrite(LED_RED_PIN, LOW);
}
