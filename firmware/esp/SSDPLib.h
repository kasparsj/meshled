#pragma once

// ESP32SSDP Library
// This implements the Simple Service Discovery Protocol for WLED app discovery

#include <WiFiUdp.h>

// SSDP constants
#define SSDP_PORT 1900
#define SSDP_MULTICAST_ADDR "239.255.255.250"
#define SSDP_UUID_PREFIX "38323636-4558-4dda-9188-cda0e6"
#define SSDP_UUID_SUFFIX "00000"
#define SSDP_MAX_PACKET_SIZE 512

// SSDP packet buffer
char packetBuffer[SSDP_MAX_PACKET_SIZE];

// UDP instance for SSDP
WiFiUDP ssdpUDP;

// Generate a UUID for the SSDP service
String generateUUID() {
  // Generate a UUID based on MAC address
  uint8_t mac[6];
  WiFi.macAddress(mac);
  
  // Format: "38323636-4558-4dda-9188-cda0e6" + MAC last 6 characters
  char uuidBuffer[48];
  sprintf(uuidBuffer, "%s%02x%02x%s", 
          SSDP_UUID_PREFIX, 
          mac[4], mac[5],
          SSDP_UUID_SUFFIX);
  
  return String(uuidBuffer);
}

// Initialize SSDP service
void setupSSDPService() {
  if (!wifiConnected) return;
  
  ssdpUDP.begin(1900); // for unicast M-SEARCH requests
  // Begin listening for UDP packets on SSDP port
  if (ssdpUDP.beginMulticast(IPAddress(239, 255, 255, 250), SSDP_PORT)) {
    LP_LOGLN("SSDP service started");
  } else {
    LP_LOGLN("Failed to start SSDP service");
  }
}

void sendSSDPNotify() {
  char notifyPacket[512];
  String uuid = generateUUID();

  sprintf(notifyPacket,
    "NOTIFY * HTTP/1.1\r\n"
    "HOST: 239.255.255.250:1900\r\n"
    "CACHE-CONTROL: max-age=60\r\n"
    "LOCATION: http://%s/description.xml\r\n"
    "NT: urn:schemas-upnp-org:device:wled:1\r\n"
    "NTS: ssdp:alive\r\n"
    "SERVER: Arduino/1.0 UPnP/1.1 HomoDeus/1.0\r\n"
    "USN: uuid:%s::urn:schemas-upnp-org:device:wled:1\r\n"
    "\r\n",
    WiFi.localIP().toString().c_str(),
    uuid.c_str()
  );

  ssdpUDP.beginPacket(IPAddress(239, 255, 255, 250), SSDP_PORT);
  ssdpUDP.write((uint8_t*)notifyPacket, strlen(notifyPacket));
  ssdpUDP.endPacket();
}

