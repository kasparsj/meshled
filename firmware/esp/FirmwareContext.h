#pragma once

#include <map>
#include <vector>

#include <IPAddress.h>
#include <WString.h>

#include "LightGraph.h"

#ifndef DEFAULT_HOSTNAME
#define DEFAULT_HOSTNAME "meshled"
#endif

#ifdef DEBUGGER_ENABLED
class Debugger;
#endif

struct UserPalette {
  String name;
  std::vector<int64_t> colors;
  std::vector<float> positions;
  int8_t colorRule = -1;
  int8_t interMode = 1;
  int8_t wrapMode = 0;
  float segmentation = 0.0f;
};

struct FirmwareContext {
  bool wifiConnected = false;
  bool apMode = false;
  unsigned long apStartTime = 0;
  String savedSSID = "";
  String savedPassword = "";
  String activeApSSID = "";

  String deviceHostname = DEFAULT_HOSTNAME;
  uint8_t maxBrightness = 255;
  uint8_t ledType = LED_WS2812;
#if defined(FASTLED_ENABLED) && !defined(NEOPIXELBUS_ENABLED)
  uint8_t ledLibrary = LIB_FASTLED;
#else
  uint8_t ledLibrary = LIB_NEOPIXELBUS;
#endif
  uint8_t colorOrder = CO_GRB;
  uint16_t pixelCount1 = 300;
  uint16_t pixelCount2 = 0;
  uint8_t pixelPin1 = 14;
  uint8_t pixelPin2 = 26;
  uint8_t pixelDensity = 60;
  bool oscEnabled = true;
  uint16_t oscPort = 54321;
  bool otaEnabled = true;
  uint16_t otaPort = 3232;
  String otaPassword = "meshled";
  bool apiAuthEnabled = false;
  String apiAuthTokenHash = "";

  bool emitterEnabled = false;
  float emitterMinSpeed = 0.5f;
  float emitterMaxSpeed = 10.0f;
  uint32_t emitterMinDur = 1920;
  uint32_t emitterMaxDur = 23040;
  uint8_t emitterMinSat = 128;
  uint8_t emitterMaxSat = 255;
  uint8_t emitterMinVal = 178;
  uint8_t emitterMaxVal = 255;
  uint16_t emitterMinNext = 2000;
  uint16_t emitterMaxNext = 20000;
  int16_t emitterFrom = -1;

  TopologyObject* object = nullptr;
  uint8_t objectType = OBJ_LINE;
  State* state = nullptr;
  float totalWattage = 0.0f;

  std::vector<UserPalette> userPalettes;
  std::map<String, std::vector<IPAddress>> deviceIPs;

#ifdef DEBUGGER_ENABLED
  Debugger* debugger = nullptr;
#endif
};

extern FirmwareContext gCtx;
