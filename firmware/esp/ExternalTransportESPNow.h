#pragma once

#include "ExternalTransport.h"

#ifdef ESPNOW_ENABLED

inline bool gESPNowTransportReady = false;

inline bool initESPNowTransportAdapter() {
  gESPNowTransportReady = initESPNow();
  return gESPNowTransportReady;
}

inline bool isESPNowTransportReady() {
  return gESPNowTransportReady;
}

inline bool sendESPNowTransport(const uint8_t* mac, uint8_t portId, RuntimeLight* const light,
                                bool sendList) {
  if (!gESPNowTransportReady) {
    return false;
  }
  return sendLightViaESPNow_impl(mac, portId, light, sendList);
}

inline void tickESPNowTransportAdapter() {
  tickESPNow();
}

inline bool discoverESPNowTransportPeers() {
  return scanForPeers();
}

inline bool isESPNowTransportDiscoveryInProgress() {
  return isESPNowDiscoveryActive();
}

inline uint16_t getESPNowTransportPeerCount() {
  return getKnownPeerCount();
}

inline bool getESPNowTransportPeerAt(uint16_t index, uint8_t mac[6], uint8_t* channel, bool* encrypted) {
  return getKnownPeerAt(index, mac, channel, encrypted);
}

inline uint16_t getESPNowTransportDroppedPackets() {
  return getESPNowDroppedPacketCount();
}

inline const char* getESPNowTransportLastError() {
  return getESPNowLastError();
}

inline ExternalTransportAdapter gESPNowExternalTransport = {
    "esp-now",
    initESPNowTransportAdapter,
    isESPNowTransportReady,
    sendESPNowTransport,
    tickESPNowTransportAdapter,
    discoverESPNowTransportPeers,
    isESPNowTransportDiscoveryInProgress,
    getESPNowTransportPeerCount,
    getESPNowTransportPeerAt,
    getESPNowTransportDroppedPackets,
    getESPNowTransportLastError,
};

inline void setupExternalTransportAdapters() {
  registerExternalTransportAdapter(&gESPNowExternalTransport);
}

#else

inline void setupExternalTransportAdapters() {
  registerExternalTransportAdapter(nullptr);
}

#endif
