#pragma once

#include <algorithm>
#include <ArduinoJson.h>
#include <functional>
#include "ExternalTransport.h"
#include "SecurityLib.h"
#include "WebServerValidation.h"

#ifdef OTA_ENABLED
#include "OTADiagnostics.h"
#endif

#define WEBSERVER_CONFIG
#define WEBSERVER_EMITTER

#ifndef DEV_ENABLED
#ifndef MESHLED_WEB_UI_CSS_URL
#define MESHLED_WEB_UI_CSS_URL "https://meshled-ui.pages.dev/assets/app.css"
#endif

#ifndef MESHLED_WEB_UI_JS_URL
#define MESHLED_WEB_UI_JS_URL "https://meshled-ui.pages.dev/assets/app.js"
#endif
#else
#ifndef MESHLED_WEB_UI_CSS_URL
#define MESHLED_WEB_UI_CSS_URL "https://kasparsj.github.io/meshled/control-panel-test/assets/app.css"
#endif

#ifndef MESHLED_WEB_UI_JS_URL
#define MESHLED_WEB_UI_JS_URL "https://kasparsj.github.io/meshled/control-panel-test/assets/app.js"
#endif
#endif

void sendCORSHeaders(String methods);

bool isApiRequestAuthorized() {
  if (!apiAuthEnabled) {
    return true;
  }

  String token = "";
  if (server.hasHeader("Authorization")) {
    token = server.header("Authorization");
    const String bearerPrefix = "Bearer ";
    if (token.startsWith(bearerPrefix)) {
      token = token.substring(bearerPrefix.length());
    }
  } else if (server.hasHeader("X-API-Token")) {
    token = server.header("X-API-Token");
  } else if (server.hasArg("token")) {
    token = server.arg("token");
  }

  return isApiAuthTokenAuthorized(token);
}

bool requireApiAuth() {
  if (isApiRequestAuthorized()) {
    return true;
  }
  sendCORSHeaders("GET, POST, PUT, DELETE");
  server.send(401, "application/json", "{\"error\":\"Unauthorized\"}");
  return false;
}

std::function<void(void)> guardMutatingRoute(void (*handler)()) {
  return [handler]() {
    if (!requireApiAuth()) {
      return;
    }
    handler();
  };
}

std::function<void(void)> guardProtectedRoute(void (*handler)()) {
  return [handler]() {
    if (!requireApiAuth()) {
      return;
    }
    handler();
  };
}

void sendCORSHeaders(String methods = "GET, POST, PUT, DELETE") {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", methods + ", OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type, Authorization, X-Requested-With");
}

#include "WebServerLayers.h"
#include "WebServerPalettes.h"

#ifdef WEBSERVER_CONFIG
#include "WebServerConfig.h"
#endif

#ifdef WEBSERVER_EMITTER
#include "WebServerEmitter.h"
#endif

#ifdef WLEDAPI_ENABLED
#include "WLEDApiLib.h"
#endif

#ifdef SSDP_ENABLED
#include "SSDPLib.h"
#endif

void sendRecoverySetupPage() {
  const String activeSSID = getActiveNetworkSSID();
  const unsigned long elapsed = millis() - apStartTime;
  const unsigned long apTimeoutMs = AP_TIMEOUT;
  const unsigned long remainingSeconds = (apMode && elapsed < apTimeoutMs) ? ((apTimeoutMs - elapsed) / 1000) : 0;

  String html;
  html.reserve(2800);
  html += "<!doctype html><html><head><meta charset='utf-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>MeshLED Setup</title>";
  html += "<style>";
  html += "body{font-family:Arial,sans-serif;margin:0;background:#101826;color:#e5e7eb;}";
  html += ".wrap{max-width:560px;margin:24px auto;padding:20px;background:#1f2937;border-radius:10px;}";
  html += "h1{margin:0 0 12px 0;font-size:24px;}p{line-height:1.4;}";
  html += "label{display:block;margin-top:12px;font-weight:600;}input{width:100%;box-sizing:border-box;padding:10px;margin-top:6px;border:1px solid #4b5563;border-radius:6px;background:#111827;color:#f9fafb;}";
  html += "button{width:100%;margin-top:14px;padding:10px;border:0;border-radius:6px;background:#2563eb;color:#fff;font-weight:600;cursor:pointer;}";
  html += "button.secondary{background:#4b5563;}";
  html += ".meta{font-size:13px;color:#9ca3af;margin-bottom:10px;}";
  html += "</style></head><body><div class='wrap'>";
  html += "<h1>MeshLED Recovery</h1>";
  html += "<p class='meta'>Device: ";
  html += deviceHostname;
  html += "</p>";

  if (apMode) {
    html += "<p>Access point mode is active. Connect to <strong>";
    html += activeSSID;
    html += "</strong> and submit WiFi credentials below.</p>";
    html += "<p class='meta'>AP timeout in ";
    html += String(remainingSeconds);
    html += " seconds</p>";
  } else {
    html += "<p>WiFi credentials can be updated here if connectivity changes.</p>";
    html += "<p class='meta'>Current network: ";
    html += activeSSID;
    html += "</p>";
  }

  html += "<form action='/update_wifi' method='POST'>";
  html += "<label for='ssid'>WiFi SSID</label>";
  html += "<input id='ssid' name='ssid' type='text' value='";
  html += savedSSID;
  html += "' required>";
  html += "<label for='password'>WiFi Password</label>";
  html += "<input id='password' name='password' type='password' placeholder='Enter WiFi password'>";
  html += "<button type='submit'>Save WiFi and Restart</button>";
  html += "</form>";
  html += "<form action='/restart' method='POST'>";
  html += "<button class='secondary' type='submit'>Restart Device</button>";
  html += "</form>";
  html += "</div></body></html>";

  server.send(200, "text/html; charset=utf-8", html);
}

void handleRoot() {
  if (apMode) {
    sendRecoverySetupPage();
    return;
  }

  String html;
  html.reserve(1600);
  html += "<!doctype html><html><head><meta charset='utf-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>MeshLED</title>";
  html += "<link rel='stylesheet' href='";
  html += MESHLED_WEB_UI_CSS_URL;
  html += "'>";
  html += "</head><body>";
  html += "<noscript>MeshLED requires JavaScript.</noscript>";
  html += "<div id='root'>Loading MeshLED control panel...</div>";
  html += "<script>";
  html += "window.__MESHLED_API_BASE=location.origin;";
  html += "window.__MESHLED_DEVICE_HOST='";
  html += deviceHostname;
  html += "';";
  html += "window.__MESHLED_AP_MODE=false;";
  html += "</script>";
  html += "<script defer src='";
  html += MESHLED_WEB_UI_JS_URL;
  html += "' onerror=\"var el=document.getElementById('root');if(el){el.innerHTML='Control panel failed to load. Open <a href=\\'/settings\\'>/settings</a> to recover WiFi, or rebuild firmware with a valid MESHLED_WEB_UI_JS_URL.';}\"></script>";
  html += "</body></html>";

  server.send(200, "text/html; charset=utf-8", html);
}

void handleSettingsPage() {
  sendRecoverySetupPage();
}

void handleWifiPage() {
  sendRecoverySetupPage();
}

