# ESP8266 WiFi connection manager and data store via JSON API

This library is intended to make it simple to create your own mobile application that can setup and configure your ESP8266.
 - Hosts a simple json api to set wifi and custom properties. (e.g. mqtt details, or led on/off).
 - Auto-reconnects to saved wifi details, otherwise enables AP mode.
 - `reset()` method to clear the saved details and re-enable AP mode.
 - Triggers callback functions when data or connection state changes.

## Basic ESP application:
```C++
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>

#include <WifiApi.h>

WifiApi wifiApi;

const int PIN_RESET_BUTTON = 3;

void setup() {
  wifiApi.autoConnect("Device SSID");
}

void loop() {
  if (digitalRead(PIN_RESET_BUTTON)) {
    wifiApi.reset();	// Clear any saved wifi settings & switch back to AP mode.
    delay(10000);
  }
  
  // <your application code goes here>
  delay(1);
}
```

## API:

*Notice: after configuring wifi details the IP address will change. Your application will need to handle this*

### Set wifi details and/or app specific data

#### `POST 192.168.4.1/config`
You can set the wifi details and app data at the same time:
```json
{
	"wifi": { "ssid":"my wifi", "pass":"12345" },
	"app": { "led": false, "myName": "You" }
}
```

#### `POST 192.168.4.1/config`
App properties will always be merged, so in this case the `myName` property would still be stored.
```json
{
	"app": { "led": true }
}
```


### Fetch the stored data

####  `GET 192.168.4.1/config`
```json
{
	"wifi": { "ssid":"my wifi", "pass":"***" },
	"app": { "led": true, "myName": "You" }
}
```
`pass` is always returned as "***" :)



## Usage Notes:
 - Include this library and the ESP8266WiFi libraries. Initialise globally with `WifiApi wifiApi;`
 - Put `wifiApi.autoConnect("My AP Name");` in `setup()`
 - Optional: Put `wifiApi.handleClient();` in `loop()` (required to be able to use the json api outside of AP mode for re-configuration & custom data)
 - Optional: Setup callback functions to perform custom actions when wifi events or data updates happen.


## Example use case:
Using `wifiApi.autoConnect("MyAP");` in `setup()`:

First power-on (or `reset` is triggered):
 1. No wifi details stored, so an AP called "MyAP" is created.
 1. Connect a phone to the AP, your custom app can make a GET request to `192.168.4.1/config` to confirm the connection to the ESP.
 1. Your app makes a POST request to `192.168.4.1/config` to set the SSID/Pass of a wifi network. e.g. `{"ssid":"my wifi", "pass":"mypassword"}`.
 1. The ESP restarts and connects to the wifi network provided and resumes running the sketch.
 1. If  `wifiApi.handleClient();` is in `loop()`, the API is still present at the ESP's new IP address, so new wifi details or custom data can be provided.
 
If the ESP is powered up again, it will automatically connect to the WiFi network previously configured.


## Features / TODO List:
 - [x] Auto connects to saved wifi details.
 - [x] Creates an AP with a custom name if no wifi details are saved*.
 - [x] Provides a simple JSON API for configuring and updating WiFi credentials*.
 - [x] The API is available both when the ESP is connected in AP mode or as a client on the network.
 - [x] Custom data can be provided to the API for use by your own application.
 - [x] A callback is triggered when custom data is set/updated
 - [ ] A callback is triggered when the wifi details are set/updated.
 - [ ] A callback is triggered on connection failure (ie you can choose to wait, start up the AP or perform any other action)
 - [ ] Built-in auto retry & reset options e.g. retry limit / fallback to AP and associated event callbacks.
