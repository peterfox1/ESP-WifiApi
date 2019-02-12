
#include <ESP8266WiFi.h>	// https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>

#include <WifiApi.h>

WifiApi wifiApi;



void setup() {
  
  Serial.begin(115200);
  while (!Serial) ; delay(100);	// Wait for serial
  Serial.println(">");  
  // Setup the wifi connection / re-connection. Awaits configuration via json api in access point mode if no existing wifi details are stored.
  wifiApi.autoConnect("ap name");
  
  Serial.print("local ip: ");
  Serial.println(WiFi.localIP());  
  
}



void loop() {
  
  bool resetButtonPressed = false;	// Link this up to a reset button using digitalRead.
  if (resetButtonPressed) {
    wifiApi.reset();	// Clear any saved wifi settings & switch back to AP mode.
    delay(1000);
  }
  
  // <your application code goes here>
  delay(1);
  
}


