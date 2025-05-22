#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ThingSpeak.h>

// WiFi credentials
const char* ssid = "Airtel_loge_5267";
const char* password = "Air@93925";

// Telegram Bot credentials
const char* botToken = "8090171057:AAFokmFhV7Kr1q7WU1SdXFLV7ej-vFpTItk";
const char* chatID = "7927407309";

// ThingSpeak settings
WiFiClient client;
unsigned long channelNumber = 2968320;
const char *writeAPIKey = "URDOOVICBCWPWKGF";

// Sensor settings
int mq135Pin = A0;
int buzzerPin = D2;
int gasThreshold = 150;
bool alertSent = false;

void setup() {
  Serial.begin(115200);
  pinMode(buzzerPin, OUTPUT);
  WiFi.begin(ssid, password);
  ThingSpeak.begin(client);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  int gasValue = analogRead(mq135Pin);
  Serial.print("Gas Value: ");
  Serial.println(gasValue);

  // Send to ThingSpeak
  ThingSpeak.setField(2, gasValue);
  ThingSpeak.writeFields(channelNumber, writeAPIKey);

  // Check for threshold
  if (gasValue > gasThreshold && !alertSent) {
    sendTelegramMessage("⚠️ Meat Waste Detected!\nGas Value: " + String(gasValue));
    digitalWrite(buzzerPin, HIGH);
    alertSent = true;
  } else if (gasValue <= gasThreshold) {
    digitalWrite(buzzerPin, LOW);
    alertSent = false;
  }

  delay(5000); // Every 15 seconds
}

void sendTelegramMessage(String message) {
  WiFiClientSecure client;
  client.setInsecure();

  if (!client.connect("api.telegram.org", 443)) {
    Serial.println("Connection to Telegram failed!");
    return;
  }

  // Encode message
  message.replace(" ", "%20");
  message.replace("\n", "%0A");
  message.replace("!", "%21");
  message.replace(":", "%3A");

  String url = String("/bot") + botToken + "/sendMessage?chat_id=" + chatID + "&text=" + message;

  Serial.println("Requesting Telegram URL: " + url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: api.telegram.org\r\n" +
               "Connection: close\r\n\r\n");

  // Quick exit for faster loop
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") break;
  }
  Serial.println("Telegram message sent");
}
