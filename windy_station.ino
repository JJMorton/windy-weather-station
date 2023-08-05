#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <DHT.h>

#include "secrets.h"


// === CONFIG =============================================

// The GPIO pin connected to the DHT data line
#define DHT_PIN 2

// The type of DHT sensor
#define DHT_TYPE DHT22

// Baud rate of the serial transmission
#define SERIAL_BAUD_RATE 74880

// Time between successive temperature measurements
const unsigned int MEASURE_INTERVAL = 1000 * 60 * 5;
const unsigned int RETRY_INTERVAL = 1000 * 10;

// === END CONFIG =========================================


enum RequestType { GET, POST, PUT };
struct HTTPSResponse
{
  int code;
  String text;
};

// Function to send an HTTPS request of any type
HTTPSResponse httpsRequest(RequestType type, String url, String data = String())
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi not connected.");
    return {-4, String()}; // Not connected error
  }

  HTTPClient http;
  HTTPSResponse res;
  std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
  // Ignore SSL certificate validation
  client->setInsecure();

  if (!http.begin(*client, url.c_str()))
  {
    Serial.println("HTTPS failed to connect");
    res.code = -1; // Connection failed error
    res.text = String();
  }
  else
  {
    // Send HTTP request
    char *stype;
    switch (type)
    {
      case GET:
        res.code = http.GET(); break;
      case POST:
        res.code = http.POST(data); break;
      case PUT:
        res.code = http.PUT(data); break;
    }

    if (res.code > 0)
    {
      Serial.print("HTTPS response code: ");
      Serial.println(res.code);
      res.text = http.getString();
    }
    else
    {
      Serial.print("HTTPS error code: ");
      Serial.print(res.code);
      Serial.print(" ");
      Serial.print(http.errorToString(res.code));
      res.text = String();
    }

    http.end();
  }

  return res;
}


DHT dht(DHT_PIN, DHT_TYPE);
float temperature = 0;
float humidity = 0;


void setup()
{
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.println();

  dht.begin();
  Serial.println("DHT initialised");

  WiFi.begin(SSID, PASS);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
}


void loop()
{
  float newT = dht.readTemperature();
  float newH = dht.readHumidity();
  if (isnan(newT) || isnan(newH))
  {
    Serial.println("Failed to read from DHT sensor!");
  }
  else
  {
    // We've got a new temperature measurement
    temperature = newT;
    humidity = newH;
    Serial.println("New reading: " + String(temperature) + "°C, " + String(humidity) + "%");

    // Request to update station at windy.com
    Serial.println("Sending HTTP request to windy...");
    HTTPSResponse res = httpsRequest(GET, String()
      + "https://stations.windy.com/pws/update/"
      + WINDY_API_KEY
      + "?station=" + String(WINDY_STATION_ID, DEC)
      + "&temp=" + String(temperature, DEC)
      + "&rh=" + String(humidity, DEC)
    );
    if (res.code > 0)
    {
      Serial.println(String("Response: ") + res.text);
      Serial.print(String("Sleeping for ") + String(MEASURE_INTERVAL, DEC) + String("ms..."));
      return delay(MEASURE_INTERVAL);
    }

  }

  Serial.println(String("Will retry in ") + String(RETRY_INTERVAL) + String("ms..."));
  delay(RETRY_INTERVAL);
}
