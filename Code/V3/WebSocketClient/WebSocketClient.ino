#include <WiFiClient.h>
#include <WiFiServer.h>
#include <ESP8266WiFi.h>

WiFiServer server(80);

const char *ssid = "TimingGate";
const char *password = "password";

const IPAddress IP(192,168,4,16);
const IPAddress mask(255,255,255,0);

IPAddress serverIP(192,168,4,15);

WiFiClient client;

#define LED LED_BUILTIN

int socketNumber;
int ledState = LOW;
bool connected = false;

void setup() {
  Serial.begin(115200);
  Serial.println("\r\n");
  
  pinMode(LED, OUTPUT);
  pinMode(A0, INPUT);

  connectToWiFi();

}

void loop() {
  led(500);
  if(wait(2000)) {
    client.connect(serverIP,8080);
    String data = readSensor();
    Serial.println(data); //Send timestamp to server
    client.print(data + "\r\n");
    client.flush();
  }
}

/*__________________________________________________________SETUP_FUNCTIONS__________________________________________________________*/

void connectToWiFi() { // Start a Wi-Fi access point, and try to connect to some given access points. Then wait for either an AP or STA connection
  //WiFi.mode(WIFI_STA);
  WiFi.config(IP,serverIP,mask);
  delay(100);
  WiFi.begin(ssid, password);             // Try to connect to WiFi
  Serial.print("Access Point \"");
  Serial.print(ssid);
  Serial.println("\" trying to connect\n");

  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {  // Wait for the Wi-Fi to connect
    yield();
    led(50);
    if(wait(250))
      Serial.print('.');
  }
  Serial.print("\nConnected to Access Point: ");
  Serial.println(WiFi.localIP());
  //APIP = WiFi.softAPIP();
}

/*__________________________________________________________HELPER_FUNCTIONS__________________________________________________________*/

String readSensor() {
  int sensorData = analogRead(A0);
  //Serial.println(sensorData);
  return String(sensorData);
}

/**
 * LED blinker that blinks without delay
 */
 
int previousTimeLED = 0;
 
void led(int interval) {
  unsigned long currentMillis = millis();
  if (currentMillis - previousTimeLED >= interval) {
    previousTimeLED = currentMillis;
    if (ledState == LOW) {
      ledState = HIGH;  // Note that this switches the LED *off*
    } else {
      ledState = LOW;  // Note that this switches the LED *on*
    }
    digitalWrite(LED, ledState);
  }
}

int previousTimeWait = 0;

bool wait(int interval) {
  unsigned long currentMillis = millis();
  if (currentMillis - previousTimeWait >= interval) {
    previousTimeWait = currentMillis;
    return true;
  }
  return false;
}
