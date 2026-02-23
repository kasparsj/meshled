#pragma once

#include <lightpath/rendering.hpp>

// Get the name of a palette by index
String getPaletteName(uint8_t index) {
    static const char* paletteNames[] = {
        "Sunset Real",
        "ES Rivendell",
        "Ocean Breeze",
        "RGI 15",
        "Retro 2",
        "Analogous 1",
        "Pink Splash 08",
        "Coral Reef",
        "Ocean Breeze 068",
        "Pink Splash 07",
        "Vintage 01",
        "Departure",
        "Landscape 64",
        "Landscape 33",
        "Rainbow Sherbet",
        "Green Hult 65",
        "Green Hult 64",
        "Dry Wet",
        "July 01",
        "Vintage 57",
        "IB 15",
        "Fuschia 7",
        "Emerald Dragon",
        "Lava",
        "Fire",
        "Colorful",
        "Magenta Evening",
        "Pink Purple",
        "Autumn 19",
        "Black Blue Magenta White",
        "Black Magenta Red",
        "Black Red Magenta Yellow",
        "Blue Cyan Yellow"
    };

    static const uint8_t nameCount = sizeof(paletteNames) / sizeof(paletteNames[0]);

    // Ensure index is in bounds
    if (index >= nameCount) {
        index = index % nameCount;
    }

    return String(paletteNames[index]);
}

// Handle toggle background
void handleToggleVisible() {
  sendCORSHeaders();

  if (server.hasArg("layer") && server.hasArg("visible")) {
    uint8_t layer = server.arg("layer").toInt();
    bool visible = (server.arg("visible") == "true");
    if (state && state->lightLists[layer]) {
      state->lightLists[layer]->visible = visible;
      LP_LOGLN("Toggle visible: " + String(state->lightLists[layer]->visible ? "ON" : "OFF"));
    } else {
      LP_LOGLN("Cannot toggle visible: state or lightList is NULL");
    }

    #ifdef SPIFFS_ENABLED
    saveLayers();
    #endif

    // Redirect back to homepage
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
  } else {
    server.send(400, "text/plain", "Missing layer parameter");
  }
}

// Handle AJAX brightness update
void handleUpdateLayerBrightness() {
  if (server.hasArg("layer") && server.hasArg("value")) {
    uint8_t layer = server.arg("layer").toInt();
    uint8_t newBrightness = server.arg("value").toInt();
    if (newBrightness >= 1 && newBrightness <= 255) {
      if (state && state->lightLists[layer]) {
        state->lightLists[layer]->minBri = newBrightness;
      }

      LP_LOGLN("Updated background brightness via AJAX: " + String(newBrightness));

      #ifdef SPIFFS_ENABLED
      saveLayers();
      #endif

      server.send(200, "text/plain", "Background brightness updated");
    } else {
      server.send(400, "text/plain", "Invalid brightness value");
    }
  } else {
    server.send(400, "text/plain", "Missing layer or value parameter");
  }
}

