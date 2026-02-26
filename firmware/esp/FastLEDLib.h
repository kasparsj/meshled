#pragma once

// todo: enable legacy conditionally
//#define RMT_LEGACY_SUPPORT

#define FASTLED_ADD_PIN(CHIPSET, PIN, ORDER, LEDS, COUNT, COLOR_ORDER)    \
  do {                                                                      \
    auto& _fastledController = FastLED.addLeds<CHIPSET, PIN, ORDER>(LEDS, COUNT); \
    if (HAS_WHITE(COLOR_ORDER)) {                                           \
      _fastledController.setRgbw();                                         \
    }                                                                       \
  } while (0)

#if defined(CONFIG_IDF_TARGET_ESP32C3)
#define FASTLED_ADD_PIN_CASES(CHIPSET, ORDER, LEDS, COUNT)           \
  case 0: FASTLED_ADD_PIN(CHIPSET, 0, ORDER, LEDS, COUNT, colorOrder); break;    \
  case 1: FASTLED_ADD_PIN(CHIPSET, 1, ORDER, LEDS, COUNT, colorOrder); break;    \
  case 2: FASTLED_ADD_PIN(CHIPSET, 2, ORDER, LEDS, COUNT, colorOrder); break;    \
  case 3: FASTLED_ADD_PIN(CHIPSET, 3, ORDER, LEDS, COUNT, colorOrder); break;    \
  case 4: FASTLED_ADD_PIN(CHIPSET, 4, ORDER, LEDS, COUNT, colorOrder); break;    \
  case 5: FASTLED_ADD_PIN(CHIPSET, 5, ORDER, LEDS, COUNT, colorOrder); break;    \
  case 6: FASTLED_ADD_PIN(CHIPSET, 6, ORDER, LEDS, COUNT, colorOrder); break;    \
  case 7: FASTLED_ADD_PIN(CHIPSET, 7, ORDER, LEDS, COUNT, colorOrder); break;    \
  case 8: FASTLED_ADD_PIN(CHIPSET, 8, ORDER, LEDS, COUNT, colorOrder); break;    \
  case 9: FASTLED_ADD_PIN(CHIPSET, 9, ORDER, LEDS, COUNT, colorOrder); break;    \
  case 10: FASTLED_ADD_PIN(CHIPSET, 10, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 18: FASTLED_ADD_PIN(CHIPSET, 18, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 19: FASTLED_ADD_PIN(CHIPSET, 19, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 20: FASTLED_ADD_PIN(CHIPSET, 20, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 21: FASTLED_ADD_PIN(CHIPSET, 21, ORDER, LEDS, COUNT, colorOrder); break;
#define FASTLED_DEFAULT_PIN 2
#else
#define FASTLED_ADD_PIN_CASES(CHIPSET, ORDER, LEDS, COUNT)           \
  case 2: FASTLED_ADD_PIN(CHIPSET, 2, ORDER, LEDS, COUNT, colorOrder); break;    \
  case 4: FASTLED_ADD_PIN(CHIPSET, 4, ORDER, LEDS, COUNT, colorOrder); break;    \
  case 5: FASTLED_ADD_PIN(CHIPSET, 5, ORDER, LEDS, COUNT, colorOrder); break;    \
  case 12: FASTLED_ADD_PIN(CHIPSET, 12, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 13: FASTLED_ADD_PIN(CHIPSET, 13, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 14: FASTLED_ADD_PIN(CHIPSET, 14, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 15: FASTLED_ADD_PIN(CHIPSET, 15, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 16: FASTLED_ADD_PIN(CHIPSET, 16, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 18: FASTLED_ADD_PIN(CHIPSET, 18, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 19: FASTLED_ADD_PIN(CHIPSET, 19, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 21: FASTLED_ADD_PIN(CHIPSET, 21, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 22: FASTLED_ADD_PIN(CHIPSET, 22, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 23: FASTLED_ADD_PIN(CHIPSET, 23, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 25: FASTLED_ADD_PIN(CHIPSET, 25, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 26: FASTLED_ADD_PIN(CHIPSET, 26, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 27: FASTLED_ADD_PIN(CHIPSET, 27, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 32: FASTLED_ADD_PIN(CHIPSET, 32, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 33: FASTLED_ADD_PIN(CHIPSET, 33, ORDER, LEDS, COUNT, colorOrder); break;
#define FASTLED_DEFAULT_PIN 14
#endif

void setupFastLED() {
  if (ledLibrary != LIB_FASTLED) return;

  leds1 = new CRGB[pixelCount1];
  leds2 = new CRGB[pixelCount2];

  // Helper function to add LEDs with the right configuration
  auto addLedsWithConfig = [](CRGB* leds, uint16_t count, uint8_t pin, uint8_t ledType, uint8_t colorOrder) {
    if (ledType == LED_WS2812) {
      if (IS_RGB(colorOrder)) {
        // WS2812 with RGB color order
        switch(pin) {
          FASTLED_ADD_PIN_CASES(WS2812, RGB, leds, count)
          default: FASTLED_ADD_PIN(WS2812, FASTLED_DEFAULT_PIN, RGB, leds, count, colorOrder); break;
        }
      }
      else { //  if (IS_GRB(colorOrder))
        // WS2812 with GRB color order
        switch(pin) {
          FASTLED_ADD_PIN_CASES(WS2812, GRB, leds, count)
          default: FASTLED_ADD_PIN(WS2812, FASTLED_DEFAULT_PIN, GRB, leds, count, colorOrder); break;
        }
      }
    } else {
      // LED_WS2811
      if (IS_RGB(colorOrder)) {
        // WS2811 with RGB color order
        switch(pin) {
          FASTLED_ADD_PIN_CASES(WS2811, RGB, leds, count)
          default: FASTLED_ADD_PIN(WS2811, FASTLED_DEFAULT_PIN, RGB, leds, count, colorOrder); break;
        }
      }
      else {
        // WS2811 with GRB color order
        switch(pin) {
          FASTLED_ADD_PIN_CASES(WS2811, GRB, leds, count)
          default: FASTLED_ADD_PIN(WS2811, FASTLED_DEFAULT_PIN, GRB, leds, count, colorOrder); break;
        }
      }
    }
  };

  // Initialize strip 1
  addLedsWithConfig(leds1, pixelCount1, pixelPin1, ledType, colorOrder);

  // Initialize strip 2 if needed
  if (pixelCount2 > 0) {
    addLedsWithConfig(leds2, pixelCount2, pixelPin2, ledType, colorOrder);
  }

  FastLED.setBrightness(maxBrightness);
  FastLED.clear();
  FastLED.show();

  LP_LOGLN("FastLED initialized with ledType = " + String(ledType == LED_WS2812 ? "WS2812" : "WS2811") +
                 ", colorOrder = " + String(colorOrder == CO_GRB ? "GRB" : "RGB"));
}

CRGB getFastLEDColor(uint16_t i) {
  ColorRGB pixel = state->getPixel(i, maxBrightness);
  CRGB color = CRGB(pixel.R, pixel.G, pixel.B);

  #ifdef DEBUGGER_ENABLED
  if (state->showConnections) {
    color.g = (debugger->isConnection(i) ? 1.f : 0.f) * maxBrightness;
  }
  if (state->showIntersections) {
    color.b = (debugger->isIntersection(i) ? 1.f : 0.f) * maxBrightness;
  }
  #endif
  return color;
}

void drawFastLED() {
  if (ledLibrary == LIB_FASTLED && leds1 != NULL) {
    for (uint16_t i=0; i<pixelCount1; i++) {
      leds1[i] = getFastLEDColor(i);
    }
    if (pixelCount2 > 0 && leds2 != NULL) {
      for (uint16_t i=0; i<pixelCount2; i++) {
        leds2[i] = getFastLEDColor(pixelCount1+i);
      }
    }
    FastLED.setBrightness(maxBrightness);
    FastLED.show();
  }
}
