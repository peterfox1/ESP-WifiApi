
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include "WifiApi.h"

const int PIN_LED_STATUS = 2;

WifiApi wifiApi;


void setup() {
  
  pinMode(PIN_LED_STATUS, OUTPUT);
  digitalWrite(PIN_LED_STATUS, LOW); // LED OFF

  Serial.begin(115200);
  Serial.println();


  // Clear any saved data or wifi settings, use it here for testing purposes. This could be used in a 'factory reset' type of function elsewhere in your project too.
  //wifiApi.reset();

  // Use these callbacks to customise how updates/events are handled.
  //wifiApi.onCustomDataChange(<callback>); // Triggered on updates/changes to custom data values received via the JSON API.
  //wifiApi.onWifiConfigChange(<callback>); // Triggered just before wifiApi switches to some updated wifi details received via the JSON API.
  //wifiApi.onFailedReconnect(<callback>);  // Define how you want a connection failure to be handled. e.g. Retry, Reset and retry (enables AP mode) or perform a custom action e.g. flash an led etc.
  
  wifiApi.autoConnect("ap name"); // Handles the wifi connection / re-connection. Awaits configuration via AP mode if no existing wifi details are stored.

  
  Serial.println("local ip");
  Serial.println(WiFi.localIP());

}

void loop() {
  wifiApi.handleClient(); // Put this in the main loop to allow the JSON API to be used when connected to the wifi network (optional)
  
  // put your main code here, to run repeatedly:
  
  digitalWrite(PIN_LED_STATUS, HIGH); // LED ON
  delay(100);
  digitalWrite(PIN_LED_STATUS, LOW); // LED OFF
  delay(100);
  
}
