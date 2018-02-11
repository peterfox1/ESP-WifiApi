
#include "WifiApi.h"



WifiApi::WifiApi() {
}



/* Attempt WiFi connection, else starts the AP */
boolean WifiApi::autoConnect() {
  String ssid = "ESP" + String(ESP.getChipId());
  autoConnect(ssid.c_str(), NULL);
}

boolean WifiApi::autoConnect(char const *apName, char const *apPassword) {

  // TODO check for config first.
  // TODO if no config -> return startAccessPoint().

  
  // attempt to connect; should it fail, fall back to AP
  WiFi.mode(WIFI_STA);

  if (connectWifi("", "") == WL_CONNECTED)   {
    //connected
    DEBUG_WM(F("IP Address:"));
    DEBUG_WM(WiFi.localIP());
    
    startApi(); // API is avaialbe in non-AP mode too.
    
    return true;
  }

  // Couldn't connect, start AP instead.
  // TODO: We should check for config first, if there's no config we start the AP. Else we try to connect. if connection fails we call the failed connection callback.
  return awaitSetupViaAccessPoint(apName, apPassword);
}


/* Perform WiFi connection */
int WifiApi::connectWifi(String ssid, String pass) {
  DEBUG_WM(F("Connecting as wifi client..."));

//  ### From WiFiManager ###
//  // check if we've got static_ip settings, if we do, use those.
//  if (_sta_static_ip) {
//    DEBUG_WM(F("Custom STA IP/GW/Subnet"));
//    WiFi.config(_sta_static_ip, _sta_static_gw, _sta_static_sn);
//    DEBUG_WM(WiFi.localIP());
//  }
  //fix for auto connect racing issue
  if (WiFi.status() == WL_CONNECTED) {
    DEBUG_WM("Already connected. Bailing out.");
    return WL_CONNECTED;
  }
  //check if we have ssid and pass and force those, if not, try with last saved values
  if (ssid != "") {
    WiFi.begin(ssid.c_str(), pass.c_str());
  } else {
    if (WiFi.SSID()) {
      DEBUG_WM("Using last saved values, should be faster");
      //trying to fix connection in progress hanging
      ETS_UART_INTR_DISABLE();
      //wifi_station_disconnect();  // TODO not sure why this: 'wifi_station_disconnect' was not declared in this scope
      ETS_UART_INTR_ENABLE();

      WiFi.begin();
    } else {
      DEBUG_WM("No saved credentials");
    }
  }

  int connRes = waitForConnectResult();
  DEBUG_WM ("Connection result: ");
  DEBUG_WM ( connRes );
//  ### From WiFiManager ###
//  //not connected, WPS enabled, no pass - first attempt
//  if (_tryWPS && connRes != WL_CONNECTED && pass == "") {
//    startWPS();
//    //should be connected at the end of WPS
//    connRes = waitForConnectResult();
//  }
  return connRes;
}

uint8_t WifiApi::waitForConnectResult() {
  if (_connectTimeout == 0) {
    return WiFi.waitForConnectResult();
  } else {
    DEBUG_WM (F("Waiting for connection result with time out"));
    unsigned long start = millis();
    boolean keepConnecting = true;
    uint8_t status;
    while (keepConnecting) {
      status = WiFi.status();
      if (millis() > start + _connectTimeout) {
        keepConnecting = false;
        DEBUG_WM (F("Connection timed out"));
      }
      if (status == WL_CONNECTED || status == WL_CONNECT_FAILED) {
        keepConnecting = false;
      }
      delay(100);
    }
    return status;
  }
}


/* Starts AccessPoint Mode to allow direct connection */
/* Starts the AP and API, awaits WiFi configuration */
boolean WifiApi::awaitSetupViaAccessPoint(char const *apName, char const *apPassword) {
  //setup AP
  WiFi.mode(WIFI_AP_STA);
  DEBUG_WM("SET AP STA");

  _apName = apName;
  _apPassword = apPassword;

//  //notify we entered AP mode
//  if ( _apcallback != NULL) {
//    _apcallback(this);
//  }

  _ap_wifi_config_provided = false;

  // Start AP
  startAccessPoint();
  startApi();

  // Wait for configuration
  while(1){

//    // check if timeout
//    if(configPortalHasTimeout()) break;

    //DNS
    dnsServer->processNextRequest();
    //HTTP
    server->handleClient();


    if (_ap_wifi_config_provided) {
      _ap_wifi_config_provided = false;
      delay(2000);
      DEBUG_WM(F("Connecting to new AP"));

      // using user-provided  _ssid, _pass in place of system-stored ssid and pass
      if (connectWifi(_ssid, _pass) != WL_CONNECTED) {
        DEBUG_WM(F("Failed to connect."));
      } else {
        //connected
        WiFi.mode(WIFI_STA);
        //startApi(); // API seems to not work in client mode here?
//        //notify that configuration has changed and any optional parameters should be saved
//        if ( _savecallback != NULL) {
//          //todo: check if any custom parameters actually exist, and check if they really changed maybe
//          _savecallback();
//        }
        break;
      }

//      if (_shouldBreakAfterConfig) {
//        //flag set to exit after config after trying to connect
//        //notify that configuration has changed and any optional parameters should be saved
//        if ( _savecallback != NULL) {
//          //todo: check if any custom parameters actually exist, and check if they really changed maybe
//          _savecallback();
//        }
//        break;
//      }
    }
    yield();
  }

  server.reset();
  dnsServer.reset();

  return (WiFi.status() == WL_CONNECTED);
}


