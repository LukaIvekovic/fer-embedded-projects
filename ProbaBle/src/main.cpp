#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Wire.h>

//BLE server name
#define bleServerName "ESP32_BLE_UPRAVLJACKO_RACUNALO"

bool deviceConnected = false;
const char* ssid = "Homebox-LukaDavid";
const char* password = "ivekovic22";

#define SERVICE_UUID "e64bdc18-de83-4e76-8b22-f2802bb71fed"
#define CHARACTERISTIC_UUID "f78ebbff-c8b7-4107-93de-889a6a06d408"

BLECharacteristic pCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor pDescriptor(BLEUUID((uint16_t)0x2902));

//Setup callbacks onConnect and onDisconnect
class CustomServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

WebServer webServer(80);

void setup() {
  // Start serial communication 
  Serial.begin(115200);

  // Create the BLE Device
  BLEDevice::init(bleServerName);

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new CustomServerCallbacks());

  // Create the BLE Service
  BLEService *bmeService = pServer->createService(SERVICE_UUID);

  bmeService->addCharacteristic(&pCharacteristic);
  pDescriptor.setValue("BME temperature Fahrenheit");
  pCharacteristic.addDescriptor(&pDescriptor);
  
  // Start the service
  bmeService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");

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

  // webServer.begin(); when we add the webserver, program breaks
}

void loop() {
  if (deviceConnected) {
    static char temperatureFTemp[6];
    dtostrf(25, 6, 2, temperatureFTemp);
    //Set temperature Characteristic value and notify connected client
    pCharacteristic.setValue(temperatureFTemp);
    pCharacteristic.notify();
    Serial.print("Temperature Fahrenheit: ");
    Serial.print(25);
    Serial.println(" ºF");
    delay(1000);
  }
}