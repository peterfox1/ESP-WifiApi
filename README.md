# ESP8266 WiFi Config via JSON API
Allows the ES8266 to be setup via JSON API, Creates an Access Point if no wifi config is already set.

This library is intended to allow you to make a custom app to connect ESP8266s to your wifi network. The app just has to send JSON to the ESP that contains the wifi credentials. The API can also be used to send custom data from the app.

Designed to be very easy to use and simple to customise if desired.

Usage:
 - Include this library and the ESP8266WiFi libraries. Initialise globally with `WifiApi wifiApi;`
 - Put `wifiApi.autoConnect("My AP Name");` in `setup()`
 - Optional: Put `wifiApi.handleClient();` in `loop()` (enables the json api outside of AP mode for re-configuration & custom data)
 - Optional: Setup callback functions to perform custom actions when wifi events or data updates happen.
 
 
## Example functionality enabled by this library.
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
 - [ ] Custom data can be provided to the API for use by your own application.
 - [x] A callback can be used to fine tune how to handle failure to connect (ie you could choose to wait, start up the AP or any other action)*
 - [ ] A callback is triggered when custom data is set/updated
 - [ ] A callback is triggered when the wifi details are set/updated.
 - [ ] Built-in option to auto-retry connection with retry limit and associated event callbacks.
 - [ ] Built-in option to auto reset and fallback to AP with associated event callback.
 
 *Currently WIP functionality, may use hard coded values.
 
