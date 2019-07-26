#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <WebSocketsServer.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

ESP8266WebServer webServer(80);          // create a web server on port 80
WebSocketsServer webSocket(81);       // create a websocket server on port 81
WiFiServer wifiServer(8080);

File fsUploadFile;                    // a File variable to temporarily store the received file

const char *ssid = "TimingGate";      // The name of the Wi-Fi network that will be created
const char *password = "";    // The password required to connect to it, leave blank for an open network

IPAddress IP(192,168,4,15);
IPAddress mask(255,255,255,0);

#define MAX_CLIENTS 2
IPAddress clientsConnected[MAX_CLIENTS];

const char *OTAName = "TimingGate";      // A name and a password for the OTA service
const char *OTAPassword = "";

#define LED  LED_BUILTIN          // specify the pins with for blinking LED
const int button = D0;                // specify the pin for the button

const char* mdnsName = "timinggate";     // Domain name for the mDNS responder

int socketNumber;
int previousTime = 0;
int ledState = LOW;

int buttonState = LOW;

/*__________________________________________________________SETUP__________________________________________________________*/

void setup() {
  pinMode(LED, OUTPUT);        // the pins with LEDs connected are outputs
  pinMode(button, INPUT);

  Serial.begin(115200);        // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println("\r\n");

  startWiFi();                 // Start a Wi-Fi access point, and try to connect to some given access points. Then wait for either an AP or STA connection
  
  startOTA();                  // Start the OTA service
  
  startSPIFFS();               // Start the SPIFFS and list all contents

  startWebSocket();            // Start a WebSocket server
  
  startMDNS();                 // Start the mDNS responder

  startWebServer();            // Start a HTTP server with a file read handler and an upload handler

  startWifiServer();
}

/*__________________________________________________________LOOP__________________________________________________________*/

bool enabled = false;             // The rainbow effect is turned off on startup

unsigned long prevMillis = millis();

void loop() {
  led(500);
  webSocket.loop();                           // constantly check for websocket events
  webServer.handleClient();                      // run the server
  ArduinoOTA.handle();                        // listen for OTA events

  if(enabled) {
    checkClientCommunication();
  }
  else if(digitalRead(button) != buttonState ) {
    buttonState = !buttonState;
    if(buttonState == LOW) {
      Serial.println("Button: not active");
      startTimer();
    }
    else {
      Serial.println("Button: active");
      //startTimer();
    }
  }
  
}

/*__________________________________________________________SETUP_FUNCTIONS__________________________________________________________*/

/*
  Start a Wi-Fi access point. Wait for the AP connection.
 */
void startWiFi() {
  WiFi.softAP(ssid, password);             // Start the access point
  WiFi.softAPConfig(IP,IP,mask);
  Serial.print("Access Point \"");
  Serial.print(ssid);
  Serial.println("\" started\r\n");

  Serial.println("\r\n");
  if(WiFi.softAPgetStationNum() == 0) {      // If the ESP is connected to an AP
    Serial.print("Connected to ");
    Serial.println(WiFi.SSID());             // Tell us what network we're connected to
    Serial.print("IP address:\t");
    Serial.print(WiFi.localIP());            // Send the IP address of the ESP8266 to the computer
  } else {                                   // If a station is connected to the ESP SoftAP
    Serial.print("Station connected to ESP8266 AP");
  }
  Serial.println("\r\n");
}

void startOTA() { // Start the OTA service
  ArduinoOTA.setHostname(OTAName);
  ArduinoOTA.setPassword(OTAPassword);

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
    //digitalWrite(LED_RED, 0);    // turn off the LEDs
    //digitalWrite(LED_GREEN, 0);
    //digitalWrite(LED_BLUE, 0);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\r\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("OTA ready\r\n");
}

