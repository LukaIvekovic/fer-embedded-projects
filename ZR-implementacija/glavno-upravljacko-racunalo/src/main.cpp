#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>

void callApi(String api, String value);
void handleOnConnect();
void handleNotFound();
void handleLightsOn();
void handleLightsOff();
void handleSprinklersOn();
String sendHTML();

const char* ssid = "Homebox-LukaDavid";
const char* password = "ivekovic22";

String upravljacPrskalicaApi = "http://192.168.0.16:80/";
String upravljacOgradomApi = "http://192.168.0.15:80/";
String upravljacRasvjetomApi = "http://192.168.0.13:80/";

WebServer webServer(80);
HTTPClient httpClient;

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
  webServer.on("/lights-on", handleLightsOn);
  webServer.on("/lights-off", handleLightsOff);
  webServer.on("/sprinklers-on", handleSprinklersOn);
  webServer.onNotFound(handleNotFound);

  webServer.begin();
  Serial.println("Server started!");
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

  String value = "data=lights-on&manual=1&duration=0";

  callApi(upravljacRasvjetomApi, value);
  webServer.send(200, "text/html", sendHTML());
}

void handleLightsOff() {
  Serial.println("Lights turning off...");

  String value = "data=lights-off&manual=1&duration=0";

  callApi(upravljacRasvjetomApi, value);
  webServer.send(200, "text/html", sendHTML());
}

void handleSprinklesOn() {
  Serial.println("Sprinklers turning on...");

  String value = "data=sprinklers-on&duration=5";

  callApi(upravljacPrskalicaApi, value);
  webServer.send(200, "text/html", sendHTML());
}

void callApi(String api, String value) {
  if (WiFi.status() == WL_CONNECTED) {
    httpClient.begin(api);
    httpClient.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpResponseCode = httpClient.POST(value);

    if (httpResponseCode < 300 && httpResponseCode >= 200) {
      String response = httpClient.getString();

      Serial.print("Response: ");
      Serial.print(String(httpResponseCode) + " ");
      Serial.println(response);
    } else {
      Serial.println("Error on sending POST: " + httpClient.errorToString(httpResponseCode));
    }

    httpClient.end();

  } else {
    Serial.println("WiFi Disconnected");
  }
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
  ptr +="<a class=\"button\" href=\"/sprinklers-on\">Turn Sprinklers On</a>\n"; // TODO add sprinklers duration
  ptr +="<a class=\"button\" href=\"/toggle-gate\">Open/Close the Gate</a>\n";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}