//#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "DHT.h"
#include <Wire.h>              // include Wire library (required for I2C devices)
#include <Adafruit_BMP280.h>   // include Adafruit BMP280 sensor library
#include <BH1750.h>            // include Light Sensor

// define device I2C address: 0x76 or 0x77 (0x77 is library default address)
#define BMP280_I2C_ADDRESS  0x76

#define DHTPIN D1
#define DHTTYPE DHT11 

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "kavindaperera"
#define AIO_KEY         "aio_eYip34tGpKJHRfjtiWFK3aE1aSBz"

//#define FIREBASE_HOST "iot4cast.firebaseio.com"     // the project name address from firebase id
//#define FIREBASE_AUTH "r13LZRtNbHvASwxEW6eAPVFeYtjjA4FZRwIVpGyE"  // the secret key generated from firebase
 
const char* ssid     = "SLT_FIBRE";
const char* password = "manager11";
//const char* host = "iot4cast.000webhostapp.com";

//FirebaseData firebaseData;

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

Adafruit_MQTT_Publish Temperature = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temp");
Adafruit_MQTT_Publish Humidity = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humidity");
Adafruit_MQTT_Publish Pressure = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/pressure");
Adafruit_MQTT_Publish AmbientLight = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/ambient_light");

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


  //Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH); 
  
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
  MQTT_connect();
  
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


  Serial.print(F("\nSending Temperature value "));
  if (! Temperature.publish(t)) { 
    Serial.println(F("Temperature Failed"));
  } else {
    Serial.println("Temperature: "+String(t)+"C");
  }
  
  Serial.print(F("\nSending Humidity val "));
  if (! Humidity.publish(h)) {
    Serial.println(F("Humidity Failed"));
  } else {
    Serial.println("Humidity: "+String(h)+"%");
  }

  Serial.print(F("\nSending Pressure val "));
  if (! Pressure.publish(p)) {
    Serial.println(F("Pressure Failed"));
  } else {
    Serial.println("Pressure: "+String(p)+"Pa");
  }

  Serial.print(F("\nSending Ambient Light val "));
  if (! AmbientLight.publish(l)) {
    Serial.println(F("Ambient Light Failed"));
  } else {
    Serial.println("Ambient Light: "+String(l)+"lx");
  }


  

  /*Serial.print("connecting to ");
  Serial.println(host);

  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  
  String url = "/api/weather/insert.php?temp=" + String(t) + "&humidity=" + String(h) + "&pressure=" + String(p)+ "&light=" + String(l);
  Serial.print("Requesting URL: ");
  Serial.println(url);
 
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  delay(500);
  
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }*/

  

  //Firebase.push(firebaseData, "/data", String(t));

  
  Serial.println();
  Serial.println("closing connection");
  delay(60000);
}


// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 5;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // reset me
        ESP.reset();
       }
  }
  Serial.println("MQTT Connected!");
}
