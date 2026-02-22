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

#include "src/objects/Heptagon919.h"
#include "src/objects/Heptagon3024.h"
#include "src/objects/Line.h"
#include "src/objects/Triangle.h"
#include "src/Globals.h"
#include "src/LightList.h"
#include "src/LPRandom.h"