// Helper function to check if intersections are already connected
bool areIntersectionsConnected(Intersection* inter1, Intersection* inter2) {
  for (uint8_t g = 0; g < MAX_GROUPS; g++) {
    for (Connection* conn : object->conn[g]) {
      if (conn && ((conn->from == inter1 && conn->to == inter2) || 
                   (conn->from == inter2 && conn->to == inter1))) {
        return true;
      }
    }
  }
  return false;
}

// Helper function to check if there are intersections between two intersections
bool hasIntersectionBetween(Intersection* inter1, Intersection* inter2) {
  if (!inter1 || !inter2) return false;
  
  uint16_t startPixel = inter1->topPixel;
  uint16_t endPixel = inter2->topPixel;
  
  // Ensure startPixel < endPixel
  if (startPixel > endPixel) {
    uint16_t temp = startPixel;
    startPixel = endPixel;
    endPixel = temp;
  }
  
  // Check if any other intersection exists between these two
  for (uint8_t g = 0; g < MAX_GROUPS; g++) {
    // Only check intersections in the same or compatible groups
    if (!(inter1->group & TopologyObject::groupMaskForIndex(g)) &&
        !(inter2->group & TopologyObject::groupMaskForIndex(g))) {
      continue;
    }
    
    for (Intersection* testInter : object->inter[g]) {
      if (!testInter || testInter == inter1 || testInter == inter2) continue;
      
      // Check if this intersection is between the two
      if (testInter->topPixel > startPixel && testInter->topPixel < endPixel) {
        // Also check bottomPixel if it exists
        bool isBlocking = true;
        
        // If both intersections have bottomPixels, check if the path is clear
        if (inter1->bottomPixel != -1 && inter2->bottomPixel != -1 && testInter->bottomPixel != -1) {
          uint16_t startBottom = inter1->bottomPixel;
          uint16_t endBottom = inter2->bottomPixel;
          if (startBottom > endBottom) {
            uint16_t temp = startBottom;
            startBottom = endBottom;
            endBottom = temp;
          }
          
          // If the blocking intersection's bottomPixel is not in the path, it's not blocking
          if (testInter->bottomPixel <= startBottom || testInter->bottomPixel >= endBottom) {
            isBlocking = false;
          }
        }
        
        if (isBlocking) {
          return true; // There is an intersection between them
        }
      }
    }
  }
  
  return false; // No intersections between them
}

// Helper function to check if intersection has available ports
bool hasAvailablePort(Intersection* intersection) {
  if (!intersection) return false;

  uint8_t usedPorts = 0;
  for (uint8_t i = 0; i < intersection->numPorts; i++) {
    if (intersection->ports[i] != nullptr) {
      usedPorts++;
    }
  }

  return usedPorts < intersection->numPorts;
}

// Helper function to get group index from group mask
uint8_t getGroupIndex(uint8_t group) {
  for (uint8_t i = 0; i < MAX_GROUPS; i++) {
    if (group & TopologyObject::groupMaskForIndex(i)) {
      return i;
    }
  }
  return MAX_GROUPS; // Invalid
}

Intersection* findIntersectionById(uint8_t intersectionId) {
  if (!object) {
    return nullptr;
  }
  for (uint8_t group = 0; group < MAX_GROUPS; group++) {
    for (Intersection* intersection : object->inter[group]) {
      if (intersection && intersection->id == intersectionId) {
        return intersection;
      }
    }
  }
  return nullptr;
}

bool parseMacAddress(const String& input, uint8_t out[6]) {
  String normalized = input;
  normalized.replace(":", "");
  normalized.replace("-", "");
  normalized.trim();
  if (normalized.length() != 12) {
    return false;
  }

  for (int i = 0; i < 6; i++) {
    const int idx = i * 2;
    char c1 = normalized.charAt(idx);
    char c2 = normalized.charAt(idx + 1);
    auto hexValue = [](char c) -> int {
      if (c >= '0' && c <= '9') return c - '0';
      if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
      if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
      return -1;
    };
    const int h1 = hexValue(c1);
    const int h2 = hexValue(c2);
    if (h1 < 0 || h2 < 0) {
      return false;
    }
    out[i] = static_cast<uint8_t>((h1 << 4) | h2);
  }
  return true;
}

// Helper function to check if group has space for new connection
// Note: Always returns true now since arrays auto-resize
bool hasSpaceForConnection(uint8_t groupIndex) {
  return groupIndex < MAX_GROUPS;  // Arrays will auto-resize as needed
}

// Helper function to recalculate connections
void recalculateConnections() {
  if (!object) return;
  
  // First pass: Remove connections that now have intersections between their endpoints
  // BUT preserve virtual connections (bridges with numLeds = 0)
  std::vector<std::pair<uint8_t, size_t>> connectionsToRemove;
  
  for (uint8_t g = 0; g < MAX_GROUPS; g++) {
    auto& connections = object->conn[g];
    for (size_t i = 0; i < connections.size(); i++) {
      Connection* conn = connections[i];
      if (!conn || !conn->from || !conn->to) continue;
      
      // Skip virtual connections (bridges) with numLeds = 0 - these should remain
      if (conn->numLeds == 0) continue;
      
      // Check if there are now intersections between the connection endpoints
      if (hasIntersectionBetween(conn->from, conn->to)) {
        connectionsToRemove.push_back({g, i});
        LP_LOGF("Removing physical connection from %d to %d (intersection now exists between them)\n", 
               conn->from->topPixel, conn->to->topPixel);
      }
    }
  }
  
  // Remove connections in reverse order to maintain indices
  for (auto it = connectionsToRemove.rbegin(); it != connectionsToRemove.rend(); ++it) {
    uint8_t group = it->first;
    size_t index = it->second;
    object->removeConnection(group, index);
  }
  
  // Second pass: Auto-connect intersections with no intersections in between
  for (uint8_t g1 = 0; g1 < MAX_GROUPS; g1++) {
    for (size_t i1 = 0; i1 < object->inter[g1].size(); i1++) {
      Intersection* inter1 = object->inter[g1][i1];
      if (!inter1) continue;
      
      for (uint8_t g2 = g1; g2 < MAX_GROUPS; g2++) {
        size_t startI2 = (g2 == g1) ? i1 + 1 : 0;
        for (size_t i2 = startI2; i2 < object->inter[g2].size(); i2++) {
          Intersection* inter2 = object->inter[g2][i2];
          if (!inter2) continue;
          
          // Only connect within same group or compatible groups
          if (inter1->group != inter2->group) continue;
          
          // Check if already connected
          if (areIntersectionsConnected(inter1, inter2)) continue;
          
          // Check if there are no intersections between them
          if (hasIntersectionBetween(inter1, inter2)) continue;
          
          // Check if both intersections have available ports
          if (hasAvailablePort(inter1) && hasAvailablePort(inter2)) {
            
            // Check if we have space for new connection
            uint8_t groupIndex = getGroupIndex(inter1->group);
            if (groupIndex < MAX_GROUPS && hasSpaceForConnection(groupIndex)) {
              
              // Calculate the number of LEDs between intersections
              uint16_t numLeds = abs((int)inter2->topPixel - (int)inter1->topPixel) - 1;
              
              // Create new connection
              Connection* newConn = new Connection(inter1, inter2, inter1->group, numLeds);
              object->addConnection(newConn);
              
              LP_LOGF("Auto-connected intersections %d (%d) and %d (%d) with %d LEDs\n", 
                     inter1->id, inter1->topPixel, inter2->id, inter2->topPixel, numLeds);
            }
          }
        }
      }
    }
  }
}

