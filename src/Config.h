#pragma once

#include <stdint.h>

#if defined (ARDUINO)

#include "Arduino.h"
#ifdef LOG_FILE
#define LP_LOG(...) do { Serial.print(__VA_ARGS__); logMessage(__VA_ARGS__); } while(0)
#define LP_LOGF(...) do { Serial.printf(__VA_ARGS__); logMessageF(__VA_ARGS__); } while(0)
#define LP_LOGLN(...) do { Serial.println(__VA_ARGS__); logMessage(__VA_ARGS__); } while(0)
#else
#define LP_LOG(...) Serial.print(__VA_ARGS__)
#define LP_LOGF(...) Serial.printf(__VA_ARGS__)
#define LP_LOGLN(...) Serial.println(__VA_ARGS__)
#endif
#define LP_RANDOM(...) random(__VA_ARGS__)
#define LP_STRING String

#ifndef MIN
    #define MIN(x,y) (((x) < (y)) ? (x) : (y))
#endif

#ifndef MAX
    #define MAX(x,y) (((x) > (y)) ? (x) : (y))
#endif

#else
//#elif defined (OPENFRAMEWORKS)

#include "ofMain.h"
#define LP_LOG(...) ofLog(OF_LOG_WARNING, __VA_ARGS__)
#define LP_LOGF(...) ofLog(OF_LOG_WARNING, __VA_ARGS__)
#define LP_LOGLN ofLogWarning
#define LP_RANDOM ofRandom
#define LP_STRING std::string

#endif

#include "LPRandom.h"

struct ColorRGB {
    uint8_t R;
    uint8_t G;
    uint8_t B;
    
    // For compatibility with ColorUtil template interface
    uint8_t r;  // lowercase to match ofColor interface
    uint8_t g;
    uint8_t b;
    
    ColorRGB(uint8_t r, uint8_t g, uint8_t b) : R(r), G(g), B(b), r(r), g(g), b(b) {}
    ColorRGB(uint32_t rgb) {
        set(rgb);
    }
    ColorRGB() : ColorRGB(0) {}
    
    ColorRGB Dim(uint8_t ratio) const {
        // specifically avoids float math
        return ColorRGB(_elementDim(R, ratio), _elementDim(G, ratio), _elementDim(B, ratio));
    }
    
    uint32_t get() const {
        return (static_cast<uint32_t>(R) << 16) |
               (static_cast<uint32_t>(G) << 8) |
               static_cast<uint32_t>(B);
    }
    
    void set(uint32_t rgb) {
        R = (rgb >> 16) & 0xFF;
        G = (rgb >> 8) & 0xFF;
        B = rgb & 0xFF;
        // Keep lowercase copies in sync
        r = R;
        g = G;
        b = B;
    }
    
    void setRandom() {
        fromHSV(LPRandom::randomHue(), LPRandom::randomSaturation(), LPRandom::randomValue());
    }
    
    void fromHSV(uint8_t h, uint8_t s, uint8_t v) {
        float hf = (h / 255.0f) * 360.0f;
        float sf = s / 255.0f;
        float vf = v / 255.0f;

        float c = vf * sf;
        float x = c * (1 - std::fabs(std::fmod(hf / 60.0f, 2) - 1));
        float m = vf - c;

        float rf, gf, bf;

        if (hf < 60) {
            rf = c; gf = x; bf = 0;
        } else if (hf < 120) {
            rf = x; gf = c; bf = 0;
        } else if (hf < 180) {
            rf = 0; gf = c; bf = x;
        } else if (hf < 240) {
            rf = 0; gf = x; bf = c;
        } else if (hf < 300) {
            rf = x; gf = 0; bf = c;
        } else {
            rf = c; gf = 0; bf = x;
        }

        R = static_cast<uint8_t>((rf + m) * 255);
        G = static_cast<uint8_t>((gf + m) * 255);
        B = static_cast<uint8_t>((bf + m) * 255);
        // Keep lowercase copies in sync
        r = R;
        g = G;
        b = B;
    }
    
    inline static uint8_t _elementDim(uint8_t value, uint8_t ratio) {
        return (static_cast<uint16_t>(value) * (static_cast<uint16_t>(ratio) + 1)) >> 8;
    }
    
    // Methods required by ColorUtil templates
    
    // Gets the hue angle in degrees (0-360)
    float getHueAngle() const {
        float rf = R / 255.0f;
        float gf = G / 255.0f;
        float bf = B / 255.0f;
        
        float max = std::max(std::max(rf, gf), bf);
        float min = std::min(std::min(rf, gf), bf);
        float delta = max - min;
        
        if (delta == 0) {
            return 0;  // Undefined, return 0
        }
        
        float hue;
        if (max == rf) {
            hue = 60.0f * fmod(((gf - bf) / delta), 6.0f);
        } else if (max == gf) {
            hue = 60.0f * (((bf - rf) / delta) + 2.0f);
        } else {
            hue = 60.0f * (((rf - gf) / delta) + 4.0f);
        }
        
        if (hue < 0) hue += 360.0f;
        
        return hue;
    }
    