void startSPIFFS() { // Start the SPIFFS and list all contents
  SPIFFS.begin();                             // Start the SPI Flash File System (SPIFFS)
  Serial.println("SPIFFS started. Contents:");
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {                      // List the file system contents
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf("\tFS File: %s, size: %s\r\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
    Serial.printf("\n");
  }
}

void startWebSocket() { // Start a WebSocket server
  webSocket.begin();                          // start the websocket server
  webSocket.onEvent(webSocketEvent);          // if there's an incomming websocket message, go to function 'webSocketEvent'
  Serial.println("WebSocket server started.");
}

void startMDNS() { // Start the mDNS responder
  MDNS.begin(mdnsName);                        // start the multicast domain name server
  Serial.print("mDNS responder started: http://");
  Serial.print(mdnsName);
  Serial.println(".local");
}

void startWebServer() { // Start a HTTP server with a file read handler and an upload handler
  webServer.on("/edit.html",  HTTP_POST, []() {  // If a POST request is sent to the /edit.html address,
    webServer.send(200, "text/plain", ""); 
  }, handleFileUpload);                       // go to 'handleFileUpload'

  webServer.onNotFound(handleNotFound);          // if someone requests any other file or page, go to function 'handleNotFound'
                                              // and check if the file exists

  webServer.begin();                             // start the HTTP server
  Serial.println("HTTP server started.");
}

void startWifiServer() {
  wifiServer.begin();
}

/*__________________________________________________________SERVER_HANDLERS__________________________________________________________*/

void handleNotFound(){ // if the requested file or page doesn't exist, return a 404 not found error
  if(!handleFileRead(webServer.uri())){          // check if the file exists in the flash memory (SPIFFS), if so, send it
    webServer.send(404, "text/plain", "404: File Not Found");
  }
}

bool handleFileRead(String path) { // send the right file to the client (if it exists)
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";          // If a folder is requested, send the index file
  String contentType = getContentType(path);             // Get the MIME type
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) { // If the file exists, either as a compressed archive, or normal
    if (SPIFFS.exists(pathWithGz))                         // If there's a compressed version available
      path += ".gz";                                         // Use the compressed verion
    File file = SPIFFS.open(path, "r");                    // Open the file
    size_t sent = webServer.streamFile(file, contentType);    // Send it to the client
    file.close();                                          // Close the file again
    Serial.println(String("\tSent file: ") + path);
    return true;
  }
  Serial.println(String("\tFile Not Found: ") + path);   // If the file doesn't exist, return false
  return false;
}

void handleFileUpload(){ // upload a new file to the SPIFFS
  HTTPUpload& upload = webServer.upload();
  String path;
  if(upload.status == UPLOAD_FILE_START){
    path = upload.filename;
    if(!path.startsWith("/")) path = "/"+path;
    if(!path.endsWith(".gz")) {                          // The file server always prefers a compressed version of a file 
      String pathWithGz = path+".gz";                    // So if an uploaded file is not compressed, the existing compressed
      if(SPIFFS.exists(pathWithGz))                      // version of that file must be deleted (if it exists)
         SPIFFS.remove(pathWithGz);
    }
    Serial.print("handleFileUpload Name: "); Serial.println(path);
    fsUploadFile = SPIFFS.open(path, "w");            // Open the file for writing in SPIFFS (create if it doesn't exist)
    path = String();
  } else if(upload.status == UPLOAD_FILE_WRITE){
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
  } else if(upload.status == UPLOAD_FILE_END){
    if(fsUploadFile) {                                    // If the file was successfully created
      fsUploadFile.close();                               // Close the file again
      Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
      webServer.sendHeader("Location","/success.html");      // Redirect the client to the success page
      webServer.send(303);
    } else {
      webServer.send(500, "text/plain", "500: couldn't create file");
    }
  }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) { // When a WebSocket message is received
  switch (type) {
    case WStype_DISCONNECTED:             // if the websocket is disconnected
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {              // if a new websocket connection is established
        socketNumber = num;
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        enabled = false;                  // Turn rainbow off when a new connection is established
        calibrateMCU();
      }
      break;
    case WStype_TEXT:                     // if new text data is received
      Serial.printf("[%u] get Text: %s\n", num, payload);
      if (payload[0] == '#') {
        
      } else if (payload[0] == 'E') {                      // the browser sends an E when the timing gate is enabled
        Serial.println("Enabled");
        enabled = true;
      } else if (payload[0] == 'D') {                      // the browser sends an D when the timing gate is disabled
        enabled = false;
      }
      break;
    case WStype_ERROR:
      Serial.printf("Error [%u] , %s\n", num, payload);
      yield();
      break;
  }
}

/*__________________________________________________________HELPER_FUNCTIONS__________________________________________________________*/

void clientStatus() {
 
  unsigned char number_client;
  struct station_info *stat_info;
  
  struct ip_addr *IPaddress;
  IPAddress address;
  int i=1;
  
  number_client= wifi_softap_get_station_num();
  stat_info = wifi_softap_get_station_info();
  
  Serial.print(" Total Connected Clients are = ");
  Serial.println(number_client);
  
    while (stat_info != NULL) {
    
      IPaddress = &stat_info->ip;
      address = IPaddress->addr;
      
      Serial.print("client= ");
      
      Serial.print(i);
      Serial.print(" IP address is = ");
      Serial.print((address));
      
      stat_info = STAILQ_NEXT(stat_info, next);
      i++;
      Serial.println();
    }
    
  delay(500);
}

