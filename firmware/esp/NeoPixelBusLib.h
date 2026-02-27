#pragma once

#include <sdkconfig.h>
#if defined(ARDUINO_ARCH_ESP32)
#include <esp_arduino_version.h>
#endif

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
#define NPB_METHOD_STRIP1_WS2812 NeoEsp32I2s0800KbpsMethod
#define NPB_METHOD_STRIP2_WS2812 NeoEsp32I2s1800KbpsMethod
#define NPB_METHOD_STRIP1_WS2811 NeoEsp32I2s0400KbpsMethod
#define NPB_METHOD_STRIP2_WS2811 NeoEsp32I2s1400KbpsMethod
#define NPB_METHOD_STRIP1_WS2813 NeoEsp32I2s0Ws2812xMethod
#define NPB_METHOD_STRIP2_WS2813 NeoEsp32I2s1Ws2812xMethod
#define NPB_METHOD_STRIP1_WS2814 NeoEsp32I2s0Ws2814Method
#define NPB_METHOD_STRIP2_WS2814 NeoEsp32I2s1Ws2814Method
#define NPB_METHOD_STRIP1_WS2816 NeoEsp32I2s0Ws2812xMethod
#define NPB_METHOD_STRIP2_WS2816 NeoEsp32I2s1Ws2812xMethod
#define NPB_METHOD_STRIP1_SK6812 NeoEsp32I2s0Sk6812Method
#define NPB_METHOD_STRIP2_SK6812 NeoEsp32I2s1Sk6812Method
#define NPB_METHOD_STRIP1_TM1814 NeoEsp32I2s0Tm1814Method
#define NPB_METHOD_STRIP2_TM1814 NeoEsp32I2s1Tm1814Method
#define NPB_METHOD_STRIP1_TM1829 NeoEsp32I2s0Tm1829Method
#define NPB_METHOD_STRIP2_TM1829 NeoEsp32I2s1Tm1829Method
#define NPB_METHOD_STRIP1_TM1914 NeoEsp32I2s0Tm1914Method
#define NPB_METHOD_STRIP2_TM1914 NeoEsp32I2s1Tm1914Method
#define NPB_METHOD_STRIP1_APA106 NeoEsp32I2s0Apa106Method
#define NPB_METHOD_STRIP2_APA106 NeoEsp32I2s1Apa106Method
#define NPB_TRANSPORT_NAME "esp32-i2s"
#elif defined(NPB_TARGET_ESP32S3)
// Keep S3 on LCD-X transport to avoid ESP-IDF RMT legacy/new-driver conflicts.
// NeoPixelBusStrip.h provides a compatibility shim for Arduino-ESP32 3.3+.
#define NPB_METHOD_STRIP1_WS2812X NeoEsp32LcdX8Ws2812xMethod
#define NPB_METHOD_STRIP2_WS2812X NeoEsp32LcdX8Ws2812xMethod
#define NPB_METHOD_STRIP1_WS2812 NeoEsp32LcdX8Ws2812Method
#define NPB_METHOD_STRIP2_WS2812 NeoEsp32LcdX8Ws2812Method
#define NPB_METHOD_STRIP1_WS2811 NeoEsp32LcdX8Ws2811Method
#define NPB_METHOD_STRIP2_WS2811 NeoEsp32LcdX8Ws2811Method
#define NPB_METHOD_STRIP1_WS2813 NeoEsp32LcdX8Ws2813Method
#define NPB_METHOD_STRIP2_WS2813 NeoEsp32LcdX8Ws2813Method
#define NPB_METHOD_STRIP1_WS2814 NeoEsp32LcdX8Ws2814Method
#define NPB_METHOD_STRIP2_WS2814 NeoEsp32LcdX8Ws2814Method
#define NPB_METHOD_STRIP1_WS2816 NeoEsp32LcdX8Ws2816Method
#define NPB_METHOD_STRIP2_WS2816 NeoEsp32LcdX8Ws2816Method
#define NPB_METHOD_STRIP1_SK6812 NeoEsp32LcdX8Sk6812Method
#define NPB_METHOD_STRIP2_SK6812 NeoEsp32LcdX8Sk6812Method
#define NPB_METHOD_STRIP1_TM1814 NeoEsp32LcdX8Tm1814Method
#define NPB_METHOD_STRIP2_TM1814 NeoEsp32LcdX8Tm1814Method
#define NPB_METHOD_STRIP1_TM1829 NeoEsp32LcdX8Tm1829Method
#define NPB_METHOD_STRIP2_TM1829 NeoEsp32LcdX8Tm1829Method
#define NPB_METHOD_STRIP1_TM1914 NeoEsp32LcdX8Tm1914Method
#define NPB_METHOD_STRIP2_TM1914 NeoEsp32LcdX8Tm1914Method
#define NPB_METHOD_STRIP1_APA106 NeoEsp32LcdX8Apa106Method
#define NPB_METHOD_STRIP2_APA106 NeoEsp32LcdX8Apa106Method
#define NPB_TRANSPORT_NAME "esp32s3-lcdx"
#else
#define NPB_METHOD_STRIP1_WS2812X NeoEsp32Rmt0Ws2812xMethod
#define NPB_METHOD_STRIP2_WS2812X NeoEsp32Rmt1Ws2812xMethod
#define NPB_METHOD_STRIP1_WS2812 NeoEsp32Rmt0800KbpsMethod
#define NPB_METHOD_STRIP2_WS2812 NeoEsp32Rmt1800KbpsMethod
#define NPB_METHOD_STRIP1_WS2811 NeoEsp32Rmt0Ws2811Method
#define NPB_METHOD_STRIP2_WS2811 NeoEsp32Rmt1Ws2811Method
#define NPB_METHOD_STRIP1_WS2813 NeoEsp32Rmt0Ws2812xMethod
#define NPB_METHOD_STRIP2_WS2813 NeoEsp32Rmt1Ws2812xMethod
#define NPB_METHOD_STRIP1_WS2814 NeoEsp32Rmt0Ws2814Method
#define NPB_METHOD_STRIP2_WS2814 NeoEsp32Rmt1Ws2814Method
#define NPB_METHOD_STRIP1_WS2816 NeoEsp32Rmt0Ws2816Method
#define NPB_METHOD_STRIP2_WS2816 NeoEsp32Rmt1Ws2816Method
#define NPB_METHOD_STRIP1_SK6812 NeoEsp32Rmt0Sk6812Method
#define NPB_METHOD_STRIP2_SK6812 NeoEsp32Rmt1Sk6812Method
#define NPB_METHOD_STRIP1_TM1814 NeoEsp32Rmt0Tm1814Method
#define NPB_METHOD_STRIP2_TM1814 NeoEsp32Rmt1Tm1814Method
#define NPB_METHOD_STRIP1_TM1829 NeoEsp32Rmt0Tm1829Method
#define NPB_METHOD_STRIP2_TM1829 NeoEsp32Rmt1Tm1829Method
#define NPB_METHOD_STRIP1_TM1914 NeoEsp32Rmt0Tm1914Method
#define NPB_METHOD_STRIP2_TM1914 NeoEsp32Rmt1Tm1914Method
#define NPB_METHOD_STRIP1_APA106 NeoEsp32Rmt0Apa106Method
#define NPB_METHOD_STRIP2_APA106 NeoEsp32Rmt1Apa106Method
#define NPB_TRANSPORT_NAME "esp32-rmt-legacy"
#endif

