#pragma once

#define LIB_NEOPIXELBUS 0
#define LIB_FASTLED 1

#define LED_WS2812 0
#define LED_WS2811 1
#define LED_WS2815 2
#define LED_WS2813 3
#define LED_WS2816 4
#define LED_SK6812 5
#define LED_TM1829 6
#define LED_APA106 7
#define LED_WS2814 8
#define LED_TM1814 9
#define LED_TM1914 10
#define LED_TM1812 11
#define LED_TM1809 12
#define LED_TM1804 13
#define LED_TM1803 14
#define LED_UCS1903 15
#define LED_UCS1903B 16
#define LED_UCS1904 17
#define LED_UCS2903 18
#define LED_SK6822 19
#define LED_APA104 20
#define LED_WS2811_400 21
#define LED_GS1903 22
#define LED_GW6205 23
#define LED_GW6205_400 24
#define LED_LPD1886 25
#define LED_LPD1886_8BIT 26
#define LED_PL9823 27
#define LED_UCS1912 28
#define LED_SM16703 29
#define LED_SM16824E 30

//#define SC_HOST "192.168.43.101"
//#define SC_PORT 57120
//#define LP_OSC_REPLY(I) OscWiFi.publish(SC_HOST, SC_PORT, "/emit", (I));

#define OBJ_HEPTAGON919 0
#define OBJ_LINE 1
#define OBJ_TRIANGLE 2
#define OBJ_HEPTAGON3024 3
#define OBJ_CUSTOM 4

#include "ColorOrder.h"
#include "src/objects/Heptagon919.h"
#include "src/objects/Heptagon3024.h"
#include "src/objects/Line.h"
#include "src/objects/Triangle.h"
#include "src/rendering/Palette.h"
#include "src/rendering/Palettes.h"
#include "src/Globals.h"
#include "src/runtime/Behaviour.h"
#include "src/runtime/EmitParams.h"
#include "src/runtime/LightList.h"
#include "src/runtime/State.h"
#include "src/topology/TopologyObject.h"
#include "src/core/Platform.h"
