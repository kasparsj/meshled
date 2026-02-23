#pragma once

#define LIB_NEOPIXELBUS 0
#define LIB_FASTLED 1

#define LED_WS2812 0
#define LED_WS2811 1

//#define SC_HOST "192.168.43.101"
//#define SC_PORT 57120
//#define LP_OSC_REPLY(I) OscWiFi.publish(SC_HOST, SC_PORT, "/emit", (I));

#define OBJ_HEPTAGON919 0
#define OBJ_LINE 1
#define OBJ_TRIANGLE 2
#define OBJ_HEPTAGON3024 3

#include "ColorOrder.h"
#include <lightpath/integration.hpp>
#include "src/core/Platform.h"
