#include <ESP8266WiFi.h>

WiFiServer server(80);
IPAddress IP(192,168,4,15);
IPAddress mask = (255, 255, 255, 0);

char ssid[] = "TimingGate";           // SSID of your AP
char pass[] = "password";         // password of your AP

const int maxClients = 3;

//IPAddress *clientIPS[maxClients] = { IP(192,168,4,15), IP(192,168,4,15), NULL}

WiFiClient *clients[maxClients] = { NULL };

unsigned long previousTime = 0;

byte ledPin = LED_BUILTIN;

int interval = 500;
int ledState = LOW;

void setup() {
  Serial.begin(115200);
  delay(2000);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, pass);
  WiFi.softAPConfig(IP, IP, mask);
  server.begin();
  pinMode(ledPin, OUTPUT);
  Serial.println("Server started.");
}

void loop() {
  WiFiClient client = server.available();
  led();
  if (client) {
    for (int i=0 ; i<maxClients ; ++i) {
      if (NULL == clients[i]) {
        clients[i] = new WiFiClient(client);
        Serial.println("New Client");
        break;
      }
    }
  }
  else {
    for (int i=0 ; i<maxClients ; ++i) {
      if (NULL != clients[i]) {
        if (clients[i]->available()) {
          digitalWrite(ledPin, HIGH);
          String request = client.readStringUntil('\r');
          Serial.println(request);
          client.flush();
          digitalWrite(ledPin, LOW);
        }
      }
    }
  }
}

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