// Add intersection to the model
void handleAddIntersection() {
  sendCORSHeaders("POST");
  
  if (!object) {
    server.send(404, "application/json", "{\"error\":\"No model object available\"}");
    return;
  }
  
  // Parse JSON parameters
  String body = server.arg("plain");
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, body);
  
  if (error) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }
  
  // Required parameters
  if (!doc.containsKey("numPorts") || !doc.containsKey("topPixel") || !doc.containsKey("group")) {
    server.send(400, "application/json", "{\"error\":\"Missing required parameters: numPorts, topPixel, group\"}");
    return;
  }
  
  uint8_t numPorts = doc["numPorts"];
  uint16_t topPixel = doc["topPixel"];
  uint8_t group = doc["group"];
  int16_t bottomPixel = doc.containsKey("bottomPixel") ? (int16_t)doc["bottomPixel"] : -1;
  
  // Validate parameters
  if (numPorts != 2 && numPorts != 4) {
    server.send(400, "application/json", "{\"error\":\"numPorts must be 2 or 4\"}");
    return;
  }
  
  const uint8_t maxGroupMask = static_cast<uint8_t>((1u << MAX_GROUPS) - 1u);
  if (group == 0 || group > maxGroupMask || (group & (group - 1)) != 0) {
    server.send(400, "application/json", "{\"error\":\"group must be a single valid group bit\"}");
    return;
  }
  
  // Get group index for validation
  uint8_t groupIndex = 0;
  for (uint8_t i = 0; i < MAX_GROUPS; i++) {
    if (group & TopologyObject::groupMaskForIndex(i)) {
      groupIndex = i;
      break;
    }
  }
  
  // Create and add the intersection
  Intersection* intersection = new Intersection(numPorts, topPixel, bottomPixel, group);
  object->addIntersection(intersection);
  
  // Recalculate connections for affected models
  recalculateConnections();
  
  server.send(200, "application/json", "{\"success\":true,\"id\":" + String(intersection->id) + "}");
}

// Helper function to clean up model weights related to an intersection
void cleanupModelWeights(Intersection* intersection) {
  if (!intersection) return;
  
  // Remove weights for all ports of this intersection from all models
  for (uint8_t m = 0; m < object->models.size(); m++) {
    if (object->models[m]) {
      for (uint8_t p = 0; p < intersection->numPorts; p++) {
        if (intersection->ports[p]) {
          object->models[m]->removePort(intersection->ports[p]);
        }
      }
    }
  }
}

// Remove intersection from the model
void handleRemoveIntersection() {
  sendCORSHeaders("POST");
  
  if (!object) {
    server.send(404, "application/json", "{\"error\":\"No model object available\"}");
    return;
  }
  
  // Parse JSON parameters
  String body = server.arg("plain");
  DynamicJsonDocument doc(512);
  DeserializationError error = deserializeJson(doc, body);
  
  if (error) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }
  
  // Required parameters
  if (!doc.containsKey("id") || !doc.containsKey("group")) {
    server.send(400, "application/json", "{\"error\":\"Missing required parameters: id, group\"}");
    return;
  }
  
  const uint8_t intersectionId = doc["id"];
  const uint8_t requestedGroup = doc["group"];
  const uint8_t maxGroupMask = static_cast<uint8_t>((1u << MAX_GROUPS) - 1u);

  // Prefer group-index lookup to preserve compatibility with current UI payloads.
  Intersection* target = nullptr;
  if (requestedGroup < MAX_GROUPS) {
    for (Intersection* intersection : object->inter[requestedGroup]) {
      if (intersection && intersection->id == intersectionId) {
        target = intersection;
        break;
      }
    }
  }

  // Fallback: group bitmask lookup (canonical API form).
  if (!target && requestedGroup > 0 && requestedGroup <= maxGroupMask && (requestedGroup & (requestedGroup - 1)) == 0) {
    for (uint8_t g = 0; g < MAX_GROUPS; g++) {
      if (!(requestedGroup & TopologyObject::groupMaskForIndex(g))) {
        continue;
      }
      for (Intersection* intersection : object->inter[g]) {
        if (intersection && intersection->id == intersectionId) {
          target = intersection;
          break;
        }
      }
      if (target) {
        break;
      }
    }
  }

  // Fallback: direct match against stored group mask in case data is out of band.
  if (!target) {
    for (uint8_t g = 0; g < MAX_GROUPS && !target; g++) {
      for (Intersection* intersection : object->inter[g]) {
        if (intersection && intersection->id == intersectionId && intersection->group == requestedGroup) {
          target = intersection;
          break;
        }
      }
    }
  }
  
  if (!target) {
    server.send(404, "application/json", "{\"error\":\"Intersection not found\"}");
    return;
  }

  // Remove related model weights first, then delegate ownership-safe removal.
  cleanupModelWeights(target);
  if (!object->removeIntersection(target)) {
    server.send(500, "application/json", "{\"error\":\"Failed to remove intersection\"}");
    return;
  }
  
  // Recalculate connections for affected models
  recalculateConnections();
  
  server.send(200, "application/json", "{\"success\":true}");
}

void handleAddExternalPort() {
  sendCORSHeaders("POST");

  if (!object) {
    server.send(404, "application/json", "{\"error\":\"No model object available\"}");
    return;
  }

  String body = server.arg("plain");
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, body);
  if (error) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }

  if (!doc.containsKey("intersectionId") || !doc.containsKey("slotIndex") || !doc.containsKey("group") ||
      !doc.containsKey("deviceMac") || !doc.containsKey("targetPortId")) {
    server.send(400, "application/json",
                "{\"error\":\"Missing required parameters: intersectionId, slotIndex, group, deviceMac, targetPortId\"}");
    return;
  }

  long intersectionId = doc["intersectionId"];
  long slotIndex = doc["slotIndex"];
  long group = doc["group"];
  long targetPortId = doc["targetPortId"];
  const bool direction = doc.containsKey("direction") ? static_cast<bool>(doc["direction"]) : false;

  if (intersectionId < 0 || intersectionId > 255 || slotIndex < 0 || slotIndex > 255 ||
      group < 1 || group > 255 || targetPortId < 0 || targetPortId > 255) {
    server.send(400, "application/json", "{\"error\":\"Invalid numeric parameter range\"}");
    return;
  }

  const uint8_t maxGroupMask = static_cast<uint8_t>((1u << MAX_GROUPS) - 1u);
  if ((group & (group - 1)) != 0 || group > maxGroupMask) {
    server.send(400, "application/json", "{\"error\":\"group must be a single valid group bit\"}");
    return;
  }

  Intersection* intersection = findIntersectionById(static_cast<uint8_t>(intersectionId));
  if (!intersection) {
    server.send(404, "application/json", "{\"error\":\"Intersection not found\"}");
    return;
  }
  if (slotIndex >= static_cast<long>(intersection->numPorts)) {
    server.send(400, "application/json", "{\"error\":\"slotIndex out of range for intersection\"}");
    return;
  }
  if (intersection->ports[slotIndex] != nullptr) {
    server.send(400, "application/json", "{\"error\":\"Requested slot is already occupied\"}");
    return;
  }

  uint8_t deviceMac[6] = {0};
  if (!parseMacAddress(String(doc["deviceMac"].as<const char*>()), deviceMac)) {
    server.send(400, "application/json", "{\"error\":\"Invalid deviceMac format\"}");
    return;
  }

  ExternalPort* created = object->addExternalPort(intersection, static_cast<uint8_t>(slotIndex), direction,
                                                  static_cast<uint8_t>(group), deviceMac,
                                                  static_cast<uint8_t>(targetPortId));
  if (!created) {
    server.send(500, "application/json", "{\"error\":\"Failed to create external port\"}");
    return;
  }

  server.send(200, "application/json", "{\"success\":true,\"id\":" + String(created->id) + "}");
}

