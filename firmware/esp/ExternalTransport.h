#pragma once

#include <cstdint>

class RuntimeLight;

struct ExternalTransportAdapter {
  const char* name;
  bool (*init)();
  bool (*ready)();
  bool (*send)(const uint8_t* mac, uint8_t portId, RuntimeLight* const light, bool sendList);
};

inline ExternalTransportAdapter* gExternalTransportAdapter = nullptr;
inline bool gExternalTransportInitialized = false;
inline bool gExternalTransportReady = false;

// Function pointer exposed by lightgraph ExternalPort runtime.
extern void (*sendLightViaESPNow)(const uint8_t* mac, uint8_t id, RuntimeLight* const light, bool sendList);

inline void registerExternalTransportAdapter(ExternalTransportAdapter* adapter) {
  gExternalTransportAdapter = adapter;
}

inline bool isExternalTransportEnabled() {
  return gExternalTransportAdapter != nullptr;
}

inline const char* externalTransportName() {
  return isExternalTransportEnabled() ? gExternalTransportAdapter->name : "none";
}

inline bool externalTransportIsReady() {
  if (!isExternalTransportEnabled() || !gExternalTransportInitialized) {
    return false;
  }
  if (gExternalTransportAdapter->ready == nullptr) {
    return gExternalTransportReady;
  }
  gExternalTransportReady = gExternalTransportAdapter->ready();
  return gExternalTransportReady;
}

inline bool externalTransportSend(const uint8_t* mac, uint8_t portId, RuntimeLight* const light,
                                  bool sendList) {
  if (!externalTransportIsReady() || gExternalTransportAdapter->send == nullptr) {
    return false;
  }
  return gExternalTransportAdapter->send(mac, portId, light, sendList);
}

inline void sendLightViaExternalTransportBridge(const uint8_t* mac, uint8_t portId,
                                                RuntimeLight* const light, bool sendList) {
  externalTransportSend(mac, portId, light, sendList);
}

inline bool initExternalTransport() {
  if (!isExternalTransportEnabled()) {
    gExternalTransportInitialized = false;
    gExternalTransportReady = false;
    sendLightViaESPNow = nullptr;
    return false;
  }

  sendLightViaESPNow = sendLightViaExternalTransportBridge;

  if (gExternalTransportAdapter->init == nullptr) {
    gExternalTransportInitialized = true;
    gExternalTransportReady = true;
    return true;
  }

  gExternalTransportInitialized = gExternalTransportAdapter->init();
  gExternalTransportReady = gExternalTransportInitialized;
  return gExternalTransportInitialized;
}
