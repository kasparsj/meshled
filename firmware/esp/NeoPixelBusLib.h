#pragma once

#define COLORGAMMA_CORRECT

#ifdef COLORGAMMA_CORRECT
NeoGamma<NeoGammaTableMethod> colorGamma;
#endif

NeoPixelBusStrip* createNeoPixelBusStrip(uint8_t ledType, uint8_t colorOrder, uint16_t pixelCount, uint8_t pixelPin) {
  static uint8_t i = 0;
  i++;

  // Determine which RMT channel to use based on which strip is being created
  if (ledType == LED_WS2812) {
    // WS2812 LED type
    if (i == 1) {
      // First strip uses RMT channel 0
      switch (colorOrder) {
        case CO_GRBW:
          return new NeoPixelBusGrbwStrip<NeoEsp32Rmt0Ws2812xMethod>(pixelCount, pixelPin);
        case CO_RGBW:
          return new NeoPixelBusRgbwStrip<NeoEsp32Rmt0Ws2812xMethod>(pixelCount, pixelPin);
        case CO_RGB:
          return new NeoPixelBusRgbStrip<NeoEsp32Rmt0Ws2812xMethod>(pixelCount, pixelPin);
        default:
        case CO_GRB:
          return new NeoPixelBusGrbStrip<NeoEsp32Rmt0Ws2812xMethod>(pixelCount, pixelPin);
      }
    } else {
      // Second strip uses RMT channel 1
      switch (colorOrder) {
        case CO_GRBW:
          return new NeoPixelBusGrbwStrip<NeoEsp32Rmt1Ws2812xMethod>(pixelCount, pixelPin);
        case CO_RGBW:
          return new NeoPixelBusRgbwStrip<NeoEsp32Rmt1Ws2812xMethod>(pixelCount, pixelPin);
        case CO_RGB:
          return new NeoPixelBusRgbStrip<NeoEsp32Rmt1Ws2812xMethod>(pixelCount, pixelPin);
        default:
        case CO_GRB:
          return new NeoPixelBusGrbStrip<NeoEsp32Rmt1Ws2812xMethod>(pixelCount, pixelPin);
      }
    }
  } else {
    // WS2811 LED type
    if (i == 1) {
      // First strip uses RMT channel 0
      switch (colorOrder) {
        case CO_GRBW:
          return new NeoPixelBusGrbwStrip<NeoEsp32Rmt0Ws2811Method>(pixelCount, pixelPin);
        case CO_RGBW:
          return new NeoPixelBusRgbwStrip<NeoEsp32Rmt0Ws2811Method>(pixelCount, pixelPin);
        case CO_RGB:
          return new NeoPixelBusRgbStrip<NeoEsp32Rmt0Ws2811Method>(pixelCount, pixelPin);
        default:
        case CO_GRB:
          return new NeoPixelBusGrbStrip<NeoEsp32Rmt0Ws2811Method>(pixelCount, pixelPin);
      }
    } else {
      // Second strip uses RMT channel 1
      switch (colorOrder) {
        case CO_GRBW:
          return new NeoPixelBusGrbwStrip<NeoEsp32Rmt1Ws2811Method>(pixelCount, pixelPin);
        case CO_RGBW:
          return new NeoPixelBusRgbwStrip<NeoEsp32Rmt1Ws2811Method>(pixelCount, pixelPin);
        case CO_RGB:
          return new NeoPixelBusRgbStrip<NeoEsp32Rmt1Ws2811Method>(pixelCount, pixelPin);
        default:
        case CO_GRB:
          return new NeoPixelBusGrbStrip<NeoEsp32Rmt1Ws2811Method>(pixelCount, pixelPin);
      }
    }
  }
}

void setupNeoPixelBus() {
  if (ledLibrary != LIB_NEOPIXELBUS) return;

  // Clean up previous instances if they exist
  if (strip1 != NULL) {
    delete strip1;
    strip1 = NULL;
  }

  if (strip2 != NULL) {
    delete strip2;
    strip2 = NULL;
  }

  strip1 = createNeoPixelBusStrip(ledType, colorOrder, pixelCount1, pixelPin1);
  strip1->Begin();
  strip1->Show();

  if (pixelCount2 > 0) {
    strip2 = createNeoPixelBusStrip(ledType, colorOrder, pixelCount2, pixelPin2);
    strip2->Begin();
    strip2->Show();
  }

  LP_LOGLN("NeoPixelBus initialized with colorOrder = " + String(colorOrder));
}

RgbwColor handleWhite(RgbwColor color) {
  if (HAS_WHITE(colorOrder)) {
    uint8_t minRGB = min(color.R, min(color.G, color.B));
    color.R -= minRGB;
    color.G -= minRGB;
    color.B -= minRGB;
    color.W = minRGB;
  }
  return color;
}

RgbwColor getNeoPixelColor(uint16_t i) {
  ColorRGB pixel = state->getPixel(state->object.translateToLogicalPixel(i), maxBrightness);
  RgbwColor color = handleWhite(RgbwColor(pixel.R, pixel.G, pixel.B, 0));
  #ifdef DEBUGGER_ENABLED
  if (state->showConnections) {
    color.G = (debugger->isConnection(i) ? 1.f : 0.f) * maxBrightness;
  }
  if (state->showIntersections) {
    color.B = (debugger->isIntersection(i) ? 1.f : 0.f) * maxBrightness;
  }
  #endif
  #ifdef COLORGAMMA_CORRECT
  return colorGamma.Correct(color);
  #else
  return color;
  #endif
}

float getWatts(RgbColor color, float wattsPerLed = 0.2f) {
  float relPixelPower = (float) (color.R + color.G + color.B) / (255.f * 3.f);
  float wattsPerPixel = wattsPerLed * 3.f;
  return relPixelPower * wattsPerPixel;
}

float getWatts(RgbwColor color, float wattsPerLed = 0.2f) {
  float relPixelPower = (float) (color.R + color.G + color.B + color.W) / (255.f * 4.f);
  float wattsPerPixel = wattsPerLed * 4.f;
  return relPixelPower * wattsPerPixel;
}

void drawNeoPixelBus() {
  if (ledLibrary == LIB_NEOPIXELBUS && strip1 != NULL) {
    totalWattage = 0;
    // For the first strip
    for (uint16_t i=0; i<pixelCount1; i++) {
      RgbwColor color = getNeoPixelColor(i);
      if (strip1->SupportsRgbw()) {
        totalWattage += getWatts(color);
        strip1->SetPixelColor(i, color);
      } else {
        RgbColor rgb = RgbColor(color.R, color.G, color.B);
        totalWattage += getWatts(rgb);
        strip1->SetPixelColor(i, rgb);
      }
    }
    strip1->Show();

    // For the second strip (if present)
    if (pixelCount2 > 0 && strip2 != NULL) {
      for (uint16_t i=0; i<pixelCount2; i++) {
        RgbwColor color = getNeoPixelColor(pixelCount1+i);
        if (strip2->SupportsRgbw()) {
          totalWattage += getWatts(color);
          strip2->SetPixelColor(i, color);
        } else {
          RgbColor rgb = RgbColor(color.R, color.G, color.B);
          totalWattage += getWatts(rgb);
          strip2->SetPixelColor(i, rgb);
        }
      }
      strip2->Show();
    }
  }
}
