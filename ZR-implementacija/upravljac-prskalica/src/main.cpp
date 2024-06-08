#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

void handleGate();
boolean changeGateState(String data);
void handleNotFound();

const char* ssid = "Homebox-LukaDavid";
const char* password = "ivekovic22";

WebServer webServer(80);

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
  Serial.println("Client sent request to change sprinkles state!");
  String data = webServer.arg("data");

  Serial.print("Data received: ");
  Serial.println(data);

  boolean success = changeGateState(data);

  if (success) {
    webServer.send(200, "text/plain", "Sprinkles state changed");
  } else {
    webServer.send(400, "text/plain", "Invalid data received");
  }
}

boolean changeGateState(String data) {
  return true;
}