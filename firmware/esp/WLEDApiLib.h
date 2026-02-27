#pragma once

// WLED API Compatibility Library
// Implements a subset of WLED's HTTP API for compatibility with apps and integrations

#if __has_include(<esp_ota_ops.h>)
#include <esp_ota_ops.h>
#define MESHLED_HAS_OTA_SHA_HELPER 1
#else
#define MESHLED_HAS_OTA_SHA_HELPER 0
#endif

// WLED API endpoint handlers

#ifndef MESHLED_VERSION
#define MESHLED_VERSION "dev"
#endif

#ifndef MESHLED_RELEASE_SHA
#define MESHLED_RELEASE_SHA "unknown"
#endif

#ifndef MESHLED_GIT_COMMIT
#define MESHLED_GIT_COMMIT "unknown"
#endif

inline bool meshledIsHexChar(char c) {
  return (c >= '0' && c <= '9') ||
         (c >= 'a' && c <= 'f') ||
         (c >= 'A' && c <= 'F');
}

inline bool meshledLooksLikeGitSha(const String& value) {
  String candidate = value;
  candidate.trim();
  const int length = candidate.length();
  if (length < 7 || length > 40) {
    return false;
  }

  for (int i = 0; i < length; i++) {
    if (!meshledIsHexChar(candidate.charAt(i))) {
      return false;
    }
  }

  return true;
}

inline String meshledExtractGitShaFromVersion(const String& version) {
  String value = version;
  value.trim();
  if (value.length() == 0) {
    return "";
  }

  const int gitMarkerPos = value.lastIndexOf("-g");
  if (gitMarkerPos >= 0) {
    const int start = gitMarkerPos + 2;
    int end = start;
    while (end < value.length() && meshledIsHexChar(value.charAt(end))) {
      end++;
    }
    String candidate = value.substring(start, end);
    if (meshledLooksLikeGitSha(candidate)) {
      return candidate;
    }
  }

  if (value.length() > 1 && (value.charAt(0) == 'g' || value.charAt(0) == 'G')) {
    String candidate = value.substring(1);
    if (meshledLooksLikeGitSha(candidate)) {
      return candidate;
    }
  }

  return "";
}

String getResolvedMeshledVersion() {
  static String cachedVersion;
  if (cachedVersion.length() > 0) {
    return cachedVersion;
  }

  cachedVersion = String(MESHLED_VERSION);
  cachedVersion.trim();
  if (cachedVersion.length() == 0) {
    cachedVersion = "dev";
  }

  return cachedVersion;
}

String getResolvedMeshledCommitSha() {
  static String cachedCommitSha;
  if (cachedCommitSha.length() > 0) {
    return cachedCommitSha;
  }

  cachedCommitSha = String(MESHLED_GIT_COMMIT);
  cachedCommitSha.trim();
  if (meshledLooksLikeGitSha(cachedCommitSha)) {
    return cachedCommitSha;
  }

  cachedCommitSha = String(MESHLED_RELEASE_SHA);
  cachedCommitSha.trim();
  if (meshledLooksLikeGitSha(cachedCommitSha)) {
    return cachedCommitSha;
  }

  cachedCommitSha = meshledExtractGitShaFromVersion(getResolvedMeshledVersion());
  cachedCommitSha.trim();
  if (meshledLooksLikeGitSha(cachedCommitSha)) {
    return cachedCommitSha;
  }

  cachedCommitSha = "unknown";
  return cachedCommitSha;
}

