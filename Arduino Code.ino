#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include "DHT.h"
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

//Inisialisasi LED
#define lampu1 D6
#define lampu2 D7
#define lampu3 D8
#define lampu4 D3
#define lampu5 D4

//Inisialisasi SENSOR
#define DHTPIN D5
#define DHTTYPE DHT11

unsigned long previousMillis = 0; // Waktu terakhir data dipublish
const long interval = 10000;

LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHTPIN, DHTTYPE);

// variable akun Wifi
const char *ssid = "Cahyo123";
const char *password = "09102024";
const char *mqtt_server = "x2.revolusi-it.com";

WiFiClient espClient;
PubSubClient client(espClient);
String messages;

float humidity;
float temperature;
const int lampu[] = {lampu1, lampu2, lampu3};

// Function to connect to MQTT server
void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT Server -> ");
    Serial.println(mqtt_server);

    if (client.connect("G.231.22.0004", "usm", "usmjaya001")) {
      Serial.println("Connected!");
      client.subscribe("g231220004/temperature");
      client.subscribe("g231220004/humidity");
      client.subscribe("g231220004/control");
    } else {
      Serial.print("Failed, rc=");
      Serial.println(client.state());
      delay(5000);
    }
  }
}

// FUNGSI UNTUK HANDLING PESAN DARI MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";

  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  Serial.print("Message received [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(message);
  
  if (String(topic) == "g231220004/control") {
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, payload, length);

    if (error) {
      Serial.print("JSON parsing failed: ");
      Serial.println(error.c_str());
      return;
    }

    // Ambil data dari JSON
    JsonObject leds = doc["leds"];
    for (int i = 0; i < 3; i++) {
      String key = "led" + String(i + 1); // Key JSON: led1, led2, led3
      if (leds.containsKey(key)) {
        int state = leds[key]; // Dapatkan status lampu
        digitalWrite(lampu[i], state == 1 ? HIGH : LOW); // Setel pin sesuai status
        Serial.print("Lampu ");
        Serial.print(i + 1);
        Serial.print(" diatur ke ");
        Serial.println(state == 1 ? "ON" : "OFF");
      }
    } 
  }
}

// Fungsi konek wifi
void connectWifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
}

// Fungsi menampilkan data ke LCD
void displayTemp() {
  lcd.clear(); 
  lcd.setCursor(0, 0);
  lcd.print("Temp : " + String(temperature) + " C");
  lcd.setCursor(0, 1);
  lcd.print("Humidity :" + String(humidity) + " %");
}

// fungsi publish DATA SUHU DAN KELEMBAPAN ke MQTT SERVER
void publishTemp() {
  char tempString[8];
  dtostrf(temperature, 1, 2, tempString);
  client.publish("g231220004/temperature", tempString, true);

  char humString[8];
  dtostrf(humidity, 1, 2, humString);
  client.publish("g231220004/humidity", humString, true);
}

void setup() {
  Serial.begin(9600);
  client.setServer(mqtt_server,1883);
  client.setCallback(callback);

  // Set LEDpin SBG OUTPUT
  pinMode(lampu1, OUTPUT);
  pinMode(lampu2, OUTPUT);
  pinMode(lampu3, OUTPUT);
  pinMode(lampu4, OUTPUT);
  pinMode(lampu5, OUTPUT);

  Wire.begin();
  lcd.begin(16, 2);
  lcd.backlight();
  dht.begin();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED)
    connectWifi(); // Reconnect WiFi if disconnected
  if (!client.connected())
    reconnect(); // Reconnect MQTT if disconnected
  client.loop(); // Run MQTT client

  // MELAKUKAN PEMBACAN SENSOR
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  displayTemp();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Get current time
  unsigned long currentMillis = millis();

  // Check if interval has passed
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis; // Update last published time
    publishTemp();                  // Publish data to MQTT
  }

  // KENDALI LAMPU BERDASARKAN SUHU
  if (temperature < 29) {
    digitalWrite(lampu4, LOW);
    digitalWrite(lampu5, LOW);
  } else if(temperature >= 29 && temperature < 30) {
    digitalWrite(lampu4, HIGH);
    digitalWrite(lampu5, LOW);
  } else if(temperature >= 30 && temperature <= 31) {
    digitalWrite(lampu4, LOW);
    digitalWrite(lampu5, HIGH);
  } else {
    digitalWrite(lampu4, HIGH);
    digitalWrite(lampu5, HIGH);
  }
}
