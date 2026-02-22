#pragma once

bool setupFileSystem() {
  // Initialize SPIFFS
  if(!SPIFFS.begin(true)){
    LP_LOGLN("An Error has occurred while mounting SPIFFS");
    return false;
  }

  LP_LOGLN("SPIFFS mounted successfully");
  return true;
}

// Function to load settings from SPIFFS using ArduinoJson
void loadSettings() {
  if (!SPIFFS.exists("/settings.json")) {
    LP_LOGLN("No settings file found, using defaults");
    return;
  }

  File file = SPIFFS.open("/settings.json", "r");
  if (!file) {
    LP_LOGLN("Failed to open settings file for reading");
    return;
  }

  // Calculate JSON document size based on file size (with some extra capacity)
  size_t fileSize = file.size();
  size_t capacity = fileSize * 1.2;

  // Allocate memory for the document
  DynamicJsonDocument doc(capacity);

  // Parse the JSON
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error) {
    LP_LOGF("Failed to parse settings JSON: %s\n", error.c_str());
    return;
  }

  // Load settings with proper type handling
  #ifdef OSC_ENABLED
  oscEnabled = doc["osc_enabled"] | oscEnabled;
  oscPort = doc["osc_port"] | oscPort;
  #endif

  maxBrightness = doc["max_brightness"] | maxBrightness;

  // Load pixel counts with legacy format support
  if (doc.containsKey("pixel_count1")) {
    pixelCount1 = doc["pixel_count1"];
  } else if (doc.containsKey("pixel_count")) {
    // Legacy support for old settings format
    pixelCount1 = doc["pixel_count"];
  }

  pixelCount2 = doc["pixel_count2"] | pixelCount2;
  pixelPin1 = doc["pixel_pin1"] | pixelPin1;
  pixelPin2 = doc["pixel_pin2"] | pixelPin2;
  pixelDensity = doc["pixel_density"] | pixelDensity;
  ledType = doc["led_type"] | ledType;
  colorOrder = doc["color_order"] | colorOrder;
  ledLibrary = doc["led_library"] | ledLibrary;
  objectType = doc["object_type"] | objectType;

  // Load emitter settings with proper type handling
  emitterMinSpeed = doc["emitter_min_speed"] | emitterMinSpeed;
  emitterMaxSpeed = doc["emitter_max_speed"] | emitterMaxSpeed;
  emitterMinDur = doc["emitter_min_dur"] | emitterMinDur;
  emitterMaxDur = doc["emitter_max_dur"] | emitterMaxDur;
  emitterMinSat = doc["emitter_min_sat"] | emitterMinSat;
  emitterMaxSat = doc["emitter_max_sat"] | emitterMaxSat;
  emitterMinVal = doc["emitter_min_val"] | emitterMinVal;
  emitterMaxVal = doc["emitter_max_val"] | emitterMaxVal;
  emitterMinNext = doc["emitter_min_next"] | emitterMinNext;
  emitterMaxNext = doc["emitter_max_next"] | emitterMaxNext;
  emitterFrom = doc["emitter_from"] | emitterFrom;
  emitterEnabled = doc["emitter_enabled"] | emitterEnabled;

  // Only update if field exists to avoid overwriting with empty string
  if (doc.containsKey("hostname")) {
    deviceHostname = doc["hostname"].as<String>();
    if (deviceHostname.length() == 0) {
      deviceHostname = DEFAULT_HOSTNAME;
    }
  }

  LP_LOGLN("Settings loaded from SPIFFS using ArduinoJson");
}

