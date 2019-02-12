
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>

#include <WifiApi.h>

const int PIN_LED_STATUS = 2;

WifiApi wifiApi;



/**
 * WifiApi Event: The app (application-specific) data was updated
 * 
 * Use this to perform an action as soon as the app data changes.
 * 
 * This example checks for an 'led' property and uses it to turn on/off the LED.
 */
void wifiApi_onAppDataChange(WifiApi* _wifiApi, JsonObject& appJson) {
  Serial.print("wifiApi_onAppDataChange: ");
  appJson.printTo(Serial); Serial.println();

  // Look for an 'led' property and use it's value to turn on or off the led.
  JsonVariant led = appJson["led"];
  if (led.success()) {
    digitalWrite(PIN_LED_STATUS, !led.as<bool>());
  }
}

/**
 * WifiApi Event: The wifi configuration was set
 * 
 * Use this if you need to trigger a reset or reconnection when the wifi connection is changed.
 */
void wifiApi_onWifiConfigChange(WifiApi* _wifiApi, JsonObject& wifiJson) {
  Serial.print("wifiApi_onWifiConfigChange: ");
  wifiJson.printTo(Serial); Serial.println();
}

/**
 * ***WIP - Currently not working***
 * 
 * WifiApi Event: Could not connect to the stored wifi details
 * 
 * This can be used to define how you want handle reconnection and update a connection status LED etc.
 * 
 * A typical option is to just call retryConnect which will endless retry for connection.
 * If you do this, you should also have a way to reset the wifi manually, e.g. have a button you press
 * and hold that triggers wifiApi.reset().
 * 
 * If your device has no phyical buttons then you could instead use this to switch back to AP mode.
 * It would be best to make this retry the connection a few times with delay just in case the wifi
 * connection is only temporarly disconnected.
 */
void wifiApi_onFailedReconnect(WifiApi* _wifiApi) {
  Serial.println("Failed Reconnect Callback");

  // If we cannot connect using stored details 
//  _wifiApi.retryConnect();
}



void setup() {
  
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, LOW); // LED OFF

  Serial.begin(115200);
  while (!Serial) ; delay(100); // Wait for serial
  Serial.println(">");
  
  // OPTIONAL: Use these callbacks to customise how updates/events are handled.
  wifiApi.onAppDataChange(wifiApi_onAppDataChange); // Triggered on updates/changes to app data values received via the JSON API.
  wifiApi.onWifiConfigChange(wifiApi_onWifiConfigChange); // Triggered just before wifiApi switches to some updated wifi details received via the JSON API.
  wifiApi.onFailedReconnect(wifiApi_onFailedReconnect);   // (WIP - Currently not working) Define how you want a connection failure to be handled. e.g. Retry, Reset and retry (enables AP mode) or perform a custom action e.g. flash an led etc.

  // REQUIRED: This is the only line you need to use this library!
  wifiApi.autoConnect("ap name"); // Handles the wifi connection / re-connection. Awaits configuration via AP mode if no existing wifi details are stored.
  
  Serial.print("local ip: ");
  Serial.println(WiFi.localIP());

}



void loop() {
  
  wifiApi.handleClient();	// Leave this here if you want the JSON API to remain active after the initial wifi connection setup (optional)
  
  bool resetButtonPressed = false;	// Link this up to a reset button using digitalRead.
  if (resetButtonPressed) {
    wifiApi.reset();	// Clear any saved wifi settings & switch back to AP mode.
    // ...If you don't have a dedicated button available for this, you could trigger this only if a button is pressed during power up by moving this code into setup()
    // ...Alternatively, you could use onFailedReconnect to automatically switch the device back to AP mode if the wifi connection fails.
    delay(1000);
  }
  
  // <your application code goes here>
  delay(1);
  
}


