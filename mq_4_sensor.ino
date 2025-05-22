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
int gasPin = A0;
int gasThreshold = 105;
bool alertSent = false;  // Prevent multiple alerts for same spike

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  ThingSpeak.begin(client);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
}

void loop() {
  int gasValue = analogRead(gasPin);
  Serial.print("Gas Value: ");
  Serial.println(gasValue);

  // Send data to ThingSpeak
  ThingSpeak.setField(1, gasValue);
  ThingSpeak.writeFields(channelNumber, writeAPIKey);

  // Check threshold
  if (gasValue > gasThreshold && !alertSent) {
    sendTelegramMessage("⚠️ Meat Waste Detected!\nGas Value: " + String(gasValue));
    alertSent = true;
  } else if (gasValue <= gasThreshold) {
    alertSent = false;
  }

  delay(15000); // ThingSpeak update interval
}

void sendTelegramMessage(String message) {
  WiFiClientSecure client;
  client.setInsecure();

  if (!client.connect("api.telegram.org", 443)) {
    Serial.println("Telegram connection failed!");
    return;
  }

  // URL encode the message content
  message.replace(" ", "%20");
  message.replace("\n", "%0A");
  message.replace("!", "%21");
  message.replace(":", "%3A");
  message.replace("/", "%2F");

  String url = String("/bot") + botToken + "/sendMessage?chat_id=" + chatID + "&text=" + message;
  Serial.println("Requesting Telegram URL: " + url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: api.telegram.org\r\n" +
               "Connection: close\r\n\r\n");

  // Read only headers and skip full response body
  unsigned long timeout = millis();
  while (client.connected() && millis() - timeout < 2000) {
    String line = client.readStringUntil('\n');
    if (line == "\r") break;
  }

  client.stop(); // Close connection early
  Serial.println("Telegram message sent (quick exit)");
}