// Helper function to process layers array data
void processLayersArray(JsonArray& layersArray) {
  for (JsonObject layerObj : layersArray) {
    if (!layerObj.containsKey("index")) continue;

    uint8_t index = layerObj["index"].as<uint8_t>();
    if (index >= MAX_LIGHT_LISTS) continue;
    if (!state->lightLists[index]) {
      state->setupBg(index);
    }

    // Set visibility
    if (layerObj.containsKey("visible")) {
      state->lightLists[index]->visible = layerObj["visible"].as<bool>();
    }

    // Set brightness
    if (layerObj.containsKey("brightness")) {
      uint8_t brightness = layerObj["brightness"].as<uint8_t>();
      state->lightLists[index]->minBri = brightness;
    }
    
    // Set blend mode
    if (layerObj.containsKey("blendMode")) {
      uint8_t blendMode = layerObj["blendMode"].as<uint8_t>();
      if (blendMode <= BLEND_PIN_LIGHT) {
        state->lightLists[index]->blendMode = static_cast<BlendMode>(blendMode);
      }
    }
    
    // Set speed
    if (layerObj.containsKey("speed")) {
      float speed = layerObj["speed"].as<float>();
      if (speed >= -10.0f && speed <= 10.0f) {
        // Save the current speed, will apply with the ease function
        state->lightLists[index]->speed = speed;
      }
    }
    
    // Set ease function
    if (layerObj.containsKey("ease")) {
      uint8_t ease = layerObj["ease"].as<uint8_t>();
      if (ease <= EASE_ELASTIC_INOUT) {
        // Use the setSpeed function to set both speed and ease
        float currentSpeed = state->lightLists[index]->speed;
        state->lightLists[index]->setSpeed(currentSpeed, ease);
      }
    }
    
    // Set fade speed
    if (layerObj.containsKey("fadeSpeed")) {
      uint8_t fadeSpeed = layerObj["fadeSpeed"].as<uint8_t>();
      uint8_t currentFadeThresh = state->lightLists[index]->fadeThresh;
      uint8_t currentFadeEase = state->lightLists[index]->fadeEaseIndex;
      state->lightLists[index]->setFade(fadeSpeed, currentFadeThresh, currentFadeEase);
    }
    
    // Set behaviour flags
    if (layerObj.containsKey("behaviourFlags")) {
      uint16_t behaviourFlags = layerObj["behaviourFlags"].as<uint16_t>();
      
      // Check if behaviour object exists
      if (!state->lightLists[index]->behaviour) {
        // Create a new behaviour object with the specified flags
        state->lightLists[index]->behaviour = new Behaviour(behaviourFlags);
      } else {
        // Update existing behaviour flags
        state->lightLists[index]->behaviour->flags = behaviourFlags;
      }
    }

    // Set offset
    if (layerObj.containsKey("offset")) {
      float offset = layerObj["offset"].as<float>();
      state->lightLists[index]->setOffset(offset);
    }

    // Set palette for this layer
    if (layerObj.containsKey("colors") && layerObj["colors"].is<JsonArray>()) {
      std::vector<int64_t> layerColors;
      std::vector<float> layerPositions;

      // Load colors
      JsonArray colorsArray = layerObj["colors"].as<JsonArray>();
      for (JsonVariant color : colorsArray) {
        layerColors.push_back(color.as<int64_t>());
      }

      // Load positions
      if (layerObj.containsKey("positions") && layerObj["positions"].is<JsonArray>()) {
        JsonArray positionsArray = layerObj["positions"].as<JsonArray>();
        for (JsonVariant pos : positionsArray) {
          layerPositions.push_back(pos.as<float>());
        }
      }

      // Ensure positions match colors
      if (layerPositions.size() != layerColors.size()) {
        layerPositions.clear();
        for (size_t i = 0; i < layerColors.size(); i++) {
          float pos = (layerColors.size() == 1) ? 0.0f :
                    static_cast<float>(i) / static_cast<float>(layerColors.size() - 1);
          layerPositions.push_back(pos);
        }
      }

      // Create palette
      if (layerColors.size() > 0) {
        Palette palette(layerColors, layerPositions);

        // Set color rule
        if (layerObj.containsKey("colorRule")) {
          palette.setColorRule(layerObj["colorRule"].as<int8_t>());
        }

        // Set interpolation mode
        if (layerObj.containsKey("interMode")) {
          palette.setInterMode(layerObj["interMode"].as<int8_t>());
        }

        // Set wrap mode
        if (layerObj.containsKey("wrapMode")) {
          palette.setWrapMode(layerObj["wrapMode"].as<int8_t>());
        }

        // Set segmentation
        if (layerObj.containsKey("segmentation")) {
          palette.setSegmentation(layerObj["segmentation"].as<float>());
        }

        // Set the palette on the light list
        state->lightLists[index]->setPalette(palette);
      }
    }
  }
}

