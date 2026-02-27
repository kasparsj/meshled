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
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
// ESP32-S3 valid output pins exclude 22-25 and 27-32 in FastLED's pin map.
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
  case 11: FASTLED_ADD_PIN(CHIPSET, 11, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 12: FASTLED_ADD_PIN(CHIPSET, 12, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 13: FASTLED_ADD_PIN(CHIPSET, 13, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 14: FASTLED_ADD_PIN(CHIPSET, 14, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 15: FASTLED_ADD_PIN(CHIPSET, 15, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 16: FASTLED_ADD_PIN(CHIPSET, 16, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 17: FASTLED_ADD_PIN(CHIPSET, 17, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 18: FASTLED_ADD_PIN(CHIPSET, 18, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 19: FASTLED_ADD_PIN(CHIPSET, 19, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 20: FASTLED_ADD_PIN(CHIPSET, 20, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 21: FASTLED_ADD_PIN(CHIPSET, 21, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 26: FASTLED_ADD_PIN(CHIPSET, 26, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 33: FASTLED_ADD_PIN(CHIPSET, 33, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 34: FASTLED_ADD_PIN(CHIPSET, 34, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 35: FASTLED_ADD_PIN(CHIPSET, 35, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 36: FASTLED_ADD_PIN(CHIPSET, 36, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 37: FASTLED_ADD_PIN(CHIPSET, 37, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 38: FASTLED_ADD_PIN(CHIPSET, 38, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 39: FASTLED_ADD_PIN(CHIPSET, 39, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 40: FASTLED_ADD_PIN(CHIPSET, 40, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 41: FASTLED_ADD_PIN(CHIPSET, 41, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 42: FASTLED_ADD_PIN(CHIPSET, 42, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 43: FASTLED_ADD_PIN(CHIPSET, 43, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 44: FASTLED_ADD_PIN(CHIPSET, 44, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 45: FASTLED_ADD_PIN(CHIPSET, 45, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 46: FASTLED_ADD_PIN(CHIPSET, 46, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 47: FASTLED_ADD_PIN(CHIPSET, 47, ORDER, LEDS, COUNT, colorOrder); break;  \
  case 48: FASTLED_ADD_PIN(CHIPSET, 48, ORDER, LEDS, COUNT, colorOrder); break;
#define FASTLED_DEFAULT_PIN 14
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

#define FASTLED_ADD_CHIPSET_FOR_ORDER(CHIPSET, ORDER, LEDS, COUNT, PIN, COLOR_ORDER) \
  do {                                                                                  \
    switch (PIN) {                                                                      \
      FASTLED_ADD_PIN_CASES(CHIPSET, ORDER, LEDS, COUNT)                               \
      default: FASTLED_ADD_PIN(CHIPSET, FASTLED_DEFAULT_PIN, ORDER, LEDS, COUNT, COLOR_ORDER); break; \
    }                                                                                   \
  } while (0)

#define FASTLED_ADD_CHIPSET(CHIPSET, LEDS, COUNT, PIN, COLOR_ORDER) \
  do {                                                               \
    if (IS_RGB(COLOR_ORDER)) {                                       \
      FASTLED_ADD_CHIPSET_FOR_ORDER(CHIPSET, RGB, LEDS, COUNT, PIN, COLOR_ORDER); \
    } else {                                                         \
      FASTLED_ADD_CHIPSET_FOR_ORDER(CHIPSET, GRB, LEDS, COUNT, PIN, COLOR_ORDER); \
    }                                                                \
  } while (0)

const char* fastLedTypeName(uint8_t ledType) {
  switch (ledType) {
    case LED_WS2812: return "WS2812";
    case LED_WS2811: return "WS2811";
    case LED_WS2815: return "WS2815";
    case LED_WS2813: return "WS2813";
    case LED_WS2816: return "WS2816";
    case LED_SK6812: return "SK6812";
    case LED_TM1829: return "TM1829";
    case LED_APA106: return "APA106";
    case LED_TM1812: return "TM1812";
    case LED_TM1809: return "TM1809";
    case LED_TM1804: return "TM1804";
    case LED_TM1803: return "TM1803";
    case LED_UCS1903: return "UCS1903";
    case LED_UCS1903B: return "UCS1903B";
    case LED_UCS1904: return "UCS1904";
    case LED_UCS2903: return "UCS2903";
    case LED_SK6822: return "SK6822";
    case LED_APA104: return "APA104";
    case LED_WS2811_400: return "WS2811_400";
    case LED_GS1903: return "GS1903";
    case LED_GW6205: return "GW6205";
    case LED_GW6205_400: return "GW6205_400";
    case LED_LPD1886: return "LPD1886";
    case LED_LPD1886_8BIT: return "LPD1886_8BIT";
    case LED_PL9823: return "PL9823";
    case LED_UCS1912: return "UCS1912";
    case LED_SM16703: return "SM16703";
    case LED_SM16824E: return "SM16824E";
    default: return "WS2812";
  }
}

void setupFastLED() {
  if (leds1 != NULL) {
    delete[] leds1;
    leds1 = NULL;
  }
  if (leds2 != NULL) {
    delete[] leds2;
    leds2 = NULL;
  }

  if (ledLibrary != LIB_FASTLED) return;

  if (pixelCount1 > 0) {
    leds1 = new CRGB[pixelCount1];
  }
  if (pixelCount2 > 0) {
    leds2 = new CRGB[pixelCount2];
  }

  if (leds1 == NULL) {
    LP_LOGLN("FastLED setup skipped: pixelCount1 must be > 0");
    return;
  }

  // Helper function to add LEDs with the right chipset and color order.
  auto addLedsWithConfig = [](CRGB* leds, uint16_t count, uint8_t pin, uint8_t ledType, uint8_t colorOrder) {
    switch (ledType) {
      case LED_WS2811: FASTLED_ADD_CHIPSET(WS2811, leds, count, pin, colorOrder); break;
      case LED_WS2815: FASTLED_ADD_CHIPSET(WS2815, leds, count, pin, colorOrder); break;
      case LED_WS2813: FASTLED_ADD_CHIPSET(WS2813, leds, count, pin, colorOrder); break;
      case LED_WS2816: FASTLED_ADD_CHIPSET(WS2816, leds, count, pin, colorOrder); break;
      case LED_SK6812: FASTLED_ADD_CHIPSET(SK6812, leds, count, pin, colorOrder); break;
      case LED_TM1829: FASTLED_ADD_CHIPSET(TM1829, leds, count, pin, colorOrder); break;
      case LED_APA106: FASTLED_ADD_CHIPSET(APA106, leds, count, pin, colorOrder); break;
      case LED_TM1812: FASTLED_ADD_CHIPSET(TM1812, leds, count, pin, colorOrder); break;
      case LED_TM1809: FASTLED_ADD_CHIPSET(TM1809, leds, count, pin, colorOrder); break;
      case LED_TM1804: FASTLED_ADD_CHIPSET(TM1804, leds, count, pin, colorOrder); break;
      case LED_TM1803: FASTLED_ADD_CHIPSET(TM1803, leds, count, pin, colorOrder); break;
      case LED_UCS1903: FASTLED_ADD_CHIPSET(UCS1903, leds, count, pin, colorOrder); break;
      case LED_UCS1903B: FASTLED_ADD_CHIPSET(UCS1903B, leds, count, pin, colorOrder); break;
      case LED_UCS1904: FASTLED_ADD_CHIPSET(UCS1904, leds, count, pin, colorOrder); break;
      case LED_UCS2903: FASTLED_ADD_CHIPSET(UCS2903, leds, count, pin, colorOrder); break;
      case LED_SK6822: FASTLED_ADD_CHIPSET(SK6822, leds, count, pin, colorOrder); break;
      case LED_APA104: FASTLED_ADD_CHIPSET(APA104, leds, count, pin, colorOrder); break;
      case LED_WS2811_400: FASTLED_ADD_CHIPSET(WS2811_400, leds, count, pin, colorOrder); break;
      case LED_GS1903: FASTLED_ADD_CHIPSET(GS1903, leds, count, pin, colorOrder); break;
      case LED_GW6205: FASTLED_ADD_CHIPSET(GW6205, leds, count, pin, colorOrder); break;
      case LED_GW6205_400: FASTLED_ADD_CHIPSET(GW6205_400, leds, count, pin, colorOrder); break;
      case LED_LPD1886: FASTLED_ADD_CHIPSET(LPD1886, leds, count, pin, colorOrder); break;
      case LED_LPD1886_8BIT: FASTLED_ADD_CHIPSET(LPD1886_8BIT, leds, count, pin, colorOrder); break;
      case LED_PL9823: FASTLED_ADD_CHIPSET(PL9823, leds, count, pin, colorOrder); break;
      case LED_UCS1912: FASTLED_ADD_CHIPSET(UCS1912, leds, count, pin, colorOrder); break;
      case LED_SM16703: FASTLED_ADD_CHIPSET(SM16703, leds, count, pin, colorOrder); break;
      case LED_SM16824E: FASTLED_ADD_CHIPSET(SM16824E, leds, count, pin, colorOrder); break;
      case LED_WS2812:
      default:
        FASTLED_ADD_CHIPSET(WS2812, leds, count, pin, colorOrder);
        break;
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

  LP_LOGLN("FastLED initialized with ledType = " + String(fastLedTypeName(ledType)) +
           ", colorOrder = " + String(IS_RGB(colorOrder) ? "RGB" : "GRB"));
}

CRGB getFastLEDColor(uint16_t i) {
  const uint8_t effectiveBrightness = wledMasterOn ? maxBrightness : 0;
  ColorRGB pixel = state->getPixel(i, effectiveBrightness);
  CRGB color = CRGB(pixel.R, pixel.G, pixel.B);

  #ifdef DEBUGGER_ENABLED
  if (state->showConnections) {
    color.g = (debugger->isConnection(i) ? 1.f : 0.f) * effectiveBrightness;
  }
  if (state->showIntersections) {
    color.b = (debugger->isIntersection(i) ? 1.f : 0.f) * effectiveBrightness;
  }
  #endif
  return color;
}

float getFastLEDWatts(const CRGB& color, float wattsPerLed = 0.2f) {
  const float relPixelPower = static_cast<float>(color.r + color.g + color.b) / (255.f * 3.f);
  const float wattsPerPixel = wattsPerLed * 3.f;
  return relPixelPower * wattsPerPixel;
}

void drawFastLED() {
  if (ledLibrary == LIB_FASTLED && leds1 != NULL) {
    const uint8_t effectiveBrightness = wledMasterOn ? maxBrightness : 0;
    totalWattage = 0.0f;
    for (uint16_t i=0; i<pixelCount1; i++) {
      leds1[i] = getFastLEDColor(i);
      totalWattage += getFastLEDWatts(leds1[i]);
    }
    if (pixelCount2 > 0 && leds2 != NULL) {
      for (uint16_t i=0; i<pixelCount2; i++) {
        leds2[i] = getFastLEDColor(pixelCount1+i);
        totalWattage += getFastLEDWatts(leds2[i]);
      }
    }
    FastLED.setBrightness(effectiveBrightness);
    FastLED.show();
  }
}