// Handler for getting predefined palette colors
void handleGetPaletteColors() {
  sendCORSHeaders();

  if (server.hasArg("index")) {
    uint8_t layer = 0;
    if (server.hasArg("layer")) {
      layer = server.arg("layer").toInt();
    }
    String indexStr = server.arg("index");

    // Check if it's a user palette (starts with 'u')
    if (indexStr.startsWith("u")) {
      // Extract the user palette index
      int userIndex = indexStr.substring(1).toInt();

      if (userIndex >= 0 && userIndex < userPalettes.size()) {
        const UserPalette& userPalette = userPalettes[userIndex];

        // Convert the palette to hex colors
        const std::vector<int64_t>& colors = userPalette.colors;
        const std::vector<float>& positions = userPalette.positions;

        String jsonResponse = "{\"colors\":[";

        // Add color values as hex
        for (size_t i = 0; i < colors.size(); i++) {
          char hexBuffer[10];
          sprintf(hexBuffer, "\"#%06X\"", (uint32_t)colors[i]);
          jsonResponse += hexBuffer;
          if (i < colors.size() - 1) {
            jsonResponse += ",";
          }
        }

        jsonResponse += "],\"positions\":[";

        // Add position values
        for (size_t i = 0; i < positions.size(); i++) {
          jsonResponse += String(positions[i], 2);
          if (i < positions.size() - 1) {
            jsonResponse += ",";
          }
        }

        // Add other palette properties
        jsonResponse += "],\"colorRule\":";
        jsonResponse += String(userPalette.colorRule);
        jsonResponse += ",\"interMode\":";
        jsonResponse += String(userPalette.interMode);
        jsonResponse += ",\"wrapMode\":";
        jsonResponse += String(userPalette.wrapMode);
        jsonResponse += ",\"segmentation\":";
        jsonResponse += String(userPalette.segmentation);
        jsonResponse += "}";

        server.send(200, "application/json", jsonResponse);
        return;
      }
    }
    else {
      // Regular predefined palette
      int paletteIndex = indexStr.toInt();

      if (paletteIndex >= 0 && paletteIndex < getPaletteCount()) {
        // Get the palette
        Palette palette = getPalette(paletteIndex);

        // Convert the palette to hex colors
        const std::vector<int64_t>& colors = palette.getColors();
        const std::vector<float>& positions = palette.getPositions();

        String jsonResponse = "{\"colors\":[";

        // Add color values as hex
        for (size_t i = 0; i < colors.size(); i++) {
          char hexBuffer[10];
          sprintf(hexBuffer, "\"#%06X\"", (uint32_t)colors[i]);
          jsonResponse += hexBuffer;
          if (i < colors.size() - 1) {
            jsonResponse += ",";
          }
        }

        jsonResponse += "],\"positions\":[";

        // Add position values
        for (size_t i = 0; i < positions.size(); i++) {
          jsonResponse += String(positions[i], 2);
          if (i < positions.size() - 1) {
            jsonResponse += ",";
          }
        }

        // Add palette properties
        jsonResponse += "],\"colorRule\":";
        jsonResponse += String(palette.getColorRule());
        jsonResponse += ",\"interMode\":";
        jsonResponse += String(palette.getInterMode());
        jsonResponse += ",\"wrapMode\":";
        jsonResponse += String(palette.getWrapMode());
        jsonResponse += ",\"segmentation\":";
        jsonResponse += String(palette.getSegmentation());
        jsonResponse += "}";

        server.send(200, "application/json", jsonResponse);
        return;
      }
    }
  }

  server.send(400, "application/json", "{\"error\":\"Invalid palette index\"}");
}