void calibrateMCU() {
  int capacity = 65; // Use arduinojson.org/assistant to compute the capacity.
  StaticJsonBuffer<65> jsonBuffer;
  
  JsonObject& root = jsonBuffer.createObject();
  
  root["IP"] = IPToString(IP);
  root["action"] = "calibrating";
  root["time"] = millis();

  String json;
  root.printTo(json);

  webSocket.sendTXT(socketNumber, json);
}

void startTimer() {
  int capacity = 65; // Use arduinojson.org/assistant to compute the capacity.
  StaticJsonBuffer<65> jsonBuffer;
  
  JsonObject& root = jsonBuffer.createObject();
  
  root["IP"] = IPToString(IP);
  root["action"] = "buttonOn";
  root["time"] = millis();

  String json;
  root.printTo(json);

  webSocket.sendTXT(socketNumber, json);
}

void stopTimer() {
  int capacity = 65; // Use arduinojson.org/assistant to compute the capacity.
  StaticJsonBuffer<65> jsonBuffer;
  
  JsonObject& root = jsonBuffer.createObject();
  
  root["IP"] = IPToString(IP);
  root["action"] = "buttonOn";
  root["time"] = millis();


  String json;
  root.printTo(json);

  webSocket.sendTXT(socketNumber, json);
}

void sendClientData(String sensorData, IPAddress clientIP) {
  // Memory pool for JSON object tree.
  //
  // Inside the brackets, 200 is the size of the pool in bytes.
  // Don't forget to change this value to match your JSON document.
  // Use arduinojson.org/assistant to compute the capacity.
  StaticJsonBuffer<200> jsonBuffer;

  // StaticJsonBuffer allocates memory on the stack, it can be
  // replaced by DynamicJsonBuffer which allocates in the heap.
  //
  // DynamicJsonBuffer  jsonBuffer(200);

  // Create the root of the object tree.
  //
  // It's a reference to the JsonObject, the actual bytes are inside the
  // JsonBuffer with all the other nodes of the object tree.
  // Memory is freed when jsonBuffer goes out of scope.
  JsonObject& root = jsonBuffer.createObject();

  // Add values in the object
  //
  // Most of the time, you can rely on the implicit casts.
  // In other case, you can do root.set<long>("time", 1351824120);
  root["IP"] = IPToString(clientIP);
  root["time"] = formatTime(millis());

  // Add a nested array.
  //
  // It's also possible to create the array separately and add it to the
  // JsonObject but it's less efficient.
  //JsonArray& data = root.createNestedArray("data");
  //data.add(48.756080);
  //data.add(2.302038);

  root.printTo(Serial);
  // This prints:
  // {"sensor":"gps","time":1351824120,"data":[48.756080,2.302038]}

  Serial.println();

  root.prettyPrintTo(Serial);
  // This prints:
  // {
  //   "sensor": "gps",
  //   "time": 1351824120,
  //   "data": [
  //     48.756080,
  //     2.302038
  //   ]
  // }

  String json;
  root.printTo(json);

  webSocket.sendTXT(socketNumber, json);
}

void checkClientCommunication() {
  if(WiFi.softAPgetStationNum() != 0) {                 // If a station is connected to the ESP SoftAP
    WiFiClient client = wifiServer.available();
    if(client) {
      String sensorData = client.readStringUntil('\r'); 
      sendClientData(sensorData, client.remoteIP());
      client.flush();
    }
  }
}

/*
void startTimer() {
  Serial.println("Sending data");
  sendClientData("Start Watch", IP);
}*/

String formatBytes(size_t bytes) { // convert sizes in bytes to KB and MB
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  }
}

String getContentType(String filename) { // determine the filetype of a given filename, based on the extension
  if (webServer.hasArg("download"))      return "application/octet-stream";
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html"))return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js"))  return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz"))  return "application/x-gzip";
  else if (filename.endsWith(".svg")) return "image/svg+xml";
  return "text/plain";
}

/**
 * LED blinker that blinks without delay
 */
void led(int interval) {
  unsigned long currentMillis = millis();
  if (currentMillis - previousTime >= interval) {
    previousTime = currentMillis;
    if (ledState == LOW) {
      ledState = HIGH;  // Note that this switches the LED *off*
    } else {
      ledState = LOW;  // Note that this switches the LED *on*
    }
    digitalWrite(LED, ledState);
  }
}

/**
 * Formats time to be seconds : milliseconds
 */
float formatTime(int currentTime) {
  return String(float(currentTime) / 1000.0, 3).toFloat();
}

/**
 * Takes an IPAddress and formats it as a string
 */
String IPToString(IPAddress ip) {
  return String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
}
