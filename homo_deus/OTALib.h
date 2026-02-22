#pragma once

void setupOTA() {
  if (!wifiConnected || !otaEnabled) {
    if (!wifiConnected) {
      LP_LOGLN("OTA setup skipped (no WiFi connection)");
    } else {
      LP_LOGLN("OTA setup skipped (disabled in settings)");
    }
    return;
  }

  // Set hostname for OTA
  if (deviceHostname.length() > 0) {
    ArduinoOTA.setHostname(deviceHostname.c_str());
  } else {
    ArduinoOTA.setHostname(DEFAULT_HOSTNAME);
  }

  ArduinoOTA.setPassword(otaPassword.c_str());
  ArduinoOTA.setPort(otaPort);

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {  // U_SPIFFS
      type = "filesystem";
    }
    
    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    LP_LOGLN("Start OTA " + type);
  });
  
  ArduinoOTA.onEnd([]() {
    LP_LOGLN("End OTA");
  });
  
  // ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
  //   LP_LOGF("Progress: %u%%\r", (progress / (total / 100)));
  // });
  
  ArduinoOTA.onError([](ota_error_t error) {
    LP_LOGF("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      LP_LOGLN("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      LP_LOGLN("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      LP_LOGLN("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      LP_LOGLN("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      LP_LOGLN("End Failed");
    }
  });
  
  ArduinoOTA.begin();
  LP_LOGLN("OTA setup complete");
}