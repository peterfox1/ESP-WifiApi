
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include "WifiApi.h"

const int PIN_LED_STATUS = 2;

WifiApi wifiApi;



void wifiApi_onAppDataChange(WifiApi* _wifiApi, JsonObject& appJson) {
  Serial.print("wifiApi_onAppDataChange: ");
  appJson.printTo(Serial);

  JsonVariant led = appJson["led"];
  if (led.success()) {
    digitalWrite(PIN_LED_STATUS, !led.as<bool>());
  }
}
void wifiApi_onWifiConfigChange(WifiApi* _wifiApi, JsonObject& wifiJson) {
  Serial.print("wifiApi_onWifiConfigChange: ");
  wifiJson.printTo(Serial);
}
void wifiApi_onFailedReconnect(WifiApi* _wifiApi) { // Currently unused.
  Serial.println("Failed Reconnect Callback");
//  _wifiApi.retryConnect();
}

void setup() {
  
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, LOW); // LED OFF

  Serial.begin(115200);
  while (!Serial) ; delay(100); // Wait for serial
  Serial.println(">");


  // Clear any saved data or wifi settings, use it here for testing purposes. This could be used in a 'factory reset' type of function elsewhere in your project too.
  //wifiApi.reset();

  // OPTIONAL: Use these callbacks to customise how updates/events are handled.
  wifiApi.onAppDataChange(wifiApi_onAppDataChange); // Triggered on updates/changes to app data values received via the JSON API.
  wifiApi.onWifiConfigChange(wifiApi_onWifiConfigChange); // Triggered just before wifiApi switches to some updated wifi details received via the JSON API.
  wifiApi.onFailedReconnect(wifiApi_onFailedReconnect);   // (Currently unused) Define how you want a connection failure to be handled. e.g. Retry, Reset and retry (enables AP mode) or perform a custom action e.g. flash an led etc.

  // The only line required:
  wifiApi.autoConnect("ap name"); // Handles the wifi connection / re-connection. Awaits configuration via AP mode if no existing wifi details are stored.
  
  Serial.print("local ip: ");
  Serial.println(WiFi.localIP());

}

void loop() {
  wifiApi.handleClient(); // Put this in the main loop to allow the JSON API to be used when connected to the wifi network (optional)
  
  // put your main code here, to run repeatedly:
  delay(1);
}


