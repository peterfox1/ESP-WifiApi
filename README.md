# ESP8266 WiFi Config via JSON API
Allows the ES8266 to be setup via JSON API, Creates an Access Point if no wifi config is already set.

This allows you to make a custom App to connect ESP8266s to your wifi network.

Designed to be very easy to use in an ardunio ESP8266 project.
Simply requires `wifiApi.autoConnect("MyAP");` in `setup()` and `wifiApi.handleClient();` in `loop()` (optional)

## Features / TODO List:
 - [x] Auto connects to saved wifi details.
 - [x] Creates an AP with a custom name if no wifi details are saved*.
 - [x] Provides a simple JSON API for configuring and updating WiFi credentials*.
 - [x] The API is available both when the ESP is connected in AP mode or as a client on the network.
 - [ ] Custom parameters can be provided to the API for use by your own application.
 - [ ] A callback can be used to fine tune how to handle failure to connect (ie you could choose to wait, start up the AP or any other action)
 - [ ] A callback is triggered when custom parameters are updated.
 
 *Currently WIP functionality, may use hard coded values.
 
## Example Flow
Functionality enabled by simply calling `wifiApi.autoConnect("MyAP");` on setup:
 1. ESP is powered up, no wifi details are present so an AP called "MyAP" is created.
 1. After connecting to "MyAP", your app can connect to the config API via `192.168.4.1/config`.
 1. Your app can provide wifi details using `192.168.4.1/config?set={"ssid":"my wifi", "pass":"mypassword"}`.
 1. The ESP restarts and connects to `my wifi` using the provided details.
 1. The API is still present at the ESP's new IP address, so new details can be provided or custom parameters can be provided.
 
 If the ESP is powered up again, it will automatically connect to the WiFi network previously configured.