    // Gets the hue in 0-255 range
    float getHue() const {
        return getHueAngle() / 360.0f * 255.0f;
    }
    
    // Gets saturation in 0-255 range
    float getSaturation() const {
        float rf = R / 255.0f;
        float gf = G / 255.0f;
        float bf = B / 255.0f;
        
        float max = std::max(std::max(rf, gf), bf);
        float min = std::min(std::min(rf, gf), bf);
        
        if (max == 0) {
            return 0;
        }
        
        return ((max - min) / max) * 255.0f;
    }
    
    // Gets brightness in 0-255 range
    float getBrightness() const {
        return std::max(std::max(R, G), B);
    }
    
    // Set hue in 0-255 range
    void setHue(float hue) {
        float s = getSaturation() / 255.0f;
        float v = getBrightness() / 255.0f;
        
        // Convert to 0-360 range
        float h = hue / 255.0f * 360.0f;
        
        // Convert HSV to RGB
        if (s == 0) {
            // Achromatic (gray)
            R = G = B = v * 255;
            r = R; g = G; b = B;
            return;
        }
        
        h /= 60.0f;
        int i = floor(h);
        float f = h - i;
        float p = v * (1 - s);
        float q = v * (1 - s * f);
        float t = v * (1 - s * (1 - f));
        
        switch (i) {
            case 0:
                R = v * 255; G = t * 255; B = p * 255;
                break;
            case 1:
                R = q * 255; G = v * 255; B = p * 255;
                break;
            case 2:
                R = p * 255; G = v * 255; B = t * 255;
                break;
            case 3:
                R = p * 255; G = q * 255; B = v * 255;
                break;
            case 4:
                R = t * 255; G = p * 255; B = v * 255;
                break;
            default: // case 5:
                R = v * 255; G = p * 255; B = q * 255;
                break;
        }
        
        r = R; g = G; b = B;
    }
    
    // Set saturation in 0-255 range
    void setSaturation(float saturation) {
        float h = getHue();
        float b = getBrightness();
        float s = saturation;
        setHsb(h, s, b);
    }
    
    // Set brightness in 0-255 range
    void setBrightness(float brightness) {
        float h = getHue();
        float s = getSaturation();
        float b = brightness;
        setHsb(h, s, b);
    }
    
    // Set HSB values (all in 0-255 range)
    void setHsb(float hue, float saturation, float brightness) {
        float h = hue / 255.0f * 360.0f;
        float s = saturation / 255.0f;
        float v = brightness / 255.0f;
        
        if (s == 0) {
            // Achromatic (gray)
            R = G = B = v * 255;
            r = R; g = G; b = B;
            return;
        }
        
        h /= 60.0f;
        int i = floor(h);
        float f = h - i;
        float p = v * (1 - s);
        float q = v * (1 - s * f);
        float t = v * (1 - s * (1 - f));
        
        switch (i) {
            case 0:
                R = v * 255; G = t * 255; B = p * 255;
                break;
            case 1:
                R = q * 255; G = v * 255; B = p * 255;
                break;
            case 2:
                R = p * 255; G = v * 255; B = t * 255;
                break;
            case 3:
                R = p * 255; G = q * 255; B = v * 255;
                break;
            case 4:
                R = t * 255; G = p * 255; B = v * 255;
                break;
            default: // case 5:
                R = v * 255; G = p * 255; B = q * 255;
                break;
        }
        
        r = R; g = G; b = B;
    }
    
    // Static fromHsb method to create from HSB values
    static ColorRGB fromHsb(float hue, float saturation, float brightness) {
        ColorRGB color;
        color.setHsb(hue, saturation, brightness);
        return color;
    }
    
    // Linear interpolation between two colors
    ColorRGB lerp(const ColorRGB& target, float amt) const {
        float invamt = 1.0f - amt;
        return ColorRGB(
            static_cast<uint8_t>(R * invamt + target.R * amt),
            static_cast<uint8_t>(G * invamt + target.G * amt),
            static_cast<uint8_t>(B * invamt + target.B * amt)
        );
    }
    
    // Static method to get the color limit (255 for RGB)
    static float limit() {
        return 255.0f;
    }
};

enum Groups {
    GROUP1 = 1,
    GROUP2 = 2,
    GROUP3 = 4,
    GROUP4 = 8,
    GROUP5 = 16,
    GROUP6 = 32,
    GROUP7 = 64,
    GROUP8 = 128,
};

