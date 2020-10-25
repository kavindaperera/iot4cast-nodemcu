#include <ESP8266WiFi.h>
#include "DHT.h"
#include <Wire.h>              // include Wire library (required for I2C devices)
#include <Adafruit_BMP280.h>   // include Adafruit BMP280 sensor library
#include <BH1750.h>            // include Light Sensor

// define device I2C address: 0x76 or 0x77 (0x77 is library default address)
#define BMP280_I2C_ADDRESS  0x76

#define DHTPIN D1
#define DHTTYPE DHT11 

const char* ssid     = "SLT_FIBRE";
const char* password = "manager11";
const char* host = "192.168.1.4";

// initialize sensor libraries
Adafruit_BMP280  bmp280;
DHT dht(DHTPIN, DHTTYPE);
BH1750 lightMeter;

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

  if (!lightMeter.begin())
  {
    //connection error!
    Serial.println("BH1750 Error");
    while(1);//stay here
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
  
  // Read humidity
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read pressure in Pascal
  float p = bmp280.readPressure();
  // Read Ambient Light in lux
  float l = lightMeter.readLightLevel();
  
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
  
  String url = "/iot4cast-api/api/insert.php?temp=" + String(t) + "&humidity=" + String(h) + "&pressure=" + String(p)+ "&light=" + String(l);
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
  Serial.println("I'm awake, but I'm going into deep sleep mode for 60 seconds");
  delay(60000);
  //ESP.deepSleep(60e6);
}
