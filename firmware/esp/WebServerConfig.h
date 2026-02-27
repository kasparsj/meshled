#pragma once

// Handle settings update
void handleUpdateSettings() {
  if (server.hasArg("max_brightness")) {
    uint8_t newBrightness = server.arg("max_brightness").toInt();
    if (newBrightness >= 1 && newBrightness <= 255) {
      maxBrightness = newBrightness;
    }
  } else if (server.hasArg("maxBrightness")) {
    // Legacy support
    uint8_t newBrightness = server.arg("maxBrightness").toInt();
    if (newBrightness >= 1 && newBrightness <= 255) {
      maxBrightness = newBrightness;
    }
  }

  if (server.hasArg("hostname")) {
    deviceHostname = server.arg("hostname");
    if (deviceHostname.length() == 0) {
      deviceHostname = DEFAULT_HOSTNAME;
    }
  }

  if (server.hasArg("api_auth_enabled")) {
    apiAuthEnabled = server.arg("api_auth_enabled").toInt() == 1;
  }

  if (server.hasArg("api_auth_token")) {
    apiAuthToken = server.arg("api_auth_token");
  }

  #ifdef OSC_ENABLED
  if (server.hasArg("osc_enabled")) {
    oscEnabled = server.arg("osc_enabled").toInt() == 1;
  }

  if (server.hasArg("osc_port")) {
    oscPort = server.arg("osc_port").toInt();
  }
  #endif

  if (server.hasArg("led_type")) {
    ledType = server.arg("led_type").toInt();
  }

  if (server.hasArg("color_order")) {
    colorOrder = server.arg("color_order").toInt();
  }

  if (server.hasArg("pixel_count1")) {
    pixelCount1 = server.arg("pixel_count1").toInt();
  } else if (server.hasArg("pixel_count")) {
    // Legacy support
    pixelCount1 = server.arg("pixel_count").toInt();
  }

  if (server.hasArg("pixel_count2")) {
    pixelCount2 = server.arg("pixel_count2").toInt();
  }

  if (server.hasArg("pixel_pin1")) {
    pixelPin1 = server.arg("pixel_pin1").toInt();
  }

  if (server.hasArg("pixel_pin2")) {
    pixelPin2 = server.arg("pixel_pin2").toInt();
  }

  if (server.hasArg("pixel_density")) {
    pixelDensity = server.arg("pixel_density").toInt();
  }

  if (server.hasArg("led_library")) {
    ledLibrary = server.arg("led_library").toInt();
  }

  if (server.hasArg("object_type")) {
    uint8_t newObjectType = server.arg("object_type").toInt();
    if (newObjectType != objectType) {
      objectType = newObjectType;
      // Flag that we need to reinitialize state as well
      // This will be handled after restarting
    }
  }

  #ifdef OTA_ENABLED
  bool otaSettingsChanged = false;

  if (server.hasArg("ota_enabled")) {
    otaEnabled = server.arg("ota_enabled").toInt() == 1;
    otaSettingsChanged = true;
  }

  if (server.hasArg("ota_port")) {
    otaPort = server.arg("ota_port").toInt();
    otaSettingsChanged = true;
  }

  if (server.hasArg("ota_password")) {
    otaPassword = server.arg("ota_password");
    otaSettingsChanged = true;
  }

  // Save credentials when any OTA setting changes
  if (otaSettingsChanged) {
    #ifdef SPIFFS_ENABLED
    saveCredentials();
    #endif
  }
  #endif

  #ifdef SPIFFS_ENABLED
  saveSettings();
  #endif

  // Reinitialize LEDs to apply new settings
  setupLEDs();

  sendCORSHeaders("POST");
  server.send(200, "text/plain", "OK");
}

// Handle restart
void handleRestart() {
  sendCORSHeaders("POST");
  
  server.send(200, "text/plain", "OK");
  LP_LOGLN("Restarting device...");
  ESP.restart();
}

// Handle WiFi save and connect
void handleUpdateWifi() {
  if (server.hasArg("ssid") && server.hasArg("password")) {
    savedSSID = server.arg("ssid");
    savedPassword = server.arg("password");

    // Save credentials to separate file
    #ifdef SPIFFS_ENABLED
    saveCredentials();
    #endif

    sendCORSHeaders("POST");
    server.send(200, "text/html",
      "<html><head><meta name='viewport' content='width=device-width, initial-scale=1'></head>"
      "<body style='font-family:Arial;background:#333;color:#fff;padding:20px'>"
      "<h2>WiFi Updated</h2><p>Restarting device to apply new settings...</p></body></html>");
    delay(150);
    ESP.restart();
  } else {
    sendCORSHeaders("POST");
    server.send(400, "text/plain", "Missing SSID or password");
  }
}

void handleGetSettings() {
  DynamicJsonDocument doc(1024);
  
  doc["maxBrightness"] = maxBrightness;
  doc["deviceHostname"] = deviceHostname;
  doc["pixelCount1"] = pixelCount1;
  doc["pixelCount2"] = pixelCount2;
  doc["pixelPin1"] = pixelPin1;
  doc["pixelPin2"] = pixelPin2;
  doc["pixelDensity"] = pixelDensity;
  doc["ledType"] = ledType;
  doc["colorOrder"] = colorOrder;
  doc["ledLibrary"] = ledLibrary;
  doc["objectType"] = objectType;
  doc["savedSSID"] = savedSSID;
  doc["savedPassword"] = savedPassword;
  doc["activeSSID"] = getActiveNetworkSSID();
  doc["apMode"] = apMode;
  doc["apiAuthEnabled"] = apiAuthEnabled;
  doc["apiAuthToken"] = apiAuthToken;
  
  #ifdef OSC_ENABLED
  doc["oscEnabled"] = oscEnabled;
  doc["oscPort"] = oscPort;
  #endif
  
  #ifdef OTA_ENABLED
  doc["otaEnabled"] = otaEnabled;
  doc["otaPort"] = otaPort;
  doc["otaPassword"] = otaPassword;
  #endif

  String jsonResponse;
  serializeJson(doc, jsonResponse);
  
  sendCORSHeaders();
  server.send(200, "application/json", jsonResponse);
}

void handleUpdateBrightness() {
  if (server.hasArg("value")) {
    uint8_t newBrightness = server.arg("value").toInt();
    if (newBrightness >= 1 && newBrightness <= 255) {
      maxBrightness = newBrightness;
      LP_LOGLN("Updated brightness via AJAX: " + String(maxBrightness));

      #ifdef SPIFFS_ENABLED
      saveSettings();
      #endif

      sendCORSHeaders("POST");
      server.send(200, "text/plain", "OK");
      return;
    }
  }

  server.send(400, "text/plain", "Invalid brightness value");
}