void handleUpdateExternalPort() {
  sendCORSHeaders("POST");

  if (!object) {
    server.send(404, "application/json", "{\"error\":\"No model object available\"}");
    return;
  }

  String body = server.arg("plain");
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, body);
  if (error) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }

  if (!doc.containsKey("portId")) {
    server.send(400, "application/json", "{\"error\":\"Missing required parameter: portId\"}");
    return;
  }

  long portId = doc["portId"];
  if (portId < 0 || portId > 255) {
    server.send(400, "application/json", "{\"error\":\"Invalid portId\"}");
    return;
  }

  Port* rawPort = Port::findById(static_cast<uint8_t>(portId));
  if (!rawPort || !rawPort->isExternal()) {
    server.send(404, "application/json", "{\"error\":\"External port not found\"}");
    return;
  }

  auto* port = static_cast<ExternalPort*>(rawPort);

  if (doc.containsKey("direction")) {
    port->direction = static_cast<bool>(doc["direction"]);
  }

  if (doc.containsKey("group")) {
    long group = doc["group"];
    const uint8_t maxGroupMask = static_cast<uint8_t>((1u << MAX_GROUPS) - 1u);
    if (group < 1 || group > maxGroupMask || (group & (group - 1)) != 0) {
      server.send(400, "application/json", "{\"error\":\"group must be a single valid group bit\"}");
      return;
    }
    port->group = static_cast<uint8_t>(group);
  }

  if (doc.containsKey("targetPortId")) {
    long targetPortId = doc["targetPortId"];
    if (targetPortId < 0 || targetPortId > 255) {
      server.send(400, "application/json", "{\"error\":\"Invalid targetPortId\"}");
      return;
    }
    port->targetId = static_cast<uint8_t>(targetPortId);
  }

  if (doc.containsKey("deviceMac")) {
    uint8_t deviceMac[6] = {0};
    if (!parseMacAddress(String(doc["deviceMac"].as<const char*>()), deviceMac)) {
      server.send(400, "application/json", "{\"error\":\"Invalid deviceMac format\"}");
      return;
    }
    for (uint8_t i = 0; i < 6; i++) {
      port->device[i] = deviceMac[i];
    }
  }

  server.send(200, "application/json", "{\"success\":true}");
}

void handleRemoveExternalPort() {
  sendCORSHeaders("POST");

  if (!object) {
    server.send(404, "application/json", "{\"error\":\"No model object available\"}");
    return;
  }

  String body = server.arg("plain");
  DynamicJsonDocument doc(512);
  DeserializationError error = deserializeJson(doc, body);
  if (error) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }

  if (!doc.containsKey("portId")) {
    server.send(400, "application/json", "{\"error\":\"Missing required parameter: portId\"}");
    return;
  }

  long portId = doc["portId"];
  if (portId < 0 || portId > 255) {
    server.send(400, "application/json", "{\"error\":\"Invalid portId\"}");
    return;
  }

  Port* port = Port::findById(static_cast<uint8_t>(portId));
  if (!port || !port->isExternal()) {
    server.send(404, "application/json", "{\"error\":\"External port not found\"}");
    return;
  }

  for (Model* model : object->models) {
    if (model) {
      model->removePort(port);
    }
  }

  if (!object->removeExternalPort(port)) {
    server.send(500, "application/json", "{\"error\":\"Failed to remove external port\"}");
    return;
  }

  server.send(200, "application/json", "{\"success\":true}");
}

