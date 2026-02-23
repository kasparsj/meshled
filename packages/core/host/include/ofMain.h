#pragma once

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <cmath>
#include <string>

enum {
    OF_LOG_WARNING = 0
};

using std::max;
using std::min;

#ifndef MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif

#ifndef MAX
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#endif

inline float ofRandom(float max) {
    if (max <= 0.0f) {
        return 0.0f;
    }
    const float unit = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
    return unit * max;
}

inline float ofRandom(float min, float max) {
    if (max <= min) {
        return min;
    }
    return min + ofRandom(max - min);
}

inline void ofLog(int level, const char* format, ...) {
    (void) level;
    va_list args;
    va_start(args, format);
    std::vfprintf(stderr, format, args);
    va_end(args);
}

inline void ofLog(int level, const std::string& message) {
    (void) level;
    std::fprintf(stderr, "%s", message.c_str());
}

inline void ofLogWarning(const char* format, ...) {
    va_list args;
    va_start(args, format);
    std::vfprintf(stderr, format, args);
    va_end(args);
    std::fprintf(stderr, "\n");
}

inline void ofLogWarning(const std::string& message) {
    std::fprintf(stderr, "%s\n", message.c_str());
}
