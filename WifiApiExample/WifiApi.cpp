
#include "WifiApi.h"



WifiApi::WifiApi() {
}



/* Attempt WiFi connection or starts the AP if no wifi details are stored. */
boolean WifiApi::autoConnect() {
  String ssid = "ESP" + String(ESP.getChipId());
  autoConnect(ssid.c_str(), NULL);
}

/* Attempt WiFi connection or starts the AP if no wifi details are stored. */
boolean WifiApi::autoConnect(char const *apName, char const *apPassword) {

  // Check for wifi config.
  if (!hasWifiConfig()) {
    // No wifi config, start access point immediately.
    DEBUG_WM(F("No stored wifi details, starting AP"));
    return awaitSetupViaAccessPoint(apName, apPassword);
  }
  
  
  // Attempt to connect using existing wifi details.
  DEBUG_WM(F("Attempting connection to existing wifi details."));
  
  WiFi.mode(WIFI_STA);

  if (connectWifi("", "") == WL_CONNECTED)   {
    //connected
    DEBUG_WM(F("IP Address:"));
    DEBUG_WM(WiFi.localIP());
    
    startApi(); // API is avaialbe in non-AP mode too.
    
    return true;
  }

  // Couldn't connect
  // Call connection failure callback. The callback can be customised to Retry the connection or fallback to AP mode etc.
  if (_fnFailedReconnect != NULL) {
    _fnFailedReconnect(this);
  }

  // FUTURE: Add a setting to enable a built-in auto-retry X attempts or reset and AP fallback.
  
  return false;
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
    if (WiFi.SSID() != "") {
      DEBUG_WM("Using last saved values, should be faster");
      //trying to fix connection in progress hanging
      ETS_UART_INTR_DISABLE();
      //wifi_station_disconnect();  // TODO not sure why we get this error: 'wifi_station_disconnect' was not declared in this scope
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
//    if(hasAccessPointExpired()) break;

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
        WiFi.mode(WIFI_STA);  // Staion mode = client only / Turns off AP?
        //startApi(); // API seems to not work in client mode here? // TODO fix this
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

  HTTPMethod currentMethod = server->method();

  if (currentMethod == HTTP_POST) {
    StaticJsonBuffer<200> postBuffer;
    JsonObject& postJson = postBuffer.parseObject(server->arg("plain"));

    if (!postJson.success()) {
      server->send(500, "application/javascript", "{\"error\":\"invalid_json\"}");
      return;
    }

    server->send(200, "application/javascript", "{\"success\":true}");
    
    saveAndApplyConfig(postJson); // Save and apply config and/or wifi details change.
    return;
  }
  
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json_root = jsonBuffer.createObject();

  JsonObject& json_wifi = json_root.createNestedObject("wifi");
  json_wifi["ssid"] = WiFi.SSID();
  json_wifi["pass"] = "***";  // It's not secure to return the SSID

  JsonObject& json_info = json_root.createNestedObject("info");
  json_info["heap"] = ESP.getFreeHeap();
  
  if (hasConfig_app()) {
    JsonObject& appJson = jsonBuffer.parseObject(getAppJson());
    json_root["app"] = appJson;
  } else {
    // No custom data, return empty object
    json_root.createNestedObject("app");
  }


  String output;
  json_root.printTo(output);

  server->send(200, "application/javascript", output);
  
}

/* Read config data from a json object. Saves custom app parameters and applies wifi ssid/pass if provided. */
void WifiApi::saveAndApplyConfig( JsonObject& postJson ) {

  if (postJson.containsKey("app")) {
    JsonObject& appJson = postJson["app"];
    // Save custom app paramters
    saveConfig_app(appJson);
  }

  if (postJson.containsKey("wifi")) {
    // Check wifi details
    JsonObject& wifiJson = postJson["wifi"];

    if (wifiJson.containsKey("ssid") && wifiJson.containsKey("pass")) {
      // Save wifi details
      
      // Apply wifi config
      _ssid = wifiJson.get<String>("ssid");
      _pass = wifiJson.get<String>("pass");

      saveConfig_wifi(wifiJson);
      
      // ssid & password were provided, notify AP mode to attempt connection.
      _ap_wifi_config_provided = true;
    }

  }

}


bool WifiApi::hasConfig_wifi() {
  return (_wifiJson != NULL);
}
bool WifiApi::hasConfig_app() {
  return (_appJson != NULL);
}

const char* WifiApi::getWifiJson() {
//  if (_wifiJson == NULL) {
//    _wifiJson = loadFromJsonFile(WIFIAPI_FILE_WIFI);
//  }

  const char* _jsonConst = _wifiJson;
  return _jsonConst;
}

const char* WifiApi::getAppJson() {
//  if (_appJson == NULL) {
//    _appJson = loadFromJsonFile(WIFIAPI_FILE_APP);
//  }

  const char* _jsonConst = _appJson;
  return _jsonConst;
}


/* Read config data from a json object. Saves custom app parameters and applies wifi ssid/pass if provided. */
void WifiApi::saveConfig_wifi( JsonObject& newWifiJson ) {
  char jsonChar[100];
  newWifiJson.printTo((char*)jsonChar, newWifiJson.measureLength() + 1);
  _wifiJson = jsonChar;

  
//  if (!startFileSystem()) {
//    DEBUG_WM(F("loadFromJsonFile - startFileSystem failed"));
//  }
//  
//  File configFile = SPIFFS.open(WIFIAPI_FILE_WIFI, "w");
//  if (!configFile) {
//    DEBUG_WM(F("saveConfig_wifi - failed to open for write"));
//    return;
//  }
//  wifiJson.printTo(configFile); // Export and save JSON object to SPIFFS area
//  configFile.close();
}
void WifiApi::saveConfig_app( JsonObject& newAppJson ) {

  if (!hasConfig_app()) {
    DEBUG_WM(F("saveConfig_app - new"));
    
    _appJson = new char [newAppJson.measureLength() + 1];
    newAppJson.printTo((char*)_appJson, newAppJson.measureLength() + 1);

    
  } else {
    DEBUG_WM(F("saveConfig_app - extend"));
    
//    StaticJsonBuffer<200> jsonBuffer;
    DynamicJsonBuffer jsonBuffer;
    JsonObject& appJson = jsonBuffer.parseObject(getAppJson());

    // Merge by copying properties from newAppJson to appJson
    for (auto kvp : newAppJson) {
      appJson[kvp.key] = kvp.value;
    }

    _appJson = new char [appJson.measureLength() + 1];
    appJson.printTo((char*)_appJson, appJson.measureLength() + 1);

    DEBUG_WM(_appJson);
    
  }


//  DEBUG_WM(F("saveConfig_app:"));
//  DEBUG_WM(_appJson);

  
//  if (!startFileSystem()) {
//    DEBUG_WM(F("saveConfig_app - startFileSystem failed"));
//  }
//  
//  File configFile = SPIFFS.open(WIFIAPI_FILE_APP, "w");
//  if (!configFile) {
//    DEBUG_WM(F("saveConfig_app - failed to open for write"));
//    return;
//  }
//  appJson.printTo(configFile); // Export and save JSON object to SPIFFS area
//  configFile.close();
}




bool WifiApi::startFileSystem() {
  // FUTURE store a bool to prevent re-starting.
  if (!SPIFFS.begin()) {
    DEBUG_WM(F("SPIFFS.begin failed"));
    return false;
  }
  return true;
}

const char* WifiApi::loadFromJsonFile(const char* filename) {
  
  StaticJsonBuffer<1> emptyJsonBuffer;
  JsonObject& emptyJson = emptyJsonBuffer.createObject();

  if (!startFileSystem()) {
    DEBUG_WM(F("loadFromJsonFile - SPIFFS.begin failed"));
  }
  
  File file = SPIFFS.open(filename, "r");
  if (!file) {
    DEBUG_WM(F("loadFromJsonFile - File not found"));
    return NULL;
    //return emptyJson;
  }
  
  size_t size = file.size();
  if (size > 1024) {
    DEBUG_WM(F("loadFromJsonFile - File to large"));
    return NULL;
    //return emptyJson;
  }
  else if (size == 0)   {
    DEBUG_WM(F("loadFromJsonFile - File is empty"));
    return NULL;
    //return emptyJson;
  }
  
  String fileStr = file.readString();
  return fileStr.c_str();

//  std::unique_ptr<char[]> buf(new char[size]);
//  file.readBytes(buf.get(), size);

//  StaticJsonBuffer<200> jsonBuffer;
//  JsonObject& json = jsonBuffer.parseObject(buf.get());
//  
//  if (!json.success()) {
//    DEBUG_WM(F("loadFromJsonFile - Failed to parse config file"));
//    // TODO delete file?
//    return NULL;
//    //return emptyJson;
//  }
//  
//  file.close();
//
//  return &json;
}




void WifiApi::handleClient() {
  server->handleClient();
}

/* Set a callback to run custom code when WifiApi receieves updated custom data via the JSON API */
void WifiApi::onCustomDataChange( void (*func)(WifiApi* myWifiApi) ) {
  _fnCustomDataChange = func;
}
/* Set a callback to run custom code before WifiApi switches to the updated wifi details recieved via the JSON API */
void WifiApi::onWifiConfigChange( void (*func)(WifiApi* myWifiApi) ) {
  _fnWifiConfigChange = func;
}

/* set a callback function to handle failed re-connect */
void WifiApi::onFailedReconnect( void (*func)(WifiApi* myWifiApi) ) {
  _fnFailedReconnect = func;
}




/* Check if we have wifi details stored */
boolean WifiApi::hasWifiConfig() {
  if (WiFi.SSID() != "") {
    return true;
  }
  // FUTURE: Check json config for stored details too.
  return false;
}


/* Clear all custom config data */
void WifiApi::resetCustomConfig() {
  // TODO
}

/* Clear wifi config */
void WifiApi::resetWifiConfig() {
  DEBUG_WM(F("settings invalidated"));
  DEBUG_WM(F("THIS MAY CAUSE AP NOT TO START UP PROPERLY. YOU NEED TO COMMENT IT OUT AFTER ERASING THE DATA."));
  WiFi.disconnect(true);
  //delay(200);
}

/* Clear all stored data and wifi config */
void WifiApi::reset() {
  resetWifiConfig();
  resetCustomConfig();
}



template <typename Generic>
void WifiApi::DEBUG_WM(Generic text) {
  if (_debug) {
    Serial.print("*WA: ");
    Serial.println(text);
  }
}



