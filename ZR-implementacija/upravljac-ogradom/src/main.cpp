#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <IRremote.h>

void handleGate();
void changeGateState(String data, int manualIndicator);
void returnResponse(int statusCode, String message);
void handleNotFound();
void moveGate(int moveGateDirection, int manualIndicator);
void stopGateObstacle();
void openGate(int manualIndicator);
void closeGate();
void blinkLed(int blinkLedPin);
void gateSuccessfulStop(int blinkLed);
void turnOffAllLeds();

#define LED_GREEN_PIN 32
#define LED_RED_PIN 13
#define LED_YELLOW_OPEN_PIN 33
#define LED_YELLOW_CLOSE_PIN 12

#define IR_RECEIVER_PIN 36
#define TRACKING_SENSOR_PIN 37

const char* ssid = "ESP32_WiFi";
const char* password = "password123";

WebServer webServer(80);
IRrecv irrecv(IR_RECEIVER_PIN);

void setup() {
  Serial.begin(115200);
  delay(100);

  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_YELLOW_OPEN_PIN, OUTPUT);
  pinMode(LED_YELLOW_CLOSE_PIN, OUTPUT);
  pinMode(TRACKING_SENSOR_PIN, INPUT);

  Serial.print("Connecting to ");
  Serial.print(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");  
  Serial.println(WiFi.localIP());

  webServer.on("/", HTTP_POST, handleGate);
  webServer.onNotFound(handleNotFound);

  webServer.begin();
  Serial.println("Server started!");

  irrecv.enableIRIn();
  Serial.println("IR receiver enabled!");
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

int buttonObstacleState;

boolean gateStoppedOnObstacle = false;
boolean closeAutomatically = false;

int delayStayOpen = 5000; // 5 seconds

decode_results irReceiverValue;
int trackerSensonValue;

void loop() {
  webServer.handleClient();

  if (closeAutomatically == true) {
    if (gateDirection != 0) {
      int blinkLedPin = LED_YELLOW_OPEN_PIN;

      if (blinkingTime < openingTime) {
        blinkLed(blinkLedPin);
        blinkingTime += delayMilliseconds;

      } else {
        gateSuccessfulStop(blinkLedPin);
      }

      delay(delayMilliseconds);
      return;
    }

    delay(delayStayOpen);

    trackerSensonValue = digitalRead(TRACKING_SENSOR_PIN);

    if (trackerSensonValue == LOW) {
      moveGate(-1, 1);
      closeAutomatically = false;

    } else {
      return;
    }
  }

  int moveGateDirection = -2;

  if (irrecv.decode(&irReceiverValue)) { // TODO change with non depracted function
    Serial.println(irReceiverValue.value, HEX);

    switch (irReceiverValue.value) {
      case 0x45481702:
        moveGateDirection = 1; // left arrow -> open gate
        break;
      case 0xF0B4BB43:
        moveGateDirection = -1; // right arrow -> close gate
        break;
      case 0xB8E7B4FE:
        moveGateDirection = 0; // OK -> stop gate
        break;
      default:
        moveGateDirection = -2; // invalid value
        break;
    }

    irrecv.resume(); // lijeva strelica = 45481702, desna = F0B4BB43, OK - B8E7B4FE
  }

  trackerSensonValue = digitalRead(TRACKING_SENSOR_PIN);
  
  if (moveGateDirection != -2) {
    gateStoppedOnObstacle = false;
    digitalWrite(LED_RED_PIN, LOW);

    if (moveGateDirection != gateDirection) {
      moveGate(moveGateDirection, 1); // prevent from consecutive button presses for the same direction
    }

  } else {
    if (gateDirection != 0) {
      int blinkLedPin = (gateDirection == 1) ? LED_YELLOW_OPEN_PIN : LED_YELLOW_CLOSE_PIN;

      if (blinkingTime < openingTime) {
        blinkLed(blinkLedPin);
        blinkingTime += delayMilliseconds;

      } else {
        gateSuccessfulStop(blinkLedPin);

        digitalWrite(LED_GREEN_PIN, LOW);
      }
    } else if (gateStoppedOnObstacle == true) {
      blinkLed(LED_RED_PIN);

    } else {
      turnOffAllLeds();
    }
  }

  if (trackerSensonValue == HIGH && gateDirection == -1) { // detect obsticles only on closing
    stopGateObstacle();
  }

  delay(delayMilliseconds);
}

void handleNotFound() {
  webServer.send(404, "text/plain", "Not found");
}

void handleGate() {
  Serial.println("Client sent request to change gate state!");
  String data = webServer.arg("data");
  int manual = webServer.arg("manual").toInt();

  Serial.print("Data received: ");
  Serial.println(data);
  Serial.print("Manual: ");
  Serial.println(manual);

  changeGateState(data, manual);
}

void changeGateState(String data, int manualIndicator) {
  int moveGateDirection = data.equals("gate-open") ? 1 : (data.equals("gate-close") ? -1 : 0);

  if (moveGateDirection == 0) {
    Serial.println("Invalid data received");
    returnResponse(400, "Invalid data received!");
    return;
  }

  if (manualIndicator == 0) { // moveGateDirection always -1 when isn't manual
    moveGate(moveGateDirection, manualIndicator);

  } else {
    if (gateDirection != moveGateDirection) {
      moveGate(moveGateDirection, manualIndicator);
      returnResponse(200, "Gate is moving!"); 

    } else {
      Serial.println("Gate is already moving in that direction!");
      returnResponse(200, "Gate is already moving in that direction!");
    }
  }
}

void returnResponse(int statusCode, String message) {
  webServer.send(statusCode, "text/plain", message);
}

void moveGate(int moveGateDirection, int manualIndicator) {
  int previousState = gateDirection;  // variable to remember gateDirection

  if (moveGateDirection == 1) {
    openGate(manualIndicator);

  } else if (moveGateDirection == -1) {
    closeGate();

  } else {
    if (gateDirection == 1) {
      digitalWrite(LED_YELLOW_OPEN_PIN, LOW);
    } else {
      digitalWrite(LED_YELLOW_CLOSE_PIN, LOW);
    }

    digitalWrite(LED_RED_PIN, HIGH);
    gateDirection = 0;
    Serial.println("Stopping the gate...");
  }

  gatePreviousDirection = previousState;
}


void openGate(int manualIndicator){
  if (gatePreviousDirection == 1 && blinkingTime == 0) {
    Serial.println("Gate already open!");
    return;
  }

  if (blinkingTime != 0 && gatePreviousDirection == -1) {
    blinkingTime = openingTime - blinkingTime;
  }

  digitalWrite(LED_YELLOW_OPEN_PIN, HIGH);
  gatePreviousDirection = gateDirection;
  gateDirection = 1;
  Serial.println("Opening the gate...");

  if (manualIndicator == 0) {
    closeAutomatically = true;
  }
}

void closeGate(){
  if (gatePreviousDirection == -1 && blinkingTime == 0) {
    Serial.println("Gate already closed!");
    return;
  }

  if (blinkingTime != 0 && gatePreviousDirection == 1) {
    blinkingTime = openingTime - blinkingTime;
  }

  digitalWrite(LED_YELLOW_CLOSE_PIN, HIGH);
  gatePreviousDirection = gateDirection;
  gateDirection = -1;
  Serial.println("Closing the gate...");
}

void stopGateObstacle(){
  if (gateDirection != 0) {
    gatePreviousDirection = gateDirection;
    gateDirection = 0;

    Serial.println("Stopping the gate due to an obstacle!");

    gateStoppedOnObstacle = true;

    turnOffAllLeds();
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