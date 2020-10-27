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

WiFiClient client;

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
  {  
    // connection error!
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
  
  // declare humidity
  float h;
  
  // declare temperature
  float t;
  
  // declare pressure
  float p;
  
  // declare Ambient Light
  float l;
  

  for(int i = 0; i<15; i++) {
    Serial.println(i);
    // Read humidity
    h += dht.readHumidity();
    
    // Read temperature as Celsius (the default)
    t += dht.readTemperature();
    
    // Read pressure in Pascal
    p += bmp280.readPressure();
    
    // Read Ambient Light in lux
    l += lightMeter.readLightLevel();
    
    Serial.println(h);
    Serial.println(t);
    Serial.println(p);
    Serial.println(l);
    delay(5000); 
  }


  Serial.println("calculate average...");
  
  h = h/15;
  t = t/15;
  p = p/15;
  l = l/15;

  Serial.println(h);
  Serial.println(t);
  Serial.println(p);
  Serial.println(l);

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("sending data...");
    sendXML(t, h, p, l);
  } else {
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(">");
    }
  }
}

void sendXML(float temperature, float humidity, float pressure, float light){
  Serial.print("connecting to ");
  Serial.println(host);

  const int httpPort = 80;

  String ptr = String("xml=<?xml version = \"1.0\" encoding = \"UTF-8\"?>\n");
         ptr += String("<alert xmlns=\"urn:oasis:names:tc:emergency:cap:1.2\">\n");
         ptr += String("<parameter>\n<valueName>Temperature</valueName>\n<value>");
         ptr += String(temperature);
         ptr += String("</value>\n</parameter>\n");
         ptr += String("<parameter>\n<valueName>Humidity</valueName>\n<value>");
         ptr += String(humidity);
         ptr += String("</value>\n</parameter>\n");
         ptr += String("<parameter>\n<valueName>Pressure</valueName>\n<value>");
         ptr += String(pressure);
         ptr += String("</value>\n</parameter>\n");
         ptr += String("<parameter>\n<valueName>Light</valueName>\n<value>");
         ptr += String(light);
         ptr += String("</value>\n</parameter>\n");
         ptr += String("</alert>"); 
         
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  
  if (client.connect(host, httpPort)) {
    client.println("POST /iot4cast-api/api/insert_data.php HTTP/1.1");
    client.println("Host: " + String(host));
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(ptr.length());
    client.println();
    client.print(ptr);
  }

  delay(500);
  
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
}
