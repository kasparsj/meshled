#define DEV_ENABLED
#define DEFAULT_HOSTNAME "homo-deus"
// #define BUTTON_PIN 25
#define SPIFFS_ENABLED
#define WIFI_ENABLED // comment out WIFI_ENABLED to disable WiFi completely
// #define WIFI_REQUIRED // Comment this out to enable AP mode when station mode fails
#define WIFI_SSID ""
#define WIFI_PASS ""
#define AP_MODE_ENABLED // Enable AP mode when station mode fails
#define AP_SSID "HomoDeus_AP" // SSID for AP mode
#define AP_PASS "homodeus123" // Password for AP mode (minimum 8 characters)
#define AP_TIMEOUT 300000     // Time in AP mode before rebooting (ms) - 5 minutes
#define OSC_ENABLED // OSC support (requires WiFi)
#define WEB_ENABLED // Web interface (requires WiFi)
// #define BLUETOOTH_ENABLED
// #define SERIAL_ENABLED   // Serial commands
// #define DEBUGGER_ENABLED // Debugging features
#define NEOPIXELBUS_ENABLED
// todo: FastLED is crashing on esp32-dev, but working on esp32-s3
// #define FASTLED_ENABLED
#define WLEDAPI_ENABLED
// #define SSDP_ENABLED
#define MDNS_ENABLED
#define ESPNOW_ENABLED
// todo: logs crashed the esp once
// #define LOG_FILE "/log.txt"
// #define CRASH_LOG_FILE "/crash_log.txt" // Path to store crash logs

#ifdef DEV_ENABLED
#define OTA_ENABLED // OTA updates (requires WiFi)
#endif

#ifdef SPIFFS_ENABLED
#include <SPIFFS.h>
#if (defined(LOG_FILE) || defined(CRASH_LOG_FILE))
#include "LogLib.h"
#endif
#endif

#ifdef WIFI_ENABLED
#include <WiFi.h>
bool wifiConnected = false;
bool apMode = false;
unsigned long apStartTime = 0;
String savedSSID = WIFI_SSID;
String savedPassword = WIFI_PASS;
#endif

#ifdef BLUETOOTH_ENABLED
#include "BluetoothSerial.h"
#include "esp_bt.h"
#endif

#include "LightPath.h"

// Web configuration variables
String deviceHostname = DEFAULT_HOSTNAME; // Device hostname for both WiFi and OTA
uint8_t maxBrightness = 255;
uint8_t ledType = LED_WS2812;
uint8_t ledLibrary = LIB_NEOPIXELBUS;
uint8_t colorOrder = CO_GRB;
uint16_t pixelCount1 = 300;
uint16_t pixelCount2 = 0;
uint8_t pixelPin1 = 14;
uint8_t pixelPin2 = 26;
uint8_t pixelDensity = 60; // Default pixel density (options: 30, 60, 144 pixels per meter)
bool oscEnabled = true;
uint16_t oscPort = 54321;
bool otaEnabled = true; // OTA updates enabled by default
uint16_t otaPort = 3232;
String otaPassword = "homodeus";

// Auto Emitter
bool emitterEnabled = false; // Default auto emitter enabled state
float emitterMinSpeed = 0.5f;      // Default min speed for random speed generation
float emitterMaxSpeed = 10.0f;     // Default max speed for random speed generation
uint32_t emitterMinDur = 1920; // Default min duration for random duration generation (120 * 16)
uint32_t emitterMaxDur = 23040; // Default max duration for random duration generation (1440 * 16)
uint8_t emitterMinSat = 128; // Default min saturation
uint8_t emitterMaxSat = 255; // Default max saturation
uint8_t emitterMinVal = 178; // Default min value (70% of 255)
uint8_t emitterMaxVal = 255; // Default max value
uint16_t emitterMinNext = 2000;    // Default min time between auto emits (ms)
uint16_t emitterMaxNext = 20000;   // Default max time between auto emits (ms)
int16_t emitterFrom = -1;   // Default starting point for auto emitter

// State
LPObject* object;
uint8_t objectType = OBJ_LINE;
State *state;
float totalWattage = 0;

