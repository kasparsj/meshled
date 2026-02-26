#pragma once

#include <sdkconfig.h>

#define COLORGAMMA_CORRECT

#ifdef COLORGAMMA_CORRECT
NeoGamma<NeoGammaTableMethod> colorGamma;
#endif

// Arduino IDE and PlatformIO builds can expose different target-id macros.
// Keep S3 detection broad so we never fall back to legacy RMT methods there.
#if defined(CONFIG_IDF_TARGET_ESP32S3) || defined(ARDUINO_ESP32S3_DEV) || defined(ARDUINO_ESP32S3_USB_OTG) || defined(ARDUINO_ESP32S3_BOX) || defined(ARDUINO_ESP32S3_BOX_3)
#define NPB_TARGET_ESP32S3
#endif

#if defined(CONFIG_IDF_TARGET_ESP32)
#define NPB_TARGET_ESP32_CLASSIC
#endif

// ESP32 Arduino core v3 can mix RMT legacy/new drivers when multiple libs touch RMT.
// On classic ESP32, use I2S methods to avoid RMT driver conflicts.
#if defined(NPB_TARGET_ESP32_CLASSIC)
#define NPB_METHOD_STRIP1_WS2812X NeoEsp32I2s0Ws2812xMethod
#define NPB_METHOD_STRIP2_WS2812X NeoEsp32I2s1Ws2812xMethod
#define NPB_METHOD_STRIP1_WS2811 NeoEsp32I2s0400KbpsMethod
#define NPB_METHOD_STRIP2_WS2811 NeoEsp32I2s1400KbpsMethod
#define NPB_TRANSPORT_NAME "esp32-i2s"
#elif defined(NPB_TARGET_ESP32S3)
// On ESP32-S3 (IDF5), avoid legacy RMT to prevent startup conflicts with core RMT NG usage.
// LCD-X methods are the native non-RMT transport in NeoPixelBus for S3.
#define NPB_METHOD_STRIP1_WS2812X NeoEsp32LcdX8Ws2812xMethod
#define NPB_METHOD_STRIP2_WS2812X NeoEsp32LcdX8Ws2812xMethod
#define NPB_METHOD_STRIP1_WS2811 NeoEsp32LcdX8Ws2811Method
#define NPB_METHOD_STRIP2_WS2811 NeoEsp32LcdX8Ws2811Method
#define NPB_TRANSPORT_NAME "esp32s3-lcdx"
#else
#define NPB_METHOD_STRIP1_WS2812X NeoEsp32Rmt0Ws2812xMethod
#define NPB_METHOD_STRIP2_WS2812X NeoEsp32Rmt1Ws2812xMethod
#define NPB_METHOD_STRIP1_WS2811 NeoEsp32Rmt0Ws2811Method
#define NPB_METHOD_STRIP2_WS2811 NeoEsp32Rmt1Ws2811Method
#define NPB_TRANSPORT_NAME "esp32-rmt-legacy"
#endif

NeoPixelBusStrip* createNeoPixelBusStrip(uint8_t ledType, uint8_t colorOrder, uint16_t pixelCount, uint8_t pixelPin) {
  static uint8_t i = 0;
  i++;

  // Determine which transport instance to use based on which strip is being created
  if (ledType == LED_WS2812) {
    // WS2812 LED type
    if (i == 1) {
      // First strip
      switch (colorOrder) {
        case CO_GRBW:
          return new NeoPixelBusGrbwStrip<NPB_METHOD_STRIP1_WS2812X>(pixelCount, pixelPin);
        case CO_RGBW:
          return new NeoPixelBusRgbwStrip<NPB_METHOD_STRIP1_WS2812X>(pixelCount, pixelPin);
        case CO_RGB:
          return new NeoPixelBusRgbStrip<NPB_METHOD_STRIP1_WS2812X>(pixelCount, pixelPin);
        default:
        case CO_GRB:
          return new NeoPixelBusGrbStrip<NPB_METHOD_STRIP1_WS2812X>(pixelCount, pixelPin);
      }
    } else {
      // Second strip
      switch (colorOrder) {
        case CO_GRBW:
          return new NeoPixelBusGrbwStrip<NPB_METHOD_STRIP2_WS2812X>(pixelCount, pixelPin);
        case CO_RGBW:
          return new NeoPixelBusRgbwStrip<NPB_METHOD_STRIP2_WS2812X>(pixelCount, pixelPin);
        case CO_RGB:
          return new NeoPixelBusRgbStrip<NPB_METHOD_STRIP2_WS2812X>(pixelCount, pixelPin);
        default:
        case CO_GRB:
          return new NeoPixelBusGrbStrip<NPB_METHOD_STRIP2_WS2812X>(pixelCount, pixelPin);
      }
    }
  } else {
    // WS2811 LED type
    if (i == 1) {
      // First strip
      switch (colorOrder) {
        case CO_GRBW:
          return new NeoPixelBusGrbwStrip<NPB_METHOD_STRIP1_WS2811>(pixelCount, pixelPin);
        case CO_RGBW:
          return new NeoPixelBusRgbwStrip<NPB_METHOD_STRIP1_WS2811>(pixelCount, pixelPin);
        case CO_RGB:
          return new NeoPixelBusRgbStrip<NPB_METHOD_STRIP1_WS2811>(pixelCount, pixelPin);
        default:
        case CO_GRB:
          return new NeoPixelBusGrbStrip<NPB_METHOD_STRIP1_WS2811>(pixelCount, pixelPin);
      }
    } else {
      // Second strip
      switch (colorOrder) {
        case CO_GRBW:
          return new NeoPixelBusGrbwStrip<NPB_METHOD_STRIP2_WS2811>(pixelCount, pixelPin);
        case CO_RGBW:
          return new NeoPixelBusRgbwStrip<NPB_METHOD_STRIP2_WS2811>(pixelCount, pixelPin);
        case CO_RGB:
          return new NeoPixelBusRgbStrip<NPB_METHOD_STRIP2_WS2811>(pixelCount, pixelPin);
        default:
        case CO_GRB:
          return new NeoPixelBusGrbStrip<NPB_METHOD_STRIP2_WS2811>(pixelCount, pixelPin);
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

  LP_LOGLN("NeoPixelBus transport = " + String(NPB_TRANSPORT_NAME));
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
