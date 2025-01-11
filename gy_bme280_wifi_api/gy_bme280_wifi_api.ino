/**
Create a file called arduino_secrets.h in the same folder as the sketch and add below
matching your wifi network

//arduino_secrets.h header file
#define SECRET_SSID "MyFIFI"
#define SECRET_PASS "MyWIFIPASSWORD"
**/

// Wifi 
#include <WiFiS3.h>
#include "arduino_secrets.h" 

// All purpose
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>

// HTTP
#include <ArduinoHttpClient.h>

// Sensors
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25) 

Adafruit_BME280 bme; // I2C
//Adafruit_BME280 bme(BME_CS); // hardware SPI
//Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI

unsigned long delayTime;


// please enter your sensitive data in arduino_secrets.h
char ssid[] = SECRET_SSID;    // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;  

// HTTP API RECEIVING ENDPOINT SETUP
char serverAddress[] = "192.168.0.3"; 
int port = 8080;

WiFiClient wifi;
HttpClient client = HttpClient(wifi, serverAddress, port);


// Main setup
void setup() {

  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);

    // Connect and wait...
    status = WiFi.begin(ssid, pass);
    delay(10000);

    // BME init
    Serial.begin(9600);
    Serial.println(F("BME280 test"));

  bool status;
  
  //status = bme.begin();
  // gy-bme has a different addressing.
  status = bme.begin(0x76);  
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
  
  Serial.println("-- Default Test --");
  delayTime = 10000;

  Serial.println();

  Serial.begin(9600);
  while (!Serial) {
  }

  }

  Serial.print("You're connected to the network");
  printCurrentNet();
  printWifiData();

}

void loop() {
  delay(10000);
  Serial.println("LOOP STARTS");
  printCurrentNet();
  printValues();

  // HTTP Post to nodeAPI
  Serial.println("Making POST request");
  String postData = "{\"temperature\":\"" + String(bme.readTemperature(), 1) + "\",\"humidity\":\"" + String(bme.readHumidity(), 1) + "\",\"pressure\":\"" + String(bme.readPressure() / 100, 0) + "\"}";

  client.beginRequest();
  client.post("/api");
  client.sendHeader("Content-Type", "application/json");
  client.sendHeader("Content-Length", postData.length());
  client.sendHeader("X-Custom-Header", "custom-header-value");
  client.beginBody();
  client.print(postData);
  client.endRequest();

  int statusCode = client.responseStatusCode();
  String response = client.responseBody();

  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);

  Serial.println("Wait five seconds");
  delay(5000);

  Serial.println("LOOP ENDS");
}

void printWifiData() {
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  
  Serial.println(ip);

  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  printMacAddress(mac);
}

void printCurrentNet() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  byte bssid[6];
  WiFi.BSSID(bssid);
  Serial.print("BSSID: ");
  printMacAddress(bssid);

  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);

  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type:");
  Serial.println(encryption, HEX);
  Serial.println();
}

void printMacAddress(byte mac[]) {
  for (int i = 0; i < 6; i++) {
    if (i > 0) {
      Serial.print(":");
    }
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
  }
  Serial.println();
}

void printValues() {
  Serial.print("Temperature = ");
  Serial.print(bme.readTemperature());
  Serial.println(" *C");
    
  Serial.print("Pressure = ");
  Serial.print(bme.readPressure() / 100.0F);
  Serial.println(" hPa");
  
  Serial.print("Approx. Altitude = ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");
  
  Serial.print("Humidity = ");
  Serial.print(bme.readHumidity());
  Serial.println(" %");  
  Serial.println();
}
