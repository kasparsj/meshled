#pragma once

#include <Arduino.h>
#include <cstring>
#include <cstdint>

class RuntimeLight;

enum class ExternalTransportRuntimeState : uint8_t {
  Disabled = 0,
  Initializing = 1,
  Ready = 2,
  Degraded = 3,
};

struct ExternalTransportAdapter {
  const char* name;
  bool (*init)();
  bool (*ready)();
  bool (*send)(const uint8_t* mac, uint8_t portId, RuntimeLight* const light, bool sendList);
  void (*tick)();
  bool (*discover)();
  bool (*discoveryInProgress)();
  uint16_t (*peerCount)();
  bool (*peerAt)(uint16_t index, uint8_t mac[6], uint8_t* channel, bool* encrypted);
  uint16_t (*droppedPackets)();
  const char* (*lastError)();
};

inline ExternalTransportAdapter* gExternalTransportAdapter = nullptr;
inline bool gExternalTransportInitialized = false;
inline bool gExternalTransportReady = false;
inline ExternalTransportRuntimeState gExternalTransportState = ExternalTransportRuntimeState::Disabled;
inline uint16_t gExternalTransportConsecutiveFailures = 0;
inline unsigned long gExternalTransportLastErrorAt = 0;
inline unsigned long gExternalTransportDegradedUntil = 0;
inline const char* gExternalTransportLastError = "none";

constexpr uint16_t EXTERNAL_TRANSPORT_FAILURE_THRESHOLD = 3;
constexpr unsigned long EXTERNAL_TRANSPORT_DEGRADE_COOLDOWN_MS = 15000;

// Function pointer exposed by lightgraph ExternalPort runtime.
extern void (*sendLightViaESPNow)(const uint8_t* mac, uint8_t id, RuntimeLight* const light, bool sendList);

inline const char* externalTransportRuntimeStateName(ExternalTransportRuntimeState state) {
  switch (state) {
    case ExternalTransportRuntimeState::Disabled:
      return "disabled";
    case ExternalTransportRuntimeState::Initializing:
      return "initializing";
    case ExternalTransportRuntimeState::Ready:
      return "ready";
    case ExternalTransportRuntimeState::Degraded:
      return "degraded";
  }
  return "unknown";
}

inline void externalTransportSetLastError(const char* error) {
  if (error != nullptr && std::strlen(error) > 0) {
    gExternalTransportLastError = error;
    gExternalTransportLastErrorAt = millis();
  }
}

inline void externalTransportMarkReady() {
  gExternalTransportReady = true;
  gExternalTransportState = ExternalTransportRuntimeState::Ready;
  gExternalTransportConsecutiveFailures = 0;
}

inline void externalTransportMarkFailure(const char* error) {
  externalTransportSetLastError(error);
  if (gExternalTransportConsecutiveFailures < UINT16_MAX) {
    gExternalTransportConsecutiveFailures++;
  }

  gExternalTransportReady = false;
  if (gExternalTransportConsecutiveFailures >= EXTERNAL_TRANSPORT_FAILURE_THRESHOLD) {
    gExternalTransportState = ExternalTransportRuntimeState::Degraded;
    gExternalTransportDegradedUntil = millis() + EXTERNAL_TRANSPORT_DEGRADE_COOLDOWN_MS;
  } else {
    gExternalTransportState = ExternalTransportRuntimeState::Initializing;
  }
}

inline void registerExternalTransportAdapter(ExternalTransportAdapter* adapter) {
  gExternalTransportAdapter = adapter;
  gExternalTransportInitialized = false;
  gExternalTransportReady = false;
  gExternalTransportConsecutiveFailures = 0;
  gExternalTransportDegradedUntil = 0;
  gExternalTransportLastError = "none";
  gExternalTransportLastErrorAt = 0;
  gExternalTransportState = (adapter != nullptr) ? ExternalTransportRuntimeState::Initializing
                                                 : ExternalTransportRuntimeState::Disabled;
}

inline bool isExternalTransportEnabled() {
  return gExternalTransportAdapter != nullptr;
}

inline const char* externalTransportName() {
  return isExternalTransportEnabled() ? gExternalTransportAdapter->name : "none";
}

inline ExternalTransportRuntimeState externalTransportRuntimeState() {
  return gExternalTransportState;
}

inline uint16_t externalTransportConsecutiveFailures() {
  return gExternalTransportConsecutiveFailures;
}

inline unsigned long externalTransportLastErrorAt() {
  return gExternalTransportLastErrorAt;
}

inline const char* externalTransportLastError() {
  if (!isExternalTransportEnabled()) {
    return "none";
  }
  if (gExternalTransportAdapter->lastError != nullptr) {
    const char* adapterError = gExternalTransportAdapter->lastError();
    if (adapterError != nullptr && std::strlen(adapterError) > 0 && std::strcmp(adapterError, "none") != 0) {
      return adapterError;
    }
  }
  return gExternalTransportLastError;
}

inline uint16_t externalTransportPeerCount() {
  if (!isExternalTransportEnabled() || gExternalTransportAdapter->peerCount == nullptr) {
    return 0;
  }
  return gExternalTransportAdapter->peerCount();
}

inline bool externalTransportGetPeerAt(uint16_t index, uint8_t mac[6], uint8_t* channel, bool* encrypted) {
  if (!isExternalTransportEnabled() || gExternalTransportAdapter->peerAt == nullptr) {
    return false;
  }
  return gExternalTransportAdapter->peerAt(index, mac, channel, encrypted);
}

