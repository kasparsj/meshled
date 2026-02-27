#pragma once

#include "ObjectTypeSupport.h"
#include "WebServerValidation.h"

struct SettingsPatch {
  bool hasMaxBrightness = false;
  uint8_t maxBrightness = 0;

  bool hasHostname = false;
  String hostname;

  bool hasApiAuthEnabled = false;
  bool apiAuthEnabled = false;

  bool hasApiAuthToken = false;
  String apiAuthToken;

  bool hasOscEnabled = false;
  bool oscEnabled = false;

  bool hasOscPort = false;
  uint16_t oscPort = 0;

  bool hasLedType = false;
  uint8_t ledType = 0;

  bool hasColorOrder = false;
  uint8_t colorOrder = 0;

  bool hasPixelCount1 = false;
  uint16_t pixelCount1 = 0;

  bool hasPixelCount2 = false;
  uint16_t pixelCount2 = 0;

  bool hasPixelPin1 = false;
  uint8_t pixelPin1 = 0;

  bool hasPixelPin2 = false;
  uint8_t pixelPin2 = 0;

  bool hasPixelDensity = false;
  uint8_t pixelDensity = 0;

  bool hasLedLibrary = false;
  uint8_t ledLibrary = 0;

  bool hasObjectType = false;
  uint8_t objectType = 0;

  bool hasOtaEnabled = false;
  bool otaEnabled = false;

  bool hasOtaPort = false;
  uint16_t otaPort = 0;

  bool hasOtaPassword = false;
  String otaPassword;
};

struct SettingsApplyResult {
  bool needsLedReinit = false;
  bool needsStateRebuild = false;
  bool requiresReboot = false;
  bool credentialsChanged = false;
};

inline bool parseStrictBoolean(const String& rawValue, bool& parsedValue) {
  String lowered = rawValue;
  lowered.trim();
  lowered.toLowerCase();
  if (lowered == "1" || lowered == "true") {
    parsedValue = true;
    return true;
  }
  if (lowered == "0" || lowered == "false") {
    parsedValue = false;
    return true;
  }
  return false;
}

inline bool parseBooleanArg(const char* argName, bool& parsedValue, bool& hasValue, String& error) {
  if (!server.hasArg(argName)) {
    hasValue = false;
    return true;
  }

  hasValue = true;
  if (!parseStrictBoolean(server.arg(argName), parsedValue)) {
    error = String("Invalid boolean value for ") + argName;
    return false;
  }
  return true;
}

inline bool parseBoundedLongArg(
    const char* argName,
    long minValue,
    long maxValue,
    long& parsedValue,
    bool& hasValue,
    String& error) {
  if (!server.hasArg(argName)) {
    hasValue = false;
    return true;
  }

  hasValue = true;
  if (!parseStrictLong(server.arg(argName), parsedValue)) {
    error = String("Invalid numeric value for ") + argName;
    return false;
  }
  if (parsedValue < minValue || parsedValue > maxValue) {
    error = String("Value out of range for ") + argName;
    return false;
  }
  return true;
}

inline bool parseBoundedLongArgWithLegacy(
    const char* primaryArgName,
    const char* legacyArgName,
    long minValue,
    long maxValue,
    long& parsedValue,
    bool& hasValue,
    String& error) {
  hasValue = false;

  if (server.hasArg(primaryArgName)) {
    return parseBoundedLongArg(primaryArgName, minValue, maxValue, parsedValue, hasValue, error);
  }

  if (legacyArgName != nullptr && server.hasArg(legacyArgName)) {
    return parseBoundedLongArg(legacyArgName, minValue, maxValue, parsedValue, hasValue, error);
  }

  return true;
}