// Unified handler for updating palette properties
void handleUpdatePalette() {
  sendCORSHeaders();

  // Check if layer parameter is provided
  if (!server.hasArg("layer")) {
    server.send(400, "text/plain", "Missing layer parameter");
    return;
  }

  uint8_t layer = server.arg("layer").toInt();

  // Check if the layer exists
  if (!state || !state->lightLists[layer]) {
    server.send(400, "text/plain", "Invalid layer index");
    return;
  }

  // Create a new palette based on the current one
  Palette newPalette = state->lightLists[layer]->palette;
  bool paletteChanged = false;

  // Update color rule if provided
  if (server.hasArg("colorRule")) {
    int ruleValue = server.arg("colorRule").toInt();
    if (ruleValue >= -1 && ruleValue <= 7) {
      newPalette.setColorRule(ruleValue);
      paletteChanged = true;

      String ruleName = (ruleValue == -1 ? "None" :
                        (ruleValue == 0 ? "Analogous" :
                         ruleValue == 1 ? "Complementary" :
                         ruleValue == 2 ? "Split Complementary" :
                         ruleValue == 3 ? "Compound" :
                         ruleValue == 4 ? "Flipped Compound" :
                         ruleValue == 5 ? "Monochrome" :
                         ruleValue == 6 ? "Triad" : "Tetrad"));
      LP_LOGLN("Updated color rule: " + ruleName);
    } else {
      server.send(400, "text/plain", "Invalid color rule value");
      return;
    }
  }

  // Update interpolation mode if provided
  if (server.hasArg("interMode")) {
    int modeValue = server.arg("interMode").toInt();
    if (modeValue >= -1 && modeValue <= 2) {
      newPalette.setInterMode(modeValue);
      paletteChanged = true;

      String modeName = (modeValue == -1 ? "None" :
                        (modeValue == 0 ? "RGB" :
                         modeValue == 1 ? "HSB" : "CIELCh"));
      LP_LOGLN("Updated interpolation mode: " + modeName);
    } else {
      server.send(400, "text/plain", "Invalid interpolation mode value");
      return;
    }
  }

  // Update wrap mode if provided
  if (server.hasArg("wrapMode")) {
    int modeValue = server.arg("wrapMode").toInt();
    if (modeValue >= -1 && modeValue <= 3) {
      newPalette.setWrapMode(modeValue);
      paletteChanged = true;

      String modeName = (modeValue == -1 ? "Nowrap" :
                        (modeValue == 0 ? "Clamp to Edge" :
                         modeValue == 1 ? "Repeat" :
                         modeValue == 2 ? "Repeat Mirror" : "Extend"));
      LP_LOGLN("Updated wrap mode: " + modeName);
    } else {
      server.send(400, "text/plain", "Invalid wrap mode value");
      return;
    }
  }

  // Update segmentation if provided
  if (server.hasArg("segmentation")) {
    float segValue = server.arg("segmentation").toFloat();
    if (segValue >= 0.0f) {
      newPalette.setSegmentation(segValue);
      paletteChanged = true;
      LP_LOGLN("Updated segmentation: " + String(segValue, 1));
    } else {
      server.send(400, "text/plain", "Invalid segmentation value");
      return;
    }
  }

  // Handle colors and positions if provided (typically via POST)
  std::vector<int64_t> colors;
  std::vector<float> positions;
  bool colorsUpdated = false;

  if (server.hasArg("colors")) {
    // Parse colors from JSON
    String colorsJson = server.arg("colors");
    const size_t capacity = JSON_ARRAY_SIZE(10) + 10*JSON_STRING_SIZE(8);
    DynamicJsonDocument doc(capacity);
    DeserializationError error = deserializeJson(doc, colorsJson);

    if (!error && doc.is<JsonArray>()) {
      JsonArray colorsArray = doc.as<JsonArray>();
      colors.clear();

      // Process each color in the array
      for (JsonVariant colorVar : colorsArray) {
        if (colorVar.is<const char*>()) {
          String colorStr = colorVar.as<String>();
          if (colorStr.startsWith("#")) {
            colorStr = colorStr.substring(1);
          }
          int64_t color = strtol(colorStr.c_str(), NULL, 16);
          colors.push_back(color);
        }
      }

      if (colors.size() > 0) {
        colorsUpdated = true;
        LP_LOGF("Parsed %d colors from JSON array\n", colors.size());
      }
    } else {
      server.send(400, "text/plain", "Error parsing colors JSON");
      return;
    }
  }

  if (server.hasArg("positions") && colorsUpdated) {
    // Parse positions from JSON
    String positionsJson = server.arg("positions");
    const size_t capacity = JSON_ARRAY_SIZE(10);
    DynamicJsonDocument doc(capacity);
    DeserializationError error = deserializeJson(doc, positionsJson);

    if (!error && doc.is<JsonArray>()) {
      JsonArray posArray = doc.as<JsonArray>();
      positions.clear();

      for (JsonVariant posVar : posArray) {
        if (posVar.is<float>()) {
          float pos = posVar.as<float>();
          pos = max(0.0f, min(1.0f, pos));
          positions.push_back(pos);
        }
      }

      LP_LOGF("Parsed %d positions from JSON array\n", positions.size());

      // Ensure positions match colors
      if (positions.size() == colors.size()) {
        // Sort colors based on positions
        std::vector<std::pair<float, int64_t>> colorPairs;
        for (size_t i = 0; i < colors.size(); i++) {
          colorPairs.push_back(std::make_pair(positions[i], colors[i]));
        }

        std::sort(colorPairs.begin(), colorPairs.end(),
                  [](const std::pair<float, int64_t>& a, const std::pair<float, int64_t>& b) {
                    return a.first < b.first;
                  });

        colors.clear();
        positions.clear();

        for (const auto& pair : colorPairs) {
          positions.push_back(pair.first);
          colors.push_back(pair.second);
        }
      } else {
        // Generate default positions if count doesn't match
        positions.clear();
        for (size_t i = 0; i < colors.size(); i++) {
          float pos = (colors.size() == 1) ? 0.0f : float(i) / float(colors.size() - 1);
          positions.push_back(pos);
        }
      }
    }
  } else if (colorsUpdated) {
    // Generate default positions if not provided
    positions.clear();
    for (size_t i = 0; i < colors.size(); i++) {
      float pos = (colors.size() == 1) ? 0.0f : float(i) / float(colors.size() - 1);
      positions.push_back(pos);
    }
  }

  // Update colors and positions if they were changed
  if (colorsUpdated) {
    newPalette.setColors(colors);
    newPalette.setPositions(positions);
    paletteChanged = true;
    LP_LOGF("Setting %d colors for palette\n", colors.size());
  }

  // Apply the updated palette if any changes were made
  if (paletteChanged) {
    state->lightLists[layer]->setPalette(newPalette);

    #ifdef SPIFFS_ENABLED
    saveLayers();
    #endif

    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "No palette properties were updated");
  }
}