// Get LED colors as JSON for visualization
void handleGetColors() {
  sendCORSHeaders("GET");

  // Use a stream-based approach to avoid building a large string in memory
  WiFiClient client = server.client();

  if (!client) {
    server.send(500, "text/plain", "Client connection error");
    return;
  }

  // Check if specific parameters are provided
  int maxColorParam = 0;
  int specificLayer = -1;
  bool toggledLayers = false;

  std::vector<bool> originalVisibility;
  BlendMode originalBlendMode;

  if (server.hasArg("maxColors")) {
    maxColorParam = server.arg("maxColors").toInt();
  }

  if (server.hasArg("layer")) {
    uint8_t requestedLayer = 0;
    if (!parseLayerArg("layer", requestedLayer)) {
      server.send(400, "application/json", "{\"error\":\"Invalid layer index\"}");
      return;
    }
    if (!state || !state->lightLists[requestedLayer]) {
      server.send(400, "application/json", "{\"error\":\"Invalid layer index\"}");
      return;
    }

    specificLayer = requestedLayer;

    // Store original visibility states.
    for (uint8_t i = 0; i < MAX_LIGHT_LISTS; i++) {
      if (state->lightLists[i]) {
        originalVisibility.push_back(state->lightLists[i]->visible);
        // Disable all layers except the requested one.
        if (i != requestedLayer) {
          toggledLayers = toggledLayers || state->lightLists[i]->visible;
          state->lightLists[i]->visible = false;
        } else {
          toggledLayers = toggledLayers || !state->lightLists[i]->visible;
          state->lightLists[i]->visible = true;
          originalBlendMode = state->lightLists[i]->blendMode;
          state->lightLists[i]->blendMode = BLEND_NORMAL;
        }
      } else {
        originalVisibility.push_back(false);
      }
    }

    // Force a redraw to apply visibility changes.
    if (toggledLayers) {
      state->update();
    }
  }

  // Send HTTP headers with CORS
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Access-Control-Allow-Origin: *");
  client.println("Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS");
  client.println("Access-Control-Allow-Headers: Content-Type, Authorization, X-Requested-With");
  client.println("Connection: close");
  client.println();

  // Start JSON object
  client.print("{\"colors\":[");

  // Reduce maximum colors when memory is low
  uint32_t freeHeap = ESP.getFreeHeap();
  int maxColors = maxColorParam > 0 ? maxColorParam : (freeHeap < 20000 ? 100 : (freeHeap < 40000 ? 200 : 300));

  int step = 1;
  uint16_t totalPixels = pixelCount1 + pixelCount2;
  if (totalPixels > maxColors) {
    step = (totalPixels + maxColors - 1) / maxColors; // Ceiling division
  }

  // Track total pixels added to enforce a hard limit
  int pixelsAdded = 0;

  #ifdef NEOPIXELBUS_ENABLED
  if (ledLibrary == LIB_NEOPIXELBUS && strip1 != NULL) {
    // Add colors from first strip
    for (uint16_t i = 0; i < pixelCount1 && pixelsAdded < maxColors; i += step) {
      if (pixelsAdded > 0) client.print(",");

      // Get the pixel color
      RgbwColor color = getNeoPixelColor(i);

      // Write JSON directly to client
      client.printf("{\"r\":%d,\"g\":%d,\"b\":%d,\"w\":%d}",
                   color.R, color.G, color.B, color.W);

      pixelsAdded++;

      // Yield to prevent watchdog timer from triggering
      if (pixelsAdded % 20 == 0) {
        yield();
      }
    }

    // Add colors from second strip if present
    if (pixelCount2 > 0 && strip2 != NULL) {
      for (uint16_t i = 0; i < pixelCount2 && pixelsAdded < maxColors; i += step) {
        client.print(",");

        // Get the pixel color
        RgbwColor color = getNeoPixelColor(pixelCount1 + i);

        // Write JSON directly to client
        client.printf("{\"r\":%d,\"g\":%d,\"b\":%d,\"w\":%d}",
                     color.R, color.G, color.B, color.W);

        pixelsAdded++;

        // Yield to prevent watchdog timer from triggering
        if (pixelsAdded % 20 == 0) {
          yield();
        }
      }
    }
  }
  #endif

  #ifdef FASTLED_ENABLED
  if (ledLibrary == LIB_FASTLED && leds1 != NULL) {
    // Add colors from first strip
    for (uint16_t i = 0; i < pixelCount1 && pixelsAdded < maxColors; i += step) {
      if (pixelsAdded > 0) client.print(",");

      // Write JSON directly to client
      client.printf("{\"r\":%d,\"g\":%d,\"b\":%d,\"w\":0}",
                   leds1[i].r, leds1[i].g, leds1[i].b);

      pixelsAdded++;

      // Yield to prevent watchdog timer from triggering
      if (pixelsAdded % 20 == 0) {
        yield();
      }
    }

    // Add colors from second strip if present
    if (pixelCount2 > 0 && leds2 != NULL) {
      for (uint16_t i = 0; i < pixelCount2 && pixelsAdded < maxColors; i += step) {
        client.print(",");

        // Write JSON directly to client
        client.printf("{\"r\":%d,\"g\":%d,\"b\":%d,\"w\":0}",
                     leds2[i].r, leds2[i].g, leds2[i].b);

        pixelsAdded++;

        // Yield to prevent watchdog timer from triggering
        if (pixelsAdded % 20 == 0) {
          yield();
        }
      }
    }
  }
  #endif

  // Close colors array and add step information
  client.print("],");
  client.printf("\"step\":%d,", step);
  client.printf("\"totalPixels\":%d", totalPixels);

  // Close the main JSON object
  client.println("}");

  // Allow time for data to be sent
  delay(1);

  // Restore original layer visibility if we modified it
  if (toggledLayers && state) {
    for (uint8_t i = 0; i < MAX_LIGHT_LISTS && i < originalVisibility.size(); i++) {
      if (state->lightLists[i]) {
        state->lightLists[i]->visible = originalVisibility[i];
      }
    }
    if (specificLayer > -1 && specificLayer < MAX_LIGHT_LISTS && state->lightLists[specificLayer]) {
      state->lightLists[specificLayer]->blendMode = originalBlendMode;
    }
    // Force a redraw to apply the restored visibility
    state->update();
  }
}

// Get LED model information as JSON
void handleGetModel() {
  sendCORSHeaders("GET");
  
  if (!object) {
    server.send(404, "application/json", "{\"error\":\"No model object available\"}");
    return;
  }

  // Use a stream-based approach to avoid building a large string in memory
  WiFiClient client = server.client();
  
  if (!client) {
    server.send(500, "text/plain", "Client connection error");
    return;
  }

  // Send HTTP headers with CORS
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Access-Control-Allow-Origin: *");
  client.println("Access-Control-Allow-Methods: GET, OPTIONS");
  client.println("Access-Control-Allow-Headers: Content-Type, Authorization, X-Requested-With");
  client.println("Connection: close");
  client.println();

  // Start JSON object
  client.print("{");
  client.print("\"schemaVersion\":2,");
  
  // Basic object information
  client.printf("\"pixelCount\":%d,", object->pixelCount);
  client.printf("\"realPixelCount\":%d,", object->realPixelCount);
  client.printf("\"modelCount\":%d,", (int)object->models.size());
  client.printf("\"gapCount\":%d,", (int)object->gaps.size());
  client.printf("\"capabilities\":{\"crossDevice\":{\"enabled\":%s,\"transport\":\"%s\",\"ready\":%s}},",
                isExternalTransportEnabled() ? "true" : "false",
                externalTransportName(),
                externalTransportIsReady() ? "true" : "false");
  
  // Intersections
  client.print("\"intersections\":[");
  bool firstInter = true;
  for (uint8_t group = 0; group < MAX_GROUPS; group++) {
    for (Intersection* intersection : object->inter[group]) {
      if (intersection) {
        if (!firstInter) client.print(",");
        client.printf("{\"id\":%d,\"group\":%d,\"numPorts\":%d,\"topPixel\":%d,\"bottomPixel\":%d,\"ports\":[",
                     intersection->id,
                     intersection->group,
                     intersection->numPorts,
                     intersection->topPixel,
                     intersection->bottomPixel);
        
        // Add port information
        for (uint8_t p = 0; p < intersection->numPorts; p++) {
          if (p > 0) client.print(",");
          const Port* port = intersection->ports[p];
          if (port) {
            if (port->isExternal()) {
              const ExternalPort* externalPort = static_cast<const ExternalPort*>(port);
              client.printf("{\"id\":%d,\"type\":\"external\",\"direction\":%s,\"group\":%d,\"device\":\"%02X:%02X:%02X:%02X:%02X:%02X\",\"targetId\":%d}",
                           externalPort->id,
                           externalPort->direction ? "true" : "false",
                           externalPort->group,
                           externalPort->device[0], externalPort->device[1], externalPort->device[2],
                           externalPort->device[3], externalPort->device[4], externalPort->device[5],
                           externalPort->targetId);
            } else {
              client.printf("{\"id\":%d,\"type\":\"internal\",\"direction\":%s,\"group\":%d}",
                           port->id,
                           port->direction ? "true" : "false",
                           port->group);
            }
          } else {
            client.print("null");
          }
        }
        client.print("]}");
        firstInter = false;
        yield(); // Prevent watchdog timeout
      }
    }
  }
  client.print("],");
  
  // Connections
  client.print("\"connections\":[");
  bool firstConn = true;
  for (uint8_t group = 0; group < MAX_GROUPS; group++) {
    for (Connection* connection : object->conn[group]) {
      if (connection) {
        if (!firstConn) client.print(",");
        client.printf("{\"group\":%d,\"fromPixel\":%d,\"toPixel\":%d,\"numLeds\":%d,\"pixelDir\":%d}",
                     connection->group,
                     connection->fromPixel,
                     connection->toPixel,
                     connection->numLeds,
                     connection->pixelDir);
        firstConn = false;
        yield(); // Prevent watchdog timeout
      }
    }
  }
  client.print("],");
  
  // Models
  client.print("\"models\":[");
  bool firstModel = true;
  for (Model* model : object->models) {
    if (!firstModel) client.print(",");
    if (model) {
      client.printf("{\"id\":%d,\"defaultW\":%d,\"emitGroups\":%d,\"maxLength\":%d}",
                   model->id,
                   model->defaultW,
                   model->emitGroups,
                   model->maxLength);
    } else {
      client.print("null");
    }
    firstModel = false;
    yield(); // Prevent watchdog timeout
  }
  client.print("],");
  
  // Gaps
  client.print("\"gaps\":[");
  bool firstGap = true;
  for (const PixelGap& gap : object->gaps) {
    if (!firstGap) client.print(",");
    client.printf("{\"fromPixel\":%d,\"toPixel\":%d}",
                 gap.fromPixel,
                 gap.toPixel);
    firstGap = false;
  }
  client.print("]");
  
  // Close the main JSON object
  client.println("}");
  
  // Allow time for data to be sent
  delay(1);
}

