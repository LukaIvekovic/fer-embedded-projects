#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
// ----------- CODE FOR HUMIDITY SENSOR IF HARDWARE AVAILABLE -----------
// #include <DHT.h>
// #define DHTPIN 2 
// #define DHTTYPE DHT22 
// ----------- CODE FOR HUMIDITY SENSOR IF HARDWARE AVAILABLE -----------

// prskalice upaljene - lampica
// senzor vlage - simulirano sklopkom -> sklopka pritisnuta - dovoljno vlazno, sklopka nije pritisnuta - suho
#define LED_GREEN_PIN 14
#define MOISTURE_SENSOR_PIN 34

void handlePrskalice();
void turnOnSprinkles(String data, int sprinklesDuration);
void handleNotFound();
void returnResponse(int statusCode, String message);

const char* ssid = "Homebox-LukaDavid";
const char* password = "ivekovic22";

WebServer webServer(80);

// ----------- CODE FOR HUMIDITY SENSOR IF HARDWARE AVAILABLE -----------
// DHT dht(DHTPIN, DHTTYPE);
// ----------- CODE FOR HUMIDITY SENSOR IF HARDWARE AVAILABLE -----------

unsigned long startTime = 0;
long interval = 5;
boolean turnOffSprinklesAfterDuration = false;
int savedSprinklesDuration = 5;

int checkHumidityAfter = 30 * 1000; // 30 seconds
int lastHumidityCheck = 0;

void setup() {
  Serial.begin(115200);
  delay(100);

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

  // ----------- CODE FOR HUMIDITY SENSOR IF HARDWARE AVAILABLE -----------
  // dht.begin();
  // ----------- CODE FOR HUMIDITY SENSOR IF HARDWARE AVAILABLE -----------
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

  // ----------- CODE FOR HUMIDITY SENSOR IF HARDWARE AVAILABLE -----------
  // if (millis() - lastHumidityCheck >= checkHumidityAfter) {
  //   lastHumidityCheck = millis();
  //   float h = dht.readHumidity();
  //   if (isnan(h)) {
  //     Serial.println("Failed to read from DHT sensor!");
  //   }
  //
  //   Serial.print("Humidity: ");
  //   Serial.println(h);
  //
  //   if (h < 20 && !isnan(h)) {
  //      Serial.println("Too low humidity detected, turning on sprinkles");
  //      digitalWrite(LED_GREEN_PIN, HIGH);
  //
  //      turnOffSprinklesAfterDuration = true;
  //      interval = savedSprinklesDuration * 60000;
  //      startTime = millis();
  //   }
  // }
  // ----------- CODE FOR HUMIDITY SENSOR IF HARDWARE AVAILABLE -----------
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
  int newSprinklesState = data.equals("sprinkles-on") ? HIGH : -1;

  if (newSprinklesState == -1) {
    Serial.println("Invalid data received");
    returnResponse(400, "Invalid data received");
    return;
  }

  // ----------- CODE FOR HUMIDITY SENSOR IF HARDWARE AVAILABLE -----------
  // float h = dht.readHumidity();
  // if (isnan(h)) {
  //   Serial.println("Failed to read from DHT sensor!");
  //   h = 0;
  // }
  // if (h > 80) {
  //   Serial.println("Too much moisture detected, sprinkles will not turn on");
  //   returnResponse(200, "Too much moisture detected, sprinkles will not turn on");
  //   return;
  // }
  // ----------- CODE FOR HUMIDITY SENSOR IF HARDWARE AVAILABLE -----------

  int moistureSensorValue = digitalRead(MOISTURE_SENSOR_PIN);

  if (moistureSensorValue == LOW) {
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