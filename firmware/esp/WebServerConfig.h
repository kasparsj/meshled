#pragma once

#include "ConfigService.h"

// Handle settings update
void handleUpdateSettings() {
  SettingsPatch patch;
  String parseError;
  if (!parseSettingsPatch(patch, parseError)) {
    sendCORSHeaders("POST");
    server.send(400, "application/json", "{\"error\":\"" + parseError + "\"}");
    return;
  }

  SettingsApplyResult applyResult;
  String applyError;
  if (!applySettingsPatch(patch, applyResult, applyError)) {
    sendCORSHeaders("POST");
    server.send(400, "application/json", "{\"error\":\"" + applyError + "\"}");
    return;
  }

  if (applyResult.needsLedReinit) {
    setupLEDs();
  }
  if (applyResult.needsStateRebuild) {
    rebuildRuntimeState();
  }

  #ifdef SPIFFS_ENABLED
  saveSettings();
  if (applyResult.credentialsChanged) {
    saveCredentials();
  }
  #endif

  sendCORSHeaders("POST");
  server.sendHeader("X-Meshled-Requires-Reboot", applyResult.requiresReboot ? "1" : "0");
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
  normalizeLedSelection();
  if (!isSupportedObjectType(objectType)) {
    objectType = OBJ_LINE;
  }

  DynamicJsonDocument doc(4096);
  
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
  doc["savedPassword"] = "";
  doc["hasSavedPassword"] = savedPassword.length() > 0;
  doc["activeSSID"] = getActiveNetworkSSID();
  doc["apMode"] = apMode;
  doc["apiAuthEnabled"] = apiAuthEnabled;
  doc["apiAuthToken"] = "";
  doc["hasApiAuthToken"] = hasApiAuthTokenConfigured();

  JsonArray availableLedLibraries = doc.createNestedArray("availableLedLibraries");
  if (isLedLibraryAvailable(LIB_NEOPIXELBUS)) {
    availableLedLibraries.add(LIB_NEOPIXELBUS);
  }
  if (isLedLibraryAvailable(LIB_FASTLED)) {
    availableLedLibraries.add(LIB_FASTLED);
  }

  JsonObject unavailableLedLibraryReasons = doc.createNestedObject("unavailableLedLibraryReasons");
  if (!isLedLibraryAvailable(LIB_NEOPIXELBUS)) {
    const char* reason = ledLibraryUnavailableReason(LIB_NEOPIXELBUS);
    unavailableLedLibraryReasons[String(LIB_NEOPIXELBUS)] = reason != NULL ? reason : "Unavailable";
  }
  if (!isLedLibraryAvailable(LIB_FASTLED)) {
    const char* reason = ledLibraryUnavailableReason(LIB_FASTLED);
    unavailableLedLibraryReasons[String(LIB_FASTLED)] = reason != NULL ? reason : "Unavailable";
  }

  JsonArray availableLedTypes = doc.createNestedArray("availableLedTypes");
  JsonObject ledTypeAvailableLibraries = doc.createNestedObject("ledTypeAvailableLibraries");
  JsonObject unavailableLedTypeReasons = doc.createNestedObject("unavailableLedTypeReasons");

  for (size_t typeIndex = 0; typeIndex < knownLedTypeCount(); typeIndex++) {
    uint8_t typeId = knownLedTypeAt(typeIndex);

    if (isLedTypeAvailable(typeId)) {
      availableLedTypes.add(typeId);
    }

    JsonArray supportedLibraries = ledTypeAvailableLibraries.createNestedArray(String(typeId));
    for (size_t libraryIndex = 0; libraryIndex < knownLedLibraryCount(); libraryIndex++) {
      uint8_t libraryId = knownLedLibraryAt(libraryIndex);
      if (isLedTypeAvailableForLibrary(typeId, libraryId)) {
        supportedLibraries.add(libraryId);
      }
    }

    if (supportedLibraries.size() == 0) {
      const char* reason = ledTypeUnavailableReason(typeId);
      unavailableLedTypeReasons[String(typeId)] = reason != NULL ? reason : "Unavailable";
    }
  }
  
  #ifdef OSC_ENABLED
  doc["oscEnabled"] = oscEnabled;
  doc["oscPort"] = oscPort;
  #endif
  
  #ifdef OTA_ENABLED
  doc["otaEnabled"] = otaEnabled;
  doc["otaPort"] = otaPort;
  doc["otaPassword"] = "";
  doc["hasOtaPassword"] = otaPassword.length() > 0;
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