// Get a user palette by index
UserPalette getUserPalette(size_t index) {
  if (index < userPalettes.size()) {
    return userPalettes[index];
  }

  // Return empty palette if not found
  UserPalette emptyPalette;
  emptyPalette.name = "Empty";
  emptyPalette.colorRule = -1;
  emptyPalette.interMode = 1;
  emptyPalette.wrapMode = 0;
  return emptyPalette;
}

// Handler for adding a new layer
void handleAddLayer() {
  sendCORSHeaders();

  uint8_t newLayerIndex = 0;
  bool layerAdded = false;

  // Find the first available slot
  for (uint8_t i = 0; i < MAX_LIGHT_LISTS; i++) {
    if (state && !state->lightLists[i]) {
      newLayerIndex = i;

      state->setupBg(newLayerIndex);

      layerAdded = true;
      LP_LOGLN("Added new layer at index: " + String(newLayerIndex));

      #ifdef SPIFFS_ENABLED
      saveLayers();
      #endif

      break;
    }
  }

  if (layerAdded) {
    server.send(200, "text/plain", "Layer added successfully");
  } else {
    server.send(400, "text/plain", "Failed to add layer. Maximum number of layers reached.");
  }
}

// Handler for removing a layer
void handleRemoveLayer() {
  sendCORSHeaders();

  if (!server.hasArg("layer")) {
    server.send(400, "text/plain", "Missing layer parameter");
    return;
  }

  uint8_t layerIndex = server.arg("layer").toInt();

  // Don't allow removing layer 0 (base layer)
  if (layerIndex == 0) {
    server.send(400, "text/plain", "Cannot remove the base layer");
    return;
  }

  if (state && state->lightLists[layerIndex] && state->lightLists[layerIndex]->editable) {
    // Clear the layer
    state->lightLists[layerIndex]->setDuration(0);

    delay(1);

    LP_LOGLN("Removed layer at index: " + String(layerIndex));

    #ifdef SPIFFS_ENABLED
    saveLayers();
    #endif

    server.send(200, "text/plain", "Layer removed successfully");
  } else {
    server.send(400, "text/plain", "Invalid layer index or layer cannot be removed");
  }
}

// Handler for updating the blend mode
// Handler for updating the speed value
void handleUpdateSpeed() {
  if (server.hasArg("layer") && server.hasArg("value")) {
    uint8_t layer = server.arg("layer").toInt();
    float newSpeed = server.arg("value").toFloat();

    if (newSpeed >= -10.0 && newSpeed <= 10.0) {
      if (state && state->lightLists[layer]) {
        state->lightLists[layer]->setSpeed(newSpeed, state->lightLists[layer]->easeIndex);

        LP_LOGLN("Updated speed for layer " + String(layer) + " to: " + String(newSpeed));

        #ifdef SPIFFS_ENABLED
        saveLayers();
        #endif

        server.send(200, "text/plain", "Speed updated");
      } else {
        server.send(400, "text/plain", "Invalid layer index");
      }
    } else {
      server.send(400, "text/plain", "Invalid speed value");
    }
  } else {
    server.send(400, "text/plain", "Missing layer or value parameter");
  }
}