bool parseBoundedLong(JsonVariantConst value, long minValue, long maxValue, long& out) {
  const bool isNumber =
      value.is<int>() || value.is<long>() || value.is<unsigned int>() || value.is<unsigned long>();
  if (!isNumber) {
    return false;
  }
  const long parsed = value.as<long>();
  if (parsed < minValue || parsed > maxValue) {
    return false;
  }
  out = parsed;
  return true;
}

bool parseTopologySnapshotFromJson(JsonObjectConst root, TopologySnapshot& snapshot, String& error) {
  long schemaVersion = 0;
  if (!parseBoundedLong(root["schemaVersion"], 0, 255, schemaVersion)) {
    error = "Missing schemaVersion";
    return false;
  }
  if (schemaVersion != 2) {
    error = "Unsupported schemaVersion; expected 2";
    return false;
  }
  snapshot.schemaVersion = static_cast<uint8_t>(schemaVersion);

  long pixelCount = 0;
  if (!parseBoundedLong(root["pixelCount"], 1, 65535, pixelCount)) {
    error = "Invalid or missing pixelCount";
    return false;
  }
  snapshot.pixelCount = static_cast<uint16_t>(pixelCount);

  JsonArrayConst intersections = root["intersections"];
  if (intersections.isNull()) {
    error = "Missing intersections array";
    return false;
  }
  for (JsonObjectConst intersectionJson : intersections) {
    long id = 0;
    long numPorts = 0;
    long topPixel = 0;
    long group = 0;
    if (!parseBoundedLong(intersectionJson["id"], 0, 255, id) ||
        !parseBoundedLong(intersectionJson["numPorts"], 1, 8, numPorts) ||
        !parseBoundedLong(intersectionJson["topPixel"], 0, 65535, topPixel) ||
        !parseBoundedLong(intersectionJson["group"], 1, 255, group)) {
      error = "Invalid intersection entry";
      return false;
    }
    long bottomPixel = -1;
    if (!intersectionJson["bottomPixel"].isNull()) {
      if (!parseBoundedLong(intersectionJson["bottomPixel"], -1, 32767, bottomPixel)) {
        error = "Invalid intersection bottomPixel";
        return false;
      }
    }
    snapshot.intersections.push_back({
      static_cast<uint8_t>(id),
      static_cast<uint8_t>(numPorts),
      static_cast<uint16_t>(topPixel),
      static_cast<int16_t>(bottomPixel),
      static_cast<uint8_t>(group),
    });
  }

  JsonArrayConst connections = root["connections"];
  if (connections.isNull()) {
    error = "Missing connections array";
    return false;
  }
  for (JsonObjectConst connectionJson : connections) {
    long fromIntersectionId = 0;
    long toIntersectionId = 0;
    long group = 0;
    long numLeds = 0;
    if (!parseBoundedLong(connectionJson["fromIntersectionId"], 0, 255, fromIntersectionId) ||
        !parseBoundedLong(connectionJson["toIntersectionId"], 0, 255, toIntersectionId) ||
        !parseBoundedLong(connectionJson["group"], 1, 255, group) ||
        !parseBoundedLong(connectionJson["numLeds"], 0, 65535, numLeds)) {
      error = "Invalid connection entry";
      return false;
    }
    snapshot.connections.push_back({
      static_cast<uint8_t>(fromIntersectionId),
      static_cast<uint8_t>(toIntersectionId),
      static_cast<uint8_t>(group),
      static_cast<uint16_t>(numLeds),
    });
  }

  JsonArrayConst ports = root["ports"];
  if (ports.isNull()) {
    error = "Missing ports array";
    return false;
  }
  for (JsonObjectConst portJson : ports) {
    long id = 0;
    long intersectionId = 0;
    long slotIndex = 0;
    long group = 0;
    if (!parseBoundedLong(portJson["id"], 0, 255, id) ||
        !parseBoundedLong(portJson["intersectionId"], 0, 255, intersectionId) ||
        !parseBoundedLong(portJson["slotIndex"], 0, 255, slotIndex) ||
        !parseBoundedLong(portJson["group"], 1, 255, group)) {
      error = "Invalid port entry";
      return false;
    }

    String type = String(portJson["type"] | "internal");
    type.toLowerCase();
    const TopologyPortType portType =
        (type == "external") ? TopologyPortType::External : TopologyPortType::Internal;
    const bool direction = portJson["direction"].isNull() ? false : portJson["direction"].as<bool>();

    std::array<uint8_t, 6> deviceMac = {0, 0, 0, 0, 0, 0};
    long targetPortId = 0;
    if (portType == TopologyPortType::External) {
      if (!portJson.containsKey("deviceMac")) {
        error = "External port missing deviceMac";
        return false;
      }
      uint8_t parsedMac[6] = {0};
      if (!parseMacAddress(String(portJson["deviceMac"].as<const char*>()), parsedMac)) {
        error = "Invalid external port deviceMac";
        return false;
      }
      for (uint8_t i = 0; i < 6; i++) {
        deviceMac[i] = parsedMac[i];
      }
      if (!parseBoundedLong(portJson["targetPortId"], 0, 255, targetPortId)) {
        error = "Invalid external port targetPortId";
        return false;
      }
    }

    snapshot.ports.push_back({
      static_cast<uint8_t>(id),
      static_cast<uint8_t>(intersectionId),
      static_cast<uint8_t>(slotIndex),
      portType,
      direction,
      static_cast<uint8_t>(group),
      deviceMac,
      static_cast<uint8_t>(targetPortId),
    });
  }

  JsonArrayConst models = root["models"];
  if (!models.isNull()) {
    for (JsonObjectConst modelJson : models) {
      long id = 0;
      long defaultWeight = 0;
      long emitGroups = 0;
      long maxLength = 0;
      long routingStrategy = 0;
      if (!parseBoundedLong(modelJson["id"], 0, 255, id) ||
          !parseBoundedLong(modelJson["defaultWeight"], 0, 255, defaultWeight) ||
          !parseBoundedLong(modelJson["emitGroups"], 0, 255, emitGroups) ||
          !parseBoundedLong(modelJson["maxLength"], 0, 65535, maxLength)) {
        error = "Invalid model entry";
        return false;
      }
      if (!modelJson["routingStrategy"].isNull() &&
          !parseBoundedLong(modelJson["routingStrategy"], 0, 1, routingStrategy)) {
        error = "Invalid model routingStrategy";
        return false;
      }

      TopologyModelSnapshot modelSnapshot{
        static_cast<uint8_t>(id),
        static_cast<uint8_t>(defaultWeight),
        static_cast<uint8_t>(emitGroups),
        static_cast<uint16_t>(maxLength),
        static_cast<RoutingStrategy>(routingStrategy),
        {},
      };

      JsonArrayConst weights = modelJson["weights"];
      if (!weights.isNull()) {
        for (JsonObjectConst weightJson : weights) {
          long outgoingPortId = 0;
          long portDefaultWeight = 0;
          if (!parseBoundedLong(weightJson["outgoingPortId"], 0, 255, outgoingPortId) ||
              !parseBoundedLong(weightJson["defaultWeight"], 0, 255, portDefaultWeight)) {
            error = "Invalid model weight entry";
            return false;
          }
          TopologyPortWeightSnapshot weightSnapshot{
            static_cast<uint8_t>(outgoingPortId),
            static_cast<uint8_t>(portDefaultWeight),
            {},
          };

          JsonArrayConst conditionals = weightJson["conditionals"];
          if (!conditionals.isNull()) {
            for (JsonObjectConst conditionalJson : conditionals) {
              long incomingPortId = 0;
              long conditionalWeight = 0;
              if (!parseBoundedLong(conditionalJson["incomingPortId"], 0, 255, incomingPortId) ||
                  !parseBoundedLong(conditionalJson["weight"], 0, 255, conditionalWeight)) {
                error = "Invalid conditional model weight entry";
                return false;
              }
              weightSnapshot.conditionals.push_back({
                static_cast<uint8_t>(incomingPortId),
                static_cast<uint8_t>(conditionalWeight),
              });
            }
          }

          modelSnapshot.weights.push_back(weightSnapshot);
        }
      }

      snapshot.models.push_back(modelSnapshot);
    }
  }

  JsonArrayConst gaps = root["gaps"];
  if (!gaps.isNull()) {
    for (JsonObjectConst gapJson : gaps) {
      long fromPixel = 0;
      long toPixel = 0;
      if (!parseBoundedLong(gapJson["fromPixel"], 0, 65535, fromPixel) ||
          !parseBoundedLong(gapJson["toPixel"], 0, 65535, toPixel)) {
        error = "Invalid gap entry";
        return false;
      }
      snapshot.gaps.push_back({
        static_cast<uint16_t>(fromPixel),
        static_cast<uint16_t>(toPixel),
      });
    }
  }

  return true;
}

