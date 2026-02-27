#pragma once

#include "FirmwareContext.h"

inline void registerTopologyRoutes(WebServer& web, FirmwareContext& context) {
  (void)context;
  web.on("/get_colors", HTTP_GET, handleGetColors);
  web.on("/get_colors", HTTP_OPTIONS, allowCORS("GET"));
  web.on("/get_model", HTTP_GET, handleGetModel);
  web.on("/get_model", HTTP_OPTIONS, allowCORS("GET"));
  web.on("/export_topology", HTTP_GET, handleExportTopology);
  web.on("/export_topology", HTTP_OPTIONS, allowCORS("GET"));
  web.on("/import_topology", HTTP_POST, guardMutatingRoute(handleImportTopology));
  web.on("/import_topology", HTTP_OPTIONS, allowCORS("POST"));
  web.on("/add_intersection", HTTP_POST, guardMutatingRoute(handleAddIntersection));
  web.on("/add_intersection", HTTP_OPTIONS, allowCORS("POST"));
  web.on("/remove_intersection", HTTP_POST, guardMutatingRoute(handleRemoveIntersection));
  web.on("/remove_intersection", HTTP_OPTIONS, allowCORS("POST"));
}

