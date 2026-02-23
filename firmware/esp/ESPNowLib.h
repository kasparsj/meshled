#pragma once

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include "lwip/err.h"
#include "lwip/ip_addr.h"
#include "lwip/etharp.h"
#include <ESPping.h>  // For pinging devices to get MAC addresses
#include <map>

// Forward declarations
class Port;
class Light;
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
esp_now_peer_info_t knownPeers[MAX_PEERS];
int peerCount = 0;

// LightList ID mapping for ESP-NOW received lists (old ID -> new ID)
static std::map<uint16_t, uint16_t> lightListIdMap;

// Broadcast MAC address for ESP-NOW discovery
uint8_t broadcastMAC[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Discovery state
bool discoveryActive = false;
unsigned long discoveryStartTime = 0;
#define DISCOVERY_TIMEOUT_MS 5000

// Function to lookup MAC from IP using LwIP ARP cache
bool getMacFromArpCache(const IPAddress& ip, uint8_t* mac) {
    ip4_addr_t ip4;
    ip4.addr = ip; // convert IPAddress to ip4_addr_t

    struct eth_addr* eth_ret;
    err_t res = etharp_find_addr(netif_default, &ip4, &eth_ret, NULL);
    if (res == ERR_OK && eth_ret != NULL) {
        memcpy(mac, eth_ret->addr, 6);
        return true;
    }
    return false;
}

// Your getMacFromIP function fixed
bool getMacFromIP(const IPAddress& ip, uint8_t* mac) {
    // 1) If device is in station list (SoftAP mode), get MAC from connected stations
    if (WiFi.getMode() & WIFI_MODE_AP) {
        wifi_sta_list_t stationList;
        esp_err_t result = esp_wifi_ap_get_sta_list(&stationList);
        if (result == ESP_OK) {
            for (int i = 0; i < stationList.num; i++) {
                wifi_sta_info_t station = stationList.sta[i];
                // We don't have IP here, only MAC - so try ARP cache for IP to MAC mapping
                if (getMacFromArpCache(ip, mac)) {
                    if (memcmp(mac, station.mac, 6) == 0) {
                        // MAC matches connected station MAC
                        return true;
                    }
          }
        }
        } else {
            LP_LOGLN("Failed to get station list");
        }
      }

    // 2) If not found yet, try ARP cache directly
    if (getMacFromArpCache(ip, mac)) {
        return true;
    }

    // 3) If not in ARP cache, try pinging the IP to populate ARP cache
    if (Ping.ping(ip, 1)) {
        LP_LOGF("Pinged %s successfully\n", ip.toString().c_str());
        // After ping, try ARP cache again
        if (getMacFromArpCache(ip, mac)) {
            return true;
  }
    } else {
        LP_LOGF("Failed to ping %s\n", ip.toString().c_str());
}

    // 4) If still not found, you cannot get MAC (IP might be offline or unreachable)
    return false;
}

// Register a peer with ESP-NOW
bool addPeer(const uint8_t* mac_addr, uint8_t channel) {
  esp_now_peer_info_t peer;
  memcpy(peer.peer_addr, mac_addr, 6);
  peer.channel = channel;
  peer.encrypt = false;

  // Check if peer exists before adding
  if (!esp_now_is_peer_exist(mac_addr)) {
    esp_err_t result = esp_now_add_peer(&peer);
    if (result != ESP_OK) {
      LP_LOGLN("Failed to add ESP-NOW peer");
      return false;
    }
    return true;
  }
  return true; // Already added
}

void mdnsDiscoverPeers() {
  // Discover devices on the network
  std::vector<IPAddress> deviceIPs = getDevices();
  LP_LOGF("Discovered %d potential devices\n", deviceIPs.size());

  // Get MAC addresses for each device
  for (const IPAddress& ip : deviceIPs) {
    if (peerCount >= MAX_PEERS) {
      LP_LOGLN("Maximum peer count reached");
      break;
    }

    uint8_t mac[6];
    if (getMacFromIP(ip, mac)) {
      // Don't add our own MAC address
      uint8_t ourMac[6];
      WiFi.macAddress(ourMac);
      if (memcmp(mac, ourMac, 6) == 0) {
        LP_LOGLN("Skipping our own MAC address");
        continue;
  }

      // Store peer info
      memcpy(knownPeers[peerCount].peer_addr, mac, 6);
      knownPeers[peerCount].channel = WiFi.channel();
      knownPeers[peerCount].encrypt = false;

      char macStr[18];
      snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
              mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
      LP_LOGF("Added peer %s at %s\n", macStr, ip.toString().c_str());

      peerCount++;
    } else {
      LP_LOGF("Could not resolve MAC for %s\n", ip.toString().c_str());
}
  }
}

void espNowDiscoverPeers() {
  // Disconnect from WiFi to scan
  WiFi.disconnect();
  delay(100);

  // Scan for networks
  int n = WiFi.scanNetworks();
  LP_LOGF("ESP-NOW scan found %d networks\n", n);

  if (n <= 0) {
    LP_LOGLN("No networks found");
  } else {
  // Add devices with open/no security as potential ESP-NOW peers
  for (int i = 0; i < n && peerCount < MAX_PEERS; i++) {
    // Check if this is a no security network
    if (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) {
      // Get BSSID (MAC address) of this network
      memcpy(knownPeers[peerCount].peer_addr, WiFi.BSSID(i), 6);
      knownPeers[peerCount].channel = WiFi.channel(i);
      knownPeers[peerCount].encrypt = false;

      peerCount++;
    }
  }
  }

  // Reconnect to WiFi if needed
  if (!apMode) {
    WiFi.begin(savedSSID.c_str(), savedPassword.c_str());
  }
}

// Broadcast discovery request to find ESP-NOW peers
void broadcastDiscoveryRequest() {
  DiscoveryRequest request;
  request.header.messageType = DISCOVERY_REQUEST;
  WiFi.macAddress(request.header.deviceId);
  request.channel = WiFi.channel();

  // Add broadcast peer if not already added
  if (!esp_now_is_peer_exist(broadcastMAC)) {
    addPeer(broadcastMAC, 0);
  }

  // Send discovery request
  esp_err_t result = esp_now_send(broadcastMAC, (uint8_t*)&request, sizeof(request));
  if (result == ESP_OK) {
    LP_LOGLN("Discovery request broadcasted");
    discoveryActive = true;
    discoveryStartTime = millis();
  } else {
    LP_LOGF("Failed to broadcast discovery request: %d\n", result);
  }
}

// Start ESP-NOW peer discovery using broadcast
void scanForPeers() {
  peerCount = 0;
  LP_LOGLN("Starting ESP-NOW broadcast discovery...");
  
  broadcastDiscoveryRequest();
  
  // Wait for replies
  while (discoveryActive && (millis() - discoveryStartTime < DISCOVERY_TIMEOUT_MS)) {
    delay(100);
  }
  
  discoveryActive = false;
  LP_LOGF("Discovery completed. Found %d ESP-NOW peers\n", peerCount);
}

// Handle received light message
void handleReceivedLight(const LightMessage* lightMsg) {
  LP_LOGF("Received light message for port %d, light idx %d\n", lightMsg->portId, lightMsg->lightIdx);
  
  // Find the target port by ID
  Port* port = Port::findById(lightMsg->portId);
  if (port == nullptr) {
    LP_LOGF("Port %d not found\n", lightMsg->portId);
    return;
  }
  
  // Check if it's an internal port (only internal ports should receive lights)
  if (port->isExternal()) {
    LP_LOGF("Port %d is an external port, cannot receive lights\n", lightMsg->portId);
    return;
  }
  InternalPort* targetPort = static_cast<InternalPort*>(port);
  
  // Create a new light with received data
  // Note: This requires access to the light creation system
  // Implementation depends on how lights are managed in the main application
  LP_LOGF("Light received: pos=%.2f, speed=%.2f, RGB=(%d,%d,%d), brightness=%d\n", 
          lightMsg->speed, 
          lightMsg->colorR, lightMsg->colorG, lightMsg->colorB, 
          lightMsg->brightness);
  
  // Create a new light with received properties
  LightList* lightList = state->findListById(lightListIdMap[lightMsg->listId]);
  RuntimeLight* light = lightList->addLightFromMsg(lightMsg);
  
  // Set color
  ColorRGB color;
  color.r = lightMsg->colorR;
  color.g = lightMsg->colorG;
  color.b = lightMsg->colorB;
  light->setColor(color);
  
  // Add light to the target port's connection
  targetPort->sendOut(light);
}

// Handle received light list message
void handleReceivedLightList(const LightListMessage* lightListMsg) {
  Port* port = Port::findById(lightListMsg->light.portId);
  if (port == nullptr) {
    LP_LOGF("Port %d not found\n", lightListMsg->light.portId);
    return;
  }
  
  // Check if it's an internal port (only internal ports should receive lights)
  if (port->isExternal()) {
    LP_LOGF("Port %d is an external port, cannot receive lights\n", lightListMsg->light.portId);
    return;
  }
  InternalPort* targetPort = static_cast<InternalPort*>(port);
  
  // Create a new LightList to hold the received lights
  LightList* receivedLightList = new LightList();
  //receivedLightList->setup(lightListMsg->numLights, 255);
  
  // Map the old LightList ID to the new one
  uint16_t oldId = lightListMsg->id;
  uint16_t newId = receivedLightList->id;
  lightListIdMap[oldId] = newId;
  LP_LOGF("Mapping LightList ID: old=%d -> new=%d\n", oldId, newId);
  
  RuntimeLight* light = receivedLightList->addLightFromMsg(&lightListMsg->light);
  
  // Add main light to the target port's connection
  targetPort->sendOut(light);
}

// Callback function for when data is sent
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

  if (status == ESP_NOW_SEND_SUCCESS) {
    LP_LOGF("ESP-NOW data sent to %s: SUCCESS\n", macStr);
  } else {
    LP_LOGF("ESP-NOW data sent to %s: FAILED\n", macStr);
  }
}