void streamTopologySnapshot(const TopologySnapshot& snapshot) {
  WiFiClient client = server.client();
  if (!client) {
    server.send(500, "text/plain", "Client connection error");
    return;
  }

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Access-Control-Allow-Origin: *");
  client.println("Access-Control-Allow-Methods: GET, OPTIONS");
  client.println("Access-Control-Allow-Headers: Content-Type, Authorization, X-Requested-With");
  client.println("Connection: close");
  client.println();

  client.printf("{\"schemaVersion\":%d,", snapshot.schemaVersion);
  client.printf("\"pixelCount\":%d,", snapshot.pixelCount);

  client.print("\"intersections\":[");
  for (size_t i = 0; i < snapshot.intersections.size(); i++) {
    if (i > 0) client.print(",");
    const TopologyIntersectionSnapshot& intersection = snapshot.intersections[i];
    client.printf(
        "{\"id\":%d,\"numPorts\":%d,\"topPixel\":%d,\"bottomPixel\":%d,\"group\":%d}",
        intersection.id,
        intersection.numPorts,
        intersection.topPixel,
        intersection.bottomPixel,
        intersection.group);
  }
  client.print("],");

  client.print("\"connections\":[");
  for (size_t i = 0; i < snapshot.connections.size(); i++) {
    if (i > 0) client.print(",");
    const TopologyConnectionSnapshot& connection = snapshot.connections[i];
    client.printf(
        "{\"fromIntersectionId\":%d,\"toIntersectionId\":%d,\"group\":%d,\"numLeds\":%d}",
        connection.fromIntersectionId,
        connection.toIntersectionId,
        connection.group,
        connection.numLeds);
  }
  client.print("],");

  client.print("\"models\":[");
  for (size_t i = 0; i < snapshot.models.size(); i++) {
    if (i > 0) client.print(",");
    const TopologyModelSnapshot& model = snapshot.models[i];
    client.printf(
        "{\"id\":%d,\"defaultWeight\":%d,\"emitGroups\":%d,\"maxLength\":%d,\"routingStrategy\":%d,\"weights\":[",
        model.id,
        model.defaultWeight,
        model.emitGroups,
        model.maxLength,
        static_cast<uint8_t>(model.routingStrategy));

    for (size_t j = 0; j < model.weights.size(); j++) {
      if (j > 0) client.print(",");
      const TopologyPortWeightSnapshot& weight = model.weights[j];
      client.printf(
          "{\"outgoingPortId\":%d,\"defaultWeight\":%d,\"conditionals\":[",
          weight.outgoingPortId,
          weight.defaultWeight);

      for (size_t k = 0; k < weight.conditionals.size(); k++) {
        if (k > 0) client.print(",");
        const TopologyWeightConditionalSnapshot& conditional = weight.conditionals[k];
        client.printf(
            "{\"incomingPortId\":%d,\"weight\":%d}",
            conditional.incomingPortId,
            conditional.weight);
      }
      client.print("]}");
    }
    client.print("]}");
  }
  client.print("],");

  client.print("\"ports\":[");
  for (size_t i = 0; i < snapshot.ports.size(); i++) {
    if (i > 0) client.print(",");
    const TopologyPortSnapshot& port = snapshot.ports[i];
    const bool isExternal = port.type == TopologyPortType::External;
    client.printf("{\"id\":%d,\"intersectionId\":%d,\"slotIndex\":%d,\"type\":\"%s\",\"direction\":%s,\"group\":%d",
                  port.id,
                  port.intersectionId,
                  port.slotIndex,
                  isExternal ? "external" : "internal",
                  port.direction ? "true" : "false",
                  port.group);
    if (isExternal) {
      client.printf(",\"deviceMac\":\"%02X:%02X:%02X:%02X:%02X:%02X\",\"targetPortId\":%d",
                    port.deviceMac[0], port.deviceMac[1], port.deviceMac[2],
                    port.deviceMac[3], port.deviceMac[4], port.deviceMac[5],
                    port.targetPortId);
    }
    client.print("}");
  }
  client.print("],");

  client.print("\"gaps\":[");
  for (size_t i = 0; i < snapshot.gaps.size(); i++) {
    if (i > 0) client.print(",");
    const PixelGap& gap = snapshot.gaps[i];
    client.printf("{\"fromPixel\":%d,\"toPixel\":%d}", gap.fromPixel, gap.toPixel);
  }
  client.print("]}");
}

void handleExportTopology() {
  sendCORSHeaders("GET");

  if (!object) {
    server.send(404, "application/json", "{\"error\":\"No model object available\"}");
    return;
  }

  streamTopologySnapshot(object->exportSnapshot());
}

