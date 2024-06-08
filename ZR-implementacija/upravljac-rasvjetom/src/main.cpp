#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

#define LED_GREEN_PIN 14

void handleGate();
boolean changeGateState(String data);
void handleNotFound();

const char* ssid = "Homebox-LukaDavid";
const char* password = "ivekovic22";

WebServer webServer(80);

void setup() {
  Serial.begin(115200);
  delay(100);

  pinMode(LED_GREEN_PIN, OUTPUT);

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
}

void loop() {
  webServer.handleClient();
}

void handleNotFound() {
  webServer.send(404, "text/plain", "Not found");
}

void handleGate() {
  Serial.println("Client sent request to change LED state!");
  String data = webServer.arg("data");

  Serial.print("Data received: ");
  Serial.println(data);

  boolean success = changeGateState(data);

  if (success) {
    webServer.send(200, "text/plain", "LED state changed");
  } else {
    webServer.send(400, "text/plain", "Invalid data received");
  }
}

boolean changeGateState(String data) {
  if (data.equals("lights-on")) {
    Serial.println("Turning LED ON");
    digitalWrite(LED_GREEN_PIN, HIGH);
    return true;

  } else if (data.equals("lights-off")) {
    Serial.println("Turning LED OFF");
    digitalWrite(LED_GREEN_PIN, LOW);
    return true;

  } else {
    Serial.println("Invalid data received");
    return false;
  }
}