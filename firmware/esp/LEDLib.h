#pragma once

#include <optional>
#include "ObjectTypeSupport.h"

#ifdef FASTLED_ENABLED
// Use I2S backend on ESP32 to avoid RMT legacy/new-driver conflicts at runtime.
#if defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_IDF_TARGET_ESP32) && !defined(FASTLED_ESP32_I2S)
#define FASTLED_ESP32_I2S 1
#endif
#include <FastLED.h>
CRGB* leds1 = NULL;
CRGB* leds2 = NULL;
#include "FastLEDLib.h"
#endif

#ifdef NEOPIXELBUS_ENABLED
#include "NeoPixelBusStrip.h"
// These are just declarations - actual initialization happens in setupNeoPixelBus()
NeoPixelBusStrip* strip1 = NULL;
NeoPixelBusStrip* strip2 = NULL;
#include "NeoPixelBusLib.h"
#endif

static constexpr uint8_t KNOWN_LED_LIBRARIES[] = {
  LIB_NEOPIXELBUS,
  LIB_FASTLED
};

static constexpr uint8_t KNOWN_LED_TYPES[] = {
  LED_WS2812,
  LED_WS2811,
  LED_WS2815,
  LED_WS2813,
  LED_WS2816,
  LED_SK6812,
  LED_TM1829,
  LED_APA106,
  LED_WS2814,
  LED_TM1814,
  LED_TM1914,
  LED_TM1812,
  LED_TM1809,
  LED_TM1804,
  LED_TM1803,
  LED_UCS1903,
  LED_UCS1903B,
  LED_UCS1904,
  LED_UCS2903,
  LED_SK6822,
  LED_APA104,
  LED_WS2811_400,
  LED_GS1903,
  LED_GW6205,
  LED_GW6205_400,
  LED_LPD1886,
  LED_LPD1886_8BIT,
  LED_PL9823,
  LED_UCS1912,
  LED_SM16703,
  LED_SM16824E
};

size_t knownLedLibraryCount() {
  return sizeof(KNOWN_LED_LIBRARIES) / sizeof(KNOWN_LED_LIBRARIES[0]);
}

uint8_t knownLedLibraryAt(size_t index) {
  return KNOWN_LED_LIBRARIES[index];
}

size_t knownLedTypeCount() {
  return sizeof(KNOWN_LED_TYPES) / sizeof(KNOWN_LED_TYPES[0]);
}

uint8_t knownLedTypeAt(size_t index) {
  return KNOWN_LED_TYPES[index];
}

bool isLedTypeKnown(uint8_t ledType) {
  for (size_t i = 0; i < knownLedTypeCount(); i++) {
    if (knownLedTypeAt(i) == ledType) {
      return true;
    }
  }
  return false;
}

bool isLedLibraryCompiled(uint8_t lib) {
  switch (lib) {
    case LIB_NEOPIXELBUS:
      #ifdef NEOPIXELBUS_ENABLED
      return true;
      #else
      return false;
      #endif
    case LIB_FASTLED:
      #ifdef FASTLED_ENABLED
      return true;
      #else
      return false;
      #endif
    default:
      return false;
  }
}

bool isLedLibraryRuntimeAllowed(uint8_t lib) {
  // ESP32-C3 can still hit legacy/new RMT conflicts depending on selected methods.
  #if defined(CONFIG_IDF_TARGET_ESP32C3)
  if (lib == LIB_NEOPIXELBUS) {
    return false;
  }
  #endif
  return true;
}

bool isLedLibraryAvailable(uint8_t lib) {
  return isLedLibraryCompiled(lib) && isLedLibraryRuntimeAllowed(lib);
}

const char* ledLibraryUnavailableReason(uint8_t lib) {
  if (!isLedLibraryCompiled(lib)) {
    return "Not compiled into this firmware";
  }

  if (!isLedLibraryRuntimeAllowed(lib)) {
    #if defined(CONFIG_IDF_TARGET_ESP32C3)
    if (lib == LIB_NEOPIXELBUS) {
      return "Disabled on ESP32-C3 to avoid RMT driver conflict";
    }
    #endif
    return "Not allowed on this target";
  }

  return NULL;
}

uint8_t defaultAvailableLedLibrary() {
  if (isLedLibraryAvailable(LIB_FASTLED)) {
    return LIB_FASTLED;
  }
  if (isLedLibraryAvailable(LIB_NEOPIXELBUS)) {
    return LIB_NEOPIXELBUS;
  }
  return ledLibrary;
}

