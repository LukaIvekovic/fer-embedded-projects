#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

// rasvjeta - lampica
// senzor svijetla - simulirano sklopkom -> sklopka pritisnuta - svijetlo, sklopka nije pritisnuta - tamno

#define LED_GREEN_PIN 12
#define LIGHT_SENSOR_PIN 13
#define BUTTON_TURN_ON_OFF 36

void handleLed();
void changeLedState(String data, int manualIndicator, int lightsDuration);
void handleNotFound();
void returnResponse(int statusCode, String message);

const char* ssid = "Homebox-Ivekovic";
const char* password = "krunkrun22";

WebServer webServer(80);

unsigned long startTime = 0;
long interval = 5;
boolean turnOffLedAfterDuration = false;
int previousLightsState = 1;
int currentLedState = 0;

void setup() {
  Serial.begin(115200);
  delay(100);

  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LIGHT_SENSOR_PIN, INPUT);
  pinMode(BUTTON_TURN_ON_OFF, INPUT);

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

  webServer.on("/", HTTP_POST, handleLed);
  webServer.onNotFound(handleNotFound);

  webServer.begin();
  Serial.println("Server started!");
}

void loop() {
  webServer.handleClient();

  int turnOnButtonState = digitalRead(BUTTON_TURN_ON_OFF);

  if (turnOnButtonState == LOW) {
    Serial.println("Changing LED state manually!");
    previousLightsState = currentLedState;

    if (previousLightsState == 0) {
      digitalWrite(LED_GREEN_PIN, HIGH);
      currentLedState = 1;
    } else {
      digitalWrite(LED_GREEN_PIN, LOW);
      currentLedState = 0;
    }

    turnOffLedAfterDuration = false;
  }

  if (turnOffLedAfterDuration) {
    if (millis() - startTime >= interval) {
      Serial.println("Turning off LED after duration!");
      digitalWrite(LED_GREEN_PIN, LOW);
      turnOffLedAfterDuration = false;
      previousLightsState = currentLedState;
      currentLedState = 0;
    }
  }
  
  delay(100);
}

void handleNotFound() {
  webServer.send(404, "text/plain", "Not found");
}

void handleLed() {
  Serial.println("Client sent request to change LED state!");
  String data = webServer.arg("data");
  int manualIndicator = webServer.arg("manual").toInt();
  int lightsDuration = webServer.arg("duration").toInt();

  Serial.print("Data received: ");
  Serial.println(data);
  Serial.print("Manually setting LED state: ");
  Serial.println(manualIndicator);
  Serial.print("Lights duration: ");
  Serial.println(lightsDuration);

  changeLedState(data, manualIndicator, lightsDuration);
}

void changeLedState(String data, int manualIndicator, int lightsDuration) {
  int newLedState = data.equals("lights-on") ? HIGH : (data.equals("lights-off") ? LOW : -1);

  if (newLedState == -1) {
    Serial.println("Invalid data received");
    returnResponse(400, "Invalid data received!");
    return;
  }

  if (manualIndicator == 0) {
    // ----------- CODE FOR LIGHT SENSOR IF HARDWARE AVAILABLE -----------
    // int lightLevel = analogRead(LIGHT_SENSOR_PIN);
    // int light = map(lightLevel, 0, 1023, 0, 100);
    // if (light > 50) { ... }
    // ----------- CODE FOR LIGHT SENSOR IF HARDWARE AVAILABLE -----------

    int lightSensorValue = digitalRead(LIGHT_SENSOR_PIN);
    
    if (lightSensorValue == LOW) {
      Serial.println("Light sensor detected light, no need to change LED state!");
      returnResponse(200, "Light sensor detected light, no need to change LED state!");
      return;
    }

    if (newLedState == HIGH) { // turn on for lights duration
      Serial.println("Led state changing to: ON");
      digitalWrite(LED_GREEN_PIN, newLedState);
      previousLightsState = currentLedState;
      currentLedState = 1;

      returnResponse(200, "LED state changed!");

      turnOffLedAfterDuration = true;
      interval = lightsDuration * 1000;
      startTime = millis();

    } else {
      Serial.println("Led state changing to: OFF");
      digitalWrite(LED_GREEN_PIN, newLedState);
      returnResponse(200, "LED state changed!");

      previousLightsState = currentLedState;
      currentLedState = 1;
    }

  } else {
    Serial.println("Led state changing to: " + String(newLedState == HIGH ? "ON" : "OFF"));
    digitalWrite(LED_GREEN_PIN, newLedState);
    returnResponse(200, "LED state changed!");

    previousLightsState = currentLedState;
    currentLedState = newLedState;
  }
}

void returnResponse(int statusCode, String message) {
  webServer.send(statusCode, "text/plain", message);
}