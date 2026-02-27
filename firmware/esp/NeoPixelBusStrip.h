#pragma once

#if defined(ARDUINO_ARCH_ESP32)
#include <esp_arduino_version.h>
#if defined(ESP_ARDUINO_VERSION) && (ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 3, 0)) && !defined(gpio_hal_iomux_func_sel)
// NeoPixelBus 2.8.4 still references this removed HAL helper in the S3 LCD-X header.
// Provide the old macro shape so header parsing succeeds on Arduino-ESP32 3.3.x.
#define gpio_hal_iomux_func_sel(reg, func) PIN_FUNC_SELECT((reg), (func))
#endif
#endif

#include <NeoPixelBus.h>

#define USE_RGBW_COLOR true

class NeoPixelBusStrip {
public:
  virtual void Begin() = 0;
  virtual void SetPixelColor(uint16_t index, RgbColor color) {};
  virtual void SetPixelColor(uint16_t index, RgbwColor color) {};
  virtual void Show() = 0;
  virtual bool SupportsRgbw() const = 0;
  virtual ~NeoPixelBusStrip() {}
};

// RGB implementation for NeoRgbFeature
template<typename T_METHOD>
class NeoPixelBusRgbStrip : public NeoPixelBusStrip {
public:
  NeoPixelBusRgbStrip(uint16_t count, uint8_t pin)
    : strip(count, pin) {}

  void Begin() override {
    strip.Begin();
  }

  virtual void SetPixelColor(uint16_t index, RgbColor color) {
    strip.SetPixelColor(index, color);
  };

  void Show() override {
    strip.Show();
  }

  bool SupportsRgbw() const override {
    return false;
  }

private:
  NeoPixelBus<NeoRgbFeature, T_METHOD> strip;
};

// RGB implementation for NeoGrbFeature
template<typename T_METHOD>
class NeoPixelBusGrbStrip : public NeoPixelBusStrip {
public:
  NeoPixelBusGrbStrip(uint16_t count, uint8_t pin)
    : strip(count, pin) {}

  void Begin() override {
    strip.Begin();
  }

  virtual void SetPixelColor(uint16_t index, RgbColor color) {
    strip.SetPixelColor(index, color);
  };

  void Show() override {
    strip.Show();
  }

  bool SupportsRgbw() const override {
    return false;
  }

private:
  NeoPixelBus<NeoGrbFeature, T_METHOD> strip;
};

// RGBW implementation for NeoRgbwFeature
template<typename T_METHOD>
class NeoPixelBusRgbwStrip : public NeoPixelBusStrip {
public:
  NeoPixelBusRgbwStrip(uint16_t count, uint8_t pin)
    : strip(count, pin) {}

  void Begin() override {
    strip.Begin();
  }

  virtual void SetPixelColor(uint16_t index, RgbColor color) {
    strip.SetPixelColor(index, color);
  };

  void SetPixelColor(uint16_t index, RgbwColor color) override {
    strip.SetPixelColor(index, color);
  }

  void Show() override {
    strip.Show();
  }

  bool SupportsRgbw() const override {
    return USE_RGBW_COLOR;
  }

private:
  NeoPixelBus<NeoRgbwFeature, T_METHOD> strip;
};

// RGBW implementation for NeoGrbwFeature
template<typename T_METHOD>
class NeoPixelBusGrbwStrip : public NeoPixelBusStrip {
public:
  NeoPixelBusGrbwStrip(uint16_t count, uint8_t pin)
    : strip(count, pin) {}

  void Begin() override {
    strip.Begin();
  }

  virtual void SetPixelColor(uint16_t index, RgbColor color) {
    strip.SetPixelColor(index, color);
  };

  void SetPixelColor(uint16_t index, RgbwColor color) override {
    strip.SetPixelColor(index, color);
  }

  void Show() override {
    strip.Show();
  }

  bool SupportsRgbw() const override {
    return USE_RGBW_COLOR;
  }

private:
  NeoPixelBus<NeoGrbwFeature, T_METHOD> strip;
};
