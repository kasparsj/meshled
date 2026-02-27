#pragma once

#include "FirmwareContext.h"

inline void registerBaseRoutes(WebServer& web, FirmwareContext& context) {
  (void)context;
  web.on("/", HTTP_GET, handleRoot);
  web.on("/settings", HTTP_GET, handleSettingsPage);
  web.on("/wifi", HTTP_GET, handleWifiPage);
  web.on("/get_devices", HTTP_GET, handleGetDevices);
}