void loadLayers() {
  // First try to load from dedicated layers.json file
  if (SPIFFS.exists("/layers.json")) {
    LP_LOGLN("Loading layers from layers.json");
    
    File file = SPIFFS.open("/layers.json", "r");
    if (!file) {
      LP_LOGLN("Failed to open layers.json file for reading");
      return;
    }

    // Calculate JSON document size based on file size (with some extra capacity)
    size_t fileSize = file.size();
    size_t capacity = fileSize * 1.2;

    // Allocate memory for the document
    DynamicJsonDocument doc(capacity);

    // Parse the JSON
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
      LP_LOGF("Failed to parse layers JSON: %s\n", error.c_str());
      return;
    }
    
    if (doc.containsKey("layers") && doc["layers"].is<JsonArray>() && state) {
      JsonArray layersArray = doc["layers"].as<JsonArray>();
      processLayersArray(layersArray);
      return;
    }
  }  
}

// Function to save settings to SPIFFS using ArduinoJson
void saveSettings() {
  // Calculate capacity for JsonDocument with support for multiple light lists
  // The rule of thumb: reserve 10 bytes per element + string lengths for object keys and values
  const size_t capacity = JSON_OBJECT_SIZE(30) +
                          JSON_ARRAY_SIZE(8) + // For bg_colors array (assuming max 8 colors)
                          JSON_ARRAY_SIZE(MAX_LIGHT_LISTS) + // For layers array
                          MAX_LIGHT_LISTS * JSON_OBJECT_SIZE(10) + // For each layer object
                          MAX_LIGHT_LISTS * JSON_ARRAY_SIZE(8) * 2 + // For colors and positions arrays per layer
                          1024; // Extra space for string values and other data

  DynamicJsonDocument doc(capacity);

  // Add settings to the JSON document
  #ifdef OSC_ENABLED
  doc["osc_enabled"] = oscEnabled;
  doc["osc_port"] = oscPort;
  #endif

  doc["max_brightness"] = maxBrightness;
  doc["pixel_count1"] = pixelCount1;
  doc["pixel_count2"] = pixelCount2;
  doc["pixel_pin1"] = pixelPin1;
  doc["pixel_pin2"] = pixelPin2;
  doc["pixel_density"] = pixelDensity;
  doc["led_type"] = ledType;
  doc["color_order"] = colorOrder;
  doc["led_library"] = ledLibrary;
  doc["object_type"] = objectType;

  // Emitter settings
  doc["emitter_min_speed"] = emitterMinSpeed;
  doc["emitter_max_speed"] = emitterMaxSpeed;
  doc["emitter_min_dur"] = emitterMinDur;
  doc["emitter_max_dur"] = emitterMaxDur;
  doc["emitter_min_sat"] = emitterMinSat;
  doc["emitter_max_sat"] = emitterMaxSat;
  doc["emitter_min_val"] = emitterMinVal;
  doc["emitter_max_val"] = emitterMaxVal;
  doc["emitter_min_next"] = emitterMinNext;
  doc["emitter_max_next"] = emitterMaxNext;
  doc["emitter_from"] = emitterFrom;
  doc["emitter_enabled"] = emitterEnabled;

  doc["hostname"] = deviceHostname;
  
  // Open the file for writing
  File file = SPIFFS.open("/settings.json", "w");
  if (!file) {
    LP_LOGLN("Failed to open settings file for writing");
    return;
  }

  // Serialize JSON to file
  if (serializeJson(doc, file) == 0) {
    LP_LOGLN("Failed to write settings to file");
  } else {
    LP_LOGLN("Settings saved using ArduinoJson");
  }

  file.close();
}

