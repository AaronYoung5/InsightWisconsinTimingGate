#include <ESP8266WiFi.h>
#include <WiFiCLient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>

#define HTMLFILE "/index.htm"

const char *ssid = "TimingGate";
const char *password = "password";

IPAddress IP(192,168,4,15);
IPAddress mask = (255,255,255,0);

ESP8266WebServer server(80);

int ledPin = LED_BUILTIN;

String path = "/index.htm";
String html = "";

double data;

void setup() {
  delay(1000);
  //led(500);
  Serial.begin(115200);
  Serial.println();
  Serial.println("Configuring Access Point");
  pinMode(ledPin,OUTPUT);

  //WiFi.mode(WIFI_STA); // Hides the view of the ESP as a wifi network
  //WiFi.mode(WIFI_AP_STA); // Both hotspot and client are enabled
  //WiFi.mode(WIFI_AP); // Just access point
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(IP,IP,mask);
  
  Serial.println("AP IP Address: " + WiFi.softAPIP());

  delay(2000);
  while (!MDNS.begin("TimingGate")) {
    Serial.println("Error setting up MDNS responder!");
    led(50);
  }

  Serial.println("mDNS responder started");
  led(500);

  if(SPIFFS.begin()) {
    Serial.println("SPIFFS BEGUN");
    if(loadFromSPIFFS(path)) {
      Serial.println("Successfully loaded SPIFFS file");
    }
    else {
      Serial.println("SPIFFS File Load Failed");
    }
  }
  
  // Sets which routine to hand at root location
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);

  // Starting server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // Handle client requests
  server.handleClient();
  led(500);
}

void handleNotFound() {
  Serial.println("Test");
  server.send(404, "text/plain", "Failed");
  while(wait(1000)) {
    led(50);
  }
}

void handleRoot() {
  if(html != "") {
    server.arg(String(readSensor()));
    server.send(200, "text/html", html);
    Serial.println("Data Sent");
  }
  else {
    handleNotFound();
    return;
  }
  while(wait(1000)) {
    led(50);
  }
}

bool loadFromSPIFFS(String path) {
  File file = SPIFFS.open(path, "r");

  if(!file) {
    Serial.print("Don't know this command and it's not a file in SPIFFS : ");
    Serial.println(path);
    return false;
  }
  else {
    Serial.println("File Found!");
    while(file.position() < file.size()) {
      html += file.readStringUntil('\n') + "\n";
      html.trim();
    }
    Serial.println(html);
    file.close();
    return true;
  }
}

int readSensor() {
  int sensorData = analogRead(A0);
  Serial.println(sensorData);
  return sensorData;
}

/**
 * LED blinker that blinks without delay
 */
unsigned long previousTime = 0;
int ledState = LOW;

void led(int interval) {
  unsigned long currentMillis = millis();
  if (currentMillis - previousTime >= interval) {
    previousTime = currentMillis;
    if (ledState == LOW) {
      ledState = HIGH;  // Note that this switches the LED *off*
    } else {
      ledState = LOW;  // Note that this switches the LED *on*
    }
    digitalWrite(ledPin, ledState);
  }
}

/**
 * Waiter without a delay
 */
bool wait(int interval) {
  return millis() - previousTime >= interval;
}
