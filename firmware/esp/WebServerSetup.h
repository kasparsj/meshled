#pragma once

#include <ArduinoJson.h>

// todo: gist does not work because of CORS
#define SCRIPTS_FALLBACK "//gist.githubusercontent.com/kasparsj/e297e6ffff5a4d4aca3657912a17f993/raw/scripts.js"

#define WEBSERVER_CONFIG
#define WEBSERVER_EMITTER
#if defined(SPIFFS_ENABLED) and defined(DEV_ENABLED)
#define SPIFFS_UPLOAD
#define SPIFFS_DELETE
//#define SPIFFS_FORMAT
#endif
#define WEBSERVER_SPIFFS defined(SPIFFS_UPLOAD) or defined(SPIFFS_DELETE) or defined(SPIFFS_FORMAT)

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

#ifdef SPIFFS_UPLOAD
#include "WebServerSpiffs.h"
#endif

String getHeaderMenu() {
  String html = "<p>";
  #ifdef WEBSERVER_EMITTER
  html += "<a href='/emitter'>Emitter</a> | ";
  #endif
  #ifdef WEBSERVER_CONFIG
  html += "<a href='/settings'>Settings</a> | ";
  #endif
  #ifdef WEBSERVER_SPIFFS
  html += "<a href='/spiffs' style='color: #00cc00;'>SPIFFS</a>";
  #endif
  html += "</p>";
  return html;
}

void streamHeader(WiFiClient &client, String title, bool includeScripts = true) {
  client.printf("<html><head><title>%s - %s</title>", title.c_str(), deviceHostname.c_str());
  client.println("<meta name='viewport' content='width=device-width, initial-scale=1'>");

  if (includeScripts) {
    // Check if scripts.js exists in SPIFFS, use fallback if not
    #ifdef SPIFFS_ENABLED
    if (SPIFFS.exists("/scripts.js")) {
      client.println("<script src='/scripts.js'></script>");
    } else {
      client.printf("<script src='%s'></script>", SCRIPTS_FALLBACK);
    }
    #else
    // If SPIFFS is not enabled, always use the fallback
    client.printf("<script src='%s'></script>", SCRIPTS_FALLBACK);
    #endif
  }

  client.println("<style>body{font-family: Arial; margin: 20px; background: #333; color: #fff;}");
  client.println(".container{max-width: 600px; margin: 0 auto; background: #444; padding: 20px; border-radius: 5px;}");
  client.println("input, select{width: 100%; padding: 8px; margin: 8px 0; box-sizing: border-box;}");
  client.println("input[type='color'] {width: 50px; padding: inherit;}");
  client.println("label{display: block; margin-top: 10px;} button{background: #0066ff; color: white; border: none; padding: 10px; cursor: pointer; margin-top: 20px; width: 100%;}");
  client.println("button:hover{background: #0044cc;} .value{color: #0066ff;}</style></head>");
  client.printf("<body><div class='container'><h1>%s - %s</h1>", title.c_str(), deviceHostname.c_str());
  client.println(getHeaderMenu());
}