String getResolvedMeshledReleaseSha() {
  static String cachedReleaseSha;
  if (cachedReleaseSha.length() > 0) {
    return cachedReleaseSha;
  }

  cachedReleaseSha = String(MESHLED_RELEASE_SHA);
  cachedReleaseSha.trim();
  if (cachedReleaseSha.length() > 0 && cachedReleaseSha != "unknown") {
    return cachedReleaseSha;
  }

#if MESHLED_HAS_OTA_SHA_HELPER
  char elfSha[65] = {0};
  int written = esp_ota_get_app_elf_sha256(elfSha, sizeof(elfSha));
  if (written > 0 && elfSha[0] != '\0') {
    cachedReleaseSha = String(elfSha);
    if (cachedReleaseSha.length() > 12) {
      cachedReleaseSha = cachedReleaseSha.substring(0, 12);
    }
    return cachedReleaseSha;
  }
#endif

  cachedReleaseSha = "unknown";
  return cachedReleaseSha;
}

uint8_t getWLEDSignalPercent(int32_t rssi) {
  if (rssi <= -100) {
    return 0;
  }
  if (rssi >= -50) {
    return 100;
  }
  return static_cast<uint8_t>(2 * (rssi + 100));
}

String getWLEDMacString() {
  String mac = WiFi.macAddress();
  mac.replace(":", "");
  mac.toLowerCase();
  return mac;
}

bool isOn() {
  return wledMasterOn;
}

bool isReportedOn() {
  return isOn();
}

void turnOn() {
  wledMasterOn = true;
}

void turnOff() {
  wledMasterOn = false;
  if (state) {
    state->autoEnabled = false;
  }
  emitterEnabled = false;
}

const Palette* getPrimaryPalette() {
  if (!state || !state->lightLists[0]) {
    return nullptr;
  }
  return &state->lightLists[0]->palette;
}

void getWLEDState(JsonObject& jsonState) {
  const bool reportedOn = isReportedOn();
  jsonState["on"] = reportedOn;
  jsonState["bri"] = maxBrightness;
  jsonState["transition"] = 7;
  jsonState["ps"] = -1;  // No preset active
  jsonState["pl"] = -1;  // No playlist active
  jsonState["ledmap"] = 0;

  // Include these fields for WLED compatibility
  jsonState["nl"]["on"] = false;
  jsonState["nl"]["dur"] = 60;
  jsonState["nl"]["mode"] = 1;
  jsonState["nl"]["tbri"] = 0;
  jsonState["nl"]["rem"] = -1;
  jsonState["udpn"]["send"] = false;
  jsonState["udpn"]["recv"] = true;
  jsonState["udpn"]["sgrp"] = 1;
  jsonState["udpn"]["rgrp"] = 1;
  jsonState["lor"] = 0;
  jsonState["mainseg"] = 0;

  // Colors using segments format
  JsonArray segments = jsonState.createNestedArray("seg");
  JsonObject mainseg = segments.createNestedObject();
  mainseg["id"] = 0;  // Main segment
  mainseg["start"] = 0;
  mainseg["stop"] = pixelCount1 + pixelCount2;
  mainseg["len"] = pixelCount1 + pixelCount2;
  mainseg["grp"] = 1;  // Grouping (1 = individual control)
  mainseg["spc"] = 0;
  mainseg["of"] = 0;
  mainseg["on"] = reportedOn;
  mainseg["frz"] = false;
  mainseg["bri"] = maxBrightness;
  mainseg["cct"] = 127;
  mainseg["set"] = 0;

  // Include palette properties
  const Palette* bgPalette = getPrimaryPalette();
  if (bgPalette) {
    mainseg["seg"] = bgPalette->getSegmentation();
  } else {
    mainseg["seg"] = 0.0f;
  }

  // Primary color - use colors from palette when available.
  JsonArray col = mainseg.createNestedArray("col");
  if (bgPalette && bgPalette->size() > 0) {
    const std::vector<int64_t>& paletteColors = bgPalette->getColors();
    for (size_t i = 0; i < paletteColors.size(); i++) {
      int64_t color = paletteColors[i];
      JsonArray rgb = col.createNestedArray();
      rgb.add((color >> 16) & 0xFF);  // R
      rgb.add((color >> 8) & 0xFF);   // G
      rgb.add(color & 0xFF);          // B
    }
  } else {
    JsonArray rgb = col.createNestedArray();
    rgb.add(255);  // Default R
    rgb.add(0);    // Default G
    rgb.add(0);    // Default B
  }

  while (col.size() < 3) {
    JsonArray rgb = col.createNestedArray();
    rgb.add(0);
    rgb.add(0);
    rgb.add(0);
  }

  // Match common WLED state keys used by mobile app clients.
  mainseg["fx"] = 0;
  mainseg["sx"] = 128;
  mainseg["ix"] = 128;
  mainseg["pal"] = 50;
  mainseg["c1"] = 128;
  mainseg["c2"] = 128;
  mainseg["c3"] = 16;
  mainseg["sel"] = true;
  mainseg["rev"] = false;
  mainseg["mi"] = false;
  mainseg["o1"] = false;
  mainseg["o2"] = false;
  mainseg["o3"] = false;
  mainseg["si"] = 0;
  mainseg["m12"] = 0;
}

