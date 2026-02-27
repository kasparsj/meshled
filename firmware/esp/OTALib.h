#pragma once

#include "OTADiagnostics.h"

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
    recordOtaStartStatus(type);
  });
  
  ArduinoOTA.onEnd([]() {
    LP_LOGLN("End OTA");
    recordOtaEndStatus();
  });
  
  // ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
  //   LP_LOGF("Progress: %u%%\r", (progress / (total / 100)));
  // });
  
  ArduinoOTA.onError([](ota_error_t error) {
    String detail;
    LP_LOGF("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      LP_LOGLN("Auth Failed");
      detail = "auth_failed";
    } else if (error == OTA_BEGIN_ERROR) {
      LP_LOGLN("Begin Failed");
      detail = "begin_failed";
    } else if (error == OTA_CONNECT_ERROR) {
      LP_LOGLN("Connect Failed");
      detail = "connect_failed";
    } else if (error == OTA_RECEIVE_ERROR) {
      LP_LOGLN("Receive Failed");
      detail = "receive_failed";
    } else if (error == OTA_END_ERROR) {
      LP_LOGLN("End Failed");
      detail = "end_failed";
    } else {
      detail = "unknown_error";
    }
    recordOtaErrorStatus(static_cast<int>(error), detail);
  });
  
  ArduinoOTA.begin();
  LP_LOGLN("OTA setup complete");
}
