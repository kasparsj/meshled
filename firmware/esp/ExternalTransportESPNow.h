#pragma once

#include "ExternalTransport.h"

#ifdef ESPNOW_ENABLED

inline bool gESPNowTransportReady = false;

inline bool initESPNowTransportAdapter() {
  gESPNowTransportReady = initESPNow();
  if (!gESPNowTransportReady) {
    return false;
  }
  scanForPeers();
  return true;
}

inline bool isESPNowTransportReady() {
  return gESPNowTransportReady;
}

inline bool sendESPNowTransport(const uint8_t* mac, uint8_t portId, RuntimeLight* const light,
                                bool sendList) {
  if (!gESPNowTransportReady) {
    return false;
  }
  sendLightViaESPNow_impl(mac, portId, light, sendList);
  return true;
}

inline ExternalTransportAdapter gESPNowExternalTransport = {
    "esp-now",
    initESPNowTransportAdapter,
    isESPNowTransportReady,
    sendESPNowTransport,
};

inline void setupExternalTransportAdapters() {
  registerExternalTransportAdapter(&gESPNowExternalTransport);
}

#else

inline void setupExternalTransportAdapters() {
  registerExternalTransportAdapter(nullptr);
}

#endif
