#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

#define LED_GREEN_PIN 12
#define MOISTURE_SENSOR_PIN 36

void handlePrskalice();
void turnOnSprinkles(String data, int sprinklesDuration);
void handleNotFound();
void returnResponse(int statusCode, String message);

const char* ssid = "ESP32_WiFi";
const char* password = "password123";

WebServer webServer(80);

unsigned long startTime = 0;
long interval = 5; 
boolean turnOffSprinklesAfterDuration = false;
int savedSprinklesDuration = 5;

int checkHumidityAfter = 1800 * 1000; // 30 minutes
int lastHumidityCheck = 0;

int HUMIDITY_TOO_LOW = 2450; //2559 je skroz suh
int HUMIDITY_TOO_HIGH = 1050; //ide do 2000 i nize s jednom kapljicom 1000 je uronjen u vodu

void setup() {
  Serial.begin(115200);
  delay(100);

  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(MOISTURE_SENSOR_PIN, INPUT);

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

  webServer.on("/", HTTP_POST, handlePrskalice);
  webServer.onNotFound(handleNotFound);

  webServer.begin();
  Serial.println("Server started!");
}

void loop() {
  webServer.handleClient();

  if (turnOffSprinklesAfterDuration) {
    if (millis() - startTime >= interval) {
      Serial.println("Turning off Sprinkles after duration!");
      digitalWrite(LED_GREEN_PIN, LOW);
      turnOffSprinklesAfterDuration = false;
    }
  }

  if (millis() - lastHumidityCheck >= checkHumidityAfter) {
    lastHumidityCheck = millis();
    int humidity = analogRead(MOISTURE_SENSOR_PIN);
  
    Serial.print("Humidity: ");
    Serial.println(humidity);
  
    if (humidity > HUMIDITY_TOO_LOW) {
      Serial.println("Too low humidity detected, turning on sprinkles");
      digitalWrite(LED_GREEN_PIN, HIGH);
  
      turnOffSprinklesAfterDuration = true;
      interval = savedSprinklesDuration * 1000;
      startTime = millis();
    }
  }
}

void handleNotFound() {
  webServer.send(404, "text/plain", "Not found");
}

void handlePrskalice() {
  Serial.println("Client sent request to change sprinkles state!");
  String data = webServer.arg("data");
  int sprinklesDuration = webServer.arg("duration").toInt();

  Serial.print("Data received: ");
  Serial.println(data);
  Serial.print("Duration received: ");
  Serial.println(sprinklesDuration);

  turnOnSprinkles(data, sprinklesDuration);
}

void turnOnSprinkles(String data, int sprinklesDuration) {
  int newSprinklesState = data.equals("sprinklers-on") ? HIGH : -1;

  if (newSprinklesState == -1) {
    Serial.println("Invalid data received");
    returnResponse(400, "Invalid data received");
    return;
  }

  int humidity = analogRead(MOISTURE_SENSOR_PIN);
  
  Serial.print("Humidity: ");
  Serial.println(humidity);

  if (humidity < HUMIDITY_TOO_HIGH) {
    Serial.println("Too much moisture detected, sprinkles will not turn on");
    returnResponse(200, "Too much moisture detected, sprinkles will not turn on");
    return;
  }

  Serial.println("Sprinkles turning ON...");
  digitalWrite(LED_GREEN_PIN, newSprinklesState);

  returnResponse(200, "Sprinkles state changed!");

  turnOffSprinklesAfterDuration = true;
  interval = sprinklesDuration * 1000;
  startTime = millis();

  savedSprinklesDuration = sprinklesDuration;
}

void returnResponse(int statusCode, String message) {
  webServer.send(statusCode, "text/plain", message);
}