// Load all user palettes from SPIFFS
void loadUserPalettes() {
  userPalettes.clear();

  // Single file approach - all palettes in one JSON file
  if (!SPIFFS.exists("/user-palettes.json")) {
    LP_LOGLN("No user palettes file found");
    return;
  }

  File file = SPIFFS.open("/user-palettes.json", "r");
  if (!file) {
    LP_LOGLN("Failed to open user palettes file for reading");
    return;
  }

  // Calculate JSON document size based on file size (with extra capacity)
  size_t fileSize = file.size();
  size_t capacity = fileSize * 1.2;

  // Allocate memory for the document (with fallback if too large)
  DynamicJsonDocument doc(capacity > 16384 ? 16384 : capacity);

  // Parse the JSON file
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error) {
    LP_LOGF("Error parsing user palettes file: %s\n", error.c_str());
    return;
  }

  // Check if the file contains a palettes array
  if (!doc.containsKey("palettes") || !doc["palettes"].is<JsonArray>()) {
    LP_LOGLN("Invalid user palettes file format - missing palettes array");
    return;
  }

  // Process each palette in the array
  JsonArray palettesArray = doc["palettes"].as<JsonArray>();
  for (JsonObject paletteObj : palettesArray) {
    UserPalette palette;

    // Load palette name
    if (paletteObj.containsKey("name")) {
      palette.name = paletteObj["name"].as<String>();
    } else {
      // Skip palettes without names
      continue;
    }

    // Load colors
    if (paletteObj.containsKey("colors") && paletteObj["colors"].is<JsonArray>()) {
      JsonArray colorsArray = paletteObj["colors"].as<JsonArray>();
      for (JsonVariant colorVar : colorsArray) {
        // Check if color is in hex string format or integer
        if (colorVar.is<const char*>()) {
          String colorStr = colorVar.as<String>();
          // Remove # prefix if present
          if (colorStr.startsWith("#")) {
            colorStr = colorStr.substring(1);
          }
          // Convert hex string to integer
          int64_t color = strtol(colorStr.c_str(), NULL, 16);
          palette.colors.push_back(color);
        } else if (colorVar.is<int>() || colorVar.is<int64_t>()) {
          palette.colors.push_back(colorVar.as<int64_t>());
        }
      }
    }

    // Load positions
    if (paletteObj.containsKey("positions") && paletteObj["positions"].is<JsonArray>()) {
      JsonArray posArray = paletteObj["positions"].as<JsonArray>();
      for (JsonVariant posVar : posArray) {
        if (posVar.is<float>()) {
          palette.positions.push_back(posVar.as<float>());
        }
      }
    }

    // If positions don't match colors, generate default positions
    if (palette.positions.size() != palette.colors.size()) {
      palette.positions.clear();
      for (size_t i = 0; i < palette.colors.size(); i++) {
        float pos = (palette.colors.size() == 1) ? 0.0f :
                    static_cast<float>(i) / static_cast<float>(palette.colors.size() - 1);
        palette.positions.push_back(pos);
      }
    }

    // Load palette properties
    palette.colorRule = paletteObj.containsKey("colorRule") ? paletteObj["colorRule"].as<int8_t>() : -1;
    palette.interMode = paletteObj.containsKey("interMode") ? paletteObj["interMode"].as<int8_t>() : 1; // Default to HSB
    palette.wrapMode = paletteObj.containsKey("wrapMode") ? paletteObj["wrapMode"].as<int8_t>() : 0;   // Default to clamp
    palette.segmentation = paletteObj.containsKey("segmentation") ? paletteObj["segmentation"].as<float>() : 0.0f; // Default to 0

    // Make sure we have at least one color
    if (palette.colors.size() > 0) {
      userPalettes.push_back(palette);
      LP_LOGF("Loaded user palette: %s with %d colors\n", palette.name.c_str(), palette.colors.size());
    }
  }

  LP_LOGF("Loaded %d user palettes\n", userPalettes.size());
}

bool saveUserPalettes(const std::vector<UserPalette>& palettes) {
  // Create JSON document for all palettes
  // Estimate capacity based on number of palettes and their complexity
  const size_t capacity = JSON_OBJECT_SIZE(1) + // Root object
                         JSON_ARRAY_SIZE(palettes.size()) + // Palettes array
                         palettes.size() * JSON_OBJECT_SIZE(7) + // Each palette object (including segmentation)
                         palettes.size() * 64 + // Palette names
                         palettes.size() * JSON_ARRAY_SIZE(32) + // For colors arrays
                         palettes.size() * JSON_ARRAY_SIZE(32) + // For positions arrays
                         1024; // Extra buffer

  DynamicJsonDocument doc(capacity);

  // Create palettes array
  JsonArray palettesArray = doc.createNestedArray("palettes");

  // Add each palette to the array
  for (const auto& p : palettes) {
    JsonObject paletteObj = palettesArray.createNestedObject();

    // Add palette properties
    paletteObj["name"] = p.name;
    paletteObj["colorRule"] = p.colorRule;
    paletteObj["interMode"] = p.interMode;
    paletteObj["wrapMode"] = p.wrapMode;
    paletteObj["segmentation"] = p.segmentation;

    // Add colors array
    JsonArray colorsArray = paletteObj.createNestedArray("colors");
    for (const auto& color : p.colors) {
      colorsArray.add(color);
    }

    // Add positions array
    JsonArray positionsArray = paletteObj.createNestedArray("positions");
    for (const auto& pos : p.positions) {
      positionsArray.add(pos);
    }
  }

  // Open file for writing
  File file = SPIFFS.open("/user-palettes.json", "w");
  if (!file) {
    LP_LOGLN("Failed to open user-palettes.json for writing");
    return false;
  }

  // Serialize JSON to file
  if (serializeJson(doc, file) == 0) {
    LP_LOGLN("Failed to write palettes to file");
    file.close();
    return false;
  }

  file.close();

  return true;
}