// Callback function for when data is received
#if defined(ESP_IDF_VERSION) && (ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0))
void onDataReceived(const esp_now_recv_info_t *esp_now_info, const uint8_t *data, int data_len) {
  const uint8_t *src_addr = esp_now_info->src_addr;
#else
void onDataReceived(const uint8_t *src_addr, const uint8_t *data, int data_len) {
#endif
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           src_addr[0], src_addr[1], src_addr[2],
           src_addr[3], src_addr[4], src_addr[5]);

  LP_LOGF("ESP-NOW data received from %s, length: %d\n", macStr, data_len);

  if (data_len < sizeof(ESPNowHeader)) {
    LP_LOGLN("ESP-NOW received data too short");
    return;
  }

  const ESPNowHeader* header = (const ESPNowHeader*)data;

  switch (header->messageType) {
    case DISCOVERY_REQUEST: {
      LP_LOGF("Received discovery request from %s\n", macStr);
      
      // Send reply
      DiscoveryReply reply;
      reply.header.messageType = DISCOVERY_REPLY;
      WiFi.macAddress(reply.header.deviceId);
      reply.channel = WiFi.channel();
      strncpy(reply.deviceName, "ESP32_Device", sizeof(reply.deviceName) - 1);
      reply.deviceName[sizeof(reply.deviceName) - 1] = '\0';
      
      // Add sender as peer and reply
      if (addPeer(src_addr, WiFi.channel())) {
        esp_now_send(src_addr, (uint8_t*)&reply, sizeof(reply));
        LP_LOGF("Sent discovery reply to %s\n", macStr);
      }
      break;
    }
    
    case DISCOVERY_REPLY: {
      if (data_len >= sizeof(DiscoveryReply)) {
        const DiscoveryReply* reply = (const DiscoveryReply*)data;
        LP_LOGF("Received discovery reply from %s (device: %s, channel: %d)\n", 
                macStr, reply->deviceName, reply->channel);
        
        // Add peer to known peers list if not already there
        bool peerExists = false;
        for (int i = 0; i < peerCount; i++) {
          if (memcmp(knownPeers[i].peer_addr, src_addr, 6) == 0) {
            peerExists = true;
            break;
          }
        }
        
        if (!peerExists && peerCount < MAX_PEERS) {
          memcpy(knownPeers[peerCount].peer_addr, src_addr, 6);
          knownPeers[peerCount].channel = reply->channel;
          knownPeers[peerCount].encrypt = false;
          
          // Also add as ESP-NOW peer for communication
          addPeer(src_addr, reply->channel);
          
          peerCount++;
          LP_LOGF("Added peer %s to known peers list\n", macStr);
        }
      }
      break;
    }
    
    case LIGHT_MESSAGE: {
      if (data_len >= sizeof(LightMessage)) {
        const LightMessage* lightMsg = (const LightMessage*)data;
        LP_LOGF("Received light message from %s\n", macStr);
        handleReceivedLight(lightMsg);
      } else {
        LP_LOGLN("Light message too short");
      }
      break;
    }
    
    case LIGHTLIST_MESSAGE: {
      if (data_len >= sizeof(LightListMessage)) {
        const LightListMessage* lightListMsg = (const LightListMessage*)data;
        LP_LOGF("Received light list message from %s\n", macStr);
        handleReceivedLightList(lightListMsg);
      } else {
        LP_LOGLN("Light list message too short");
      }
      break;
    }

    default:
      LP_LOGF("Unknown ESP-NOW message type: %d\n", header->messageType);
      break;
    }
}

