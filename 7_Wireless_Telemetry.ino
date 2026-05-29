void setupWiFi() {
  if (enableWiFi == 1) {
    Serial.print("> Connecting to WiFi: ");
    Serial.println(ssid);
    
    WiFi.begin(ssid, pass);
    
    int timeout = 0;
    while (WiFi.status() != WL_CONNECTED && timeout < 20) {
      delay(500);
      Serial.print(".");
      timeout++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\n> WiFi Connected!");
      Serial.print("> Access Dashboard at: http://");
      Serial.println(WiFi.localIP());

      // =================== CORS PREFLIGHT HANDLER ===================
      // Handles browser OPTIONS preflight requests for cross-origin access
      server.on("/api/*", HTTP_OPTIONS, [](AsyncWebServerRequest *request){
        AsyncWebServerResponse *response = request->beginResponse(204);
        response->addHeader("Access-Control-Allow-Origin", "*");
        response->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        response->addHeader("Access-Control-Allow-Headers", "Content-Type");
        response->addHeader("Access-Control-Max-Age", "86400");
        request->send(response);
      });

      // =================== ROOT PAGE ===================
      // Serves the minimal info page with API endpoint documentation
      server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", index_html);
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
      });

      // =================== TELEMETRY DATA API ===================
      // GET /api/data - Returns all real-time telemetry data as JSON
      server.on("/api/data", HTTP_GET, [](AsyncWebServerRequest *request){
        StaticJsonDocument<1024> doc;
        
        // Power & Energy
        doc["powerInput"]       = powerInput;
        doc["powerOutput"]      = powerOutput;
        doc["voltageInput"]     = voltageInput;
        doc["voltageOutput"]    = voltageOutput;
        doc["currentInput"]     = currentInput;
        doc["currentOutput"]    = currentOutput;
        doc["batteryPercent"]   = batteryPercent;
        doc["Wh"]               = Wh;
        doc["kWh"]              = kWh;
        doc["MWh"]              = MWh;
        doc["energySavings"]    = energySavings;
        doc["peakPower"]        = peakPower;
        
        // System Status
        doc["temperature"]      = temperature;
        doc["fanStatus"]        = fanStatus;
        doc["buckEnable"]       = buckEnable;
        doc["bypassEnable"]     = bypassEnable;
        doc["inputSource"]      = inputSource;
        doc["chargeState"]      = chargeState;
        doc["daysRunning"]      = daysRunning;
        doc["secondsElapsed"]   = secondsElapsed;
        doc["loopTime"]         = loopTime;
        doc["outputDeviation"]  = outputDeviation;
        
        // PWM Data
        doc["PWM"]              = PWM;
        doc["PPWM"]             = PPWM;
        doc["pwmMax"]           = pwmMax;
        doc["pwmMaxLimited"]    = pwmMaxLimited;
        
        // Algorithm & Mode
        doc["MPPT_Mode"]        = MPPT_Mode;
        doc["output_Mode"]      = output_Mode;
        
        // Error Flags
        doc["ERR"]              = ERR;
        doc["FLV"]              = FLV;
        doc["BNC"]              = BNC;
        doc["IUV"]              = IUV;
        doc["IOC"]              = IOC;
        doc["OOV"]              = OOV;
        doc["OOC"]              = OOC;
        doc["OTE"]              = OTE;
        doc["REC"]              = REC;
        
        // Current Configuration (so dashboard can display active settings)
        doc["voltageBatteryMax"]  = voltageBatteryMax;
        doc["voltageBatteryMin"]  = voltageBatteryMin;
        doc["currentCharging"]   = currentCharging;
        doc["enableFan"]         = enableFan;
        doc["enableWiFi"]        = enableWiFi;
        doc["temperatureFan"]    = temperatureFan;
        doc["temperatureMax"]    = temperatureMax;
        doc["backlightSleepMode"]= backlightSleepMode;
        
        String jsonResponse;
        serializeJson(doc, jsonResponse);
        
        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", jsonResponse);
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
      });

      // =================== SETTINGS READ API ===================
      // GET /api/settings - Returns current device configuration
      server.on("/api/settings", HTTP_GET, [](AsyncWebServerRequest *request){
        StaticJsonDocument<512> doc;
        
        doc["MPPT_Mode"]          = MPPT_Mode;
        doc["output_Mode"]        = output_Mode;
        doc["voltageBatteryMax"]  = voltageBatteryMax;
        doc["voltageBatteryMin"]  = voltageBatteryMin;
        doc["currentCharging"]    = currentCharging;
        doc["enableFan"]          = enableFan;
        doc["enableWiFi"]         = enableWiFi;
        doc["temperatureFan"]     = temperatureFan;
        doc["temperatureMax"]     = temperatureMax;
        doc["backlightSleepMode"] = backlightSleepMode;
        doc["enableLCD"]          = enableLCD;
        doc["enableBluetooth"]    = enableBluetooth;
        doc["flashMemLoad"]       = flashMemLoad;
        doc["electricalPrice"]    = electricalPrice;
        
        String jsonResponse;
        serializeJson(doc, jsonResponse);
        
        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", jsonResponse);
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
      });

      // =================== SETTINGS WRITE API ===================
      // POST /api/settings - Updates device configuration and saves to EEPROM
      server.on("/api/settings", HTTP_POST, 
        [](AsyncWebServerRequest *request){},  // No body handler needed for form data
        NULL,                                   // No upload handler
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
          // Parse the incoming JSON body
          StaticJsonDocument<512> doc;
          DeserializationError error = deserializeJson(doc, data, len);
          
          if (error) {
            AsyncWebServerResponse *response = request->beginResponse(400, "application/json", 
              "{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
            response->addHeader("Access-Control-Allow-Origin", "*");
            request->send(response);
            return;
          }
          
          // Update global variables from JSON payload
          if (doc.containsKey("MPPT_Mode"))          MPPT_Mode          = doc["MPPT_Mode"];
          if (doc.containsKey("output_Mode"))         output_Mode        = doc["output_Mode"];
          if (doc.containsKey("voltageBatteryMax"))   voltageBatteryMax  = doc["voltageBatteryMax"];
          if (doc.containsKey("voltageBatteryMin"))   voltageBatteryMin  = doc["voltageBatteryMin"];
          if (doc.containsKey("currentCharging"))     currentCharging    = doc["currentCharging"];
          if (doc.containsKey("enableFan"))           enableFan          = doc["enableFan"];
          if (doc.containsKey("enableWiFi"))          enableWiFi         = doc["enableWiFi"];
          if (doc.containsKey("temperatureFan"))      temperatureFan     = doc["temperatureFan"];
          if (doc.containsKey("temperatureMax"))      temperatureMax     = doc["temperatureMax"];
          if (doc.containsKey("backlightSleepMode"))  backlightSleepMode = doc["backlightSleepMode"];
          
          // Persist to EEPROM
          saveSettings();
          
          Serial.println("> Settings updated from Web Dashboard");
          
          AsyncWebServerResponse *response = request->beginResponse(200, "application/json", 
            "{\"status\":\"ok\",\"message\":\"Settings saved to EEPROM\"}");
          response->addHeader("Access-Control-Allow-Origin", "*");
          request->send(response);
        }
      );

      // =================== FACTORY RESET API ===================
      // POST /api/reset - Restores factory default settings
      server.on("/api/reset", HTTP_POST, [](AsyncWebServerRequest *request){
        factoryReset();
        Serial.println("> Factory Reset triggered from Web Dashboard");
        
        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", 
          "{\"status\":\"ok\",\"message\":\"Factory reset complete\"}");
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
      });

      // Start server
      server.begin();
      Serial.println("> Web API Server Started");
    } else {
      Serial.println("\n> WiFi Connection Failed.");
    }
  }
}

// AsyncWebServer handles everything in the background via interrupts.
// This function runs on Core 0 but no active polling is needed.
void Wireless_Telemetry() {
  delay(10); 
}
