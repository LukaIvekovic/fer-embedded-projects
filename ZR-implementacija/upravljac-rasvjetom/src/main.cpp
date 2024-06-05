#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

void changeLedState(char*);

#define SERVICE_UUID        "cffc0a62-cadd-43b5-b1e3-faff06483314"
#define CHARACTERISTIC_UUID "d7e5028b-7370-4cef-b44e-33b0dae524b7"

#define LED_GREEN_PIN 14

static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;

static void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  Serial.print("Received data to change LED state: ");
  Serial.println((char*)pData);

  changeLedState((char*)pData);
}

class CustomClientCallbacks : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
    Serial.println("Connected via BLE!");
  }
  void onDisconnect(BLEClient* pclient) {
    Serial.println("Disconnected, scanning for devices...");
    BLEDevice::getScan()->start(0);
  }
};

bool connectToServer() {
  BLEClient* pClient = BLEDevice::createClient();
  pClient->setClientCallbacks(new CustomClientCallbacks());

  pClient->connect(myDevice);
  BLERemoteService* pRemoteService = pClient->getService(SERVICE_UUID);
  if (pRemoteService == nullptr) {
    Serial.println("Failed to find service UUID");
    pClient->disconnect();
    return false;
  }

  pRemoteCharacteristic = pRemoteService->getCharacteristic(CHARACTERISTIC_UUID);
  if (pRemoteCharacteristic == nullptr) {
    Serial.println("Failed to find characteristic UUID");
    pClient->disconnect();
    return false;
  }

  if(pRemoteCharacteristic->canNotify())
    pRemoteCharacteristic->registerForNotify(notifyCallback);

  return true;
}

class CustomAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(BLEUUID(SERVICE_UUID))) {
      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      connectToServer();
    }
  }
};

void setup() {
  Serial.begin(115200);

  pinMode(LED_GREEN_PIN, OUTPUT);

  BLEDevice::init("ESP32 upravljac rasvjetom");
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new CustomAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(0);
}

void loop() {
  if (pRemoteCharacteristic != nullptr) {
    String value = "Hello from ESP32 BLE Client";
    pRemoteCharacteristic->writeValue(value.c_str(), value.length());
  }
  delay(2000);
}

void changeLedState(char* data) {
  if (strcmp(data, "lights-on") == 0) {
    Serial.println("Turning LED ON");
    digitalWrite(LED_GREEN_PIN, HIGH);

  } else if (strcmp(data, "lights-off") == 0) {
    Serial.println("Turning LED OFF");
    digitalWrite(LED_GREEN_PIN, LOW);
  }
}