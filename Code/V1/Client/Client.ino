#include <ESP8266WiFi.h>  //WiFi Library

#define analog A0 //Analog pin

byte ledPin = LED_BUILTIN;
char ssid[] = "TimingGate"; // SSID of the AP
char pass[] = "password";   // password of the AP

IPAddress IP(192,168,4,17);  //IP address of client, one client must be (x.x.x.16) and one must be (x.x.x.17)
IPAddress gateway(192,168,4,15);  //gateway IP of server to used to configure localIP of client
IPAddress subnet(255,255,255,0);  //IP of server to used to configure localIP of client

IPAddress server(192,168,4,15);   //IP address of the AP

WiFiClient client;  //Technically the server that will have information sent using WiFiClient object

boolean laserConnected = false; //Variable describing if laser has been connected

int interval = 500; //Blink interval of LED

float savedTimeStamp = 0; //Time stamp being saved

boolean timeSent = false; //Variable describing if the time was sent

/**
 * Setup method run once
 */
void setup() {
  //Begin serial communication
  Serial.begin(115200);
  Serial.println("\r\n");

  //Set pinModes of pins being used
  pinMode(analog,INPUT);
  pinMode(ledPin, OUTPUT);

  //Set WiFi mode to station mode
  WiFi.mode(WIFI_STA);
  //Configures client to have the specified localIP
  WiFi.config(IP, gateway, subnet);
  //Begin connection process to WiFi
  WiFi.begin(ssid, pass);

  Serial.println(WiFi.localIP());
  //Attempt to connect to WiFi
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println(WiFi.localIP());
  
}

/**
 * Main Loop run repetitively
 */
void loop() {
  if(WiFi.status() != WL_CONNECTED) {
    timeSent = false;
  }
  
  led();

  //Read photoresistor value
  float sensorData = analogRead(analog);

  //If starting time has not been sent
  if(!timeSent) {
    sendTime();
    timeSent = true;
  }

  //Laser Connection Analysis
  if(sensorData < 100 && !laserConnected) { //If laser is not connected and has not connected, continue
    interval = 500;
    return;
  }
  else if(sensorData > 100 && !laserConnected) { //If laser is connected and has not connected, set laser as connected
    laserConnected = true;
    return;
  }
  else if(sensorData > 100 && laserConnected) { //If laser is connected and has connected
    interval = 1000;
    return;
  }
  else if(sensorData < 100 && laserConnected) { //If laser is not connected and has connected (broken beam), store time stamp
    interval = 50;

    if(savedTimeStamp == 0) //If no timestamp has occured, save timestamp
        savedTimeStamp = millis();
        
    client.connect(server, 8080); //Connect to server with correct "port" number

    Serial.println(client.print("1 " + formatTime(savedTimeStamp) + "\r\n")); //Send timestamp to server

    savedTimeStamp = 0;
    laserConnected = false;
    client.flush();
  }
}

/**
 * Sends time to server. Used to offset startup time and 
 * variance in internal clocks between client and server
 */
boolean sendTime() {
  interval = 100;
  client.connect(server, 8080);
  client.println("0 " + String(millis()));
  client.flush();
}

/**
 * Formats time to be seconds : milliseconds
 */
String formatTime(int currentTime) {
  return String(currentTime / 1000.0, 3);
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
