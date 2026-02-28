#pragma once

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <esp_arduino_version.h>
#include <freertos/FreeRTOS.h>
#include <freertos/portmacro.h>
#include <map>
#include <cstring>

// Forward declarations
class Port;
class Intersection;

// Maximum size for ESP-NOW payload (250 bytes)
#define MAX_ESPNOW_PAYLOAD_SIZE 250

// Message type identifier
enum ESPNowMessageType {
  DISCOVERY_REQUEST = 0x01,
  DISCOVERY_REPLY = 0x02,
  LIGHT_MESSAGE = 0x10,
  LIGHTLIST_MESSAGE = 0x11
};

// Structure for ESP-NOW message header
typedef struct {
  uint8_t messageType;       // Type of message (see ESPNowMessageType)
  uint8_t deviceId[6];       // MAC address of sending device
} ESPNowHeader;

// Discovery request message
typedef struct {
  ESPNowHeader header;
  uint8_t channel;           // Current channel of requesting device
} DiscoveryRequest;

// Discovery reply message
typedef struct {
  ESPNowHeader header;
  uint8_t channel;           // Current channel of replying device
  char deviceName[32];       // Optional device identifier
} DiscoveryReply;

// LightList message structure
typedef struct {
  uint8_t messageType;
  uint16_t id;
  LightMessage light;
} LightListMessage;

// For storing discovered peers during scanning
#define MAX_PEERS 20
inline esp_now_peer_info_t knownPeers[MAX_PEERS] = {};
inline uint16_t peerCount = 0;

