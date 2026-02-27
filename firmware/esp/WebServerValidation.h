#pragma once

#include <errno.h>
#include <stdlib.h>

inline bool parseStrictLong(const String& rawValue, long& parsedValue) {
  if (rawValue.length() == 0) {
    return false;
  }

  errno = 0;
  char* end = nullptr;
  const long value = strtol(rawValue.c_str(), &end, 10);
  if (errno != 0 || end == rawValue.c_str() || *end != '\0') {
    return false;
  }

  parsedValue = value;
  return true;
}

inline bool parseArgLong(const char* argName, long minValue, long maxValue, long& parsedValue) {
  if (!server.hasArg(argName)) {
    return false;
  }

  long value = 0;
  if (!parseStrictLong(server.arg(argName), value)) {
    return false;
  }

  if (value < minValue || value > maxValue) {
    return false;
  }

  parsedValue = value;
  return true;
}

inline bool parseLayerArg(const char* argName, uint8_t& layerIndex) {
  long parsed = 0;
  if (!parseArgLong(argName, 0, MAX_LIGHT_LISTS - 1, parsed)) {
    return false;
  }
  layerIndex = static_cast<uint8_t>(parsed);
  return true;
}