inline bool parseSettingsPatch(SettingsPatch& patch, String& error) {
  long parsedLong = 0;

  if (!parseBoundedLongArgWithLegacy(
          "max_brightness",
          "maxBrightness",
          1,
          255,
          parsedLong,
          patch.hasMaxBrightness,
          error)) {
    return false;
  }
  if (patch.hasMaxBrightness) {
    patch.maxBrightness = static_cast<uint8_t>(parsedLong);
  }

  if (server.hasArg("hostname")) {
    patch.hasHostname = true;
    patch.hostname = server.arg("hostname");
    patch.hostname.trim();
    if (patch.hostname.length() > 63) {
      error = "hostname must be at most 63 characters";
      return false;
    }
  }

  if (!parseBooleanArg("api_auth_enabled", patch.apiAuthEnabled, patch.hasApiAuthEnabled, error)) {
    return false;
  }

  if (server.hasArg("api_auth_token")) {
    patch.hasApiAuthToken = true;
    patch.apiAuthToken = server.arg("api_auth_token");
  }

  if (!parseBooleanArg("osc_enabled", patch.oscEnabled, patch.hasOscEnabled, error)) {
    return false;
  }

  if (!parseBoundedLongArg("osc_port", 1, 65535, parsedLong, patch.hasOscPort, error)) {
    return false;
  }
  if (patch.hasOscPort) {
    patch.oscPort = static_cast<uint16_t>(parsedLong);
  }

  if (!parseBoundedLongArg("led_type", 0, 255, parsedLong, patch.hasLedType, error)) {
    return false;
  }
  if (patch.hasLedType) {
    patch.ledType = static_cast<uint8_t>(parsedLong);
  }

  if (!parseBoundedLongArg("color_order", 0, 255, parsedLong, patch.hasColorOrder, error)) {
    return false;
  }
  if (patch.hasColorOrder) {
    patch.colorOrder = static_cast<uint8_t>(parsedLong);
  }

  if (!parseBoundedLongArgWithLegacy(
          "pixel_count1",
          "pixel_count",
          1,
          65535,
          parsedLong,
          patch.hasPixelCount1,
          error)) {
    return false;
  }
  if (patch.hasPixelCount1) {
    patch.pixelCount1 = static_cast<uint16_t>(parsedLong);
  }

  if (!parseBoundedLongArg("pixel_count2", 0, 65535, parsedLong, patch.hasPixelCount2, error)) {
    return false;
  }
  if (patch.hasPixelCount2) {
    patch.pixelCount2 = static_cast<uint16_t>(parsedLong);
  }

  if (!parseBoundedLongArg("pixel_pin1", 0, 255, parsedLong, patch.hasPixelPin1, error)) {
    return false;
  }
  if (patch.hasPixelPin1) {
    patch.pixelPin1 = static_cast<uint8_t>(parsedLong);
  }

  if (!parseBoundedLongArg("pixel_pin2", 0, 255, parsedLong, patch.hasPixelPin2, error)) {
    return false;
  }
  if (patch.hasPixelPin2) {
    patch.pixelPin2 = static_cast<uint8_t>(parsedLong);
  }

  if (!parseBoundedLongArg("pixel_density", 1, 255, parsedLong, patch.hasPixelDensity, error)) {
    return false;
  }
  if (patch.hasPixelDensity) {
    patch.pixelDensity = static_cast<uint8_t>(parsedLong);
  }

  if (!parseBoundedLongArg("led_library", 0, 255, parsedLong, patch.hasLedLibrary, error)) {
    return false;
  }
  if (patch.hasLedLibrary) {
    patch.ledLibrary = static_cast<uint8_t>(parsedLong);
  }

  if (!parseBoundedLongArg("object_type", 0, 255, parsedLong, patch.hasObjectType, error)) {
    return false;
  }
  if (patch.hasObjectType) {
    patch.objectType = static_cast<uint8_t>(parsedLong);
  }

  if (!parseBooleanArg("ota_enabled", patch.otaEnabled, patch.hasOtaEnabled, error)) {
    return false;
  }

  if (!parseBoundedLongArg("ota_port", 1, 65535, parsedLong, patch.hasOtaPort, error)) {
    return false;
  }
  if (patch.hasOtaPort) {
    patch.otaPort = static_cast<uint16_t>(parsedLong);
  }

  if (server.hasArg("ota_password")) {
    patch.hasOtaPassword = true;
    patch.otaPassword = server.arg("ota_password");
  }

  return true;
}

inline bool isColorOrderSupportedValue(uint8_t order) {
  return IS_RGB(order) ||
         IS_GRB(order) ||
         IS_BRG(order) ||
         IS_RBG(order) ||
         IS_BGR(order) ||
         IS_GBR(order);
}

inline bool isLedLibraryKnown(uint8_t libraryId) {
  for (size_t i = 0; i < knownLedLibraryCount(); i++) {
    if (knownLedLibraryAt(i) == libraryId) {
      return true;
    }
  }
  return false;
}

