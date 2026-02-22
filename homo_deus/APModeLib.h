#pragma once

void startAPMode() {
  apMode = true;
  apStartTime = millis();

  // Generate a unique AP SSID with the chip ID
  uint64_t chipId = ESP.getEfuseMac();
  String uniqueId = String((uint32_t)(chipId >> 32), HEX) + String((uint32_t)chipId, HEX);
  uniqueId.toUpperCase(); // Make the hex ID uppercase for better readability
  
  // Use the last 6 characters of the chip ID to keep it short
  String uniqueSSID = String(AP_SSID) + "_" + uniqueId.substring(uniqueId.length() - 6);
  
  LP_LOGLN("Starting AP mode with SSID: " + uniqueSSID);

  // Configure the access point
  WiFi.disconnect(true);
  delay(500);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(uniqueSSID.c_str(), AP_PASS);

  IPAddress apIP = WiFi.softAPIP();
  LP_LOGLN("AP IP address: " + apIP.toString());

  LP_LOGLN("AP mode active. Connect to '" + uniqueSSID + "' WiFi network and browse to http://" + apIP.toString());
}

// Check if we need to exit AP mode due to timeout
void checkAPModeTimeout() {
  if (apMode && (millis() - apStartTime > AP_TIMEOUT)) {
    LP_LOGLN("AP mode timeout reached. Restarting...");
    ESP.restart();
  }
}