// Initialize ESP-NOW
bool initESPNow() {
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    LP_LOGLN("Error initializing ESP-NOW");
    return false;
  }

  // Register callbacks
  esp_now_register_send_cb(onDataSent);
  esp_now_register_recv_cb(onDataReceived);

  LP_LOGLN("ESP-NOW initialized successfully");
  return true;
}

LightMessage packLight(uint8_t portId, RuntimeLight* const light) {
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

void sendLightViaESPNow_impl(const uint8_t* mac, uint8_t portId, RuntimeLight* const light, bool sendList = false) {
    if (sendList && light->list) {
        LightListMessage msg;
        msg.messageType = LIGHTLIST_MESSAGE;
        msg.id = light->list->id;
        msg.light = packLight(portId, light);
        
        esp_err_t result = esp_now_send(mac, (uint8_t*)&msg, sizeof(msg));
        if (result != ESP_OK) {
            LP_LOGF("Failed to send LightList message: %d\n", result);
        }
    } else {
        LightMessage msg = packLight(portId, light);
        esp_err_t result = esp_now_send(mac, (uint8_t*)&msg, sizeof(msg));
        if (result != ESP_OK) {
            LP_LOGF("Failed to send Light message: %d\n", result);
        }
    }
}

// Forward declaration of function pointer from Port.h
extern void (*sendLightViaESPNow)(const uint8_t* mac, uint8_t id, RuntimeLight* const light, bool sendList);

// Auto-assign the implementation when this header is included
static void __attribute__((constructor)) assignESPNowFunction() {
    sendLightViaESPNow = sendLightViaESPNow_impl;
}