void getWLEDInfo(JsonObject& info) {
  // Device info - following the WLED JSON API structure
  info["ver"] = "0.15.0";  // Mimic WLED version for compatibility
  info["vid"] = 2412100;  // Build ID (use WLED format YYMMDD0)
  info["cn"] = "Kōsen";
  info["release"] = "ESP32";
  info["meshledVersion"] = getResolvedMeshledVersion();
  info["meshledCommitSha"] = getResolvedMeshledCommitSha();
  info["meshledBuildSha"] = getResolvedMeshledReleaseSha();
  info["meshledReleaseSha"] = getResolvedMeshledReleaseSha();

  // LED information
  info["leds"]["count"] = pixelCount1 + pixelCount2;
  info["leds"]["rgbw"] = HAS_WHITE(colorOrder);
  info["leds"]["pwr"] = totalWattage;
  info["leds"]["fps"] = 40;
  info["leds"]["maxpwr"] = 2400;
  info["leds"]["maxseg"] = 32;
  info["leds"]["bootps"] = 0;
  JsonArray seglc = info["leds"].createNestedArray("seglc");
  seglc.add(1);
  info["leds"]["lc"] = 1;
  info["leds"]["wv"] = 0;
  info["leds"]["cct"] = 0;

  // Network information - essential for app connectivity
  info["str"] = false;
  info["name"] = deviceHostname;
  info["udpport"] = 21324;
  info["simplifiedui"] = false;
  info["live"] = false;
  info["liveseg"] = -1;
  info["lm"] = "";
  info["lip"] = "";
  info["ws"] = 1;
  info["fxcount"] = 187;
  info["palcount"] = 71;
  info["cpalcount"] = 0;

  JsonArray maps = info.createNestedArray("maps");
  JsonObject mainmap = maps.createNestedObject();
  mainmap["id"] = 0;

  JsonObject wifi = info.createNestedObject("wifi");
  wifi["ssid"] = getActiveNetworkSSID();
  wifi["mode"] = apMode ? "ap" : "sta";
  wifi["ap"] = apMode;
  wifi["bssid"] = WiFi.BSSIDstr();
  const int32_t rssi = WiFi.RSSI();
  wifi["rssi"] = rssi;
  wifi["signal"] = getWLEDSignalPercent(rssi);
  wifi["channel"] = WiFi.channel();

  JsonObject fs = info.createNestedObject("fs");
  fs["u"] = 12;
  fs["t"] = 983;
  fs["pmt"] = 1746784873;

  info["arch"] = "esp32";
  info["core"] = ESP.getSdkVersion();
  info["freeheap"] = ESP.getFreeHeap();
  info["uptime"] = millis() / 1000;
  info["time"] = "2025-5-14, 18:18:29";
  info["opt"] = 79;
  info["brand"] = "WLED";  // Needed for app to recognize as valid WLED device
  info["product"] = "FOSS";  // Needed for app compatibility
  info["mac"] = getWLEDMacString();
  info["ip"] = WiFi.localIP().toString();
}

