#pragma once

// Base components
#define CO_R 0
#define CO_G 1
#define CO_B 2
#define CO_W 3

// Flag for RGBW orders
#define FLAG_RGBW (1 << 7)

// Encoders
#define CO_3(r, g, b)     (((r) << 4) | ((g) << 2) | (b))                           // 6-bit RGB
#define CO_4(r, g, b, w)  (FLAG_RGBW | ((r) << 6) | ((g) << 4) | ((b) << 2) | (w))  // 8-bit RGBW

// Standard 3-component orders
#define CO_RGB  CO_3(CO_R, CO_G, CO_B)
#define CO_GRB  CO_3(CO_G, CO_R, CO_B)
#define CO_BRG  CO_3(CO_B, CO_R, CO_G)
#define CO_RBG  CO_3(CO_R, CO_B, CO_G)
#define CO_BGR  CO_3(CO_B, CO_G, CO_R)
#define CO_GBR  CO_3(CO_G, CO_B, CO_R)

// 4-component RGBW variants
#define CO_RGBW CO_4(CO_R, CO_G, CO_B, CO_W)
#define CO_GRBW CO_4(CO_G, CO_R, CO_B, CO_W)

// Helper: Extract top 6 bits (RGB) from any CO_x value
#define RGB_MASK 0b111111
#define EXTRACT_RGB(order) (((order) & FLAG_RGBW) ? (((order) >> 2) & RGB_MASK) : (order))

// Matchers that work for both RGB and RGBW formats
#define IS_RGB(order) (EXTRACT_RGB(order) == CO_RGB)
#define IS_GRB(order) (EXTRACT_RGB(order) == CO_GRB)
#define IS_BRG(order) (EXTRACT_RGB(order) == CO_BRG)
#define IS_RBG(order) (EXTRACT_RGB(order) == CO_RBG)
#define IS_BGR(order) (EXTRACT_RGB(order) == CO_BGR)
#define IS_GBR(order) (EXTRACT_RGB(order) == CO_GBR)

// New: check if order includes white component
#define HAS_WHITE(order) (((order) & FLAG_RGBW) != 0)