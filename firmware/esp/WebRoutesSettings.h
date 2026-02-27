#pragma once

#include "FirmwareContext.h"

inline void registerSettingsRoutes(WebServer& web, FirmwareContext& context) {
  (void)context;
  web.on("/get_settings", HTTP_GET, guardProtectedRoute(handleGetSettings));
  web.on("/get_settings", HTTP_OPTIONS, allowCORS("GET"));
  web.on("/update_settings", HTTP_POST, guardMutatingRoute(handleUpdateSettings));
  web.on("/update_settings", HTTP_OPTIONS, allowCORS("POST"));
  web.on("/restart", HTTP_POST, guardMutatingRoute(handleRestart));
  web.on("/restart", HTTP_OPTIONS, allowCORS("POST"));
  web.on("/update_wifi", HTTP_POST, guardMutatingRoute(handleUpdateWifi));
  web.on("/update_wifi", HTTP_OPTIONS, allowCORS("POST"));
  web.on("/update_brightness", HTTP_POST, guardMutatingRoute(handleUpdateBrightness));
  web.on("/update_brightness", HTTP_OPTIONS, allowCORS("POST"));

#ifdef OTA_ENABLED
  web.on("/ota_status", HTTP_GET, handleOtaStatus);
  web.on("/ota_status", HTTP_OPTIONS, allowCORS("GET"));
#endif

#ifdef CRASH_LOG_FILE
  web.on("/trigger_crash", HTTP_POST, guardMutatingRoute(handleTriggerCrash));
#endif

#ifdef DEBUGGER_ENABLED
  web.on("/state_debug", HTTP_GET, handleStateDebug);
  web.on("/dump_connections", HTTP_GET, handleDumpConnections);
  web.on("/dump_intersections", HTTP_GET, handleDumpIntersections);
#endif
}

