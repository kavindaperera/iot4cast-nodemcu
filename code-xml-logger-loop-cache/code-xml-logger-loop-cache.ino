#include <ESP8266WiFi.h>
#include "DHT.h"
#include <Wire.h>              // include Wire library (required for I2C devices)
#include <Adafruit_BMP280.h>   // include Adafruit BMP280 sensor library
#include <BH1750.h>            // include Light Sensor

// define device I2C address: 0x76 or 0x77 (0x77 is library default address)
#define BMP280_I2C_ADDRESS  0x76

#define DHTPIN D1
#define DHTTYPE DHT11 

const char* ssid     = "Stormbreaker";
const char* password = "pin12345";
const char* host = "936816ca28f9.ngrok.io";

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
  
//  while (WiFi.status() != WL_CONNECTED) {
//    delay(500);
//    Serial.print(".");
//  }

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

  
    float h=0;
    float t=0;
    float p=0;
    float l=0;
    
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
      Serial.println("I'm awake but sleeping for 60 seconds...");
      delay(60000); 
    }

    if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
    }
  
    Serial.println("calculating average...");
    
    t = t/15;
    h = h/15;
    p = p/15;
    l = l/15;
  
    

  if (WiFi.status() == WL_CONNECTED) { 
    
    Serial.println("sending data...");  
    sendXML(t, h, p, l);   
    
  } else {
    float Matrix[100][4];
    int counter = 0;
    Serial.println("No Wifi....Starting Caching...");
    while (WiFi.status() != WL_CONNECTED) {
      
      Serial.print("No Wifi....Caching: ");
      Serial.println(counter);
      
      float h_cache=0;
      float t_cache=0;
      float p_cache=0;
      float l_cache=0;
      for(int i = 0; i<15; i++) {
        Serial.println(i);
        // Read humidity
        h_cache += dht.readHumidity();
        
        // Read temperature as Celsius (the default)
        t_cache += dht.readTemperature();
        
        // Read pressure in Pascal
        p_cache += bmp280.readPressure();
        
        // Read Ambient Light in lux
        l_cache += lightMeter.readLightLevel();
       
        Serial.println("I'm awake but sleeping for 60 seconds...");
        delay(60000); 
      }
      Serial.println("calculating average...");
    
      t_cache = t_cache/15;
      h_cache = h_cache/15;
      p_cache = p_cache/15;
      l_cache = l_cache/15;

      Matrix[counter][0] = t_cache;
      Matrix[counter][1] = h_cache;
      Matrix[counter][2] = p_cache;
      Matrix[counter][3] = l_cache; 
        
      Serial.println(">");
      counter++;
    }
    
    Serial.println("Got back Connection.....");
    
    if(h!=0){
      Serial.println("sending first data...");  
      sendXML(t, h, p, l);
    }
    
    for ( int i = 0; i < counter; ++i ) {

      Serial.println("sending cached data...");  
      Serial.println(i);
      Serial.println("sending data...");
      sendXML(Matrix[i][0], Matrix[i][1], Matrix[i][2], Matrix[i][3]);
      delay(500);
      
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
