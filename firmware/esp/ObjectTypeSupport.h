#pragma once

inline bool isSupportedObjectType(uint8_t type) {
  switch (type) {
    case OBJ_HEPTAGON919:
    case OBJ_LINE:
    case OBJ_TRIANGLE:
    case OBJ_HEPTAGON3024:
      return true;
    default:
      return false;
  }
}

