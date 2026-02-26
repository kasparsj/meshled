#pragma once

// mDNS service implementation for WLED compatibility
// This allows discovery of the device via mDNS with standard WLED service names

#include <ESPmDNS.h>
#include <esp_arduino_version.h>
#include <vector>

#define MDNS_XLED_SERVICE "_xled"
#define MDNS_WLED_SERVICE "_wled"
#define MDNS_HTTP_SERVICE "_http"
#define MDNS_TCP "_tcp"
#define MDNS_UDP "_udp"

// Initialize mDNS with WLED compatibility
void setupMDNSService() {
  if (!wifiConnected) return;
  
  // Initialize mDNS service
  if (MDNS.begin(deviceHostname.c_str())) {
    LP_LOGLN("mDNS service started with hostname: " + deviceHostname);
    
    MDNS.addService(MDNS_XLED_SERVICE, MDNS_TCP, 80);
    MDNS.addServiceTxt(MDNS_XLED_SERVICE, MDNS_TCP, "mac", WiFi.macAddress());
    MDNS.addServiceTxt(MDNS_XLED_SERVICE, MDNS_TCP, "ip", WiFi.localIP().toString());
    MDNS.addServiceTxt(MDNS_XLED_SERVICE, MDNS_TCP, "name", deviceHostname);
    MDNS.addServiceTxt(MDNS_XLED_SERVICE, MDNS_TCP, "type", "DIY");
    MDNS.addServiceTxt(MDNS_XLED_SERVICE, MDNS_TCP, "lights", String(pixelCount1 + pixelCount2));
    MDNS.addServiceTxt(MDNS_XLED_SERVICE, MDNS_TCP, "version", "1.0.0");

    MDNS.addService(MDNS_WLED_SERVICE, MDNS_TCP, 80);
    MDNS.addServiceTxt(MDNS_WLED_SERVICE, MDNS_TCP, "mdns", "true");
    MDNS.addServiceTxt(MDNS_WLED_SERVICE, MDNS_TCP, "mac", WiFi.macAddress());
    MDNS.addServiceTxt(MDNS_WLED_SERVICE, MDNS_TCP, "ip", WiFi.localIP().toString());
    MDNS.addServiceTxt(MDNS_WLED_SERVICE, MDNS_TCP, "name", deviceHostname);
    MDNS.addServiceTxt(MDNS_WLED_SERVICE, MDNS_TCP, "type", "DIY");
    MDNS.addServiceTxt(MDNS_WLED_SERVICE, MDNS_TCP, "lights", String(pixelCount1 + pixelCount2));
    MDNS.addServiceTxt(MDNS_WLED_SERVICE, MDNS_TCP, "version", "1.0.0");
    
    MDNS.addService(MDNS_HTTP_SERVICE, MDNS_TCP, 80);
    MDNS.addServiceTxt(MDNS_HTTP_SERVICE, MDNS_TCP, "mac", WiFi.macAddress());
    MDNS.addServiceTxt(MDNS_HTTP_SERVICE, MDNS_TCP, "ip", WiFi.localIP().toString());
    MDNS.addServiceTxt(MDNS_HTTP_SERVICE, MDNS_TCP, "name", deviceHostname);
    MDNS.addServiceTxt(MDNS_HTTP_SERVICE, MDNS_TCP, "type", "DIY");
    MDNS.addServiceTxt(MDNS_HTTP_SERVICE, MDNS_TCP, "lights", String(pixelCount1 + pixelCount2));
    MDNS.addServiceTxt(MDNS_HTTP_SERVICE, MDNS_TCP, "version", "1.0.0");
    
    LP_LOGLN("mDNS services added: _wled._tcp and _http._tcp");
  } else {
    LP_LOGLN("Error setting up mDNS service");
  }
}

// Check if hostname is already taken with ".local" and suggest an alternative
// This is useful for multiple WLED devices on the same network
String checkAndAdjustHostname(String hostname) {
  if (!wifiConnected) return hostname;
  
  // Check if hostname is already in use via mDNS query
  IPAddress existingIP = MDNS.queryHost(hostname);
  
  // If the hostname is not in use or resolves to our own IP, it's fine
  if (existingIP == INADDR_NONE || existingIP == WiFi.localIP()) {
    return hostname;
  }
  
  // If the hostname is taken, append a number
  LP_LOGLN("Hostname " + hostname + " is already in use by " + existingIP.toString());
  
  // Use last part of MAC address to make unique
  String macSuffix = WiFi.macAddress().substring(12, 14) + 
                     WiFi.macAddress().substring(15, 17);
  macSuffix.toLowerCase();
  
  String newHostname = hostname + "-" + macSuffix;
  LP_LOGLN("Using alternative hostname: " + newHostname);
  
  return newHostname;
}

std::vector<IPAddress> discoverMDNSDevices(String service, String proto) {
  std::vector<IPAddress> deviceIPs;

  int n = MDNS.queryService(service, proto);
  LP_LOGF("Found %d devices on network\n", n);
  
  for (int i = 0; i < n; i++) {
    String hostname = MDNS.hostname(i);
#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    IPAddress ip = MDNS.address(i);
#else
    IPAddress ip = MDNS.IP(i);
#endif
    LP_LOGF("Found device: %s at %s\n", hostname.c_str(), ip.toString().c_str());

    deviceIPs.push_back(ip);
  }

  return deviceIPs;
}