// Structure to represent a saved user palette
struct UserPalette {
  String name;
  std::vector<int64_t> colors;
  std::vector<float> positions;
  int8_t colorRule;
  int8_t interMode;
  int8_t wrapMode;
  float segmentation;
};

std::vector<UserPalette> userPalettes;

std::map<String, std::vector<IPAddress>> deviceIPs;

#ifdef SPIFFS_ENABLED
#include <ArduinoJson.h>
#include "FSLib.h"
#endif

bool updateUserPalette(const UserPalette& palette) {
  // Load existing palettes first
  std::vector<UserPalette> existingPalettes = userPalettes;

  // Check if a palette with this name already exists
  bool updated = false;
  for (size_t i = 0; i < existingPalettes.size(); i++) {
    if (existingPalettes[i].name == palette.name) {
      // Update existing palette
      existingPalettes[i] = palette;
      updated = true;
      break;
    }
  }

  // If palette doesn't exist, add it
  if (!updated) {
    existingPalettes.push_back(palette);
  }

  #ifdef SPIFFS_ENABLED
  saveUserPalettes(existingPalettes);
  #endif

  LP_LOGF("Saved user palette: %s with %d colors\n", palette.name.c_str(), palette.colors.size());

  // Reload user palettes to include the new one
  userPalettes = existingPalettes;
  return true;
}

#ifdef DEBUGGER_ENABLED
#include <lightpath/integration/debug.hpp>
LPDebugger *debugger;
#endif

#include "LEDLib.h"

#ifdef OSC_ENABLED
#include <ArduinoOSCWiFi.h>
#include "OSCLib.h"
#endif

#ifdef MDNS_ENABLED
#include "MDNSLib.h"

std::vector<IPAddress> getDevices(String service = MDNS_XLED_SERVICE, String proto = MDNS_TCP, bool forceDiscover = false) {
    // Ensure the map has the vector initialized once
  if (deviceIPs.find(service) == deviceIPs.end() || forceDiscover) {
    deviceIPs[service] = discoverMDNSDevices(service, proto);
  }
  return deviceIPs[service];
}
#endif

#ifdef ESPNOW_ENABLED
#include "ESPNowLib.h"
#endif

#ifdef WEB_ENABLED
#include <WiFiServer.h>
#include <WebServer.h>
#include <ArduinoJson.h>
WebServer server(80);
#include "WebServerSetup.h"
#endif

#ifdef AP_MODE_ENABLED
#include "APModeLib.h"
#endif

#ifdef OTA_ENABLED
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "OTALib.h"
#endif

#include "SetupLib.h"

void setup() {
  Serial.begin(115200);
  LP_LOGLN("HomoDeus starting up...");

  #ifdef SPIFFS_ENABLED
  // Setup file system and load settings
  if (!setupFileSystem()) {
    LP_LOGLN("SPIFFS setup failed");
    return;
  }
  loadCredentials();
  loadSettings();

  #ifdef CRASH_LOG_FILE
  checkForCrash();
  #endif
  #endif

  // Setup communications (WiFi, Bluetooth, etc)
  setupComms();

  // Initialize LEDs
  setupLEDs();

  // Setup IO pins
  setupIO();

  // Update random generation parameters
  updateLPRandomConstants();

  // Initialize state
  setupState();
  loadLayers();
  loadUserPalettes();

  #ifdef DEBUGGER_ENABLED
  debugger = new LPDebugger(*object);
  LP_LOGLN("Debugger initialized");
  #endif

  LP_LOGLN("Setup complete!");
}

void loop() {
  #ifdef AP_MODE_ENABLED
  checkAPModeTimeout();
  #endif

  if (wifiConnected) {
    #ifdef OSC_ENABLED
    OscWiFi.update();
    #endif

    #ifdef OTA_ENABLED
    if (!apMode && otaEnabled) {
      ArduinoOTA.handle();
    }
    #endif

    #ifdef SSDP_ENABLED
    handleSSDPDiscovery();
    #endif
  }

  #ifdef WEB_ENABLED
  server.handleClient();
  #endif

  updateLEDs();
  drawLEDs();

  #ifdef SERIAL_ENABLED
  readSerial();
  #endif
}
