#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>

#define RED_LED_PIN 14

void handleOnConnect();
void handleLedOn();
void handleLedOff();
void handleNotFound();
void askApiForLedState();
String SendHTML(bool);

/* Put your SSID & Password */
const char* ssid = "Homebox-Ivekovic";  // for AP mode use any ssid
const char* password = "krunkrun22";  // for AP mode use any password

String apiName = "http://192.168.0.21:8080/demo/get";

// -------- CODE FOR ACCESS POINT MODE STARTS HERE (esp32 starts a new wifi network) -------------
/* Put IP Address details */
// IPAddress local_ip(192,168,1,1);
// IPAddress gateway(192,168,1,1);
// IPAddress subnet(255,255,255,0);
// -------- CODE FOR ACCESS POINT MODE ENDS HERE (esp32 starts a new wifi network) ---------------

WebServer webServer(80);

bool ledState = LOW;

void setup() {
  pinMode(RED_LED_PIN, OUTPUT);
  Serial.begin(115200);
  delay(100);

  // -------- CODE FOR ACCESS POINT MODE STARTS HERE (esp32 starts a new wifi network) -------------
  // WiFi.softAP(ssid, password);
  // WiFi.softAPConfig(local_ip, gateway, subnet);
  // delay(100);
  // -------- CODE FOR ACCESS POINT MODE ENDS HERE (esp32 starts a new wifi network) ---------------

  // -------- CODE FOR STATION MODE STARTS HERE (esp32 connects to existing network) ---------------
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
  // -------- CODE FOR STATION MODE ENDS HERE (esp32 connects to existing network) -----------------

  webServer.on("/", handleOnConnect);
  webServer.on("/led-on", handleLedOn);
  webServer.on("/led-off", handleLedOff);
  webServer.on("/ask-api", askApiForLedState);
  webServer.onNotFound(handleNotFound);

  webServer.begin();
  Serial.println("Server started!");
}

void loop() {
  webServer.handleClient();

  digitalWrite(RED_LED_PIN, ledState);
}

void handleOnConnect() {
  ledState = LOW;
  Serial.println("LED status: LOW");
  webServer.send(200, "text/html", SendHTML(ledState));
}

void handleLedOn() {
  ledState = HIGH;
  Serial.println("LED status: ON");
  webServer.send(200, "text/html", SendHTML(ledState));
}

void handleLedOff() {
  ledState = LOW;
  Serial.println("LED status: OFF");
  webServer.send(200, "text/html", SendHTML(ledState));
}

void handleNotFound() {
  webServer.send(404, "text/plain", "Not found");
}

void askApiForLedState() {
  if(WiFi.status()== WL_CONNECTED) {
    HTTPClient client;

    client.begin(apiName.c_str());

    // If you need Node-RED/server authentication, insert user and password below
    //http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");

    int responseCode = client.GET();

    if (responseCode < 300 && responseCode >= 200) {
        Serial.print("HTTP Response code: ");
        Serial.println(responseCode);

        String payload = client.getString();
        Serial.println("Api returned: " + payload);

        ledState = payload.toInt();
    } else {
      Serial.print("Error code: ");
      Serial.println(responseCode);
    }

    client.end();

  } else {
    Serial.println("WiFi Disconnected");
  }

  webServer.send(200, "text/html", SendHTML(ledState));
}

String SendHTML(bool ledState){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>LED Control</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #3498db;}\n";
  ptr +=".button-on:active {background-color: #2980b9;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>ESP32 Web Server</h1>\n";
  ptr +="<h3>Using Station (STA) Mode</h3>\n";
  
  if(ledState) {
    ptr +="<p>LED Status: ON</p><a class=\"button button-off\" href=\"/led-off\">OFF</a>\n";
  }else {
    ptr +="<p>LED Status: OFF</p><a class=\"button button-on\" href=\"/led-on\">ON</a>\n";
  }

  ptr += "<p>Ask Api for status</p><a class=\"button button-on\" href=\"/ask-api\">ASK</a>\n";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}