void handleImportTopology() {
  sendCORSHeaders("POST");

  if (!object) {
    server.send(404, "application/json", "{\"error\":\"No model object available\"}");
    return;
  }

  String body = server.arg("plain");
  if (body.length() == 0) {
    server.send(400, "application/json", "{\"error\":\"Missing JSON payload\"}");
    return;
  }

  const size_t docCapacity = std::max<size_t>(8192, body.length() * 2);
  DynamicJsonDocument doc(docCapacity);
  const DeserializationError parseError = deserializeJson(doc, body);
  if (parseError) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }

  TopologySnapshot snapshot;
  String validationError;
  if (!parseTopologySnapshotFromJson(doc.as<JsonObjectConst>(), snapshot, validationError)) {
    server.send(400, "application/json", "{\"error\":\"" + validationError + "\"}");
    return;
  }

  if (!object->importSnapshot(snapshot, true)) {
    server.send(400, "application/json", "{\"error\":\"Topology import validation failed\"}");
    return;
  }

  if (state != nullptr) {
    delete state;
    state = nullptr;
  }
  state = new State(*object);
  State::autoParams.from = emitterFrom;
  state->autoEnabled = emitterEnabled;

  #ifdef DEBUGGER_ENABLED
  if (debugger != nullptr) {
    delete debugger;
  }
  debugger = new Debugger(*object);
  #endif

  server.send(
      200,
      "application/json",
      "{\"success\":true,\"intersectionCount\":" + String(snapshot.intersections.size()) +
          ",\"connectionCount\":" + String(snapshot.connections.size()) + "}");
}
#ifdef DEBUGGER_ENABLED
// Handler for State::debug()
void handleStateDebug() {
  state->debug();

  // Redirect back to logs page
  server.sendHeader("Location", "/logs", true);
  server.send(302, "text/plain", "");
}

// Handler for Debugger::dumpConnections()
void handleDumpConnections() {
  #ifdef DEBUGGER_ENABLED
  debugger->dumpConnections();
  #endif

  // Redirect back to logs page
  server.sendHeader("Location", "/logs", true);
  server.send(302, "text/plain", "");
}

// Handler for Debugger::dumpIntersections()
void handleDumpIntersections() {
  #ifdef DEBUGGER_ENABLED
  debugger->dumpIntersections();
  #endif

  // Redirect back to logs page
  server.sendHeader("Location", "/logs", true);
  server.send(302, "text/plain", "");
}
#endif

#ifdef CRASH_LOG_FILE
// Handle trigger test crash
void handleTriggerCrash() {
  // Send response before crashing
  server.send(200, "text/html", "<html><head><title>Testing Crash...</title><meta http-equiv='refresh' content='5;url=/' /></head><body><h1>Testing Crash Logging</h1><p>The system will crash for testing purposes and restart in a few seconds...</p></body></html>");

  // Delay to allow response to be sent
  delay(1000);

  // Trigger the crash
  triggerCrash();
}
#endif

// Simple icon handler for WLED app compatibility
void handleIcon() {
  // Unfortunately we don't have an icon, so just return 404
  // In a future update, you could embed a base64 encoded icon and return it
  server.send(404, "text/plain", "Icon not found");
}

void handleGetDevices() {
  sendCORSHeaders("GET");

  DynamicJsonDocument doc(1024);
  JsonArray devices = doc.to<JsonArray>();

  // Discover devices on the network
  #ifdef MDNS_ENABLED
  std::vector<IPAddress> deviceIPs = getDevices();
  LP_LOGF("Discovered %d potential devices for sync\n", deviceIPs.size());

  // Send palette list to each device
  for (const IPAddress& ip : deviceIPs) {
    String ipStr = ip.toString();

    // Skip our own IP
    if (ipStr == WiFi.localIP().toString()) {
      LP_LOGLN("Skipping our own IP address");
      continue;
    }

    devices.add(ip.toString());
  }
  #else
  LP_LOGLN("MDNS not enabled, cannot discover devices");
  // todo: add error to response
  #endif

  String jsonResponse;
  serializeJson(doc, jsonResponse);
  server.send(200, "application/json", jsonResponse);
}

#ifdef OTA_ENABLED
void handleOtaStatus() {
  sendCORSHeaders("GET");

  DynamicJsonDocument doc(2048);
  JsonObject root = doc.to<JsonObject>();
  root["meshledVersion"] = getResolvedMeshledVersion();
  root["meshledCommitSha"] = getResolvedMeshledCommitSha();
  root["meshledBuildSha"] = getResolvedMeshledReleaseSha();
  root["meshledReleaseSha"] = getResolvedMeshledReleaseSha();
  root["uptimeSec"] = millis() / 1000;
  root["freeHeap"] = ESP.getFreeHeap();
  root["sketchMD5"] = ESP.getSketchMD5();

#if MESHLED_HAS_RESET_REASON
  const esp_reset_reason_t resetReason = esp_reset_reason();
  root["resetReasonCode"] = static_cast<int>(resetReason);
  root["resetReason"] = meshledResetReasonToString(resetReason);
#endif

#if MESHLED_HAS_OTA_PARTITION_INFO
  const esp_partition_t* runningPartition = esp_ota_get_running_partition();
  if (runningPartition != nullptr) {
    root["runningPartition"] = runningPartition->label;
    root["runningPartitionAddress"] = runningPartition->address;
  }

  esp_ota_img_states_t runningState;
  if (runningPartition != nullptr &&
      esp_ota_get_state_partition(runningPartition, &runningState) == ESP_OK) {
    root["runningOtaState"] = meshledOtaImageStateToString(runningState);
  }
#endif

  JsonObject lastOta = root.createNestedObject("lastOta");
  if (!populateOtaStatus(lastOta)) {
    lastOta["stage"] = "missing";
    lastOta["detail"] = "no_ota_status_file";
  }

  String output;
  serializeJson(doc, output);
  server.send(200, "application/json", output);
}
#endif

// Global CORS handler for OPTIONS requests
void handleCORS() {
  sendCORSHeaders();
  server.sendHeader("Access-Control-Max-Age", "86400");
  server.send(200, "text/plain", "");
}

std::function<void(void)> allowCORS(const String& methods) {
  return [methods]() {
    sendCORSHeaders(methods);
    server.sendHeader("Access-Control-Max-Age", "86400");
    server.send(200, "text/plain", "");
  };
}

#include "WebRoutesBase.h"
#include "WebRoutesLayers.h"
#include "WebRoutesSettings.h"
#include "WebRoutesTopology.h"
#include "WebRoutesWLED.h"

void setupWebServer() {
  static const char* headerKeys[] = {"Authorization", "X-API-Token"};
  server.collectHeaders(headerKeys, 2);

  registerBaseRoutes(server, gCtx);
  registerLayerRoutes(server, gCtx);
  registerPaletteRoutes(server, gCtx);
  registerEmitterRoutes(server, gCtx);
  registerSettingsRoutes(server, gCtx);
  registerTopologyRoutes(server, gCtx);
  registerWledRoutes(server, gCtx);

  server.begin();
  LP_LOGLN("Web server started");
}
