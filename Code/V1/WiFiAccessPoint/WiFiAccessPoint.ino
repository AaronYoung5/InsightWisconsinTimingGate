#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

#define NUMCLIENTS 2
#define ledPin LED_BUILTIN

IPAddress IP(192,168,4,15); //IP of AP to be produced
IPAddress mask = (255, 255, 255, 0);

IPAddress clients[NUMCLIENTS] = {IPAddress(192,168,4,16), IPAddress(192,168,4,17)}; //Hard coded IP addresses of clients

const char *ssid = "TimingGate";    // SSID of the AP being produced
const char *password = "password";  // Password of the AP being produced

ESP8266WebServer server(80); //Server for communication to user
WiFiServer clientServer(8080); //Server for communication to clients

int interval = 500; //Blink interval of LED

boolean newDataPresent = false; //Variable describing if new data has been sent to server
String informationToSend = ""; //Variable describing the information to send to user

IPAddress lastIP(0,0,0,0); //IP of the last client to have sent information to server

float clientTimes[NUMCLIENTS] = { 0.0 }; // Array that stores the variation in startup times to be used to offset timestamp
//boolean clientsConnected[NUMCLIENTS] = { false }; // Array with variables describing if index client has connected to server

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  Serial.print("Configuring access point...");
  pinMode(ledPin, OUTPUT);
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(IP, IP, mask);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.on("/Python", handleRoot);
  server.on("/", handleRoot);
  server.begin();
  clientServer.begin();
  Serial.println("HTTP server started");
}

void loop() {
  led();
  server.handleClient();
  WiFiClient client = clientServer.available();
  if (!client) {return;}
  interval = 500;
  IPAddress currentIP = client.remoteIP();
  String response = readClient(client);
  analyzeResponse(response);
}

/**
 * Handles data requests
 */
void handleRoot() {
  //If new data is present, send this data
  if(newDataPresent) {
    server.send(200, "text/plain", String(informationToSend));
    newDataPresent = false;
  }
  else //If no new data is present, send "No New Data"
    server.send(200, "text/plain", "No New Data");
}

/**
 * Read client for time
 */
String readClient(WiFiClient client) {
  String request = client.readStringUntil('\r');
  client.flush();
  return request + "\t" + IPToString(client.remoteIP());
}

/**
 * Analyzes response from clients
 * 
 * Example response:
 * [command] [time] [client IP]
 * 
 * 0 = time command from client connection
 * 1 = time command from sensor broken
 */
void analyzeResponse(String response) {
  //Serial.println(response);

  //Command type
  int command = response.substring(0,1).toInt();

  //Switch statement analyzing the command type
  switch (command) {
    case 0: //Sets times sent to client times
    {
      int spaceTwo = response.indexOf('\t', 2);
      float clientTime = response.substring(2, spaceTwo).toFloat();
      IPAddress clientIP = StringToIP(response.substring(spaceTwo));
      float currentTime = formatTime(millis());
      if(clientIP == clients[0])
        clientTimes[0] = currentTime - formatTime(clientTime);
      else if (clientIP == clients[1])
        clientTimes[1] = currentTime - formatTime(clientTime);
      break;
    }
    case 1: //Reads new time and sets new data to true
    {
      int spaceTwo = response.indexOf('\t', 2);
      float recordedTime = response.substring(2, spaceTwo).toFloat();
      IPAddress clientIP = StringToIP(response.substring(spaceTwo));
      if(clientIP == clients[0])
        recordedTime += clientTimes[0];
      else if (clientIP == clients[1])
        recordedTime += clientTimes[1];
      informationToSend = String(recordedTime) + " " + response.substring(spaceTwo + 1);
      newDataPresent = true;
      break;
    }
    default:
      Serial.println("Analyzation of command quit. Command type must be 0 or 1");
      break;
  }
}

/**
 * Reads IP in string format and outputs as an IPAddress
 */
IPAddress StringToIP(String ip) {
  int periodIndicies[3];
  periodIndicies[0] = ip.indexOf('.');
  periodIndicies[1] = ip.indexOf('.', periodIndicies[0] + 1);
  periodIndicies[2] = ip.indexOf('.', periodIndicies[1] + 1);
  return IPAddress(ip.substring(0,periodIndicies[0]).toInt(),
            ip.substring(periodIndicies[0] + 1,periodIndicies[1]).toInt(),
            ip.substring(periodIndicies[1] + 1,periodIndicies[2]).toInt(),
            ip.substring(periodIndicies[2] + 1).toInt());
}

/**
 * Takes an IPAddress and formats it as a string
 */
String IPToString(IPAddress ip) {
  return String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
}


/**
 * Formats time to be seconds : milliseconds
 */
float formatTime(int currentTime) {
  return String(float(currentTime) / 1000.0, 3).toFloat();
}


/**
 * LED blinker that blinks without delay
 */
unsigned long previousTime = 0;
int ledState = LOW;

void led() {
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
