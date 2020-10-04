#include <ESP8266WiFi.h>
#include "DHT.h"
#include <Wire.h>              // include Wire library (required for I2C devices)
#include <Adafruit_BMP280.h>   // include Adafruit BMP280 sensor library

// define device I2C address: 0x76 or 0x77 (0x77 is library default address)
#define BMP280_I2C_ADDRESS  0x76

// initialize Adafruit BMP280 library
Adafruit_BMP280  bmp280;

#define DHTPIN D1
#define DHTTYPE DHT11 
 
const char* ssid     = "SLT_FIBRE";
const char* password = "manager11";
const char* host = "iot4cast.000webhostapp.com";
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  delay(100);
  dht.begin();
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password); 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Wire.begin(D6, D5);  // set I2C pins [SDA = D6, SCL = D5], default clock is 100kHz
  if ( !bmp280.begin(BMP280_I2C_ADDRESS) )
  {  // connection error!
    Serial.println("BMP280 Error");
    while(1);// stay here
  }
 
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Netmask: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("Gateway: ");
  Serial.println(WiFi.gatewayIP());
}
void loop() {
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  float p = bmp280.readPressure();
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("connecting to ");
  Serial.println(host);

  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  
  String url = "/api/weather/insert.php?temp=" + String(t) + "&humidity=" + String(h) + "&pressure=" + String(p);
  Serial.print("Requesting URL: ");
  Serial.println(url);
  
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  delay(500);
  
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  
  Serial.println();
  Serial.println("closing connection");
  delay(60000);
}
