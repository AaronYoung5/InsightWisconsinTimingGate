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
String html = "";

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
  
  initSPIFFS();
  
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

void initSPIFFS() {
  if(SPIFFS.begin()) {
    delay(1000);
    Serial.println("Reading SPIFFS File");
    delay(1000);
    if(loadFromSPIFFS(HTMLFILE)) {
      html = prepareHTMLPage();
      Serial.println(html);
    }
  }
  else {
    Serial.println("Unable to activate SPIFFS");
  }
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
  else if (!webPageConnected()) {
    handleNotFound();
    return;
  }
  //server.send(200, "text/html", prepareHTMLPage());
  while(wait(1000)) {
    led(50);
  }
}

String indexProcessor(const String& key) {
  Serial.println(String("KEY IS ") + key);
  if (key == "LEDSTATUS") return String(readSensor());
  
  return "oops";
}

bool webPageConnected() {
  return html != "";
}

//tell the client what to do with data
bool loadFromSPIFFS(String path) {
  
  String dataType = "text/plain";
  if (path.endsWith("/")) path += "index.htm"; //this is where index.htm is created

  if (path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
  else if (path.endsWith(".htm")) dataType = "text/html";
  else if (path.endsWith(".css")) dataType = "text/css";
  else if (path.endsWith(".js")) dataType = "application/javascript";
  else if (path.endsWith(".png")) dataType = "image/png";
  else if (path.endsWith(".gif")) dataType = "image/gif";
  else if (path.endsWith(".jpg")) dataType = "image/jpeg";
  else if (path.endsWith(".ico")) dataType = "image/x-icon";
  else if (path.endsWith(".xml")) dataType = "text/xml";
  else if (path.endsWith(".pdf")) dataType = "application/pdf";
  else if (path.endsWith(".zip")) dataType = "application/zip";

  File file = SPIFFS.open(path, "r");   //open file to read
  
  if (!file) { //unsuccesful open
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
    if (server.hasArg("download")) {
      Serial.println("Has Arg");
    }
    if(server.streamFile(file, "text/html") != file.size()) {
      Serial.println("Stream File");
    }
    
    file.close();
    return true;
  }
}

int readSensor() {
  return analogRead(A0);
}

/*
 * Prepares a String that will serve as the HTML code for the website
 */
String prepareHTMLPage() {
  if(html != "") {
    
    //String htmlPage =
       //String("<html>\n<h1>Welcome to ESP8266.</h1>\n</html>");
    return html;
  }
  return "<html>\n<h1>404 ERROR</h1>\n</html>";
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