bool doesLedLibrarySupportType(uint8_t lib, uint8_t ledType) {
  switch (lib) {
    case LIB_FASTLED:
      switch (ledType) {
        case LED_WS2812:
        case LED_WS2811:
        case LED_WS2815:
        case LED_WS2813:
        case LED_WS2816:
        case LED_SK6812:
        case LED_TM1829:
        case LED_APA106:
        case LED_TM1812:
        case LED_TM1809:
        case LED_TM1804:
        case LED_TM1803:
        case LED_UCS1903:
        case LED_UCS1903B:
        case LED_UCS1904:
        case LED_UCS2903:
        case LED_SK6822:
        case LED_APA104:
        case LED_WS2811_400:
        case LED_GS1903:
        case LED_GW6205:
        case LED_GW6205_400:
        case LED_LPD1886:
        case LED_LPD1886_8BIT:
        case LED_PL9823:
        case LED_UCS1912:
        case LED_SM16703:
        case LED_SM16824E:
          return true;
        default:
          return false;
      }
    case LIB_NEOPIXELBUS:
      switch (ledType) {
        case LED_WS2812:
        case LED_WS2811:
        case LED_WS2815:
        case LED_WS2813:
        case LED_WS2816:
        case LED_SK6812:
        case LED_TM1829:
        case LED_APA106:
        case LED_WS2814:
        case LED_TM1814:
        case LED_TM1914:
          return true;
        default:
          return false;
      }
    default:
      return false;
  }
}

bool isLedTypeAvailableForLibrary(uint8_t ledType, uint8_t lib) {
  return isLedLibraryAvailable(lib) && doesLedLibrarySupportType(lib, ledType);
}

uint8_t availableLedLibraryCountForType(uint8_t ledType) {
  uint8_t count = 0;
  for (size_t i = 0; i < knownLedLibraryCount(); i++) {
    if (isLedTypeAvailableForLibrary(ledType, knownLedLibraryAt(i))) {
      count++;
    }
  }
  return count;
}

bool isLedTypeAvailable(uint8_t ledType) {
  return availableLedLibraryCountForType(ledType) > 0;
}

const char* ledTypeUnavailableReason(uint8_t ledType) {
  if (!isLedTypeKnown(ledType)) {
    return "Unknown LED type";
  }
  if (isLedTypeAvailable(ledType)) {
    return NULL;
  }
  return "Not supported by installed LED libraries";
}

uint8_t defaultAvailableLedType() {
  if (isLedTypeAvailable(ledType)) {
    return ledType;
  }

  for (size_t i = 0; i < knownLedTypeCount(); i++) {
    uint8_t candidateType = knownLedTypeAt(i);
    if (isLedTypeAvailable(candidateType)) {
      return candidateType;
    }
  }

  return ledType;
}

uint8_t firstAvailableLedLibraryForType(uint8_t ledType) {
  for (size_t i = 0; i < knownLedLibraryCount(); i++) {
    uint8_t candidateLibrary = knownLedLibraryAt(i);
    if (isLedTypeAvailableForLibrary(ledType, candidateLibrary)) {
      return candidateLibrary;
    }
  }
  return defaultAvailableLedLibrary();
}

void normalizeLedSelection() {
  if (!isLedLibraryAvailable(ledLibrary)) {
    const char* reason = ledLibraryUnavailableReason(ledLibrary);
    if (reason != NULL) {
      LP_LOGLN("Selected LED library is unavailable: " + String(reason));
    } else {
      LP_LOGLN("Selected LED library is unavailable");
    }

    uint8_t fallbackLibrary = defaultAvailableLedLibrary();
    if (fallbackLibrary != ledLibrary) {
      LP_LOGLN("Switching LED library to " + String(fallbackLibrary == LIB_FASTLED ? "FastLED" : "NeoPixelBus"));
      ledLibrary = fallbackLibrary;
    }
  }

  if (!isLedTypeAvailable(ledType)) {
    const char* reason = ledTypeUnavailableReason(ledType);
    if (reason != NULL) {
      LP_LOGLN("Selected LED type is unavailable: " + String(reason));
    } else {
      LP_LOGLN("Selected LED type is unavailable");
    }

    uint8_t fallbackType = defaultAvailableLedType();
    if (fallbackType != ledType) {
      LP_LOGLN("Switching LED type to " + String(fallbackType));
      ledType = fallbackType;
    }
  }

  if (!isLedTypeAvailableForLibrary(ledType, ledLibrary)) {
    uint8_t fallbackLibrary = firstAvailableLedLibraryForType(ledType);
    if (fallbackLibrary != ledLibrary && isLedTypeAvailableForLibrary(ledType, fallbackLibrary)) {
      LP_LOGLN("Switching LED library to match LED type " + String(ledType) + ": " +
               String(fallbackLibrary == LIB_FASTLED ? "FastLED" : "NeoPixelBus"));
      ledLibrary = fallbackLibrary;
    }
  }
}

void normalizeLedLibrarySelection() {
  normalizeLedSelection();
}

void normalizeObjectTypeSelection() {
  if (!isSupportedObjectType(objectType)) {
    LP_LOGLN("Unsupported objectType=" + String(objectType) + ", falling back to OBJ_LINE");
    objectType = OBJ_LINE;
  }
}