enum BehaviourFlags {
    B_POS_CHANGE_FADE = 1,
    B_BRI_CONST_NOISE = 2,
    B_RENDER_SEGMENT = 4,
    B_ALLOW_BOUNCE = 8,
    B_FORCE_BOUNCE = 16,
    B_EXPIRE_IMMEDIATE = 32,
    B_EMIT_FROM_CONN = 64,
    B_FILL_EASE = 128,
    B_RANDOM_COLOR = 256,
    B_MIRROR_FLIP = 512,
    B_MIRROR_ROTATE = 1024,
    B_SMOOTH_CHANGES = 2048,
};

enum ListOrder { 
    LIST_ORDER_SEQUENTIAL,
    LIST_ORDER_RANDOM,
    LIST_ORDER_NOISE,
    LIST_ORDER_OFFSET,
    LO_FIRST = LIST_ORDER_SEQUENTIAL,
    LO_LAST = LIST_ORDER_OFFSET,
};

enum ListHead {
    LIST_HEAD_FRONT,
    LIST_HEAD_MIDDLE,
    LIST_HEAD_BACK,
};

enum BlendMode {
    BLEND_NORMAL = 0,    // Just add color values
    BLEND_ADD = 1,       // Add without division
    BLEND_MULTIPLY = 2,  // Multiply colors together
    BLEND_SCREEN = 3,    // Inverse multiply (brightening)
    BLEND_OVERLAY = 4,   // Combination of multiply and screen
    BLEND_REPLACE = 5,   // Replace previous colors
    BLEND_SUBTRACT = 6,  // Subtract values (darkening)
    BLEND_DIFFERENCE = 7, // Absolute difference between colors
    BLEND_EXCLUSION = 8, // Similar to difference with lower contrast
    BLEND_DODGE = 9,     // Brightens base color based on blend color
    BLEND_BURN = 10,     // Darkens base color based on blend color
    BLEND_HARD_LIGHT = 11, // Similar to overlay but with blend color as base
    BLEND_SOFT_LIGHT = 12, // Softer version of hard light
    BLEND_LINEAR_LIGHT = 13, // Combination of dodge and burn
    BLEND_VIVID_LIGHT = 14, // More extreme version of linear light
    BLEND_PIN_LIGHT = 15    // Replaces colors based on blend color brightness
};

enum Ease {
    EASE_NONE = 0,
    EASE_LINEAR_IN = 1,
    EASE_LINEAR_OUT = 2,
    EASE_LINEAR_INOUT = 3,
    EASE_SINE_IN = 4,
    EASE_SINE_OUT = 5,
    EASE_SINE_INOUT = 6,
    EASE_CIRCULAR_IN = 7,
    EASE_CIRCULAR_OUT = 8,
    EASE_CIRCULAR_INOUT = 9,
    EASE_QUADRATIC_IN = 10,
    EASE_QUADRATIC_OUT = 11,
    EASE_QUADRATIC_INOUT = 12,
    EASE_CUBIC_IN = 13,
    EASE_CUBIC_OUT = 14,
    EASE_CUBIC_INOUT = 15,
    EASE_QUARTIC_IN = 16,
    EASE_QUARTIC_OUT = 17,
    EASE_QUARTIC_INOUT = 18,
    EASE_QUINTIC_IN = 19,
    EASE_QUINTIC_OUT = 20,
    EASE_QUINTIC_INOUT = 21,
    EASE_EXPONENTIAL_IN = 22,
    EASE_EXPONENTIAL_OUT = 23,
    EASE_EXPONENTIAL_INOUT = 24,
    EASE_BACK_IN = 25,
    EASE_BACK_OUT = 26,
    EASE_BACK_INOUT = 27,
    EASE_BOUNCE_IN = 28,
    EASE_BOUNCE_OUT = 29,
    EASE_BOUNCE_INOUT = 30,
    EASE_ELASTIC_IN = 31,
    EASE_ELASTIC_OUT = 32,
    EASE_ELASTIC_INOUT = 33,
};

#ifndef UINT32_MAX
#define UINT32_MAX 4294967295
#endif

#define MAX_GROUPS 5
#define MAX_LIGHT_LISTS 20
#define MAX_CONDITIONAL_WEIGHTS 10
#define MAX_TOTAL_LIGHTS 1500
#define MAX_NOTES_SET 7
#define CONNECTION_MAX_MULT 10
#define CONNECTION_MAX_LIGHTS 340
#define CONNECTION_MAX_LEDS 48
#define OUT_PORTS_MEMORY 3
//#define LP_OSC_REPLY(I) OscWiFi.publish(SC_HOST, SC_PORT, "/emit", (I));
#define RANDOM_MODEL -1
#define RANDOM_SPEED -1
#define RANDOM_DURATION 0
#define RANDOM_COLOR -1
#define INFINITE_DURATION 0x7FFFFFFF
#define DEFAULT_SPEED 1.0
#define FULL_BRIGHTNESS 255