// Function to load credentials from a separate file
void loadCredentials() {
  if (!SPIFFS.exists("/credentials.json")) {
    LP_LOGLN("No credentials file found, using defaults");
    return;
  }

  File file = SPIFFS.open("/credentials.json", "r");
  if (!file) {
    LP_LOGLN("Failed to open credentials file for reading");
    return;
  }

  // Calculate JSON document size based on file size (with some extra capacity)
  size_t fileSize = file.size();
  size_t capacity = fileSize * 1.2;

  // Allocate memory for the document
  DynamicJsonDocument doc(capacity);

  // Parse the JSON
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error) {
    LP_LOGF("Failed to parse credentials JSON: %s\n", error.c_str());
    return;
  }

  // Load WiFi credentials
  if (doc.containsKey("wifi_ssid")) {
    savedSSID = doc["wifi_ssid"].as<String>();
  }

  if (doc.containsKey("wifi_pass")) {
    savedPassword = doc["wifi_pass"].as<String>();
  }

  // Load OTA settings if present
  #ifdef OTA_ENABLED
  if (doc.containsKey("ota_password")) {
    otaPassword = doc["ota_password"].as<String>();
  }
  if (doc.containsKey("ota_enabled")) {
    otaEnabled = doc["ota_enabled"].as<bool>();
  }
  if (doc.containsKey("ota_port")) {
    otaPort = doc["ota_port"].as<uint16_t>();
  }
  #endif

  LP_LOGLN("Credentials loaded from SPIFFS");
}

// Function to save credentials to a separate file
void saveCredentials() {
  const size_t capacity = JSON_OBJECT_SIZE(5) + 256; // Space for SSID, password, OTA password, enabled state, and port
  DynamicJsonDocument doc(capacity);

  // Add credentials to the JSON document
  doc["wifi_ssid"] = savedSSID;
  doc["wifi_pass"] = savedPassword;
  
  #ifdef OTA_ENABLED
  doc["ota_password"] = otaPassword;
  doc["ota_enabled"] = otaEnabled;
  doc["ota_port"] = otaPort;
  #endif

  // Open the file for writing
  File file = SPIFFS.open("/credentials.json", "w");
  if (!file) {
    LP_LOGLN("Failed to open credentials file for writing");
    return;
  }

  // Serialize JSON to file
  if (serializeJson(doc, file) == 0) {
    LP_LOGLN("Failed to write credentials to file");
  } else {
    LP_LOGLN("Credentials saved to SPIFFS");
  }

  file.close();
}

