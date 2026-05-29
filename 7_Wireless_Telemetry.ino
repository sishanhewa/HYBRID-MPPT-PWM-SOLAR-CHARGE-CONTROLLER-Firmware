unsigned long lastTelemetrySync = 0;
unsigned long lastSettingsSync = 0;
const unsigned long TELEMETRY_INTERVAL = 5000;  // 5 seconds
const unsigned long SETTINGS_INTERVAL = 10000;  // 10 seconds

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
      Serial.print("> Local IP: ");
      Serial.println(WiFi.localIP());
      Serial.println("> Supabase Cloud Sync Enabled");
    } else {
      Serial.println("\n> WiFi Connection Failed.");
    }
  }
}

// Runs on Core 0
void Wireless_Telemetry() {
  if (WiFi.status() == WL_CONNECTED && enableWiFi == 1) {
    unsigned long currentMillis = millis();

    // 1. PUSH TELEMETRY (Every 5 seconds)
    if (currentMillis - lastTelemetrySync >= TELEMETRY_INTERVAL) {
      lastTelemetrySync = currentMillis;
      pushTelemetryToSupabase();
    }

    // 2. PULL SETTINGS (Every 10 seconds)
    if (currentMillis - lastSettingsSync >= SETTINGS_INTERVAL) {
      lastSettingsSync = currentMillis;
      pullSettingsFromSupabase();
    }
  }
  delay(10); // Yield to Watchdog
}

void pushTelemetryToSupabase() {
  if (supabaseUrl == "https://your-project-id.supabase.co") return; // Not configured

  HTTPClient http;
  String url = supabaseUrl + "/rest/v1/telemetry_data?id=eq.1";
  http.begin(url);
  http.addHeader("apikey", supabaseKey);
  http.addHeader("Authorization", "Bearer " + supabaseKey);
  http.addHeader("Content-Type", "application/json");

  // Create JSON payload
  StaticJsonDocument<1024> doc;
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
  
  doc["PWM"]              = PWM;
  doc["PPWM"]             = PPWM;
  doc["pwmMax"]           = pwmMax;
  doc["pwmMaxLimited"]    = pwmMaxLimited;
  
  doc["MPPT_Mode"]        = MPPT_Mode;
  doc["output_Mode"]      = output_Mode;
  
  doc["ERR"]              = ERR;
  doc["FLV"]              = FLV;
  doc["BNC"]              = BNC;
  doc["IUV"]              = IUV;
  doc["IOC"]              = IOC;
  doc["OOV"]              = OOV;
  doc["OOC"]              = OOC;
  doc["OTE"]              = OTE;
  doc["REC"]              = REC;

  String requestBody;
  serializeJson(doc, requestBody);

  int httpResponseCode = http.PATCH(requestBody);
  
  if (httpResponseCode > 0) {
    if (httpResponseCode != 200 && httpResponseCode != 204) {
      Serial.print("> Supabase Telemetry Push Error: ");
      Serial.println(httpResponseCode);
    }
  } else {
    Serial.print("> Supabase Connection Error: ");
    Serial.println(http.errorToString(httpResponseCode).c_str());
  }
  
  http.end();
}

void pullSettingsFromSupabase() {
  if (supabaseUrl == "https://your-project-id.supabase.co") return; // Not configured

  HTTPClient http;
  String url = supabaseUrl + "/rest/v1/device_settings?id=eq.1&select=*";
  http.begin(url);
  http.addHeader("apikey", supabaseKey);
  http.addHeader("Authorization", "Bearer " + supabaseKey);

  int httpResponseCode = http.GET();
  
  if (httpResponseCode == 200) {
    String payload = http.getString();
    
    // Supabase returns an array for select queries: [{...}]
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, payload);
    
    if (!error && doc.size() > 0) {
      JsonObject settings = doc[0];
      
      bool settingsChanged = false;
      
      // Factory Reset Check
      if (settings["factoryResetTrigger"] == 1) {
        Serial.println("> Factory Reset Triggered from Cloud!");
        // Acknowledge the reset by clearing the flag in DB first
        clearFactoryResetFlag();
        factoryReset();
        return;
      }
      
      // Update variables if they exist in JSON
      if (settings.containsKey("MPPT_Mode") && MPPT_Mode != settings["MPPT_Mode"].as<bool>()) {
        MPPT_Mode = settings["MPPT_Mode"]; settingsChanged = true;
      }
      if (settings.containsKey("output_Mode") && output_Mode != settings["output_Mode"].as<bool>()) {
        output_Mode = settings["output_Mode"]; settingsChanged = true;
      }
      if (settings.containsKey("voltageBatteryMax") && abs(voltageBatteryMax - settings["voltageBatteryMax"].as<float>()) > 0.01) {
        voltageBatteryMax = settings["voltageBatteryMax"]; settingsChanged = true;
      }
      if (settings.containsKey("voltageBatteryMin") && abs(voltageBatteryMin - settings["voltageBatteryMin"].as<float>()) > 0.01) {
        voltageBatteryMin = settings["voltageBatteryMin"]; settingsChanged = true;
      }
      if (settings.containsKey("currentCharging") && abs(currentCharging - settings["currentCharging"].as<float>()) > 0.01) {
        currentCharging = settings["currentCharging"]; settingsChanged = true;
      }
      if (settings.containsKey("enableFan") && enableFan != settings["enableFan"].as<bool>()) {
        enableFan = settings["enableFan"]; settingsChanged = true;
      }
      if (settings.containsKey("enableWiFi") && enableWiFi != settings["enableWiFi"].as<bool>()) {
        enableWiFi = settings["enableWiFi"]; settingsChanged = true;
      }
      if (settings.containsKey("temperatureFan") && temperatureFan != settings["temperatureFan"].as<int>()) {
        temperatureFan = settings["temperatureFan"]; settingsChanged = true;
      }
      if (settings.containsKey("temperatureMax") && temperatureMax != settings["temperatureMax"].as<int>()) {
        temperatureMax = settings["temperatureMax"]; settingsChanged = true;
      }
      if (settings.containsKey("backlightSleepMode") && backlightSleepMode != settings["backlightSleepMode"].as<int>()) {
        backlightSleepMode = settings["backlightSleepMode"]; settingsChanged = true;
      }
      
      if (settingsChanged) {
        Serial.println("> Cloud settings received! Saving to EEPROM.");
        saveSettings();
      }
    }
  }
  
  http.end();
}

void clearFactoryResetFlag() {
  HTTPClient http;
  String url = supabaseUrl + "/rest/v1/device_settings?id=eq.1";
  http.begin(url);
  http.addHeader("apikey", supabaseKey);
  http.addHeader("Authorization", "Bearer " + supabaseKey);
  http.addHeader("Content-Type", "application/json");

  String requestBody = "{\"factoryResetTrigger\":0}";
  http.PATCH(requestBody);
  http.end();
}