const char* neoPixelBusLedTypeName(uint8_t ledType) {
  switch (ledType) {
    case LED_WS2812: return "WS2812";
    case LED_WS2811: return "WS2811";
    case LED_WS2815: return "WS2815";
    case LED_WS2813: return "WS2813";
    case LED_WS2816: return "WS2816";
    case LED_SK6812: return "SK6812";
    case LED_TM1829: return "TM1829";
    case LED_APA106: return "APA106";
    case LED_WS2814: return "WS2814";
    case LED_TM1814: return "TM1814";
    case LED_TM1914: return "TM1914";
    default: return "WS2812";
  }
}

template<typename T_METHOD>
NeoPixelBusStrip* createNeoPixelBusStripWithMethod(uint8_t colorOrder, uint16_t pixelCount, uint8_t pixelPin) {
  switch (colorOrder) {
    case CO_GRBW:
      return new NeoPixelBusGrbwStrip<T_METHOD>(pixelCount, pixelPin);
    case CO_RGBW:
      return new NeoPixelBusRgbwStrip<T_METHOD>(pixelCount, pixelPin);
    case CO_RGB:
      return new NeoPixelBusRgbStrip<T_METHOD>(pixelCount, pixelPin);
    default:
    case CO_GRB:
      return new NeoPixelBusGrbStrip<T_METHOD>(pixelCount, pixelPin);
  }
}