void WifiApi::startAccessPoint() {

  dnsServer.reset(new DNSServer());

  DEBUG_WM(F("Configuring access point... "));
  DEBUG_WM(_apName);
  if (_apPassword != NULL) {
    if (strlen(_apPassword) < 8 || strlen(_apPassword) > 63) {
      // fail passphrase to short or long!
      DEBUG_WM(F("Invalid AccessPoint password. Ignoring"));
      _apPassword = NULL;
    }
    DEBUG_WM(_apPassword);
  }

//  //optional soft ip config
//  if (_ap_static_ip) {
//    DEBUG_WM(F("Custom AP IP/GW/Subnet"));
//    WiFi.softAPConfig(_ap_static_ip, _ap_static_gw, _ap_static_sn);
//  }

  if (_apPassword != NULL) {
    WiFi.softAP(_apName, _apPassword);//password option
  } else {
    WiFi.softAP(_apName);
  }

  delay(500); // Without delay I've seen the IP address blank
  DEBUG_WM(F("AP IP address: "));
  DEBUG_WM(WiFi.softAPIP());

  /* Setup the DNS server redirecting all the domains to the apIP */
  dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer->start(DNS_PORT, "*", WiFi.softAPIP());
  
}




/* Configures and starts the web server for the API */
void WifiApi::startApi() {
  server.reset(new ESP8266WebServer(80));

  /* Setup web pages: root, wifi config pages, SO captive portal detectors and not found. */
  server->on("/config", std::bind(&WifiApi::apiHandleConfig, this));
  server->begin(); // Web server start
  DEBUG_WM(F("HTTP server started"));
}

/* JSON api handler
 *  - Accepts JSON via 'set' parameter
 *  - Responds with the current/updated json config.
 */
void WifiApi::apiHandleConfig() {

  // TODO read current json config
  String cfg_json_string = "{\"ssid\":\"test\"}";
  
  // TODO read any provided json config
  String get_cfg_json_string = server->arg("set");
  if (get_cfg_json_string != "") {
    // TODO Set/save config
    cfg_json_string = get_cfg_json_string;

    saveConfig(); // TODO supply json_string to this function
  }

  // TODO respond with current/updated json config
  
//  String message = "world-";
//  message += server.arg("text");
//  
//  String response = "{\"hello\":\""+ message +"\"}";
  
  server->send(200, "application/javascript", cfg_json_string);
}

void WifiApi::handleClient() {
  server->handleClient();
}


void WifiApi::saveConfig() {

  // TODO read details from config json
  _ssid = "xx"; // Set these for testing
  _pass = "xx";

  // TODO if ssid & password are provided, notify AP mode to attempt connection.
  _ap_wifi_config_provided = true;
  
}




/* set a callback function to handle failed re-connect */
void WifiApi::onFailedReconnect( void (*func)(void) ) {
  _fnFailedReconnect = func;
}

/* set a callback function to handle an updated config */
void WifiApi::onConfigChange( void (*func)(void) ) {
  _fnConfigChange = func;
}


/* DEBUG function to force reset stored wifi settings */
void WifiApi::resetSettings() {
  DEBUG_WM(F("settings invalidated"));
  DEBUG_WM(F("THIS MAY CAUSE AP NOT TO START UP PROPERLY. YOU NEED TO COMMENT IT OUT AFTER ERASING THE DATA."));
  WiFi.disconnect(true);
  //delay(200);
}




template <typename Generic>
void WifiApi::DEBUG_WM(Generic text) {
  if (_debug) {
    Serial.print("*WM: ");
    Serial.println(text);
  }
}



