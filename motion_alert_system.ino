#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// WiFi and Telegram credentials
const char* ssid = "Lenovo";
const char* password = "123456789";
#define BOTtoken "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx" // Bot token from BotFather
#define CHAT_ID "xxxxxxxxxxxx" // Your chat ID

X509List cert(TELEGRAM_CERTIFICATE_ROOT); 
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

int const trigPin = 4;
int const echoPin = 5;
int const buzzPin = 12;
const int LED1 = 13;

bool alertTriggered = false;

void setup() {
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzPin, OUTPUT);
  pinMode(LED1, OUTPUT);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");

  // Send message to Telegram when WiFi is connected
  bot.sendMessage(CHAT_ID, "WiFi Connected!", "");
  Serial.println("WiFi connection notification sent.");

  // Set time via NTP
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  Serial.println("Waiting for time sync...");
  while (time(nullptr) < 1000000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Time synchronized");

  // Set root certificates for Telegram
  client.setTrustAnchors(&cert);

  // Send message to Telegram when system setup is complete (time sync done)
  bot.sendMessage(CHAT_ID, "System setup complete! Time synchronized.", "");
  Serial.println("System setup notification sent.");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi not connected. Reconnecting...");
    WiFi.reconnect();
    delay(3000);  // Retry WiFi connection every 5 seconds
  }

  int duration, distance;
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10); // Pulse trigger pin
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration / 2) / 29.1;

  if (distance <= 10 && distance >= 0) {
    if (!alertTriggered && WiFi.status() == WL_CONNECTED) {
      digitalWrite(buzzPin, HIGH);
      digitalWrite(LED1, HIGH);
      Serial.println("ALERT! MOTION DETECTED! Sending Telegram message...");
      if (bot.sendMessage(CHAT_ID, "ALERT! MOTION DETECTED!!", "")) {
        Serial.println("Telegram message sent successfully.");
      } else {
        Serial.println("Telegram message failed.");
      }
      alertTriggered = true;

      // Shorter delay for turning off the LED and buzzer
      delay(500);  // Adjust the time LED and buzzer stay on (2 seconds)
      digitalWrite(buzzPin, LOW);
      digitalWrite(LED1, LOW);
    }
  } else {
    digitalWrite(buzzPin, LOW);
    digitalWrite(LED1, LOW);
    alertTriggered = false; // Reset alert state
  }

  delay(500);  // Small delay to avoid rapid looping
}