void setupLEDs() {
  normalizeObjectTypeSelection();

  if (objectType == OBJ_HEPTAGON919) {
    pixelCount1 = HEPTAGON919_PIXEL_COUNT1;
    pixelCount2 = HEPTAGON919_PIXEL_COUNT2;
    pixelPin1 = 14;
    pixelPin2 = 26;
  }
  else if (objectType == OBJ_HEPTAGON3024) {
    pixelCount1 = HEPTAGON3024_REAL_PIXEL_COUNT1;
    pixelCount2 = HEPTAGON3024_REAL_PIXEL_COUNT2;
    pixelPin1 = 14;
    pixelPin2 = 26;
  }

  normalizeLedLibrarySelection();

  #ifdef NEOPIXELBUS_ENABLED
  setupNeoPixelBus();
  #endif

  #ifdef FASTLED_ENABLED
  setupFastLED();
  #endif
}

void setupState() {
  normalizeObjectTypeSelection();

  if (objectType == OBJ_HEPTAGON919) {
    object = new Heptagon919();
  }
  else if (objectType == OBJ_HEPTAGON3024) {
    object = new Heptagon3024();
  }
  else if (objectType == OBJ_LINE) {
    object = new Line(pixelCount1 + pixelCount2);
  }
  else if (objectType == OBJ_TRIANGLE) {
    object = new Triangle(pixelCount1 + pixelCount2);
  }
  else {
    object = new Line(pixelCount1 + pixelCount2);
  }

  state = new State(*object);

  // Set auto emitter parameters
  State::autoParams.from = emitterFrom;

  // Set auto emitter enabled state
  state->autoEnabled = emitterEnabled;

  LP_LOGLN("State initialized");
}

void rebuildRuntimeState() {
  #ifdef DEBUGGER_ENABLED
  if (debugger != nullptr) {
    delete debugger;
    debugger = nullptr;
  }
  #endif

  if (state != nullptr) {
    delete state;
    state = nullptr;
  }
  if (object != nullptr) {
    delete object;
    object = nullptr;
  }

  setupState();

  #ifdef SPIFFS_ENABLED
  loadLayers();
  #endif

  #ifdef DEBUGGER_ENABLED
  debugger = new Debugger(*object);
  #endif
}

void updateLEDs() {
  gMillis = millis();
  #ifdef DEBUGGER_ENABLED
  debugger->update(gMillis);
  #endif
  state->autoEmit(gMillis);
  state->update();
}

void drawLEDs() {
  #ifdef NEOPIXELBUS_ENABLED
  drawNeoPixelBus();
  #endif

  #ifdef FASTLED_ENABLED
  drawFastLED();
  #endif
}

void doEmit(EmitParams &params) {
  int8_t i = state->emit(params);
  #ifdef DEBUGGER_ENABLED
  debugger->countEmit();
  #endif
  // todo: I think HD_DEBUG does not exist anymore
  #ifdef HD_DEBUG
  LP_LOGF("emitting %d %s lights (%d/%.1f/%d/%d/%d/%d), total: %d (%d)\n",
    state->lightLists[i]->numLights,
    (params.linked ? "linked" : "random"),
    params.model,
    params.speed,
    state->lightLists[i]->length,
    state->lightLists[i]->lifeMillis,
    state->lightLists[i]->maxBri,
    params.fadeSpeed,
    state->totalLights + state->lightLists[i]->numLights,
    state->totalLightLists + 1);
  #endif
}

void doCommand(char command) {
  switch (command) {
    case 'r':
      ESP.restart();
      break;
    case '.':
      state->stopAll();
      break;
    case '!':
      state->colorAll();
      break;
    case 's':
      state->splitAll();
      break;
    case 'h':
      LP_LOGF("Free heap: %d\n", ESP.getFreeHeap());
      break;
    #ifdef DEBUGGER_ENABLED
    case 'f':
      LP_LOGF("FPS: %f (%d)\n", debugger->getFPS(), state->totalLights);
      break;
    case 'e':
      LP_LOGF("Emits per frame: %f\n", debugger->getNumEmits());
      break;
    case 'a':
      if (state && state->lightLists[0]) {
        state->lightLists[0]->visible = !state->lightLists[0]->visible;
      }
      #ifdef SPIFFS_ENABLED
      saveLayers();
      #endif
      break;
    case 'i':
      state->showIntersections = !state->showIntersections;
      break;
    case 'c':
      state->showConnections = !state->showConnections;
      break;
    case '>':
      if (state->currentPalette < 32)
      state->currentPalette++;
      break;
    case '<':
      if (state->currentPalette > 0) {
        state->currentPalette--;
      }
      break;
    case 'l':
      LP_LOGF("Total %d lights\n", state->totalLights);
      break;
    case 'E':
      state->autoEnabled = !state->autoEnabled;
      // Update the global emitterEnabled variable to match state->autoEnabled
      emitterEnabled = state->autoEnabled;
      LP_LOGF("AutoEmitter is %s\n", state->autoEnabled ? "enabled" : "disabled");
      #ifdef SPIFFS_ENABLED
      saveSettings();
      #endif
      break;
    case 'L':
      state->debug();
      break;
    case 'C':
      debugger->dumpConnections();
      break;
    case 'I':
      debugger->dumpIntersections();
      break;
    #endif
    default:
      if (std::optional<EmitParams> params = object->getParams(command)) {
        doEmit(*params);
      }
      break;
  }
}
