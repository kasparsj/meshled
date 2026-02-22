#pragma once

#ifdef FASTLED_ENABLED
#include <FastLED.h>
CRGB* leds1;
CRGB* leds2;
#include "FastLEDLib.h"
#endif

#ifdef NEOPIXELBUS_ENABLED
#include "NeoPixelBusStrip.h"
// These are just declarations - actual initialization happens in setupNeoPixelBus()
NeoPixelBusStrip* strip1 = NULL;
NeoPixelBusStrip* strip2 = NULL;
#include "NeoPixelBusLib.h"
#endif

void setupLEDs() {
  if (objectType == OBJ_HEPTAGON919) {
    pixelCount1 = HEPTAGON919_PIXEL_COUNT1;
    pixelCount2 = HEPTAGON919_PIXEL_COUNT2;
    pixelPin1 = 14;
    pixelPin2 = 26;
  }
  else if (objectType == OBJ_HEPTAGON3024) {
    pixelCount1 = HEPTAGON3024_REAL_PIXEL_COUNT1;
    pixelCount2 = HEPTAGON3024_REAL_PIXEL_COUNT2;
    pixelPin1 = 14;
    pixelPin2 = 26;
  }

  #ifdef NEOPIXELBUS_ENABLED
  setupNeoPixelBus();
  #endif

  #ifdef FASTLED_ENABLED
  setupFastLED();
  #endif
}

void setupState() {
  if (objectType == OBJ_HEPTAGON919) {
    object = new Heptagon919();
  }
  else if (objectType == OBJ_HEPTAGON3024) {
    object = new Heptagon3024();
  }
  else if (objectType == OBJ_LINE) {
    object = new Line(pixelCount1 + pixelCount2);
  }
  else if (objectType == OBJ_TRIANGLE) {
    object = new Triangle(pixelCount1 + pixelCount2);
  }

  state = new State(*object);

  // Set auto emitter parameters
  State::autoParams.from = emitterFrom;

  // Set auto emitter enabled state
  state->autoEnabled = emitterEnabled;

  LP_LOGLN("State initialized");
}

void updateLEDs() {
  gMillis = millis();
  #ifdef DEBUGGER_ENABLED
  debugger->update(gMillis);
  #endif
  state->autoEmit(gMillis);
  state->update();
}

void drawLEDs() {
  #ifdef NEOPIXELBUS_ENABLED
  drawNeoPixelBus();
  #endif

  #ifdef FASTLED_ENABLED
  drawFastLED();
  #endif
}

void doEmit(EmitParams &params) {
  int8_t i = state->emit(params);
  #ifdef DEBUGGER_ENABLED
  debugger->countEmit();
  #endif
  // todo: I think HD_DEBUG does not exist anymore
  #ifdef HD_DEBUG
  LP_LOGF("emitting %d %s lights (%d/%.1f/%d/%d/%d/%d), total: %d (%d)\n",
    state->lightLists[i]->numLights,
    (params.linked ? "linked" : "random"),
    params.model,
    params.speed,
    state->lightLists[i]->length,
    state->lightLists[i]->lifeMillis,
    state->lightLists[i]->maxBri,
    params.fadeSpeed,
    state->totalLights + state->lightLists[i]->numLights,
    state->totalLightLists + 1);
  #endif
}

void doCommand(char command) {
  switch (command) {
    case 'r':
      ESP.restart();
      break;
    case '.':
      state->stopAll();
      break;
    case '!':
      state->colorAll();
      break;
    case 's':
      state->splitAll();
      break;
    case 'h':
      LP_LOGF("Free heap: %d\n", ESP.getFreeHeap());
      break;
    #ifdef DEBUGGER_ENABLED
    case 'f':
      LP_LOGF("FPS: %f (%d)\n", debugger->getFPS(), state->totalLights);
      break;
    case 'e':
      LP_LOGF("Emits per frame: %f\n", debugger->getNumEmits());
      break;
    case 'a':
      if (state && state->lightLists[0]) {
        state->lightLists[0]->visible = !state->lightLists[0]->visible;
      }
      #ifdef SPIFFS_ENABLED
      saveLayers();
      #endif
      break;
    case 'i':
      state->showIntersections = !state->showIntersections;
      break;
    case 'c':
      state->showConnections = !state->showConnections;
      break;
    case '>':
      if (state->currentPalette < 32)
      state->currentPalette++;
      break;
    case '<':
      if (state->currentPalette > 0) {
        state->currentPalette--;
      }
      break;
    case 'l':
      LP_LOGF("Total %d lights\n", state->totalLights);
      break;
    case 'E':
      state->autoEnabled = !state->autoEnabled;
      // Update the global emitterEnabled variable to match state->autoEnabled
      emitterEnabled = state->autoEnabled;
      LP_LOGF("AutoEmitter is %s\n", state->autoEnabled ? "enabled" : "disabled");
      #ifdef SPIFFS_ENABLED
      saveSettings();
      #endif
      break;
    case 'L':
      state->debug();
      break;
    case 'C':
      debugger->dumpConnections();
      break;
    case 'I':
      debugger->dumpIntersections();
      break;
    #endif
    default:
      EmitParams* params = object->getParams(command);
      if (params != NULL) {
        doEmit(*params);
      }
      delete params;
      break;
  }
}