inline bool externalTransportDiscoveryInProgress() {
  if (!isExternalTransportEnabled() || gExternalTransportAdapter->discoveryInProgress == nullptr) {
    return false;
  }
  return gExternalTransportAdapter->discoveryInProgress();
}

inline uint16_t externalTransportDroppedPackets() {
  if (!isExternalTransportEnabled() || gExternalTransportAdapter->droppedPackets == nullptr) {
    return 0;
  }
  return gExternalTransportAdapter->droppedPackets();
}

inline bool externalTransportInDegradedCooldown() {
  return gExternalTransportState == ExternalTransportRuntimeState::Degraded &&
         millis() < gExternalTransportDegradedUntil;
}

inline bool externalTransportTryInit() {
  if (!isExternalTransportEnabled()) {
    return false;
  }
  if (externalTransportInDegradedCooldown()) {
    return false;
  }

  gExternalTransportState = ExternalTransportRuntimeState::Initializing;
  if (gExternalTransportAdapter->init == nullptr) {
    gExternalTransportInitialized = true;
    externalTransportMarkReady();
    return true;
  }

  const bool initialized = gExternalTransportAdapter->init();
  gExternalTransportInitialized = initialized;
  if (!initialized) {
    externalTransportMarkFailure("init_failed");
    return false;
  }

  if (gExternalTransportAdapter->ready == nullptr || gExternalTransportAdapter->ready()) {
    externalTransportMarkReady();
  } else {
    gExternalTransportReady = false;
    gExternalTransportState = ExternalTransportRuntimeState::Initializing;
  }

  return true;
}

inline bool externalTransportIsReady() {
  if (!isExternalTransportEnabled() || !gExternalTransportInitialized) {
    return false;
  }
  if (externalTransportInDegradedCooldown()) {
    gExternalTransportReady = false;
    return false;
  }
  if (gExternalTransportAdapter->ready == nullptr) {
    return gExternalTransportReady && gExternalTransportState == ExternalTransportRuntimeState::Ready;
  }
  if (!gExternalTransportAdapter->ready()) {
    gExternalTransportReady = false;
    if (gExternalTransportState == ExternalTransportRuntimeState::Ready) {
      gExternalTransportState = ExternalTransportRuntimeState::Initializing;
    }
    return false;
  }
  gExternalTransportReady = true;
  gExternalTransportState = ExternalTransportRuntimeState::Ready;
  return gExternalTransportReady;
}

inline bool externalTransportSend(const uint8_t* mac, uint8_t portId, RuntimeLight* const light,
                                  bool sendList) {
  if (!isExternalTransportEnabled() || gExternalTransportAdapter->send == nullptr) {
    return false;
  }
  if (!externalTransportIsReady() && !externalTransportTryInit()) {
    return false;
  }
  if (!externalTransportIsReady()) {
    return false;
  }
  if (!gExternalTransportAdapter->send(mac, portId, light, sendList)) {
    externalTransportMarkFailure("send_failed");
    return false;
  }
  externalTransportMarkReady();
  return true;
}

inline void sendLightViaExternalTransportBridge(const uint8_t* mac, uint8_t portId,
                                                RuntimeLight* const light, bool sendList) {
  externalTransportSend(mac, portId, light, sendList);
}

inline bool externalTransportStartDiscovery() {
  if (!isExternalTransportEnabled() || gExternalTransportAdapter->discover == nullptr) {
    return false;
  }
  if (!externalTransportIsReady() && !externalTransportTryInit()) {
    return false;
  }
  if (!gExternalTransportAdapter->discover()) {
    externalTransportMarkFailure("discover_failed");
    return false;
  }
  return true;
}

inline void tickExternalTransport() {
  if (!isExternalTransportEnabled()) {
    return;
  }

  if (externalTransportInDegradedCooldown()) {
    gExternalTransportReady = false;
    if (gExternalTransportAdapter->tick != nullptr) {
      gExternalTransportAdapter->tick();
    }
    return;
  }

  if (!gExternalTransportInitialized || gExternalTransportState == ExternalTransportRuntimeState::Degraded ||
      gExternalTransportState == ExternalTransportRuntimeState::Initializing) {
    externalTransportTryInit();
  }

  if (gExternalTransportAdapter->tick != nullptr) {
    gExternalTransportAdapter->tick();
  }

  if (gExternalTransportAdapter->ready != nullptr) {
    if (gExternalTransportAdapter->ready()) {
      // Preserve failure counters while already healthy; only transition once.
      if (!gExternalTransportReady || gExternalTransportState != ExternalTransportRuntimeState::Ready) {
        externalTransportMarkReady();
      } else {
        gExternalTransportReady = true;
      }
    } else {
      gExternalTransportReady = false;
      if (gExternalTransportState == ExternalTransportRuntimeState::Ready) {
        gExternalTransportState = ExternalTransportRuntimeState::Initializing;
      }
    }
  }
}

inline bool initExternalTransport() {
  if (!isExternalTransportEnabled()) {
    gExternalTransportInitialized = false;
    gExternalTransportReady = false;
    gExternalTransportState = ExternalTransportRuntimeState::Disabled;
    sendLightViaESPNow = nullptr;
    return false;
  }

  sendLightViaESPNow = sendLightViaExternalTransportBridge;
  return externalTransportTryInit();
}