// Returns basic device info in JSON format
void handleWLEDInfo() {
  sendCORSHeaders();
  
  String output;

  DynamicJsonDocument doc(3072);
  JsonObject info = doc.to<JsonObject>();
  getWLEDInfo(info);

  serializeJson(doc, output);
  server.send(200, "application/json", output);
}

void handleWLEDSI() {
  String output;

  DynamicJsonDocument doc(6144);

  JsonObject state = doc.createNestedObject("state");
  getWLEDState(state);

  JsonObject info = doc.createNestedObject("info");
  getWLEDInfo(info);

  serializeJson(doc, output);
  server.send(200, "application/json", output);
}

void sendWLEDState() {
    // Return current state in WLED-compatible JSON format
  DynamicJsonDocument doc(3072);

  // Create state object with WLED-compatible structure
  JsonObject state = doc.to<JsonObject>();
  getWLEDState(state);

  String output;
  serializeJson(doc, output);
  server.send(200, "application/json", output);
}

void handleWLEDPost() {
  if (!requireApiAuth()) {
    return;
  }

  String body = server.arg("plain");
  DynamicJsonDocument doc(6144);
  DeserializationError error = deserializeJson(doc, body);

  if (error) {
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }

  bool shouldReturnState = doc.containsKey("v") && (bool) doc["v"];

  // Process the state update
  bool shouldSaveSettings = false;

  // Handle brightness
  if (doc.containsKey("bri")) {
    const int newBrightness = doc["bri"].as<int>();
    if (newBrightness <= 0) {
      // WLED app slider to zero is treated as power off.
      turnOff();
      shouldSaveSettings = true;
    } else if (newBrightness <= 255) {
      maxBrightness = static_cast<uint8_t>(newBrightness);
      if (!isReportedOn()) {
        turnOn();
      }
      shouldSaveSettings = true;
    }
  }

  // Handle on/off state
  if (doc.containsKey("on")) {
    bool newState = doc["on"];
    if (newState) {
      turnOn();
    } else {
      turnOff();
    }
    shouldSaveSettings = true;
  }

  // Handle colors
  if (doc.containsKey("col")) {
    JsonArray colArray = doc["col"];

    // Clear existing colors if we have new ones
    if (colArray.size() > 0) {
      std::vector<int64_t> newColors;

      // Process each color
      for (JsonVariant colorVar : colArray) {
        if (colorVar.is<JsonArray>()) {
          JsonArray rgbArray = colorVar.as<JsonArray>();
          if (rgbArray.size() >= 3) {
            // Convert RGB to hex color value
            int64_t color = (((int64_t)rgbArray[0]) << 16) |
                            (((int64_t)rgbArray[1]) << 8) |
                            ((int64_t)rgbArray[2]);
            newColors.push_back(color);
          }
        }
      }

      // Ensure at least one color
      if (newColors.empty()) {
        newColors.push_back(0xFF0000);  // Default red
      }

      // Create a new palette with these colors (keeping positions if possible)
      Palette& bgPalette = state->lightLists[0]->palette;
      if (bgPalette.size() == newColors.size()) {
        // Keep the same positions, just update colors
        const std::vector<float>& oldPositions = bgPalette.getPositions();
        bgPalette = Palette(newColors, oldPositions);
      } else {
        // New number of colors, so generate default positions
        bgPalette = Palette(newColors);
      }

      // If background is active, refresh it
      if (state && state->lightLists[0]) {
        state->lightLists[0]->setPalette(bgPalette);
      }

      shouldSaveSettings = true;
    }
  }

  // Save settings if needed
  if (shouldSaveSettings) {
    #ifdef SPIFFS_ENABLED
    saveSettings();
    saveLayers();
    #endif
  }

  if (shouldReturnState) {
    sendWLEDState();
    return;
  }
  server.send(200, "application/json", "{\"success\":true}");
}