// Handle SSDP discovery requests
void handleSSDPDiscovery() {
  if (!wifiConnected) return;
  
  // Check if there's a packet available
  int packetSize = ssdpUDP.parsePacket();
  
  if (packetSize) {
    // Read the packet into the buffer
    int len = ssdpUDP.read(packetBuffer, SSDP_MAX_PACKET_SIZE);
    if (len > 0) {
      packetBuffer[len] = 0; // Null-terminate the received data
      
      // Convert to String for easier parsing
      String request = String(packetBuffer);
      
      // Check if this is a M-SEARCH request
      if (request.indexOf("M-SEARCH") >= 0) {
        // Check if this is searching for all devices or specifically Philips Hue
        bool respondAsWLED = request.indexOf("ST: ssdp:all") >= 0 || 
                             request.indexOf("ST: upnp:rootdevice") >= 0 ||
                             request.indexOf("ST: urn:schemas-upnp-org:device:basic:1") >= 0;
                             
        bool respondAsHue = request.indexOf("ST: ssdp:all") >= 0 || 
                           request.indexOf("ST: upnp:rootdevice") >= 0 || 
                           request.indexOf("ST: urn:schemas-upnp-org:device:basic:1") >= 0 ||
                           request.indexOf("ST: urn:schemas-upnp-org:device:lightdevices:1") >= 0;
        
        if (respondAsWLED || respondAsHue) {
          // Generate UUID
          String uuid = generateUUID();
          
          // Get the remote address and port
          IPAddress remoteIP = ssdpUDP.remoteIP();
          uint16_t remotePort = ssdpUDP.remotePort();
          
          // Prepare response buffer
          char responseBuffer[512];
          
          // Build WLED response
          if (respondAsWLED) {
            sprintf(responseBuffer, 
              "HTTP/1.1 200 OK\r\n"
              "EXT:\r\n"
              "CACHE-CONTROL: max-age=60\r\n"
              "LOCATION: http://%s/win\r\n"
              "SERVER: WLED 0.15.0\r\n"
              "ST: urn:schemas-upnp-org:device:wled:1\r\n"
              "USN: uuid:%s::urn:schemas-upnp-org:device:wled:1\r\n"
              "\r\n",
              WiFi.localIP().toString().c_str(),
              uuid.c_str()
            );
            
            // Send WLED response
            ssdpUDP.beginPacket(remoteIP, remotePort);
            ssdpUDP.write((uint8_t*)responseBuffer, strlen(responseBuffer));
            ssdpUDP.endPacket();
          }
          
          // Build Hue Bridge response (for Alexa/WLED app compatibility)
          if (respondAsHue) {
            sprintf(responseBuffer, 
              "HTTP/1.1 200 OK\r\n"
              "EXT:\r\n"
              "CACHE-CONTROL: max-age=60\r\n"
              "LOCATION: http://%s/description.xml\r\n"
              "SERVER: Linux/3.14.0 UPnP/1.0 IpBridge/1.24.0\r\n"
              "hue-bridgeid: %02X%02X%02X%02X%02X%02X\r\n"
              "ST: urn:schemas-upnp-org:device:basic:1\r\n"
              "USN: uuid:%s::upnp:rootdevice\r\n"
              "\r\n",
              WiFi.localIP().toString().c_str(),
              WiFi.macAddress()[0], WiFi.macAddress()[1], WiFi.macAddress()[2], 
              WiFi.macAddress()[3], WiFi.macAddress()[4], WiFi.macAddress()[5],
              uuid.c_str()
            );
            
            // Send Hue response
            ssdpUDP.beginPacket(remoteIP, remotePort);
            ssdpUDP.write((uint8_t*)responseBuffer, strlen(responseBuffer));
            ssdpUDP.endPacket();
          }
          
          LP_LOGLN("Sent SSDP discovery response");
        }
      }
    }
  }

  static unsigned long lastSSDPNotify = 0;
  if (millis() - lastSSDPNotify > 60000) {
    sendSSDPNotify();
    lastSSDPNotify = millis();
  }
}

// Simple XML description for WLED/Hue
void handleDescriptionXML() {
  WiFiClient client = server.client();
  
  // Check if the client is valid
  if (!client) {
    server.send(500, "text/plain", "Client connection error");
    return;
  }

  // Get MAC address
  uint8_t mac[6];
  WiFi.macAddress(mac);
  
  // Generate UUID
  String uuid = generateUUID();
  
  // Create bridgeid from MAC
  char bridgeID[13];
  sprintf(bridgeID, "%02X%02X%02X%02X%02X%02X", 
          mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  
  // Send HTTP headers
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/xml");
  client.println("Connection: close");
  client.println();

  // Stream content directly
  client.println("<?xml version=\"1.0\" ?>");
  client.println("<root xmlns=\"urn:schemas-upnp-org:device-1-0\">");
  client.println("<specVersion><major>1</major><minor>0</minor></specVersion>");
  
  // Device info
  client.println("<device>");
  client.print("<deviceType>urn:schemas-upnp-org:device:Basic:1</deviceType>");
  client.print("<friendlyName>"); 
  client.print(deviceHostname); 
  client.println("</friendlyName>");
  client.println("<manufacturer>HomoDeus</manufacturer>");
  client.println("<manufacturerURL>https://github.com/kasparsj</manufacturerURL>");
  client.println("<modelDescription>WS2812 LED Controller</modelDescription>");
  client.println("<modelName>HomoDeus</modelName>");
  client.println("<modelNumber>1.0</modelNumber>");
  client.println("<modelURL>https://github.com/kasparsj/homo_deus</modelURL>");
  client.print("<serialNumber>"); 
  client.print(bridgeID); 
  client.println("</serialNumber>");
  client.print("<UDN>uuid:"); 
  client.print(uuid); 
  client.println("</UDN>");
  
  // Present as both WLED device and Hue Bridge for maximum compatibility
  client.println("<presentationURL>/</presentationURL>");
  
  // For WLED app compatibility
  client.println("<iconList>");
  client.println("<icon>");
  client.println("<mimetype>image/png</mimetype>");
  client.println("<height>48</height>");
  client.println("<width>48</width>");
  client.println("<depth>24</depth>");
  client.println("<url>icon48.png</url>");
  client.println("</icon>");
  client.println("</iconList>");
  
  client.println("</device>");
  client.println("</root>");
  
  // Allow time for data to be sent
  delay(1);
}