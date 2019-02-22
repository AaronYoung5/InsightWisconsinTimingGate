#include <ESPTemplateProcessor.h>
#include <ESP8266WiFi.h>
#include <WiFiCLient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>

#define HTMLFILE "/index.htm"

const char* ssid = "TimingGate";
const char* password = "password";

IPAddress IP(192,168,4,15);
IPAddress mask = (255,255,255,0);

ESP8266WebServer server(80);

int ledPin = LED_BUILTIN;

String htmlPath = "/";

void setup() {
  led(500);
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  pinMode(ledPin,OUTPUT);

  //WiFi.mode(WIFI_STA); // Hides the view of the ESP as a wifi network
  //WiFi.mode(WIFI_AP_STA); // Both hotspot and client are enabled
  WiFi.mode(WIFI_AP); // Just access point
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(IP,IP,mask);

  while (!MDNS.begin("TimingGate")) {
    Serial.println("Error setting up MDNS responder!");
    led(50);
  }

  Serial.println("mDNS responder started");
  led(500);
  
  if(SPIFFS.begin()) {
    Serial.println("SPIFFS BEGUN");
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
  server.send(404, "text/plain", "Failed");
  while(wait(1000)) {
    led(50);
  }
}

void handleRoot() {
  if(ESPTemplateProcessor(server).send(String("/index.htm"), indexProcessor)) {
    Serial.println("Data Sent");
  }
  else if{
    handleNotFound();
    return;
  }
  while(wait(1000)) {
    led(50);
  }
}

String indexProcessor(const String& key) {
  Serial.println(String("KEY IS ") + key);
  if (key == "LEDSTATUS") return String(readSensor());
  
  return "oops";
}

int readSensor() {
  return analogRead(A0);
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
