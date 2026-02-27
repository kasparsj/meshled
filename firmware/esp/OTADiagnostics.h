#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#ifdef SPIFFS_ENABLED
#include <SPIFFS.h>
#endif

#if __has_include(<esp_system.h>)
#include <esp_system.h>
#define MESHLED_HAS_RESET_REASON 1
#else
#define MESHLED_HAS_RESET_REASON 0
#endif

#if __has_include(<esp_ota_ops.h>)
#include <esp_ota_ops.h>
#define MESHLED_HAS_OTA_PARTITION_INFO 1
#else
#define MESHLED_HAS_OTA_PARTITION_INFO 0
#endif

#ifndef MESHLED_OTA_STATUS_PATH
#define MESHLED_OTA_STATUS_PATH "/ota_status.json"
#endif

#ifndef MESHLED_VERSION
#define MESHLED_VERSION "dev"
#endif

#ifndef MESHLED_RELEASE_SHA
#define MESHLED_RELEASE_SHA "unknown"
#endif

inline String meshledOtaErrorCodeName(int errorCode) {
  switch (errorCode) {
    case 0: return "AUTH_ERROR";
    case 1: return "BEGIN_ERROR";
    case 2: return "CONNECT_ERROR";
    case 3: return "RECEIVE_ERROR";
    case 4: return "END_ERROR";
    default: return "UNKNOWN_ERROR";
  }
}

inline String meshledCurrentSketchMd5() {
  String sketchMd5 = ESP.getSketchMD5();
  sketchMd5.trim();
  return sketchMd5;
}

inline bool meshledWriteOtaStatusDoc(DynamicJsonDocument& doc) {
#ifdef SPIFFS_ENABLED
  File statusFile = SPIFFS.open(MESHLED_OTA_STATUS_PATH, FILE_WRITE);
  if (!statusFile) {
    return false;
  }
  serializeJson(doc, statusFile);
  statusFile.close();
  return true;
#else
  (void) doc;
  return false;
#endif
}

inline bool meshledReadOtaStatusDoc(DynamicJsonDocument& doc) {
#ifdef SPIFFS_ENABLED
  if (!SPIFFS.exists(MESHLED_OTA_STATUS_PATH)) {
    return false;
  }

  File statusFile = SPIFFS.open(MESHLED_OTA_STATUS_PATH, FILE_READ);
  if (!statusFile) {
    return false;
  }

  const DeserializationError err = deserializeJson(doc, statusFile);
  statusFile.close();
  return !err;
#else
  (void) doc;
  return false;
#endif
}

inline void recordOtaStartStatus(const String& commandType) {
  DynamicJsonDocument doc(768);
  doc["stage"] = "start";
  doc["commandType"] = commandType;
  doc["errorCode"] = -1;
  doc["errorName"] = "";
  doc["detail"] = "ota_started";
  doc["sourceSketchMd5"] = meshledCurrentSketchMd5();
  doc["sourceVersion"] = MESHLED_VERSION;
  doc["sourceReleaseSha"] = MESHLED_RELEASE_SHA;
  doc["uptimeMs"] = millis();
  doc["freeHeap"] = ESP.getFreeHeap();
  meshledWriteOtaStatusDoc(doc);
}

inline void recordOtaEndStatus() {
  DynamicJsonDocument doc(768);
  meshledReadOtaStatusDoc(doc);
  doc["stage"] = "end";
  doc["errorCode"] = -1;
  doc["errorName"] = "";
  doc["detail"] = "upload_complete_waiting_reboot";
  doc["uptimeMs"] = millis();
  doc["freeHeap"] = ESP.getFreeHeap();
  meshledWriteOtaStatusDoc(doc);
}

inline void recordOtaErrorStatus(int errorCode, const String& detail) {
  DynamicJsonDocument doc(768);
  meshledReadOtaStatusDoc(doc);
  doc["stage"] = "error";
  doc["errorCode"] = errorCode;
  doc["errorName"] = meshledOtaErrorCodeName(errorCode);
  doc["detail"] = detail;
  doc["uptimeMs"] = millis();
  doc["freeHeap"] = ESP.getFreeHeap();
  meshledWriteOtaStatusDoc(doc);
}

inline void finalizeOtaBootStatus() {
  DynamicJsonDocument doc(1024);
  if (!meshledReadOtaStatusDoc(doc)) {
    return;
  }

  const String stage = doc["stage"] | "";
  if (stage != "start" && stage != "end") {
    return;
  }

  const String sourceSketchMd5 = doc["sourceSketchMd5"] | "";
  const String currentSketchMd5 = meshledCurrentSketchMd5();
  doc["bootSketchMd5"] = currentSketchMd5;
  doc["bootVersion"] = MESHLED_VERSION;
  doc["bootReleaseSha"] = MESHLED_RELEASE_SHA;
  doc["bootUptimeMs"] = millis();
  doc["bootFreeHeap"] = ESP.getFreeHeap();

  if (stage == "start") {
    doc["stage"] = "interrupted";
    doc["detail"] = "ota_did_not_reach_onEnd";
  } else if (sourceSketchMd5.length() > 0 && currentSketchMd5.length() > 0) {
    if (sourceSketchMd5 == currentSketchMd5) {
      doc["stage"] = "reverted_or_not_applied";
      doc["detail"] = "rebooted_into_same_sketch_after_ota";
    } else {
      doc["stage"] = "booted_new_firmware";
      doc["detail"] = "new_sketch_hash_detected_after_ota";
    }
  } else {
    doc["stage"] = "booted_after_ota";
    doc["detail"] = "missing_sketch_hash_for_comparison";
  }

  meshledWriteOtaStatusDoc(doc);
}

inline bool populateOtaStatus(JsonObject target) {
  DynamicJsonDocument doc(1024);
  if (!meshledReadOtaStatusDoc(doc)) {
    return false;
  }

  JsonObject source = doc.as<JsonObject>();
  for (JsonPair pair : source) {
    target[pair.key().c_str()] = pair.value();
  }

  return true;
}

#if MESHLED_HAS_RESET_REASON
inline String meshledResetReasonToString(esp_reset_reason_t reason) {
  switch (reason) {
    case ESP_RST_UNKNOWN: return "unknown";
    case ESP_RST_POWERON: return "power_on";
    case ESP_RST_EXT: return "external_reset";
    case ESP_RST_SW: return "software_reset";
    case ESP_RST_PANIC: return "panic";
    case ESP_RST_INT_WDT: return "interrupt_watchdog";
    case ESP_RST_TASK_WDT: return "task_watchdog";
    case ESP_RST_WDT: return "watchdog";
    case ESP_RST_DEEPSLEEP: return "deep_sleep";
    case ESP_RST_BROWNOUT: return "brownout";
    case ESP_RST_SDIO: return "sdio";
    default: return "unmapped";
  }
}
#endif

#if MESHLED_HAS_OTA_PARTITION_INFO
inline String meshledOtaImageStateToString(esp_ota_img_states_t state) {
  switch (state) {
    case ESP_OTA_IMG_NEW: return "new";
    case ESP_OTA_IMG_PENDING_VERIFY: return "pending_verify";
    case ESP_OTA_IMG_VALID: return "valid";
    case ESP_OTA_IMG_INVALID: return "invalid";
    case ESP_OTA_IMG_ABORTED: return "aborted";
#if defined(ESP_OTA_IMG_UNDEFINED)
    case ESP_OTA_IMG_UNDEFINED: return "undefined";
#endif
    default: return "unknown";
  }
}
#endif