void sendWLEDJson() {
  DynamicJsonDocument doc(6144);

  JsonObject state = doc.createNestedObject("state");
  getWLEDState(state);

  JsonObject info = doc.createNestedObject("info");
  getWLEDInfo(info);

  JsonArray effects = doc.createNestedArray("effects");
  JsonArray palettes = doc.createNestedArray("palettes");

  String output;
  serializeJson(doc, output);
  server.send(200, "application/json", output);
}

// WLED API: /json
void handleWLEDJson() {
  if (server.method() == HTTP_GET) {
    sendWLEDJson();
    return;
  }

  if (server.method() == HTTP_POST) {
    handleWLEDPost();
    return;
  }

  server.send(405, "text/plain", "Method Not Allowed");
}

// WLED API: /state
void handleWLEDState() {
  if (server.method() == HTTP_GET) {
    sendWLEDState();
    return;
  }

  if (server.method() == HTTP_POST) {
    handleWLEDPost();
    return;
  }

  server.send(405, "text/plain", "Method Not Allowed");
}

// WLED API: Simple on/off control
void handleWLEDOn() {
  if (!requireApiAuth()) {
    return;
  }
  turnOn();

  #ifdef SPIFFS_ENABLED
  saveSettings();
  saveLayers();
  #endif

  server.send(200, "text/plain", "ON");
}

void handleWLEDOff() {
  if (!requireApiAuth()) {
    return;
  }
  turnOff();

  #ifdef SPIFFS_ENABLED
  saveSettings();
  saveLayers();
  #endif

  server.send(200, "text/plain", "OFF");
}

// WLED API: Get version info
void handleWLEDVersion() {
  server.send(200, "text/plain", "MeshLED WLED Compatible API v1.0.0");
}

bool parseWLEDWinLong(const String& rawValue, long& parsedValue) {
  if (rawValue.length() == 0) {
    return false;
  }
  char* end = nullptr;
  const long value = strtol(rawValue.c_str(), &end, 10);
  if (end == rawValue.c_str() || *end != '\0') {
    return false;
  }
  parsedValue = value;
  return true;
}

void applyWLEDWinArg(const String& rawKey, const String& rawValue, bool& shouldSaveSettings) {
  String key = rawKey;
  key.toUpperCase();

  long parsed = 0;
  if (key == "A" && parseWLEDWinLong(rawValue, parsed)) {
    const long clamped = constrain(parsed, 0, 255);
    if (clamped == 0) {
      turnOff();
    } else {
      maxBrightness = static_cast<uint8_t>(clamped);
      turnOn();
    }
    shouldSaveSettings = true;
    return;
  }

  if (key == "T" && parseWLEDWinLong(rawValue, parsed)) {
    switch (parsed) {
      case 0:
        turnOff();
        shouldSaveSettings = true;
        break;
      case 1:
        turnOn();
        shouldSaveSettings = true;
        break;
      case 2:
        if (isReportedOn()) {
          turnOff();
        } else {
          turnOn();
        }
        shouldSaveSettings = true;
        break;
      default:
        break;
    }
  }
}

void applyWLEDWinUriArgs(bool& shouldSaveSettings) {
  const String uri = server.uri();
  if (!uri.startsWith("/win?") && !uri.startsWith("/win&")) {
    return;
  }

  const int queryStart = 5;  // "/win?" and "/win&"
  if (uri.length() <= queryStart) {
    return;
  }

  const String query = uri.substring(queryStart);
  int start = 0;

  while (start < query.length()) {
    const int nextAmp = query.indexOf('&', start);
    const String token = nextAmp >= 0 ? query.substring(start, nextAmp) : query.substring(start);
    if (token.length() > 0) {
      const int eq = token.indexOf('=');
      const String key = eq >= 0 ? token.substring(0, eq) : token;
      const String value = eq >= 0 ? token.substring(eq + 1) : "";
      applyWLEDWinArg(key, value, shouldSaveSettings);
    }
    if (nextAmp < 0) {
      break;
    }
    start = nextAmp + 1;
  }
}