template<typename T_METHOD_STRIP1, typename T_METHOD_STRIP2>
NeoPixelBusStrip* createNeoPixelBusStripWithMethodPair(
    uint8_t colorOrder,
    uint16_t pixelCount,
    uint8_t pixelPin,
    bool isSecondStrip) {
  if (isSecondStrip) {
    return createNeoPixelBusStripWithMethod<T_METHOD_STRIP2>(colorOrder, pixelCount, pixelPin);
  }
  return createNeoPixelBusStripWithMethod<T_METHOD_STRIP1>(colorOrder, pixelCount, pixelPin);
}

NeoPixelBusStrip* createNeoPixelBusStrip(
    uint8_t ledType,
    uint8_t colorOrder,
    uint16_t pixelCount,
    uint8_t pixelPin,
    bool isSecondStrip) {
  switch (ledType) {
    case LED_WS2811:
      return createNeoPixelBusStripWithMethodPair<NPB_METHOD_STRIP1_WS2811, NPB_METHOD_STRIP2_WS2811>(
          colorOrder, pixelCount, pixelPin, isSecondStrip);
    case LED_WS2815:
      return createNeoPixelBusStripWithMethodPair<NPB_METHOD_STRIP1_WS2812X, NPB_METHOD_STRIP2_WS2812X>(
          colorOrder, pixelCount, pixelPin, isSecondStrip);
    case LED_WS2813:
      return createNeoPixelBusStripWithMethodPair<NPB_METHOD_STRIP1_WS2813, NPB_METHOD_STRIP2_WS2813>(
          colorOrder, pixelCount, pixelPin, isSecondStrip);
    case LED_WS2816:
      return createNeoPixelBusStripWithMethodPair<NPB_METHOD_STRIP1_WS2816, NPB_METHOD_STRIP2_WS2816>(
          colorOrder, pixelCount, pixelPin, isSecondStrip);
    case LED_SK6812:
      return createNeoPixelBusStripWithMethodPair<NPB_METHOD_STRIP1_SK6812, NPB_METHOD_STRIP2_SK6812>(
          colorOrder, pixelCount, pixelPin, isSecondStrip);
    case LED_TM1829:
      return createNeoPixelBusStripWithMethodPair<NPB_METHOD_STRIP1_TM1829, NPB_METHOD_STRIP2_TM1829>(
          colorOrder, pixelCount, pixelPin, isSecondStrip);
    case LED_APA106:
      return createNeoPixelBusStripWithMethodPair<NPB_METHOD_STRIP1_APA106, NPB_METHOD_STRIP2_APA106>(
          colorOrder, pixelCount, pixelPin, isSecondStrip);
    case LED_WS2814:
      return createNeoPixelBusStripWithMethodPair<NPB_METHOD_STRIP1_WS2814, NPB_METHOD_STRIP2_WS2814>(
          colorOrder, pixelCount, pixelPin, isSecondStrip);
    case LED_TM1814:
      return createNeoPixelBusStripWithMethodPair<NPB_METHOD_STRIP1_TM1814, NPB_METHOD_STRIP2_TM1814>(
          colorOrder, pixelCount, pixelPin, isSecondStrip);
    case LED_TM1914:
      return createNeoPixelBusStripWithMethodPair<NPB_METHOD_STRIP1_TM1914, NPB_METHOD_STRIP2_TM1914>(
          colorOrder, pixelCount, pixelPin, isSecondStrip);
    case LED_WS2812:
    default:
      return createNeoPixelBusStripWithMethodPair<NPB_METHOD_STRIP1_WS2812, NPB_METHOD_STRIP2_WS2812>(
          colorOrder, pixelCount, pixelPin, isSecondStrip);
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

  strip1 = createNeoPixelBusStrip(ledType, colorOrder, pixelCount1, pixelPin1, false);
  strip1->Begin();
  strip1->Show();

  if (pixelCount2 > 0) {
    strip2 = createNeoPixelBusStrip(ledType, colorOrder, pixelCount2, pixelPin2, true);
    strip2->Begin();
    strip2->Show();
  }

  LP_LOGLN("NeoPixelBus transport = " + String(NPB_TRANSPORT_NAME));
  LP_LOGLN("NeoPixelBus initialized with ledType = " + String(neoPixelBusLedTypeName(ledType)) +
           ", colorOrder = " + String(colorOrder));
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
