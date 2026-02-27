// Handler for deleting a user palette
void handleDeletePalette() {
  sendCORSHeaders("GET");

  // Make sure index is provided
  if (!server.hasArg("index")) {
    server.send(400, "application/json", "{\"error\":\"Palette index is required\"}");
    return;
  }

  // Get and validate the index
  size_t paletteIndex = server.arg("index").toInt();
  if (paletteIndex >= userPalettes.size()) {
    server.send(400, "application/json", "{\"error\":\"Invalid palette index\"}");
    return;
  }

  // Get the palette name for the response
  String paletteName = userPalettes[paletteIndex].name;

  #ifdef SPIFFS_ENABLED
  bool success = deletePalette(paletteIndex);
  #else
  bool success = false;
  #endif

  if (success) {
    // Return success response with palette name
    String jsonResponse = "{\"success\":true,\"message\":\"Palette '";
    jsonResponse += paletteName;
    jsonResponse += "' deleted successfully\"}";
    server.send(200, "application/json", jsonResponse);
  } else {
    server.send(500, "application/json", "{\"error\":\"Failed to delete palette\"}");
  }
}

UserPalette paletteFromJson(const JsonObject& paletteObj, const String& paletteName) {
  UserPalette newPalette;

  // Extract basic properties
  if (paletteObj.containsKey("name")) {
    newPalette.name = paletteObj["name"].as<String>();
  } else {
    newPalette.name = paletteName;
  }

  newPalette.colorRule = paletteObj.containsKey("colorRule") ? paletteObj["colorRule"].as<int8_t>() : -1;
  newPalette.interMode = paletteObj.containsKey("interMode") ? paletteObj["interMode"].as<int8_t>() : 1;
  newPalette.wrapMode = paletteObj.containsKey("wrapMode") ? paletteObj["wrapMode"].as<int8_t>() : 0;
  newPalette.segmentation = paletteObj.containsKey("segmentation") ? paletteObj["segmentation"].as<float>() : 0.0f;

  // Extract colors
  if (paletteObj.containsKey("colors") && paletteObj["colors"].is<JsonArray>()) {
    JsonArray colorsArray = paletteObj["colors"];

    for (JsonVariant colorVar : colorsArray) {
      String colorStr = colorVar.as<String>();

      // Remove # prefix if present
      if (colorStr.startsWith("#")) {
        colorStr = colorStr.substring(1);
      }

      // Convert hex string to integer
      int64_t colorValue = strtoll(colorStr.c_str(), NULL, 16);
      newPalette.colors.push_back(colorValue);
    }
  }

  // Extract positions
  if (paletteObj.containsKey("positions") && paletteObj["positions"].is<JsonArray>()) {
    JsonArray positionsArray = paletteObj["positions"];

    for (JsonVariant posVar : positionsArray) {
      float position = posVar.as<float>();
      newPalette.positions.push_back(position);
    }
  }

  // If positions array is missing or has wrong size, generate default positions
  if (newPalette.positions.size() != newPalette.colors.size()) {
    newPalette.positions.clear();
    for (size_t i = 0; i < newPalette.colors.size(); i++) {
      float pos = (newPalette.colors.size() == 1) ? 0.0f :
                  float(i) / float(newPalette.colors.size() - 1);
      newPalette.positions.push_back(pos);
    }
  }

  return newPalette;
}

void paletteToJson(JsonObject& paletteObj, const UserPalette& palette, bool fullDetails) {
  if (fullDetails) {
    paletteObj["name"] = palette.name;
    paletteObj["colorRule"] = palette.colorRule;
    paletteObj["interMode"] = palette.interMode;
    paletteObj["wrapMode"] = palette.wrapMode;
    paletteObj["segmentation"] = palette.segmentation;

    // Add colors array with hex values
    JsonArray colorsArray = paletteObj.createNestedArray("colors");
    for (const int64_t& color : palette.colors) {
      char hexBuffer[10];
      sprintf(hexBuffer, "#%06X", (uint32_t)color);
      colorsArray.add(hexBuffer);
    }

    // Add positions array
    JsonArray positionsArray = paletteObj.createNestedArray("positions");
    for (const float& position : palette.positions) {
      positionsArray.add(position);
    }
  } else {
    // Names-only response
    paletteObj["name"] = palette.name;
  }
}