void applyWLEDWinArgs(bool& shouldSaveSettings) {
  for (int i = 0; i < server.args(); i++) {
    applyWLEDWinArg(server.argName(i), server.arg(i), shouldSaveSettings);
  }
  // Legacy WLED mobile app style: "/win&T=2" (without '?').
  applyWLEDWinUriArgs(shouldSaveSettings);
}

// Generate the WLED /win XML response
// This endpoint is critical for WLED app discovery and control
void handleWLEDWin() {
  bool shouldSaveSettings = false;
  applyWLEDWinArgs(shouldSaveSettings);

  if (shouldSaveSettings) {
    #ifdef SPIFFS_ENABLED
    saveSettings();
    saveLayers();
    #endif
  }

  // Create XML string builder
  String xml = "<?xml version=\"1.0\" ?><vs>";

  // "ac" in WLED XML is current brightness (0 means off).
  const bool reportedOn = isReportedOn();
  xml += "<ac>" + String(reportedOn ? maxBrightness : 0) + "</ac>";

  // Add colors from the current palette
  const Palette* bgPalette = getPrimaryPalette();
  if (bgPalette && bgPalette->size() > 0) {
    // First color (primary)
    const std::vector<int64_t>& paletteColors = bgPalette->getColors();
    int64_t color = paletteColors[0];
    xml += "<cl>" + String((color >> 16) & 0xFF) + "</cl>"; // R
    xml += "<cl>" + String((color >> 8) & 0xFF) + "</cl>";  // G
    xml += "<cl>" + String(color & 0xFF) + "</cl>";         // B

    // If there's a second color, use it for secondary color
    if (paletteColors.size() > 1) {
      int64_t color2 = paletteColors[1];
      xml += "<cs>" + String((color2 >> 16) & 0xFF) + "</cs>"; // R
      xml += "<cs>" + String((color2 >> 8) & 0xFF) + "</cs>";  // G
      xml += "<cs>" + String(color2 & 0xFF) + "</cs>";         // B
    } else {
      // Default secondary color if not available
      xml += "<cs>255</cs><cs>255</cs><cs>255</cs>";
    }
  } else {
    // Default colors if none are set
    xml += "<cl>255</cl><cl>0</cl><cl>0</cl>";         // Red
    xml += "<cs>255</cs><cs>255</cs><cs>255</cs>";     // White
  }

  // Add settings required by WLED app
  xml += "<ns>0</ns>";   // Notification Sender
  xml += "<nr>1</nr>";   // Notification Receiver
  xml += "<nl>0</nl>";   // Nightlight active
  xml += "<nf>1</nf>";   // Nightlight fade
  xml += "<nd>60</nd>";  // Nightlight delay
  xml += "<nt>0</nt>";   // Nightlight target brightness

  // Effect settings (we use 0 for solid color)
  xml += "<fx>0</fx>";   // Effect ID
  xml += "<sx>128</sx>"; // Effect speed
  xml += "<ix>128</ix>"; // Effect intensity
  xml += "<fp>0</fp>";   // Effect palette

  // White channel setting (-1 means not used)
  xml += "<wv>" + String(HAS_WHITE(colorOrder) ? 0 : -1) + "</wv>";

  // Additional settings
  xml += "<ws>0</ws>";   // White balance setting
  xml += "<ps>0</ps>";   // Preset ID
  xml += "<cy>0</cy>";   // Preset cycle active

  // Device name
  xml += "<ds>" + deviceHostname + "</ds>";

  // Sync settings
  xml += "<ss>0</ss>";   // Sync send

  // Close XML
  xml += "</vs>";

  // Send the XML
  server.send(200, "application/xml", xml);
}