void streamConfigDisplay(WiFiClient &client) {
  // Only yield periodically to prevent watchdog reset but minimize lag
  unsigned long lastYield = millis();
  #define YIELD_IF_NEEDED() if (millis() - lastYield > 20) { yield(); lastYield = millis(); }

  client.print("<div>");

  client.print("<p>WiFi SSID: <span class='value'>");
  client.print(savedSSID);
  client.print("</span></p>");
  YIELD_IF_NEEDED();

  client.print("<p>IP Address: <span class='value'>");
  client.print(WiFi.localIP().toString());
  client.print("</span></p>");
  YIELD_IF_NEEDED();

  client.print("<p>Total consumption: <span class='value'>");
  client.print(String(totalWattage, 2));
  client.print(" watts</span></p>");
  YIELD_IF_NEEDED();

  // Add file system information
  #ifdef SPIFFS_ENABLED
  size_t totalBytes = SPIFFS.totalBytes();
  size_t usedBytes = SPIFFS.usedBytes();
  float usedPercent = (float)usedBytes / totalBytes * 100;

  client.print("<p>Storage: <span class='value'>");
  client.print(String(usedBytes / 1024));
  client.print(" KB used of ");
  client.print(String(totalBytes / 1024));
  client.print(" KB (");
  client.print(String(usedPercent, 1));
  client.print("%)</span></p>");
  YIELD_IF_NEEDED();

  #ifdef LOG_FILE
  // Show log file sizes if they exist
  if (SPIFFS.exists(LOG_FILE)) {
    File f = SPIFFS.open(LOG_FILE, FILE_READ);
    size_t logSize = f.size();
    f.close();
    client.print("<p>Log file: <span class='value'>");
    client.print(String(logSize / 1024.0, 1));
    client.print(" KB</span></p>");
    YIELD_IF_NEEDED();
  }
  #endif

  #ifdef CRASH_LOG_FILE
  if (SPIFFS.exists(CRASH_LOG_FILE)) {
    File f = SPIFFS.open(CRASH_LOG_FILE, FILE_READ);
    size_t crashLogSize = f.size();
    f.close();
    client.print("<p>Crash log: <span class='value'>");
    client.print(String(crashLogSize / 1024.0, 1));
    client.print(" KB</span></p>");
    YIELD_IF_NEEDED();
  }
  #endif
  #endif

  if (apMode) {
    client.print("<p><strong>Device is in Access Point mode.</strong> Connect to WiFi network: <span class='value'>");
    client.print(String(AP_SSID));
    client.print("</span></p>");
    unsigned long timeLeft = (AP_TIMEOUT - (millis() - apStartTime)) / 1000;
    client.print("<p>The device will restart in <span class='value'>");
    client.print(String(timeLeft));
    client.print("</span> seconds.</p>");
    YIELD_IF_NEEDED();
  }

  client.print("<p>Current state:</p><ul>");
  client.print("<li>Active Lights: <span class='value'>");
  client.print(String(state->totalLights));
  client.print(" (");
  client.print(String(state->totalLightLists));
  client.print(")</span></li>");
  YIELD_IF_NEEDED();

  client.print("<li>Free Memory: <span class='value'>");
  client.print(String(ESP.getFreeHeap() / 1024));
  client.print(" KB</span></li>");
  YIELD_IF_NEEDED();

  #ifdef DEBUGGER_ENABLED
  client.print("<li>FPS: <span class='value'>");
  client.print(String(debugger->getFPS(), 1));
  client.print("</span></li>");

  client.print("<li>Emits per frame: <span class='value'>");
  client.print(String(debugger->getNumEmits(), 2));
  client.print("</span></li>");
  YIELD_IF_NEEDED();
  #endif
  client.print("</ul></div>");
}

String getFooterMenu() {
  String menu = "";
  menu += "<p><a href='/wifi'>Configure WiFi</a> | <a href='/restart'>Restart Device</a></p>";
  return menu;
}

void streamFooter(WiFiClient &client) {
  client.print(getFooterMenu());
  streamConfigDisplay(client);
  client.print("</div></body></html>");
}

// Stream the homepage HTML directly to client to save memory
void streamHomePage(WiFiClient &client) {
  streamHeader(client, "Layers");

  yield(); // Allow system to process other tasks

  #if defined(WEBSERVER_EMITTER)
  streamEmitter(client);
  #elif defined(WEBSERVER_CONFIG)
  streamSettings(client);
  #endif

  yield();

  // Use our optimized footer function that includes menu and config display
  streamFooter(client);
}

#ifdef WEBSERVER_EMITTER
void streamEmitterPage(WiFiClient &client) {
  streamHeader(client, "Emitter");

  streamEmitter(client);

  streamFooter(client);
}
#endif

void lowMemory() {
  // Send a simplified page if memory is low
  server.send(200, "text/html",
    "<html><body style='font-family:Arial;background:#333;color:#fff;text-align:center'>"
    "<h1>Low Memory Warning</h1>"
    "<p>System memory is low (" + String(ESP.getFreeHeap() / 1024) + " KB available). "
    "Try reducing the number of active lights or restart the device.</p>"
    "<p><a href='/restart' style='color:#0066ff'>Restart Device</a></p>"
    "</body></html>");
}