void handleSyncPalettes() {
  if (server.hasArg("plain")) {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "POST, OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type");

    bool push = false;
    bool pull = false;
    if (server.hasArg("push")) {
      String pushArg = server.arg("push");
      push = (pushArg == "1" || pushArg == "true");
    }
    if (server.hasArg("pull")) {
      String pullArg = server.arg("pull");
      pull = (pullArg == "1" || pullArg == "true");
    }

    if (!push && !pull) {
      server.send(400, "application/json", "{\"error\":\"push or pull not set\"}");
      return;
    }

    String payload = server.arg("plain");
    DynamicJsonDocument doc(2048); // Adjust size as needed
    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
      LP_LOGF("Failed to parse JSON: %s\n", error.c_str());
      server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
      return;
    }

    if (doc.is<JsonArray>()) {
      JsonArray palettesArray = doc.as<JsonArray>();
      std::vector<UserPalette> palettesToReturn = userPalettes;

      // Response for the client
      DynamicJsonDocument responseDoc(1024);

      for (JsonObject paletteObj : palettesArray) {
        if (paletteObj.containsKey("name")) {
          String paletteName = paletteObj["name"].as<String>();

          bool savePalette = true;
          for (uint8_t i=0; i<palettesToReturn.size(); i++) {
            const UserPalette& existing = palettesToReturn[i];
            if (existing.name == paletteName) {
              savePalette = false;
              palettesToReturn.erase(palettesToReturn.begin() + i);
              break;
            }
          }

          if (savePalette && push) {
            updateUserPalette(paletteFromJson(paletteObj, paletteName));
          }
        }
      }

      JsonArray responseArray = responseDoc.to<JsonArray>();
      if (pull) {
        for (const UserPalette& toReturn : palettesToReturn) {
          JsonObject paletteObj = responseArray.createNestedObject();
          paletteToJson(paletteObj, toReturn, true);
        }
      }

      // Send response
      String jsonResponse;
      serializeJson(responseDoc, jsonResponse);
      server.send(200, "application/json", jsonResponse);
    } else {
      server.send(400, "application/json", "{\"error\":\"Expected JSON array\"}");
    }
  } else {
    server.send(400, "application/json", "{\"error\":\"Expected JSON payload\"}");
  }
}

// Handler for fetching user palettes as JSON
void handleGetPalettes() {
  sendCORSHeaders();

  // Check for the 'v' parameter - if true, return full palette objects
  // Default is to return just names
  bool fullDetails = false;
  if (server.hasArg("v")) {
    String verboseArg = server.arg("v");
    fullDetails = (verboseArg == "1" || verboseArg == "true");
  }

  // Check if a specific palette name is requested
  bool specificPalette = false;
  String requestedName = "";
  if (server.hasArg("name")) {
    specificPalette = true;
    requestedName = server.arg("name");
    // When a specific palette is requested, force full details
    fullDetails = true;
  }

  // Count how many palettes we'll include in the response
  int paletteCount = 0;
  if (specificPalette) {
    // Count matching palettes
    for (const UserPalette& palette : userPalettes) {
      if (palette.name == requestedName) {
        paletteCount = 1;
        break;
      }
    }
  } else {
    // All palettes
    paletteCount = userPalettes.size();
  }

  // If no matching palette found for specific request
  if (specificPalette && paletteCount == 0) {
    server.send(404, "application/json", "{\"error\":\"Palette not found\"}");
    return;
  }

  // Estimate JSON document size based on the response type
  size_t estimatedSize;

  if (fullDetails) {
    // Full details - need space for all palette properties
    estimatedSize = JSON_ARRAY_SIZE(paletteCount) +
                    paletteCount * (
                      JSON_OBJECT_SIZE(7) +  // 7 properties per palette
                      50 +                   // name string
                      JSON_ARRAY_SIZE(20) +  // color array (max 20 colors)
                      JSON_ARRAY_SIZE(20) +  // positions array (max 20 positions)
                      20 * 10 +              // estimated 10 bytes per color/position
                      100                    // buffer for other fields
                    );
  } else {
    // Names only - much smaller response
    estimatedSize = JSON_ARRAY_SIZE(paletteCount) +
                    paletteCount * (
                      JSON_OBJECT_SIZE(1) +  // name
                      50                     // name string
                    );
  }

  DynamicJsonDocument doc(estimatedSize);
  JsonArray palettesArray = doc.to<JsonArray>();

  // Process each palette
  for (size_t i = 0; i < userPalettes.size(); i++) {
    const UserPalette& palette = userPalettes[i];

    // Skip if not matching the requested name
    if (specificPalette && palette.name != requestedName) {
      continue;
    }

    JsonObject paletteObj = palettesArray.createNestedObject();
    paletteToJson(paletteObj, palette, fullDetails);
  }

  // Serialize and send the JSON response
  String jsonResponse;
  serializeJson(doc, jsonResponse);

  server.send(200, "application/json", jsonResponse);
}

