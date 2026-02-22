#pragma once

// todo: enable legacy conditionally
//#define RMT_LEGACY_SUPPORT

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
          case 2: FastLED.addLeds<WS2812, 2, RGB>(leds, count); break;
          case 4: FastLED.addLeds<WS2812, 4, RGB>(leds, count); break;
          case 5: FastLED.addLeds<WS2812, 5, RGB>(leds, count); break;
          case 12: FastLED.addLeds<WS2812, 12, RGB>(leds, count); break;
          case 13: FastLED.addLeds<WS2812, 13, RGB>(leds, count); break;
          case 14: FastLED.addLeds<WS2812, 14, RGB>(leds, count); break;
          case 15: FastLED.addLeds<WS2812, 15, RGB>(leds, count); break;
          case 16: FastLED.addLeds<WS2812, 16, RGB>(leds, count); break;
          case 18: FastLED.addLeds<WS2812, 18, RGB>(leds, count); break;
          case 19: FastLED.addLeds<WS2812, 19, RGB>(leds, count); break;
          case 21: FastLED.addLeds<WS2812, 21, RGB>(leds, count); break;
          case 22: FastLED.addLeds<WS2812, 22, RGB>(leds, count); break;
          case 23: FastLED.addLeds<WS2812, 23, RGB>(leds, count); break;
          case 25: FastLED.addLeds<WS2812, 25, RGB>(leds, count); break;
          case 26: FastLED.addLeds<WS2812, 26, RGB>(leds, count); break;
          case 27: FastLED.addLeds<WS2812, 27, RGB>(leds, count); break;
          case 32: FastLED.addLeds<WS2812, 32, RGB>(leds, count); break;
          case 33: FastLED.addLeds<WS2812, 33, RGB>(leds, count); break;
          default: FastLED.addLeds<WS2812, 14, RGB>(leds, count); break;
        }
      }
      else { //  if (IS_GRB(colorOrder))
        // WS2812 with GRB color order
        switch(pin) {
          case 2: FastLED.addLeds<WS2812, 2, GRB>(leds, count); break;
          case 4: FastLED.addLeds<WS2812, 4, GRB>(leds, count); break;
          case 5: FastLED.addLeds<WS2812, 5, GRB>(leds, count); break;
          case 12: FastLED.addLeds<WS2812, 12, GRB>(leds, count); break;
          case 13: FastLED.addLeds<WS2812, 13, GRB>(leds, count); break;
          case 14: FastLED.addLeds<WS2812, 14, GRB>(leds, count); break;
          case 15: FastLED.addLeds<WS2812, 15, GRB>(leds, count); break;
          case 16: FastLED.addLeds<WS2812, 16, GRB>(leds, count); break;
          case 18: FastLED.addLeds<WS2812, 18, GRB>(leds, count); break;
          case 19: FastLED.addLeds<WS2812, 19, GRB>(leds, count); break;
          case 21: FastLED.addLeds<WS2812, 21, GRB>(leds, count); break;
          case 22: FastLED.addLeds<WS2812, 22, GRB>(leds, count); break;
          case 23: FastLED.addLeds<WS2812, 23, GRB>(leds, count); break;
          case 25: FastLED.addLeds<WS2812, 25, GRB>(leds, count); break;
          case 26: FastLED.addLeds<WS2812, 26, GRB>(leds, count); break;
          case 27: FastLED.addLeds<WS2812, 27, GRB>(leds, count); break;
          case 32: FastLED.addLeds<WS2812, 32, GRB>(leds, count); break;
          case 33: FastLED.addLeds<WS2812, 33, GRB>(leds, count); break;
          default: FastLED.addLeds<WS2812, 14, GRB>(leds, count); break;
        }
      }
    } else {
      // LED_WS2811
      if (IS_RGB(colorOrder)) {
        // WS2811 with RGB color order
        switch(pin) {
          case 2: FastLED.addLeds<WS2811, 2, RGB>(leds, count); break;
          case 4: FastLED.addLeds<WS2811, 4, RGB>(leds, count); break;
          case 5: FastLED.addLeds<WS2811, 5, RGB>(leds, count); break;
          case 12: FastLED.addLeds<WS2811, 12, RGB>(leds, count); break;
          case 13: FastLED.addLeds<WS2811, 13, RGB>(leds, count); break;
          case 14: FastLED.addLeds<WS2811, 14, RGB>(leds, count); break;
          case 15: FastLED.addLeds<WS2811, 15, RGB>(leds, count); break;
          case 16: FastLED.addLeds<WS2811, 16, RGB>(leds, count); break;
          case 18: FastLED.addLeds<WS2811, 18, RGB>(leds, count); break;
          case 19: FastLED.addLeds<WS2811, 19, RGB>(leds, count); break;
          case 21: FastLED.addLeds<WS2811, 21, RGB>(leds, count); break;
          case 22: FastLED.addLeds<WS2811, 22, RGB>(leds, count); break;
          case 23: FastLED.addLeds<WS2811, 23, RGB>(leds, count); break;
          case 25: FastLED.addLeds<WS2811, 25, RGB>(leds, count); break;
          case 26: FastLED.addLeds<WS2811, 26, RGB>(leds, count); break;
          case 27: FastLED.addLeds<WS2811, 27, RGB>(leds, count); break;
          case 32: FastLED.addLeds<WS2811, 32, RGB>(leds, count); break;
          case 33: FastLED.addLeds<WS2811, 33, RGB>(leds, count); break;
          default: FastLED.addLeds<WS2811, 14, RGB>(leds, count); break;
        }
      }
      else {
        // WS2811 with GRB color order
        switch(pin) {
          case 2: FastLED.addLeds<WS2811, 2, GRB>(leds, count); break;
          case 4: FastLED.addLeds<WS2811, 4, GRB>(leds, count); break;
          case 5: FastLED.addLeds<WS2811, 5, GRB>(leds, count); break;
          case 12: FastLED.addLeds<WS2811, 12, GRB>(leds, count); break;
          case 13: FastLED.addLeds<WS2811, 13, GRB>(leds, count); break;
          case 14: FastLED.addLeds<WS2811, 14, GRB>(leds, count); break;
          case 15: FastLED.addLeds<WS2811, 15, GRB>(leds, count); break;
          case 16: FastLED.addLeds<WS2811, 16, GRB>(leds, count); break;
          case 18: FastLED.addLeds<WS2811, 18, GRB>(leds, count); break;
          case 19: FastLED.addLeds<WS2811, 19, GRB>(leds, count); break;
          case 21: FastLED.addLeds<WS2811, 21, GRB>(leds, count); break;
          case 22: FastLED.addLeds<WS2811, 22, GRB>(leds, count); break;
          case 23: FastLED.addLeds<WS2811, 23, GRB>(leds, count); break;
          case 25: FastLED.addLeds<WS2811, 25, GRB>(leds, count); break;
          case 26: FastLED.addLeds<WS2811, 26, GRB>(leds, count); break;
          case 27: FastLED.addLeds<WS2811, 27, GRB>(leds, count); break;
          case 32: FastLED.addLeds<WS2811, 32, GRB>(leds, count); break;
          case 33: FastLED.addLeds<WS2811, 33, GRB>(leds, count); break;
          default: FastLED.addLeds<WS2811, 14, GRB>(leds, count); break;
        }
      }
    }
    if (HAS_WHITE(colorOrder)) {
      c.setRgbw(RgbwDefault());
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
