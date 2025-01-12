#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <DHT.h>
#include <DHT_U.h>
#include <MQ135.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ThingSpeak.h>

// Sensor Pins
#define DHTPIN D3    // GPIO0
#define DHTTYPE DHT11
#define MQ135_PIN A0

// Create sensor objects
Adafruit_BMP280 bmp;
DHT dht(DHTPIN, DHTTYPE);
MQ135 mq135(MQ135_PIN);

// WiFi credentials
const char* ssid = "MotorolaArnob";       // Replace with your WiFi SSID
const char* password = "19421942";   // Replace with your WiFi Password

// ThingSpeak settings
const char* server = "api.thingspeak.com";
unsigned long myChannelNumber = 2805107;  // Replace with your ThingSpeak Channel ID
const char* myWriteAPIKey = "ZKIIKLXHHF2JG14A";       // Replace with your Write API Key

WiFiClient client;

void setup() {
  Serial.begin(115200);
  delay(100);

  // Initialize WiFi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");

  // Initialize ThingSpeak
  ThingSpeak.begin(client);

  // Initialize sensors
  if (!bmp.begin(0x76)) {
    Serial.println("BMP280 initialization failed!");
    while (1);
  }
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL, 
                  Adafruit_BMP280::SAMPLING_X2, 
                  Adafruit_BMP280::SAMPLING_X16, 
                  Adafruit_BMP280::FILTER_X16, 
                  Adafruit_BMP280::STANDBY_MS_500);

  dht.begin();
  Serial.println("Sensors initialized!");
}

void loop() {
  // Read BMP280 values
  float pressure = bmp.readPressure() / 100.0F;  // Convert to hPa
  float temperatureBMP = bmp.readTemperature();

  // Read DHT11 values
  float temperatureDHT = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Read MQ135 value
  int airQuality = mq135.getPPM();

  Serial.println("==== Weather Station Data ====");

  // Check BMP280 readings
  if (!isnan(temperatureBMP) && !isnan(pressure)) {
    Serial.print("BMP280 Temperature: ");
    Serial.print(temperatureBMP);
    Serial.println(" °C");
    Serial.print("Pressure: ");
    Serial.print(pressure);
    Serial.println(" hPa");
  } else {
    Serial.println("Error reading BMP280 data!");
  }

  // Check DHT11 readings
  if (!isnan(temperatureDHT) && !isnan(humidity)) {
    Serial.print("DHT11 Temperature: ");
    Serial.print(temperatureDHT);
    Serial.println(" °C");
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
  } else {
    Serial.println("Error reading DHT11 data!");
  }

  // Check MQ135 reading
  Serial.print("Air Quality (PPM): ");
  Serial.println(airQuality);

  // Send data to ThingSpeak
  ThingSpeak.setField(1, temperatureDHT);
  ThingSpeak.setField(2, humidity);
  ThingSpeak.setField(3, pressure);
  ThingSpeak.setField(4, airQuality);
  int statusCode = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

  if (statusCode == 200) {
    Serial.println("Data sent to ThingSpeak successfully!");
  } else {
    Serial.print("Error sending data to ThingSpeak: ");
    Serial.println(statusCode);
  }

  // Wait 15 seconds (ThingSpeak API limit: 15s update rate)
  delay(1000);
}