// Handler for saving a user palette
void handleSavePalette() {
  sendCORSHeaders();

  // Check if the request is a POST
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
    return;
  }

  // Get the JSON body of the request
  String jsonBody = server.arg("plain");

  // Parse the JSON
  const size_t capacity = JSON_OBJECT_SIZE(7) + // Including segmentation
                         JSON_ARRAY_SIZE(32) + // Colors
                         JSON_ARRAY_SIZE(32) + // Positions
                         512; // Buffer for strings
  DynamicJsonDocument doc(capacity);

  DeserializationError error = deserializeJson(doc, jsonBody);

  if (error) {
    LP_LOGF("Failed to parse JSON: %s\n", error.c_str());
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }

  // Create a UserPalette from the JSON
  UserPalette palette;

  // Extract the palette name
  if (doc.containsKey("name")) {
    palette.name = doc["name"].as<String>();
  } else {
    server.send(400, "application/json", "{\"error\":\"Palette name is required\"}");
    return;
  }

  // Extract colors
  if (doc.containsKey("colors") && doc["colors"].is<JsonArray>()) {
    JsonArray colorsArray = doc["colors"].as<JsonArray>();
    for (JsonVariant colorVar : colorsArray) {
      // Colors might be in hex string format
      if (colorVar.is<const char*>()) {
        String colorStr = colorVar.as<String>();
        // Remove # prefix if present
        if (colorStr.startsWith("#")) {
          colorStr = colorStr.substring(1);
        }
        // Convert hex string to integer
        int64_t color = strtol(colorStr.c_str(), NULL, 16);
        palette.colors.push_back(color);
      }
    }
  }

  // Make sure we have at least one color
  if (palette.colors.empty()) {
    server.send(400, "application/json", "{\"error\":\"At least one color is required\"}");
    return;
  }

  // Extract positions
  if (doc.containsKey("positions") && doc["positions"].is<JsonArray>()) {
    JsonArray posArray = doc["positions"].as<JsonArray>();
    for (JsonVariant posVar : posArray) {
      if (posVar.is<float>()) {
        palette.positions.push_back(posVar.as<float>());
      }
    }
  }

  // Ensure positions match colors
  if (palette.positions.size() != palette.colors.size()) {
    // Generate default positions
    palette.positions.clear();
    for (size_t i = 0; i < palette.colors.size(); i++) {
      float pos = (palette.colors.size() == 1) ? 0.0f :
                  static_cast<float>(i) / static_cast<float>(palette.colors.size() - 1);
      palette.positions.push_back(pos);
    }
  }

  // Extract other properties
  palette.colorRule = doc.containsKey("colorRule") ? doc["colorRule"].as<int8_t>() : -1;
  palette.interMode = doc.containsKey("interMode") ? doc["interMode"].as<int8_t>() : 1; // Default to HSB
  palette.wrapMode = doc.containsKey("wrapMode") ? doc["wrapMode"].as<int8_t>() : 0;   // Default to clamp
  palette.segmentation = doc.containsKey("segmentation") ? doc["segmentation"].as<float>() : 0.0f;  // Default to 0

  bool success = updateUserPalette(palette);

  if (success) {
    server.send(200, "application/json", "{\"success\":true,\"message\":\"Palette saved successfully\"}");
  } else {
    server.send(500, "application/json", "{\"error\":\"Failed to save palette\"}");
  }
}
