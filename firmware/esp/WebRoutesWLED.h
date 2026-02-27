#pragma once

#include "FirmwareContext.h"

inline void registerWledRoutes(WebServer& web, FirmwareContext& context) {
  (void)context;

#ifdef SSDP_ENABLED
  setupSSDPService();
  web.on("/description.xml", HTTP_GET, handleDescriptionXML);
  web.on("/icon48.png", HTTP_GET, handleIcon);
#endif

#ifdef WLEDAPI_ENABLED
  web.on("/json", HTTP_ANY, handleWLEDJson);
  web.on("/json/info", HTTP_GET, handleWLEDInfo);
  web.on("/json/info", HTTP_OPTIONS, handleCORS);
  web.on("/device_info", HTTP_GET, handleWLEDInfo);
  web.on("/device_info", HTTP_OPTIONS, handleCORS);
  web.on("/json/state", HTTP_ANY, handleWLEDState);
  web.on("/json/si", HTTP_GET, handleWLEDSI);
  web.on("/on", HTTP_GET, handleWLEDOn);
  web.on("/off", HTTP_GET, handleWLEDOff);
  web.on("/version", HTTP_GET, handleWLEDVersion);
  web.on("/win", HTTP_GET, handleWLEDWin);

  web.onNotFound([]() {
    if (server.method() == HTTP_GET && server.uri().startsWith("/win&")) {
      handleWLEDWin();
      return;
    }
    server.send(404, "text/plain", "Not Found");
  });
#endif
}