// Broadcast MAC address for ESP-NOW discovery
inline uint8_t broadcastMAC[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Discovery state
inline bool discoveryActive = false;
inline unsigned long discoveryStartTime = 0;
#define DISCOVERY_TIMEOUT_MS 5000

inline const char* gESPNowLastError = "none";
inline bool gESPNowInitialized = false;

constexpr uint8_t MAX_ESPNOW_RX_QUEUE = 24;
constexpr uint8_t MAX_ESPNOW_PROCESS_PER_TICK = 8;

struct ESPNowQueuedPacket {
  uint8_t type = 0;
  uint8_t src[6] = {0};
  uint8_t len = 0;
  uint8_t payload[sizeof(LightListMessage)] = {0};
};

inline ESPNowQueuedPacket gESPNowRxQueue[MAX_ESPNOW_RX_QUEUE];
inline volatile uint8_t gESPNowRxHead = 0;
inline volatile uint8_t gESPNowRxTail = 0;
inline volatile uint8_t gESPNowRxCount = 0;
inline volatile uint16_t gESPNowDroppedPackets = 0;
inline portMUX_TYPE gESPNowQueueMux = portMUX_INITIALIZER_UNLOCKED;

constexpr size_t MAX_REMOTE_LIGHT_LISTS = 24;
inline std::map<uint16_t, LightList*> gRemoteLightLists;

inline void setESPNowLastError(const char* error) {
  if (error != nullptr && error[0] != '\0') {
    gESPNowLastError = error;
  }
}

inline const char* getESPNowLastError() {
  return gESPNowLastError;
}

inline uint16_t getESPNowDroppedPacketCount() {
  return gESPNowDroppedPackets;
}

inline bool isESPNowDiscoveryActive() {
  return discoveryActive;
}

inline uint16_t getKnownPeerCount() {
  return peerCount;
}

inline bool getKnownPeerAt(uint16_t index, uint8_t mac[6], uint8_t* channel, bool* encrypted) {
  if (index >= peerCount) {
    return false;
  }
  if (mac != nullptr) {
    std::memcpy(mac, knownPeers[index].peer_addr, 6);
  }
  if (channel != nullptr) {
    *channel = knownPeers[index].channel;
  }
  if (encrypted != nullptr) {
    *encrypted = knownPeers[index].encrypt;
  }
  return true;
}

inline bool enqueueESPNowPacketFromISR(const uint8_t* src_addr, uint8_t type, const uint8_t* payload,
                                       uint8_t payloadLen) {
  if (src_addr == nullptr || payload == nullptr || payloadLen > sizeof(ESPNowQueuedPacket::payload)) {
    return false;
  }

  bool queued = false;
  portENTER_CRITICAL_ISR(&gESPNowQueueMux);
  if (gESPNowRxCount < MAX_ESPNOW_RX_QUEUE) {
    ESPNowQueuedPacket& slot = gESPNowRxQueue[gESPNowRxHead];
    slot.type = type;
    slot.len = payloadLen;
    std::memcpy(slot.src, src_addr, sizeof(slot.src));
    std::memcpy(slot.payload, payload, payloadLen);

    gESPNowRxHead = static_cast<uint8_t>((gESPNowRxHead + 1) % MAX_ESPNOW_RX_QUEUE);
    gESPNowRxCount++;
    queued = true;
  } else {
    gESPNowDroppedPackets++;
  }
  portEXIT_CRITICAL_ISR(&gESPNowQueueMux);

  return queued;
}

inline bool dequeueESPNowPacket(ESPNowQueuedPacket& packet) {
  bool hasPacket = false;
  portENTER_CRITICAL(&gESPNowQueueMux);
  if (gESPNowRxCount > 0) {
    packet = gESPNowRxQueue[gESPNowRxTail];
    gESPNowRxTail = static_cast<uint8_t>((gESPNowRxTail + 1) % MAX_ESPNOW_RX_QUEUE);
    gESPNowRxCount--;
    hasPacket = true;
  }
  portEXIT_CRITICAL(&gESPNowQueueMux);
  return hasPacket;
}

inline bool isKnownPeer(const uint8_t* mac_addr) {
  for (uint16_t i = 0; i < peerCount; i++) {
    if (std::memcmp(knownPeers[i].peer_addr, mac_addr, 6) == 0) {
      return true;
    }
  }
  return false;
}

inline bool rememberKnownPeer(const uint8_t* mac_addr, uint8_t channel, bool encrypted) {
  if (mac_addr == nullptr) {
    return false;
  }

  for (uint16_t i = 0; i < peerCount; i++) {
    if (std::memcmp(knownPeers[i].peer_addr, mac_addr, 6) == 0) {
      knownPeers[i].channel = channel;
      knownPeers[i].encrypt = encrypted;
      return true;
    }
  }

  if (peerCount >= MAX_PEERS) {
    setESPNowLastError("peer_list_full");
    return false;
  }

  std::memcpy(knownPeers[peerCount].peer_addr, mac_addr, 6);
  knownPeers[peerCount].channel = channel;
  knownPeers[peerCount].encrypt = encrypted;
  peerCount++;
  return true;
}

// Register a peer with ESP-NOW
inline bool addPeer(const uint8_t* mac_addr, uint8_t channel) {
  if (mac_addr == nullptr) {
    setESPNowLastError("peer_null_mac");
    return false;
  }

  if (esp_now_is_peer_exist(mac_addr)) {
    return true;
  }

  esp_now_peer_info_t peer = {};
  std::memcpy(peer.peer_addr, mac_addr, 6);
  peer.channel = channel;
  peer.encrypt = false;
  peer.ifidx = WIFI_IF_STA;

  wifi_mode_t mode = WIFI_MODE_NULL;
  if (esp_wifi_get_mode(&mode) == ESP_OK) {
    if (mode & WIFI_MODE_STA) {
      peer.ifidx = WIFI_IF_STA;
    } else if (mode & WIFI_MODE_AP) {
      peer.ifidx = WIFI_IF_AP;
    }
  }

  const esp_err_t result = esp_now_add_peer(&peer);
  if (result != ESP_OK) {
    setESPNowLastError("peer_add_failed");
    return false;
  }
  return true;
}

inline uint8_t resolvePeerChannel(const uint8_t* mac_addr) {
  if (mac_addr != nullptr) {
    for (uint16_t i = 0; i < peerCount; i++) {
      if (std::memcmp(knownPeers[i].peer_addr, mac_addr, 6) == 0) {
        return knownPeers[i].channel;
      }
    }
  }
  return WiFi.channel();
}

inline bool ensurePeerForSend(const uint8_t* mac_addr) {
  if (mac_addr == nullptr) {
    setESPNowLastError("peer_null_mac");
    return false;
  }
  if (esp_now_is_peer_exist(mac_addr)) {
    return true;
  }

  const uint8_t channel = resolvePeerChannel(mac_addr);
  if (!addPeer(mac_addr, channel)) {
    setESPNowLastError("peer_add_before_send_failed");
    return false;
  }
  rememberKnownPeer(mac_addr, channel, false);
  return true;
}

inline LightList* getOrCreateRemoteLightList(uint16_t remoteListId) {
  auto it = gRemoteLightLists.find(remoteListId);
  if (it != gRemoteLightLists.end()) {
    return it->second;
  }

  if (gRemoteLightLists.size() >= MAX_REMOTE_LIGHT_LISTS) {
    auto evict = gRemoteLightLists.begin();
    if (evict->second != nullptr) {
      delete evict->second;
    }
    gRemoteLightLists.erase(evict);
  }

  auto* created = new LightList();
  gRemoteLightLists[remoteListId] = created;
  return created;
}

inline void handleReceivedLight(const LightMessage* lightMsg) {
  if (lightMsg == nullptr) {
    return;
  }

  Port* port = Port::findById(lightMsg->portId);
  if (port == nullptr || port->isExternal()) {
    return;
  }
  InternalPort* targetPort = static_cast<InternalPort*>(port);

  LightList* lightList = getOrCreateRemoteLightList(lightMsg->listId);
  if (lightList == nullptr) {
    setESPNowLastError("remote_list_alloc_failed");
    return;
  }

  RuntimeLight* light = lightList->addLightFromMsg(lightMsg);
  if (light == nullptr) {
    setESPNowLastError("remote_light_alloc_failed");
    return;
  }

  ColorRGB color;
  color.r = lightMsg->colorR;
  color.g = lightMsg->colorG;
  color.b = lightMsg->colorB;
  light->setColor(color);

  targetPort->sendOut(light);
}

inline void handleReceivedLightList(const LightListMessage* lightListMsg) {
  if (lightListMsg == nullptr) {
    return;
  }

  LightMessage translated = lightListMsg->light;
  translated.listId = lightListMsg->id;
  handleReceivedLight(&translated);
}

inline void handleDiscoveryRequestPacket(const uint8_t* src_addr) {
  if (src_addr == nullptr) {
    return;
  }

  DiscoveryReply reply;
  reply.header.messageType = DISCOVERY_REPLY;
  WiFi.macAddress(reply.header.deviceId);
  reply.channel = WiFi.channel();
  std::strncpy(reply.deviceName, "ESP32_Device", sizeof(reply.deviceName) - 1);
  reply.deviceName[sizeof(reply.deviceName) - 1] = '\0';

  if (!addPeer(src_addr, WiFi.channel())) {
    return;
  }
  rememberKnownPeer(src_addr, WiFi.channel(), false);

  if (esp_now_send(src_addr, reinterpret_cast<const uint8_t*>(&reply), sizeof(reply)) != ESP_OK) {
    setESPNowLastError("discovery_reply_send_failed");
  }
}

inline void handleDiscoveryReplyPacket(const uint8_t* src_addr, const DiscoveryReply* reply) {
  if (src_addr == nullptr || reply == nullptr) {
    return;
  }

  rememberKnownPeer(src_addr, reply->channel, false);
  addPeer(src_addr, reply->channel);
}

inline void processQueuedESPNowPackets() {
  ESPNowQueuedPacket packet;
  for (uint8_t i = 0; i < MAX_ESPNOW_PROCESS_PER_TICK; i++) {
    if (!dequeueESPNowPacket(packet)) {
      break;
    }

    switch (packet.type) {
      case DISCOVERY_REQUEST:
        if (packet.len >= sizeof(DiscoveryRequest)) {
          handleDiscoveryRequestPacket(packet.src);
        }
        break;
      case DISCOVERY_REPLY:
        if (packet.len >= sizeof(DiscoveryReply)) {
          const auto* reply = reinterpret_cast<const DiscoveryReply*>(packet.payload);
          handleDiscoveryReplyPacket(packet.src, reply);
        }
        break;
      case LIGHT_MESSAGE:
        if (packet.len >= sizeof(LightMessage)) {
          const auto* lightMsg = reinterpret_cast<const LightMessage*>(packet.payload);
          handleReceivedLight(lightMsg);
        }
        break;
      case LIGHTLIST_MESSAGE:
        if (packet.len >= sizeof(LightListMessage)) {
          const auto* lightListMsg = reinterpret_cast<const LightListMessage*>(packet.payload);
          handleReceivedLightList(lightListMsg);
        }
        break;
      default:
        break;
    }
  }
}

// Callback function for when data is sent
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
inline void onDataSent(const esp_now_send_info_t* /*tx_info*/, esp_now_send_status_t status) {
#else
inline void onDataSent(const uint8_t* /*mac_addr*/, esp_now_send_status_t status) {
#endif
  if (status != ESP_NOW_SEND_SUCCESS) {
    setESPNowLastError("send_failed");
  }
}

// Callback function for when data is received
#if defined(ESP_IDF_VERSION) && (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0))
inline void onDataReceived(const esp_now_recv_info_t* esp_now_info, const uint8_t* data, int data_len) {
  const uint8_t* src_addr = (esp_now_info != nullptr) ? esp_now_info->src_addr : nullptr;
#else
inline void onDataReceived(const uint8_t* src_addr, const uint8_t* data, int data_len) {
#endif
  if (src_addr == nullptr || data == nullptr || data_len < static_cast<int>(sizeof(ESPNowHeader))) {
    return;
  }

  const auto* header = reinterpret_cast<const ESPNowHeader*>(data);
  uint8_t messageLen = 0;

  switch (header->messageType) {
    case DISCOVERY_REQUEST:
      if (data_len < static_cast<int>(sizeof(DiscoveryRequest))) return;
      messageLen = static_cast<uint8_t>(sizeof(DiscoveryRequest));
      break;
    case DISCOVERY_REPLY:
      if (data_len < static_cast<int>(sizeof(DiscoveryReply))) return;
      messageLen = static_cast<uint8_t>(sizeof(DiscoveryReply));
      break;
    case LIGHT_MESSAGE:
      if (data_len < static_cast<int>(sizeof(LightMessage))) return;
      messageLen = static_cast<uint8_t>(sizeof(LightMessage));
      break;
    case LIGHTLIST_MESSAGE:
      if (data_len < static_cast<int>(sizeof(LightListMessage))) return;
      messageLen = static_cast<uint8_t>(sizeof(LightListMessage));
      break;
    default:
      return;
  }

  enqueueESPNowPacketFromISR(src_addr, header->messageType, data, messageLen);
}

// Initialize ESP-NOW
inline bool initESPNow() {
  if (gESPNowInitialized) {
    return true;
  }

  if (esp_now_init() != ESP_OK) {
    setESPNowLastError("init_failed");
    return false;
  }

  esp_now_register_send_cb(onDataSent);
  esp_now_register_recv_cb(onDataReceived);

  gESPNowInitialized = true;
  setESPNowLastError("none");
  return true;
}

inline bool broadcastDiscoveryRequest() {
  if (!gESPNowInitialized) {
    setESPNowLastError("not_initialized");
    return false;
  }

  DiscoveryRequest request;
  request.header.messageType = DISCOVERY_REQUEST;
  WiFi.macAddress(request.header.deviceId);
  request.channel = WiFi.channel();

  if (!esp_now_is_peer_exist(broadcastMAC) && !addPeer(broadcastMAC, 0)) {
    return false;
  }

  const esp_err_t result = esp_now_send(broadcastMAC, reinterpret_cast<const uint8_t*>(&request), sizeof(request));
  if (result != ESP_OK) {
    setESPNowLastError("discovery_broadcast_failed");
    return false;
  }

  discoveryActive = true;
  discoveryStartTime = millis();
  return true;
}

// Start ESP-NOW peer discovery in non-blocking mode.
inline bool scanForPeers() {
  peerCount = 0;
  discoveryActive = false;
  discoveryStartTime = 0;
  return broadcastDiscoveryRequest();
}

inline void tickESPNow() {
  processQueuedESPNowPackets();

  if (discoveryActive && (millis() - discoveryStartTime > DISCOVERY_TIMEOUT_MS)) {
    discoveryActive = false;
  }
}

inline LightMessage packLight(uint8_t portId, RuntimeLight* const light) {
  LightMessage msg;
  msg.messageType = LIGHT_MESSAGE;
  msg.portId = portId;
  msg.listId = light->list ? light->list->id : 0;
  msg.lightIdx = light->idx;
  msg.brightness = light->getBrightness();

  ColorRGB color = light->getColor();
  msg.colorR = color.r;
  msg.colorG = color.g;
  msg.colorB = color.b;

  msg.speed = light->getSpeed();
  msg.life = light->getLife();

  return msg;
}

inline bool sendLightViaESPNow_impl(const uint8_t* mac, uint8_t portId, RuntimeLight* const light,
                                    bool sendList = false) {
  if (mac == nullptr || light == nullptr) {
    setESPNowLastError("send_invalid_args");
    return false;
  }

  if (!ensurePeerForSend(mac)) {
    return false;
  }

  if (sendList && light->list) {
    LightListMessage msg;
    msg.messageType = LIGHTLIST_MESSAGE;
    msg.id = light->list->id;
    msg.light = packLight(portId, light);

    if (esp_now_send(mac, reinterpret_cast<const uint8_t*>(&msg), sizeof(msg)) != ESP_OK) {
      setESPNowLastError("send_lightlist_failed");
      return false;
    }
  } else {
    LightMessage msg = packLight(portId, light);
    if (esp_now_send(mac, reinterpret_cast<const uint8_t*>(&msg), sizeof(msg)) != ESP_OK) {
      setESPNowLastError("send_light_failed");
      return false;
    }
  }
  return true;
}

// Forward declaration of function pointer from Port.h
extern void (*sendLightViaESPNow)(const uint8_t* mac, uint8_t id, RuntimeLight* const light, bool sendList);