// Generic page handler to reduce code duplication
void handleGenericPage(void (*streamPageFunc)(WiFiClient&)) {
  if (ESP.getFreeHeap() < 10000) {
    lowMemory();
    return;
  }

  WiFiClient client = server.client();

  if (!client) {
    server.send(500, "text/plain", "Client connection error");
    return;
  }

  // Send HTTP headers
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();

  // Stream the page content directly to the client
  streamPageFunc(client);

  // Allow time for data to be sent
  delay(1);
}

void handleRoot() {
  handleGenericPage(streamHomePage);
}

#ifdef WEBSERVER_EMITTER
void handleEmitter() {
  handleGenericPage(streamEmitterPage);
}
#endif

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
    if (!(inter1->group & LPObject::groupMaskForIndex(g)) &&
        !(inter2->group & LPObject::groupMaskForIndex(g))) {
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
  
  // Count used ports by checking all connections
  for (uint8_t g = 0; g < MAX_GROUPS; g++) {
    for (Connection* conn : object->conn[g]) {
      if (conn && (conn->from == intersection || conn->to == intersection)) {
        usedPorts++;
      }
    }
  }
  
  return usedPorts < intersection->numPorts;
}

// Helper function to get group index from group mask
uint8_t getGroupIndex(uint8_t group) {
  for (uint8_t i = 0; i < MAX_GROUPS; i++) {
    if (group & LPObject::groupMaskForIndex(i)) {
      return i;
    }
  }
  return MAX_GROUPS; // Invalid
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
  
  if (group >= MAX_GROUPS) {
    server.send(400, "application/json", "{\"error\":\"group must be less than MAX_GROUPS\"}");
    return;
  }
  
  // Get group index for validation
  uint8_t groupIndex = 0;
  for (uint8_t i = 0; i < MAX_GROUPS; i++) {
    if (group & LPObject::groupMaskForIndex(i)) {
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
    if (object->models[m] && object->models[m]->weights) {
      for (uint8_t p = 0; p < intersection->numPorts; p++) {
        if (intersection->ports[p]) {
          uint8_t portId = intersection->ports[p]->id;
          
          // Remove this port's weights
          Weight* weight = object->models[m]->weights->get(portId);
          if (weight) {
            delete weight;
            object->models[m]->weights->remove(portId);
          }
          
          // Remove references to this port from other ports' weights
          for (uint8_t otherId = 0; otherId < 255; otherId++) {
            Weight* otherWeight = object->models[m]->weights->get(otherId);
            if (otherWeight) {
              otherWeight->remove(intersection->ports[p]);
            }
          }
        }
      }
    }
  }
}

// Helper function to remove connections involving an intersection
void removeConnections(Intersection* intersection) {
  if (!intersection) return;
  
  // Remove connections that involve this intersection
  for (uint8_t g = 0; g < MAX_GROUPS; g++) {
    size_t i = 0;
    while (i < object->conn[g].size()) {
      Connection* connection = object->conn[g][i];
      if (connection && (connection->from == intersection || connection->to == intersection)) {
        object->removeConnection(g, i);
      } else {
        i++;
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
  
  uint8_t intersectionId = doc["id"];
  uint8_t group = doc["group"];
  
  // Find and remove the intersection
  bool found = false;
  if (group < MAX_GROUPS) {
    auto& intersections = object->inter[group];
    for (auto it = intersections.begin(); it != intersections.end(); ++it) {
      if (*it && (*it)->id == intersectionId) {
        Intersection* intersection = *it;
        
        // Remove related model weights
        cleanupModelWeights(intersection);
        
        // Remove connections involving this intersection
        removeConnections(intersection);
        
        // Delete the intersection and remove from vector
        delete intersection;
        intersections.erase(it);
        
        found = true;
        break;
      }
    }
  }
  
  if (!found) {
    server.send(404, "application/json", "{\"error\":\"Intersection not found\"}");
    return;
  }
  
  // Recalculate connections for affected models
  recalculateConnections();
  
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
    specificLayer = server.arg("layer").toInt();

    // Store original visibility states
    if (state) {
      for (uint8_t i = 0; i < MAX_LIGHT_LISTS; i++) {
        if (state->lightLists[i]) {
          originalVisibility.push_back(state->lightLists[i]->visible);
          // Disable all layers except the requested one
          if (i != specificLayer) {
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
    }

    // Force a redraw to apply visibility changes
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
    if (specificLayer > -1) {
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
  
  // Basic object information
  client.printf("\"pixelCount\":%d,", object->pixelCount);
  client.printf("\"realPixelCount\":%d,", object->realPixelCount);
  client.printf("\"modelCount\":%d,", (int)object->models.size());
  client.printf("\"gapCount\":%d,", (int)object->gaps.size());
  
  // Intersections
  client.print("\"intersections\":[");
  bool firstInter = true;
  for (uint8_t group = 0; group < MAX_GROUPS; group++) {
    for (Intersection* intersection : object->inter[group]) {
      if (intersection) {
        if (!firstInter) client.print(",");
        client.printf("{\"id\":%d,\"group\":%d,\"numPorts\":%d,\"topPixel\":%d,\"bottomPixel\":%d,\"ports\":[",
                     intersection->id,
                     group,
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
                     group,
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
#endif

#ifdef DEBUGGER_ENABLED
// Handler for State::debug()
void handleStateDebug() {
  state->debug();

  // Redirect back to logs page
  server.sendHeader("Location", "/logs", true);
  server.send(302, "text/plain", "");
}

// Handler for LPDebugger::dumpConnections()
void handleDumpConnections() {
  #ifdef DEBUGGER_ENABLED
  debugger->dumpConnections();
  #endif

  // Redirect back to logs page
  server.sendHeader("Location", "/logs", true);
  server.send(302, "text/plain", "");
}

// Handler for LPDebugger::dumpIntersections()
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

#ifdef SPIFFS_UPLOAD
// Handler for the file upload page
void handleUploadPage() {
  #ifdef SPIFFS_ENABLED
  WiFiClient client = server.client();

  if (!client) {
    server.send(500, "text/plain", "Client connection error");
    return;
  }

  // Send HTTP headers
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();

  // Don't include scripts in this page to avoid any interference with showing script file content
  streamHeader(client, "SPIFFS", false);

  streamSpiffs(client);

  streamFooter(client);
  #else
  server.send(501, "text/plain", "SPIFFS not enabled");
  #endif
}
#endif

void handleGetDevices() {
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

#ifdef SPIFFS_ENABLED
// Serve static files from SPIFFS
bool handleStaticFile(String path) {
  if (SPIFFS.exists(path)) {
    String contentType = "text/plain";
    if (path.endsWith(".html")) contentType = "text/html";
    else if (path.endsWith(".css")) contentType = "text/css";
    else if (path.endsWith(".js")) contentType = "application/javascript";
    else if (path.endsWith(".json")) contentType = "application/json";
    else if (path.endsWith(".png")) contentType = "image/png";
    else if (path.endsWith(".jpg")) contentType = "image/jpeg";

    File file = SPIFFS.open(path, "r");
    if (file) {
      server.streamFile(file, contentType);
      file.close();
      return true;
    }
  }
  return false;
}

// Handler for serving static files
void handleStatic() {
  String path = "/";

  if (server.uri() != "/") {
    path += server.uri().substring(1);
  } else {
    return; // Root path is handled separately
  }

  if (handleStaticFile(path)) {
    return;
  }

  // If file not found
  server.send(404, "text/plain", "File not found");
}
#endif

// Global CORS handler for OPTIONS requests
void handleCORS() {
  sendCORSHeaders();
  server.sendHeader("Access-Control-Max-Age", "86400");
  server.send(200, "text/plain", "");
}

typedef void (*Cors)();
static String corsMethods;
Cors allowCORS(String methods) {
  corsMethods = methods;
  return []() {
    sendCORSHeaders(corsMethods);
    server.sendHeader("Access-Control-Max-Age", "86400");
    server.send(200, "text/plain", "");
  };
}

void setupWebServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/get_devices", HTTP_GET, handleGetDevices);

  // Serve static files
  #ifdef SPIFFS_ENABLED
  server.on("/scripts.js", HTTP_GET, []() {
    handleStaticFile("/scripts.js");
  });
  #endif

  // LAYERS
  server.on("/get_layers", HTTP_GET, handleGetLayers);
  server.on("/get_layers", HTTP_OPTIONS, allowCORS("GET"));
  server.on("/toggle_visible", HTTP_GET, handleToggleVisible);
  server.on("/toggle_visible", HTTP_OPTIONS, allowCORS("GET"));
  server.on("/update_palette", HTTP_ANY, handleUpdatePalette);
  server.on("/update_palette", HTTP_OPTIONS, handleCORS);
  server.on("/update_layer_brightness", HTTP_GET, handleUpdateLayerBrightness);
  server.on("/update_layer_brightness", HTTP_OPTIONS, allowCORS("GET"));
  server.on("/get_palette_colors", HTTP_GET, handleGetPaletteColors);
  server.on("/get_palette_colors", HTTP_OPTIONS, allowCORS("GET"));
  server.on("/save_palette", HTTP_POST, handleSavePalette);
  server.on("/save_palette", HTTP_OPTIONS, allowCORS("POST"));
  server.on("/add_layer", HTTP_POST, handleAddLayer);
  server.on("/add_layer", HTTP_OPTIONS, allowCORS("POST"));
  server.on("/remove_layer", HTTP_POST, handleRemoveLayer);
  server.on("/remove_layer", HTTP_OPTIONS, allowCORS("POST"));
  server.on("/update_blend_mode", HTTP_GET, handleUpdateBlendMode);
  server.on("/update_speed", HTTP_GET, handleUpdateSpeed);
  server.on("/update_speed", HTTP_OPTIONS, allowCORS("GET"));
  server.on("/update_ease", HTTP_GET, handleUpdateEase);
  server.on("/update_ease", HTTP_OPTIONS, allowCORS("GET"));
  server.on("/update_fade_speed", HTTP_GET, handleUpdateFadeSpeed);
  server.on("/update_fade_speed", HTTP_OPTIONS, allowCORS("GET"));
  server.on("/update_behaviour_flags", HTTP_GET, handleUpdateBehaviourFlags);
  server.on("/update_behaviour_flags", HTTP_OPTIONS, allowCORS("GET"));
  server.on("/update_layer_offset", HTTP_GET, handleUpdateLayerOffset);
  server.on("/update_layer_offset", HTTP_OPTIONS, allowCORS("GET"));
  server.on("/reset_layer", HTTP_GET, handleResetLayer);
  server.on("/reset_layer", HTTP_OPTIONS, allowCORS("GET"));

  // PALETTES
  server.on("/delete_palette", HTTP_GET, handleDeletePalette);
  server.on("/delete_palette", HTTP_OPTIONS, allowCORS("GET"));
  server.on("/sync_palettes", HTTP_POST, handleSyncPalettes);
  server.on("/get_palettes", HTTP_GET, handleGetPalettes);
  server.on("/get_palettes", HTTP_OPTIONS, allowCORS("GET"));

  #ifdef WEBSERVER_EMITTER
  server.on("/emitter", HTTP_GET, handleEmitter);
  server.on("/toggle_auto", HTTP_GET, handleToggleAuto);
  server.on("/update_emitter_min_speed", HTTP_GET, handleUpdateEmitterMinSpeed);
  server.on("/update_emitter_max_speed", HTTP_GET, handleUpdateEmitterMaxSpeed);
  server.on("/update_emitter_min_dur", HTTP_GET, handleUpdateEmitterMinDuration);
  server.on("/update_emitter_max_dur", HTTP_GET, handleUpdateEmitterMaxDuration);
  server.on("/update_emitter_min_sat", HTTP_GET, handleUpdateEmitterMinSat);
  server.on("/update_emitter_max_sat", HTTP_GET, handleUpdateEmitterMaxSat);
  server.on("/update_emitter_min_val", HTTP_GET, handleUpdateEmitterMinVal);
  server.on("/update_emitter_max_val", HTTP_GET, handleUpdateEmitterMaxVal);
  server.on("/update_emitter_min_next", HTTP_GET, handleUpdateEmitterMinNext);
  server.on("/update_emitter_max_next", HTTP_GET, handleUpdateEmitterMaxNext);
  server.on("/update_emitter_from", HTTP_GET, handleUpdateEmitterFrom);
  #endif

  server.on("/get_settings", HTTP_GET, handleGetSettings);
  server.on("/get_settings", HTTP_OPTIONS, allowCORS("GET"));
  server.on("/update_settings", HTTP_POST, handleUpdateSettings);
  server.on("/update_settings", HTTP_OPTIONS, allowCORS("POST"));
  server.on("/restart", HTTP_GET, handleRestart);
  server.on("/restart", HTTP_OPTIONS, allowCORS("GET"));
  server.on("/update_wifi", HTTP_POST, handleUpdateWifi);
  server.on("/update_wifi", HTTP_OPTIONS, allowCORS("POST"));
  server.on("/update_brightness", HTTP_GET, handleUpdateBrightness);
  server.on("/update_brightness", HTTP_OPTIONS, allowCORS("GET"));

  #ifdef CRASH_LOG_FILE
  server.on("/trigger_crash", HTTP_GET, handleTriggerCrash);
  #endif

  #ifdef DEBUGGER_ENABLED
  server.on("/state_debug", HTTP_GET, handleStateDebug);
  server.on("/dump_connections", HTTP_GET, handleDumpConnections);
  server.on("/dump_intersections", HTTP_GET, handleDumpIntersections);
  #endif

  server.on("/get_colors", HTTP_GET, handleGetColors);
  server.on("/get_colors", HTTP_OPTIONS, allowCORS("GET"));
  server.on("/get_model", HTTP_GET, handleGetModel);
  server.on("/get_model", HTTP_OPTIONS, allowCORS("GET"));
  server.on("/add_intersection", HTTP_POST, handleAddIntersection);
  server.on("/add_intersection", HTTP_OPTIONS, allowCORS("POST"));
  server.on("/remove_intersection", HTTP_POST, handleRemoveIntersection);
  server.on("/remove_intersection", HTTP_OPTIONS, allowCORS("POST"));
  
  #ifdef SSDP_ENABLED
  setupSSDPService();
  server.on("/description.xml", HTTP_GET, handleDescriptionXML);
  server.on("/icon48.png", HTTP_GET, handleIcon);
  #endif

  #ifdef WLEDAPI_ENABLED
  server.on("/json", HTTP_ANY, handleWLEDJson);
  server.on("/json/info", HTTP_GET, handleWLEDInfo);
  server.on("/json/info", HTTP_OPTIONS, handleCORS);
  server.on("/device_info", HTTP_GET, handleWLEDInfo);
  server.on("/device_info", HTTP_OPTIONS, handleCORS);
  server.on("/json/state", HTTP_ANY, handleWLEDState);
  server.on("/json/si", HTTP_GET, handleWLEDSI);
  server.on("/on", HTTP_GET, handleWLEDOn);
  server.on("/off", HTTP_GET, handleWLEDOff);
  server.on("/version", HTTP_GET, handleWLEDVersion);
  server.on("/win", HTTP_GET, handleWLEDWin);
  #endif

  #ifdef SPIFFS_UPLOAD
  server.on("/spiffs", HTTP_GET, handleUploadPage);
  server.on("/spiffs", HTTP_POST, handleUploadComplete, handleFileUpload);
  #endif
  #ifdef SPIFFS_DELETE
  server.on("/delete", HTTP_GET, handleDeleteFile);
  #endif
  #ifdef SPIFFS_FORMAT
  server.on("/format-spiffs", HTTP_POST, handleFormatSpiffs);
  #endif

  server.begin();
  LP_LOGLN("Web server started");
}
