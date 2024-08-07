#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>

void callApi(String api, String value);
void handleOnConnect();
void handleNotFound();
void handleEnterHome();
void handleExitHome();
void handleLightsOn();
void handleLightsOff();
void handleSprinklesOn();
void handleSprinklesOnManual();
void handleSetDurations();
void handleOpenGate();
void handleCloseGate();
String sendHTML();

const char* ap_ssid = "ESP32_WiFi"; // for AP mode
const char* ap_password = "password123";

const char* ssid = "Luka-iPhone"; // for Station mode (home wifi)
const char* password = "zasirotinju123";

String upravljacPrskalicaApi = "http://192.168.4.4:80/";
String upravljacOgradomApi = "http://192.168.4.2:80/";
String upravljacRasvjetomApi = "http://192.168.4.3:80/";

int lightsDuration = 5;
int sprinklesDuration = 5;

boolean alertSprinklesAfterLightsOff = false;
int lightsStartTime = 0;

WebServer webServer(80);
HTTPClient httpClient;

void setup() {
  Serial.begin(115200);
  delay(100);

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ap_ssid, ap_password);
  delay(100);
  Serial.println("Access Point started!");
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  Serial.println();

  Serial.print("Connecting to ");
  Serial.print(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Home WiFi IP (web application IP available at): ");
  Serial.println(WiFi.localIP());

  webServer.on("/", handleOnConnect);
  webServer.on("/enter-home", handleEnterHome);
  webServer.on("/exit-home", handleExitHome);
  webServer.on("/lights-on", handleLightsOn);
  webServer.on("/lights-off", handleLightsOff);
  webServer.on("/sprinklers-on", handleSprinklesOn);
  webServer.on("/set-durations", handleSetDurations);
  webServer.on("/open-gate", handleOpenGate);
  webServer.on("/close-gate", handleCloseGate);
  webServer.onNotFound(handleNotFound);

  webServer.begin();
  Serial.println("Server started!");
}

void loop() {
  webServer.handleClient();

  if (alertSprinklesAfterLightsOff) {
    if (millis() - lightsStartTime >= lightsDuration * 1000) {
      handleSprinklesOnManual();
      alertSprinklesAfterLightsOff = false;
    }
  }
}

void handleOnConnect() {
  Serial.println("Client connected to server");
  webServer.send(200, "text/html", sendHTML());
}

void handleNotFound() {
  webServer.send(404, "text/plain", "Not found");
}

void handleSetDurations() {
  if (webServer.hasArg("lightsDuration")) {
    lightsDuration = webServer.arg("lightsDuration").toInt();
  }
  if (webServer.hasArg("sprinklesDuration")) {
    sprinklesDuration = webServer.arg("sprinklesDuration").toInt(); 
  }
  webServer.send(200, "text/html", sendHTML());
}

void handleEnterHome() {
  Serial.println("Starting smart home function entering home...");

  String valueOgradaApi = "data=gate-open&manual=0";
  String valueRasvjetaApi = "data=lights-on&manual=0&duration=" + String(lightsDuration);

  callApi(upravljacOgradomApi, valueOgradaApi);
  callApi(upravljacRasvjetomApi, valueRasvjetaApi);
  webServer.send(200, "text/html", sendHTML());

  alertSprinklesAfterLightsOff = true;
  lightsStartTime = millis();
}

void handleExitHome() {
  Serial.println("Starting smart home function exiting home...");

  String valueOgradaApi = "data=gate-open&manual=0";
  String valueRasvjetaApi = "data=lights-on&manual=0&duration=" + String(lightsDuration);

  callApi(upravljacOgradomApi, valueOgradaApi);
  callApi(upravljacRasvjetomApi, valueRasvjetaApi);
  webServer.send(200, "text/html", sendHTML());

  alertSprinklesAfterLightsOff = true;
  lightsStartTime = millis();
}


void handleLightsOn() {
  Serial.println("Lights turning on...");

  String value = "data=lights-on&manual=1&duration=" + String(lightsDuration);

  callApi(upravljacRasvjetomApi, value);
  webServer.send(200, "text/html", sendHTML());
}

void handleLightsOff() {
  Serial.println("Lights turning off...");

  String value = "data=lights-off&manual=1&duration=" + String(lightsDuration);

  callApi(upravljacRasvjetomApi, value);
  webServer.send(200, "text/html", sendHTML());
}

void handleSprinklesOn() {
  Serial.println("Sprinklers turning on...");

  String value = "data=sprinklers-on&duration=" + String(sprinklesDuration);

  callApi(upravljacPrskalicaApi, value);
  webServer.send(200, "text/html", sendHTML());
}

void handleSprinklesOnManual() {
  Serial.println("Sprinklers turning on...");

  String value = "data=sprinklers-on&duration=" + String(sprinklesDuration);

  callApi(upravljacPrskalicaApi, value);
}

void handleOpenGate() {
  Serial.println("Gate opening...");

  String value = "data=gate-open&manual=1";

  callApi(upravljacOgradomApi, value);
  webServer.send(200, "text/html", sendHTML());
}

void handleCloseGate() {
  Serial.println("Gate closing...");

  String value = "data=gate-close&manual=1";

  callApi(upravljacOgradomApi, value);
  webServer.send(200, "text/html", sendHTML());
}

void callApi(String api, String value) {
  if (WiFi.status() == WL_CONNECTED || WiFi.softAPgetStationNum() > 0) {
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
  String html = R"(
    <!DOCTYPE html>
    <html>
    <head>
      <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
      <title>Smart Home Controls</title>
      <style>
        html { font-family: Helvetica; display: inline-block; margin: 10px 10px 100px 10px; text-align: center;}
        body { margin-top: 50px; }
        h1 { color: #444444; margin: 50px auto 30px; }
        h3 { color: #444444; margin-bottom: 50px; }
        .button { display: block; width: 200px; background-color: #3498db; border: none; color: white; padding: 13px 30px; text-decoration: none; font-size: 25px; margin: 0px auto 35px; cursor: pointer; border-radius: 4px; }
        .button:active { background-color: #2980b9; }
      </style>
    </head>
    <body>
      <h1>Smart Home Controls</h1>
      <a class="button" href="/enter-home">Enter the Home</a>
      <a class="button" href="/exit-home">Exit the Home</a>
      
      <h1>Control Panel</h1>
      <a class="button" href="/lights-on">Turn Lights On</a>
      <a class="button" href="/lights-off">Turn Lights Off</a>
      <a class="button" href="/sprinklers-on">Turn Sprinklers On</a>
      <a class="button" href="/open-gate">Open the Gate</a>
      <a class="button" href="/close-gate">Close the Gate</a>
      
      <h1>Configuration</h1>
      <form action="/set-durations">
        Lights Duration (seconds): <input type="number" name="lightsDuration" value=")" + String(lightsDuration) + R"(">
        <input type="submit" value="Apply">
      </form><br>
      
      <form action="/set-durations">  
        Sprinkles Duration (seconds): <input type="number" name="sprinklesDuration" value=")" + String(sprinklesDuration) + R"(">
        <input type="submit" value="Apply">
      </form>
    </body>
    </html>
    )";
  return html;
}