// Handler for updating the fade speed value
void handleUpdateFadeSpeed() {
  if (server.hasArg("layer") && server.hasArg("value")) {
    uint8_t layer = server.arg("layer").toInt();
    uint8_t newFadeSpeed = server.arg("value").toInt();

    if (newFadeSpeed >= 0 && newFadeSpeed <= 255) {
      if (state && state->lightLists[layer]) {
        state->lightLists[layer]->setFade(newFadeSpeed, state->lightLists[layer]->fadeThresh, state->lightLists[layer]->fadeEaseIndex);

        LP_LOGLN("Updated fade speed for layer " + String(layer) + " to: " + String(newFadeSpeed));

        #ifdef SPIFFS_ENABLED
        saveLayers();
        #endif

        server.send(200, "text/plain", "Fade speed updated");
      } else {
        server.send(400, "text/plain", "Invalid layer index");
      }
    } else {
      server.send(400, "text/plain", "Invalid fade speed value");
    }
  } else {
    server.send(400, "text/plain", "Missing layer or value parameter");
  }
}

// Handler for updating the easing function
void handleUpdateEase() {
  sendCORSHeaders();

  if (server.hasArg("layer") && server.hasArg("ease")) {
    uint8_t layer = server.arg("layer").toInt();
    uint8_t ease = server.arg("ease").toInt();

    if (ease >= EASE_NONE && ease <= EASE_ELASTIC_INOUT) {
      if (state && state->lightLists[layer]) {
        state->lightLists[layer]->setSpeed(state->lightLists[layer]->speed, ease);

        LP_LOGLN("Updated easing for layer " + String(layer) + " to ease: " + String(ease));

        #ifdef SPIFFS_ENABLED
        saveLayers();
        #endif

        server.send(200, "text/plain", "Easing updated");
      } else {
        server.send(400, "text/plain", "Invalid layer index");
      }
    } else {
      server.send(400, "text/plain", "Invalid easing value");
    }
  } else {
    server.send(400, "text/plain", "Missing layer or ease parameter");
  }
}

void handleUpdateBlendMode() {
  if (server.hasArg("layer") && server.hasArg("mode")) {
    uint8_t layer = server.arg("layer").toInt();
    uint8_t mode = server.arg("mode").toInt();

    if (mode >= BLEND_NORMAL && mode <= BLEND_PIN_LIGHT) {
      if (state && state->lightLists[layer]) {
        state->lightLists[layer]->blendMode = static_cast<BlendMode>(mode);

        // Simply log the mode value instead of using a large switch statement
        LP_LOGLN("Updated blend mode for layer " + String(layer) + " to mode: " + String(mode));

        #ifdef SPIFFS_ENABLED
        saveLayers();
        #endif

        server.send(200, "text/plain", "Blend mode updated");
      } else {
        server.send(400, "text/plain", "Invalid layer index");
      }
    } else {
      server.send(400, "text/plain", "Invalid blend mode value");
    }
  } else {
    server.send(400, "text/plain", "Missing layer or mode parameter");
  }
}

// Handler for updating behaviour flags
void handleUpdateBehaviourFlags() {
  sendCORSHeaders();
  
  if (server.hasArg("layer") && server.hasArg("flags")) {
    uint8_t layer = server.arg("layer").toInt();
    uint16_t flags = server.arg("flags").toInt();

    if (state && state->lightLists[layer]) {
      if (flags > 0 && state->lightLists[layer]->numLights == 0) {
        // convert BgLight to LightList
        LightList* lightList = new LightList(*state->lightLists[layer]);
        delete state->lightLists[layer];
        state->lightLists[layer] = lightList;
        state->doEmit(state->object.getIntersection(0, 0), state->lightLists[layer]);
      }
      else if (flags == 0) {
        // todo: convert LightList to BgLight
      }
      if (!state->lightLists[layer]->behaviour) {
        state->lightLists[layer]->behaviour = new Behaviour(flags);
      } else {
        state->lightLists[layer]->behaviour->flags = flags;
      }

      LP_LOGF("Updated behaviour flags for layer %d to: %d\n", layer, flags);

      #ifdef SPIFFS_ENABLED
      saveLayers();
      #endif

      server.send(200, "text/plain", "Behaviour flags updated");
    } else {
      server.send(400, "text/plain", "Invalid layer index");
    }
  } else {
    server.send(400, "text/plain", "Missing layer or flags parameter");
  }
}

