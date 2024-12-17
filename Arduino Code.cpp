#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include "DHT.h"

// Define the pins for LED and DHT sensor
#define lampu1 12   // D6 = GPIO12
#define lampu2 13   // D7 = GPIO13
#define lampu3 15   // D8 = GPIO15
#define DHTPIN 14   // D5 = GPIO14 (for DHT sensor)
#define DHTTYPE DHT11  // DHT11 sensor type

// Wi-Fi and MQTT configuration
const char* ssid = "Cahyo123";
const char* password = "09102024";
const char* mqtt_server = "x2.revolusi-it.com";
const char* topic_suhu = "iot/suhu/0406";  // Use NIM in topic (0406)
const char* topik = "iot/kendali/0406";    // Use NIM in topic (0406)

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);  // LCD I2C address 0x27, 16 columns, 2 rows

// Callback for MQTT messages
void callback(char* topic, byte* payload, unsigned int length) {
    String message = "";
    for (int i = 0; i < length; i++) {
        message += (char)payload[i];
    }

    Serial.print("Message from MQTT [");
    Serial.print(topic);
    Serial.print("]: ");
    Serial.println(message);

    // Control LEDs based on received messages
    if (String(topic) == topik) {
        if (message == "lampu1_on") {
            digitalWrite(lampu1, HIGH);
            Serial.println("Lampu 1 ON");
        }
        if (message == "lampu1_off") {
            digitalWrite(lampu1, LOW);
            Serial.println("Lampu 1 OFF");
        }
        if (message == "lampu2_on") {
            digitalWrite(lampu2, HIGH);
            Serial.println("Lampu 2 ON");
        }
        if (message == "lampu2_off") {
            digitalWrite(lampu2, LOW);
            Serial.println("Lampu 2 OFF");
        }
        if (message == "lampu3_on") {
            digitalWrite(lampu3, HIGH);
            Serial.println("Lampu 3 ON");
        }
        if (message == "lampu3_off") {
            digitalWrite(lampu3, LOW);
            Serial.println("Lampu 3 OFF");
        }
    }
}

// Connect to MQTT broker
void reconnect() {
    while (!client.connected()) {
        Serial.print("Connecting to MQTT server: ");
        Serial.println(mqtt_server);
        if (client.connect("client-0406", "usm", "usmjaya001")) {
            Serial.println("Connected to MQTT");
            client.subscribe(topik);  // Subscribe to the topic for LED control
        } else {
            Serial.print("Failed, rc=");
            Serial.print(client.state());
            Serial.println(" Try again in 5 seconds...");
            delay(5000);
        }
    }
}

// Connect to Wi-Fi network
void connectWiFi() {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected");
}

void setup() {
    Wire.begin(4, 5);
    Serial.begin(115200);

    // Set up Wi-Fi and MQTT
    connectWiFi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);

    // Set up pin modes for LEDs
    pinMode(lampu1, OUTPUT);
    pinMode(lampu2, OUTPUT);
    pinMode(lampu3, OUTPUT);

    // Initialize LEDs and DHT
    digitalWrite(lampu1, LOW);
    digitalWrite(lampu2, LOW);
    digitalWrite(lampu3, LOW);
    dht.begin();

    // Initialize LCD
    lcd.begin(16, 2);
    lcd.backlight();
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    // Read temperature and humidity
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    // Handle invalid sensor data
    if (isnan(h) || isnan(t)) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Sensor error");
        Serial.println("Sensor error");
        return;
    }

    // Display temperature and humidity on the LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(t);
    lcd.print(" C");
    lcd.setCursor(0, 1);
    lcd.print("Hum: ");
    lcd.print(h);
    lcd.print(" %");

    // Print to Serial Monitor for Temperature
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.print(" C, ");

    // Print to Serial Monitor for Humidity
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.println(" %");

    // Publish temperature and humidity to MQTT
    String payload = "Suhu: " + String(t) + "C, Kelembaban: " + String(h) + "%";
    client.publish(topic_suhu, payload.c_str());
    Serial.println("Published: " + payload);

    // Temperature-based LED and Beep Logic
    if (t > 29 && t < 30) {
        digitalWrite(lampu1, HIGH);  // Turn on lampu1
        digitalWrite(lampu2, LOW);   // Turn off lampu2
        digitalWrite(lampu3, LOW);   // Turn off lampu3
        beep(1);  // Beep once
    } else if (t >= 30 && t <= 31) {
        digitalWrite(lampu1, HIGH);
        digitalWrite(lampu2, HIGH);  // Turn on lampu2
        digitalWrite(lampu3, LOW);
        beep(2);  // Beep twice
    } else if (t > 31) {
        digitalWrite(lampu1, HIGH);
        digitalWrite(lampu2, HIGH);
        digitalWrite(lampu3, HIGH);  // Turn on lampu3
        beep(3);  // Beep three times
    } else {
        // Turn off all LEDs for t < 31
        digitalWrite(lampu1, LOW);
        digitalWrite(lampu2, LOW);
        digitalWrite(lampu3, LOW);
        beep(0);  // No beep
    }

    // Humidity-based LED Control
    if (h >= 30 && h < 60) {
        // Humidity is dry/normal
        Serial.println("Tingkat Kelembaban: Kering/Aman");
        digitalWrite(lampu1, LOW);  // Turn off lampu1
        digitalWrite(lampu2, LOW);  // Turn off lampu2
        digitalWrite(lampu3, LOW);  // Turn off lampu3
        beep(0);  // No beep
    } else if (h >= 60 && h < 70) {
        // Humidity is starting to increase
        Serial.println("Tingkat Kelembaban: Mulai banyak uap air/Normal");
        digitalWrite(lampu1, HIGH);  // Turn on lampu1
        digitalWrite(lampu2, LOW);   // Keep lampu2 off
        digitalWrite(lampu3, LOW);   // Keep lampu3 off
        beep(1);  // Beep once
    } else if (h >= 70) {
        // Humidity is very high
        Serial.println("Tingkat Kelembaban: Terdapat banyak uap air");
        digitalWrite(lampu1, HIGH);  // Turn on lampu1
        digitalWrite(lampu2, HIGH);  // Turn on lampu2
        digitalWrite(lampu3, LOW);   // Keep lampu3 off
        beep(3);  // Beep three times
    }

    delay(2000);  // Wait for 2 seconds before the next loop
}

// Beep function: Simulate beep logic
void beep(int count) {
    for (int i = 0; i < count; i++) {
        Serial.println("Beep " + String(i + 1) + "x");
        delay(300);  // Simulate beep duration
    }
}
