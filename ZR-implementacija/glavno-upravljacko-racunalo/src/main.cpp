#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <BLEDevice.h>
#include <BLEServer.h>

void handleOnConnect();
void handleNotFound();
void handleLightsOn();
void handleLightsOff();
String sendHTML();

const char* ssid = "Homebox-LukaDavid";
const char* password = "ivekovic22";

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;

#define SERVICE_UUID        "cffc0a62-cadd-43b5-b1e3-faff06483314"
#define CHARACTERISTIC_UUID "d7e5028b-7370-4cef-b44e-33b0dae524b7"

class CustomCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (value.length() > 0) {
      Serial.println("Received data: " + String(value.c_str()));
    }
  }
};

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

  webServer.on("/", handleOnConnect);
  webServer.on("lights-on", handleOnConnect);
  webServer.on("lights-off", handleOnConnect);
  webServer.onNotFound(handleNotFound);

  webServer.begin();
  Serial.println("Server started!");

  BLEDevice::init("ESP32 BLE upravljacko racunalo");
  pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                                CHARACTERISTIC_UUID,
                                BLECharacteristic::PROPERTY_READ |
                                BLECharacteristic::PROPERTY_WRITE
                              );
  pCharacteristic->setCallbacks(new CustomCallbacks());
  pCharacteristic->setValue("Hello from ESP32 BLE Server");
  pService->start();
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}

void loop() {
  webServer.handleClient();
}

void handleOnConnect() {
  Serial.println("Client connected to server");
  webServer.send(200, "text/html", sendHTML());
}

void handleNotFound() {
  webServer.send(404, "text/plain", "Not found");
}

void handleLightsOn() {
  Serial.println("Lights turning on...");

  String value = "lights-on";
  pCharacteristic->setValue(value.c_str());
  pCharacteristic->notify();

  webServer.send(200, "text/html", sendHTML());
}

void handleLightsOff() {
  Serial.println("Lights turning off...");

  String value = "lights-off";
  pCharacteristic->setValue(value.c_str());
  pCharacteristic->notify();

  webServer.send(200, "text/html", sendHTML());
}

String sendHTML(){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>Smart Home Controls</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=".button {display: block;width: 200px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button:active {background-color: #2980b9;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>Smart Home Controls</h1>\n";
  ptr +="<a class=\"button\" href=\"/enter-home\">Enter the Home</a>\n";
  ptr +="<a class=\"button\" href=\"/exit-home\">Exit the Home</a>\n";
  ptr +="<h1>Control Panel</h1>\n";
  ptr +="<a class=\"button\" href=\"/lights-on\">Turn Lights On</a>\n";
  ptr +="<a class=\"button\" href=\"/lights-off\">Turn Lights Off</a>\n";
  ptr +="<a class=\"button\" href=\"/sprinklers-on\">Turn Sprinklers On</a>\n";
  ptr +="<a class=\"button\" href=\"/toggle-gate\">Open/Close the Gate</a>\n";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}