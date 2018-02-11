
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include "WifiApi.h"

const int PIN_LED_STAT1 = 2;

WifiApi wifiApi;


void setup() {
  
  pinMode(PIN_LED_STAT1, OUTPUT);
  digitalWrite(PIN_LED_STAT1, HIGH); // LED_STAT1 OFF


  
  
  Serial.begin(115200);
  Serial.println();


  
  //reset saved settings
  //wifiApi.resetSettings();
  //wifiApi.onFailedReconnect(<callback>);
  //wifiApi.onConfigChange(<callback>);
  
  wifiApi.autoConnect("ap name");

  
  Serial.println("local ip");
  Serial.println(WiFi.localIP());

}

void loop() {
  wifiApi.handleClient();
  // put your main code here, to run repeatedly:
  digitalWrite(PIN_LED_STAT1, LOW); // LED_STAT1 ON
  delay(100);
  digitalWrite(PIN_LED_STAT1, HIGH); // LED_STAT1 OFF
  delay(100);
}
