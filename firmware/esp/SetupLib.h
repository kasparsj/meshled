#pragma once

void setupIO() {
  #ifdef BUTTON_PIN
    pinMode(BUTTON_PIN, INPUT);
    LP_LOGLN("Button pin initialized");
  #endif
}

void setupWiFi() {
  // First, check if we have valid WiFi credentials
  bool shouldAttemptConnection = true;

  #ifdef AP_MODE_ENABLED
  if (savedSSID.length() == 0) {
    LP_LOGLN("No valid WiFi credentials found. Starting AP mode...");
    shouldAttemptConnection = false;
    startAPMode();
  }
  #endif

  if (shouldAttemptConnection) {
    #ifdef ESP_PLATFORM
    WiFi.disconnect(true);  // disable wifi, erase ap info
    WiFi.mode(WIFI_STA);
    delay(1000);
    #endif

    uint8_t numTries = 0;
    // Set hostname (if available)
    if (deviceHostname.length() > 0) {
      WiFi.setHostname(deviceHostname.c_str());
      LP_LOGLN("Setting hostname: " + deviceHostname);
    }
    WiFi.begin(savedSSID.c_str(), savedPassword.c_str());
    LP_LOGLN("Attempting to connect to WiFi network: " + savedSSID);

    while (numTries < 20 && WiFi.status() != WL_CONNECTED) {
        numTries++;
        Serial.print(".");
        delay(500);
    }

    if (WiFi.status() == WL_CONNECTED) {
      wifiConnected = true;
      apMode = false;
      activeApSSID = "";
      LP_LOGLN("WiFi connected, IP = " + WiFi.localIP().toString());
      WiFi.setAutoReconnect(true);
      WiFi.persistent(true);

      #ifdef OTA_ENABLED
      setupOTA();
      #endif

      #ifdef OSC_ENABLED
      setupOSC();
      #endif

      #ifdef MDNS_ENABLED
      // Check if current hostname is available or make unique
      deviceHostname = checkAndAdjustHostname(deviceHostname);
      setupMDNSService();
      #endif

      #ifdef ESPNOW_ENABLED
      setupExternalTransportAdapters();
      #else
      registerExternalTransportAdapter(nullptr);
      #endif
      initExternalTransport();
    }
    else {
      // WiFi connection failed
      WiFi.disconnect(true, true);
      LP_LOGLN("WiFi failed to connect to: " + savedSSID);

      #ifdef WIFI_REQUIRED
      LP_LOGLN("WiFi required but failed to connect. Restarting...");
      ESP.restart();
      #else
      #ifdef AP_MODE_ENABLED
      LP_LOGLN("Starting access point mode for configuration...");
      startAPMode();
      #else
      LP_LOGLN("Continuing without WiFi...");
      wifiConnected = false;
      #endif
      #endif
    }
  }

  #ifdef WEB_ENABLED
  setupWebServer();
  #endif
}

void setupComms() {
  Serial.begin(115200);

  #ifdef WIFI_ENABLED
  setupWiFi();
  #endif
}
