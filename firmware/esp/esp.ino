// Define DEV_ENABLED from build flags for development builds.
#define DEV_ENABLED
#define DEFAULT_HOSTNAME "meshled"
// #define BUTTON_PIN 25
#define SPIFFS_ENABLED // SPIFFS needs to be enabled to save wifi settings in AP mode
#define WIFI_ENABLED // comment out WIFI_ENABLED to disable WiFi completely
// #define WIFI_REQUIRED // Comment this out to enable AP mode when station mode fails
#define WIFI_SSID ""
#define WIFI_PASS ""
#define AP_MODE_ENABLED // Enable AP mode when station mode fails
#define AP_SSID "MeshLED_AP" // SSID for AP mode
#define AP_PASS "meshled123" // Password for AP mode (minimum 8 characters)
#define AP_TIMEOUT 300000     // Time in AP mode before rebooting (ms) - 5 minutes
#define OSC_ENABLED // OSC support (requires WiFi)
#define WEB_ENABLED // Web interface (requires WiFi)
// #define BLUETOOTH_ENABLED
// #define SERIAL_ENABLED   // Serial commands
// #define DEBUGGER_ENABLED // Debugging features
#define NEOPIXELBUS_ENABLED
// #define FASTLED_ENABLED
#define WLEDAPI_ENABLED
#define SSDP_ENABLED
#define MDNS_ENABLED
// #define ESPNOW_ENABLED

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

#ifdef OTA_ENABLED
#include "OTADiagnostics.h"
#endif

#ifdef WIFI_ENABLED
#include <WiFi.h>
#endif

#ifdef BLUETOOTH_ENABLED
#include "BluetoothSerial.h"
#include "esp_bt.h"
#endif

#include "LightGraph.h"
#include "FirmwareContext.h"

FirmwareContext gCtx = []() {
  FirmwareContext ctx;
  ctx.savedSSID = WIFI_SSID;
  ctx.savedPassword = WIFI_PASS;
  return ctx;
}();

bool& wifiConnected = gCtx.wifiConnected;
bool& apMode = gCtx.apMode;
unsigned long& apStartTime = gCtx.apStartTime;
String& savedSSID = gCtx.savedSSID;
String& savedPassword = gCtx.savedPassword;
String& activeApSSID = gCtx.activeApSSID;

String& deviceHostname = gCtx.deviceHostname;
uint8_t& maxBrightness = gCtx.maxBrightness;
uint8_t& ledType = gCtx.ledType;
uint8_t& ledLibrary = gCtx.ledLibrary;
uint8_t& colorOrder = gCtx.colorOrder;
uint16_t& pixelCount1 = gCtx.pixelCount1;
uint16_t& pixelCount2 = gCtx.pixelCount2;
uint8_t& pixelPin1 = gCtx.pixelPin1;
uint8_t& pixelPin2 = gCtx.pixelPin2;
uint8_t& pixelDensity = gCtx.pixelDensity;
bool& oscEnabled = gCtx.oscEnabled;
uint16_t& oscPort = gCtx.oscPort;
bool& otaEnabled = gCtx.otaEnabled;
uint16_t& otaPort = gCtx.otaPort;
String& otaPassword = gCtx.otaPassword;
bool& apiAuthEnabled = gCtx.apiAuthEnabled;
String& apiAuthTokenHash = gCtx.apiAuthTokenHash;

bool& emitterEnabled = gCtx.emitterEnabled;
float& emitterMinSpeed = gCtx.emitterMinSpeed;
float& emitterMaxSpeed = gCtx.emitterMaxSpeed;
uint32_t& emitterMinDur = gCtx.emitterMinDur;
uint32_t& emitterMaxDur = gCtx.emitterMaxDur;
uint8_t& emitterMinSat = gCtx.emitterMinSat;
uint8_t& emitterMaxSat = gCtx.emitterMaxSat;
uint8_t& emitterMinVal = gCtx.emitterMinVal;
uint8_t& emitterMaxVal = gCtx.emitterMaxVal;
uint16_t& emitterMinNext = gCtx.emitterMinNext;
uint16_t& emitterMaxNext = gCtx.emitterMaxNext;
int16_t& emitterFrom = gCtx.emitterFrom;

TopologyObject*& object = gCtx.object;
uint8_t& objectType = gCtx.objectType;
State*& state = gCtx.state;
float& totalWattage = gCtx.totalWattage;
bool wledMasterOn = true;

std::vector<UserPalette>& userPalettes = gCtx.userPalettes;
std::map<String, std::vector<IPAddress>>& deviceIPs = gCtx.deviceIPs;

String getActiveNetworkSSID() {
#ifdef WIFI_ENABLED
  if (apMode) {
    String ssid = WiFi.softAPSSID();
    if (ssid.length() > 0) {
      return ssid;
    }
    if (activeApSSID.length() > 0) {
      return activeApSSID;
    }
    #ifdef AP_MODE_ENABLED
    return String(AP_SSID);
    #endif
    return "";
  }

  String ssid = WiFi.SSID();
  if (ssid.length() > 0) {
    return ssid;
  }
  if (savedSSID.length() > 0) {
    return savedSSID;
  }
#endif
  return "";
}

#ifdef SPIFFS_ENABLED
#include <ArduinoJson.h>
#include "SecurityLib.h"
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
#include <lightgraph/integration/debug.hpp>
Debugger*& debugger = gCtx.debugger;
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
  LP_LOGLN("MeshLED starting up...");

  #ifdef SPIFFS_ENABLED
  // Setup file system and load settings
  if (!setupFileSystem()) {
    LP_LOGLN("SPIFFS setup failed");
    return;
  }
  loadCredentials();
  loadSettings();
  #ifdef OTA_ENABLED
  finalizeOtaBootStatus();
  #endif

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
  #ifdef SPIFFS_ENABLED
  loadLayers();
  loadUserPalettes();
  #endif

  #ifdef DEBUGGER_ENABLED
  debugger = new Debugger(*object);
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