inline bool applySettingsPatch(const SettingsPatch& patch, SettingsApplyResult& result, String& error) {
  if (patch.hasHostname) {
    String nextHostname = patch.hostname;
    if (nextHostname.length() == 0) {
      nextHostname = DEFAULT_HOSTNAME;
    }
    if (nextHostname != deviceHostname) {
      deviceHostname = nextHostname;
      result.requiresReboot = true;
    }
  }

  if (patch.hasMaxBrightness) {
    maxBrightness = patch.maxBrightness;
  }

  if (patch.hasLedType) {
    if (!isLedTypeKnown(patch.ledType)) {
      error = "Unsupported led_type";
      return false;
    }
    if (patch.ledType != ledType) {
      ledType = patch.ledType;
      result.needsLedReinit = true;
    }
  }

  if (patch.hasColorOrder) {
    if (!isColorOrderSupportedValue(patch.colorOrder)) {
      error = "Unsupported color_order";
      return false;
    }
    if (patch.colorOrder != colorOrder) {
      colorOrder = patch.colorOrder;
      result.needsLedReinit = true;
    }
  }

  if (patch.hasPixelCount1 && patch.pixelCount1 != pixelCount1) {
    pixelCount1 = patch.pixelCount1;
    result.needsLedReinit = true;
    result.needsStateRebuild = true;
  }

  if (patch.hasPixelCount2 && patch.pixelCount2 != pixelCount2) {
    pixelCount2 = patch.pixelCount2;
    result.needsLedReinit = true;
    result.needsStateRebuild = true;
  }

  if (patch.hasPixelPin1 && patch.pixelPin1 != pixelPin1) {
    pixelPin1 = patch.pixelPin1;
    result.needsLedReinit = true;
  }

  if (patch.hasPixelPin2 && patch.pixelPin2 != pixelPin2) {
    pixelPin2 = patch.pixelPin2;
    result.needsLedReinit = true;
  }

  if (patch.hasPixelDensity) {
    pixelDensity = patch.pixelDensity;
  }

  if (patch.hasLedLibrary) {
    if (!isLedLibraryKnown(patch.ledLibrary)) {
      error = "Unsupported led_library";
      return false;
    }
    if (patch.ledLibrary != ledLibrary) {
      ledLibrary = patch.ledLibrary;
      result.needsLedReinit = true;
    }
  }

  if (patch.hasObjectType) {
    if (!isSupportedObjectType(patch.objectType)) {
      error = "Unsupported object_type";
      return false;
    }
    if (patch.objectType != objectType) {
      objectType = patch.objectType;
      result.needsLedReinit = true;
      result.needsStateRebuild = true;
    }
  }

  bool requestedApiAuthEnabled = apiAuthEnabled;
  if (patch.hasApiAuthEnabled) {
    requestedApiAuthEnabled = patch.apiAuthEnabled;
  }

  if (patch.hasApiAuthToken) {
    setApiAuthToken(patch.apiAuthToken);
  }

  if (requestedApiAuthEnabled && !hasApiAuthTokenConfigured()) {
    error = "api_auth_token is required when enabling API auth";
    return false;
  }
  apiAuthEnabled = requestedApiAuthEnabled;

#ifdef OSC_ENABLED
  if (patch.hasOscEnabled && patch.oscEnabled != oscEnabled) {
    oscEnabled = patch.oscEnabled;
    result.requiresReboot = true;
  }

  if (patch.hasOscPort && patch.oscPort != oscPort) {
    oscPort = patch.oscPort;
    result.requiresReboot = true;
  }
#endif

#ifdef OTA_ENABLED
  if (patch.hasOtaEnabled && patch.otaEnabled != otaEnabled) {
    otaEnabled = patch.otaEnabled;
    result.credentialsChanged = true;
    result.requiresReboot = true;
  }

  if (patch.hasOtaPort && patch.otaPort != otaPort) {
    otaPort = patch.otaPort;
    result.credentialsChanged = true;
    result.requiresReboot = true;
  }

  if (patch.hasOtaPassword && patch.otaPassword != otaPassword) {
    otaPassword = patch.otaPassword;
    result.credentialsChanged = true;
    result.requiresReboot = true;
  }
#endif

  normalizeLedSelection();

  return true;
}

