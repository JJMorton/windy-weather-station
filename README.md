# [Windy](https://windy.com) weather station

A homemade weather station project using the *ESP-01* WiFi microchip and the *DHT22* temperature/humidity sensor.
Measurements taken using the sensor are uploaded to a weather station created on [windy](https://windy.com).

Programmed using the [ESP8266 Arduino core](https://github.com/esp8266/Arduino).
You will need the DHT sensor library from [Adafruit](https://learn.adafruit.com/dht/using-a-dhtxx-sensor).

**NOTE**: You must create the file `secrets.h` which looks like the following:
```c
// WiFi connection
const char *SSID = "your_ssid";
const char *PASS = "your_password";

// Windy station
static const char *WINDY_API_KEY = "your.api.key";
static const int WINDY_STATION_ID = 0;
```