void saveLayers() {
  // Calculate capacity for JsonDocument with support for multiple light lists
  const size_t capacity = JSON_OBJECT_SIZE(1) + // For root object with layers array
                         JSON_ARRAY_SIZE(MAX_LIGHT_LISTS) + // For layers array
                         MAX_LIGHT_LISTS * JSON_OBJECT_SIZE(10) + // For each layer object
                         MAX_LIGHT_LISTS * JSON_ARRAY_SIZE(8) * 2 + // For colors and positions arrays per layer
                         1024; // Extra space for values

  DynamicJsonDocument layersDoc(capacity);
  
  // Save light lists as layers
  if (state) {
    JsonArray layersArray = layersDoc.createNestedArray("layers");

    // Save each editable light list
    for (uint8_t i = 0; i < MAX_LIGHT_LISTS; i++) {
      if (state->lightLists[i]) {
        JsonObject layerObj = layersArray.createNestedObject();

        // Save layer index
        layerObj["index"] = i;

        // Save visibility
        layerObj["visible"] = state->lightLists[i]->visible;

        // Save brightness
        layerObj["brightness"] = state->lightLists[i]->minBri;
        
        // Save blend mode
        layerObj["blendMode"] = static_cast<uint8_t>(state->lightLists[i]->blendMode);
        
        // Save speed
        layerObj["speed"] = state->lightLists[i]->speed;
        
        // Use the stored easeIndex
        layerObj["ease"] = state->lightLists[i]->easeIndex;
        
        // Save fade speed
        layerObj["fadeSpeed"] = state->lightLists[i]->fadeSpeed;
        
        // Save offset
        layerObj["offset"] = state->lightLists[i]->getOffset();
        
        // Save behaviour flags
        if (state->lightLists[i]->behaviour) {
          layerObj["behaviourFlags"] = state->lightLists[i]->behaviour->flags;
        }

        // Save palette if available
        if (state->lightLists[i]->hasPalette()) {
          const Palette& palette = state->lightLists[i]->getPalette();

          // Save palette colors
          JsonArray colorsArray = layerObj.createNestedArray("colors");
          const std::vector<int64_t>& colors = palette.getColors();
          for (const auto& color : colors) {
            colorsArray.add(color);
          }

          // Save palette positions
          JsonArray positionsArray = layerObj.createNestedArray("positions");
          const std::vector<float>& positions = palette.getPositions();
          for (const auto& pos : positions) {
            positionsArray.add(pos);
          }

          // Save palette properties
          layerObj["colorRule"] = palette.getColorRule();
          layerObj["interMode"] = palette.getInterMode();
          layerObj["wrapMode"] = palette.getWrapMode();
          layerObj["segmentation"] = palette.getSegmentation();
        }
      }
    }
  }
  
  // Open the file for writing
  File file = SPIFFS.open("/layers.json", "w");
  if (!file) {
    LP_LOGLN("Failed to open layers.json file for writing");
    return;
  }

  // Serialize JSON to file
  if (serializeJson(layersDoc, file) == 0) {
    LP_LOGLN("Failed to write layers to file");
  } else {
    LP_LOGLN("Layers saved to layers.json");
  }

  file.close();
}

// Delete a user palette by index
bool deletePalette(size_t index) {
  // Check if index is valid
  if (index >= userPalettes.size()) {
    LP_LOGF("Invalid palette index: %d\n", index);
    return false;
  }

  // Get the name for logging
  String paletteName = userPalettes[index].name;

  // Remove the palette from the vector
  std::vector<UserPalette> updatedPalettes = userPalettes;
  updatedPalettes.erase(updatedPalettes.begin() + index);

  // Create JSON document for all remaining palettes
  const size_t capacity = JSON_OBJECT_SIZE(1) + // Root object
                         JSON_ARRAY_SIZE(updatedPalettes.size()) + // Palettes array
                         updatedPalettes.size() * JSON_OBJECT_SIZE(7) + // Each palette object (including segmentation)
                         updatedPalettes.size() * 64 + // Palette names
                         updatedPalettes.size() * JSON_ARRAY_SIZE(32) + // For colors arrays
                         updatedPalettes.size() * JSON_ARRAY_SIZE(32) + // For positions arrays
                         1024; // Extra buffer

  DynamicJsonDocument doc(capacity);

  // Create palettes array
  JsonArray palettesArray = doc.createNestedArray("palettes");

  // Add each remaining palette to the array
  for (const auto& p : updatedPalettes) {
    JsonObject paletteObj = palettesArray.createNestedObject();

    // Add palette properties
    paletteObj["name"] = p.name;
    paletteObj["colorRule"] = p.colorRule;
    paletteObj["interMode"] = p.interMode;
    paletteObj["wrapMode"] = p.wrapMode;
    paletteObj["segmentation"] = p.segmentation;

    // Add colors array
    JsonArray colorsArray = paletteObj.createNestedArray("colors");
    for (const auto& color : p.colors) {
      colorsArray.add(color);
    }

    // Add positions array
    JsonArray positionsArray = paletteObj.createNestedArray("positions");
    for (const auto& pos : p.positions) {
      positionsArray.add(pos);
    }
  }

  // Open file for writing
  File file = SPIFFS.open("/user-palettes.json", "w");
  if (!file) {
    LP_LOGLN("Failed to open user-palettes.json for writing");
    return false;
  }

  // Serialize JSON to file
  if (serializeJson(doc, file) == 0) {
    LP_LOGLN("Failed to write palettes to file");
    file.close();
    return false;
  }

  file.close();

  LP_LOGF("Deleted user palette: %s\n", paletteName.c_str());

  // Update the in-memory palette list
  userPalettes = updatedPalettes;
  return true;
}