// Handler for updating layer offset
void handleUpdateLayerOffset() {
  sendCORSHeaders();
  
  if (server.hasArg("layer") && server.hasArg("offset")) {
    uint8_t layer = server.arg("layer").toInt();
    float offset = server.arg("offset").toFloat();

    if (state && state->lightLists[layer]) {
      state->lightLists[layer]->setOffset(offset);
      
      LP_LOGF("Updated offset for layer %d to: %f\n", layer, offset);

      #ifdef SPIFFS_ENABLED
      saveLayers();
      #endif

      server.send(200, "text/plain", "Layer offset updated");
    } else {
      server.send(400, "text/plain", "Invalid layer index");
    }
  } else {
    server.send(400, "text/plain", "Missing layer or offset parameter");
  }
}

// Handler for getting layers as JSON
void handleGetLayers() {
  sendCORSHeaders();
  
  DynamicJsonDocument doc(2048);
  JsonArray layersArray = doc.to<JsonArray>();

  // Iterate through all light lists and build JSON response
  for (uint8_t i = 0; i < MAX_LIGHT_LISTS; i++) {
    if (state && state->lightLists[i] && state->lightLists[i]->editable) {
      JsonObject layer = layersArray.createNestedObject();
      
      layer["id"] = i;
      layer["visible"] = state->lightLists[i]->visible;
      layer["brightness"] = state->lightLists[i]->minBri;
      layer["speed"] = state->lightLists[i]->speed;
      layer["fadeSpeed"] = state->lightLists[i]->fadeSpeed;
      layer["easing"] = state->lightLists[i]->easeIndex;
      layer["blendMode"] = state->lightLists[i]->blendMode;
      
      // Add behaviour flags if behaviour exists
      if (state->lightLists[i]->behaviour) {
        layer["behaviourFlags"] = state->lightLists[i]->behaviour->flags;
      } else {
        layer["behaviourFlags"] = 0;
      }
      
      // Add layer offset (position of first light)
      layer["offset"] = state->lightLists[i]->getOffset();

      // Add palette information
      JsonObject palette = layer.createNestedObject("palette");
      const Palette& layerPalette = state->lightLists[i]->palette;
      
      JsonArray colors = palette.createNestedArray("colors");
      const std::vector<int64_t>& paletteColors = layerPalette.getColors();
      for (size_t j = 0; j < paletteColors.size(); j++) {
        char hexColor[8];
        sprintf(hexColor, "#%06X", (uint32_t)paletteColors[j]);
        colors.add(hexColor);
      }
      
      JsonArray positions = palette.createNestedArray("positions");
      const std::vector<float>& palettePositions = layerPalette.getPositions();
      for (size_t j = 0; j < palettePositions.size(); j++) {
        positions.add(palettePositions[j]);
      }
      
      palette["colorRule"] = layerPalette.getColorRule();
      palette["interMode"] = layerPalette.getInterMode();
      palette["wrapMode"] = layerPalette.getWrapMode();
      palette["segmentation"] = layerPalette.getSegmentation();
    }
  }

  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

// Handle reset layer
void handleResetLayer() {
  sendCORSHeaders();

  if (server.hasArg("layer")) {
    uint8_t layer = server.arg("layer").toInt();
    
    if (state && state->lightLists[layer]) {
      // Reset the layer to default values
      state->lightLists[layer]->reset();
      
      LP_LOGLN("Reset layer: " + String(layer));

      #ifdef SPIFFS_ENABLED
      saveLayers();
      #endif

      server.send(200, "text/plain", "Layer reset successfully");
    } else {
      LP_LOGLN("Cannot reset layer: state or lightList is NULL");
      server.send(404, "text/plain", "Layer not found");
    }
  } else {
    server.send(400, "text/plain", "Missing layer parameter");
  }
}
