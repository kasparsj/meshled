#pragma once

#include <vector>

void parseParam(EmitParams &p, const OscMessage &m, EmitParam &param, uint8_t j) {
  switch (param) {
      case P_MODEL:
        p.model = m.arg<uint8_t>(j);
        break;
      case P_SPEED:
        p.speed = m.arg<float>(j);
        break;
      case P_EASE:
        p.ease = m.arg<uint8_t>(j);
        break;
      case P_LENGTH:
        p.setLength(m.arg<uint16_t>(j));
        break;
      case P_TRAIL:
        p.trail = m.arg<uint16_t>(j);
        break;
      case P_FADE:
        p.fadeSpeed = m.arg<uint8_t>(j);
        break;
      case P_FADE_THRESH:
        p.fadeThresh = m.arg<uint8_t>(j);
        break;
      case P_ORDER: {
        uint8_t order = m.arg<uint8_t>(j);
        if (order >= ListOrder::LO_FIRST && order <= ListOrder::LO_LAST) {
          p.order = static_cast<ListOrder>(order);
        }
        break;
      }
      case P_LINKED:
        p.linked = m.arg<uint8_t>(j) > 0;
        break;
      case P_FROM:
        p.from = m.arg<int8_t>(j);
        break;
      case P_DURATION_MS:
        p.duration = m.arg<uint32_t>(j);
        break;
      case P_DURATION_FRAMES:
        p.duration = m.arg<uint32_t>(j);
        break;
      case P_COLOR:
        p.setColors(m.arg<uint32_t>(j));
        break;
      case P_COLOR_INDEX: {
        int16_t index = m.arg<int16_t>(j);
        if (index < 0) {
          p.setColors(RANDOM_COLOR);
        }
        else {
          p.setColors(state->paletteColor(index).get());
        }
        break;
      }
      case P_NOTE_ID:
        p.noteId = m.arg<uint16_t>(j);
        break;
      case P_MIN_BRI:
        p.minBri = m.arg<uint8_t>(j);
        break;
      case P_MAX_BRI:
        p.maxBri = m.arg<uint8_t>(j);
        break;
      case P_BEHAVIOUR:
        p.behaviourFlags = m.arg<uint16_t>(j);
        break;
      case P_EMIT_GROUPS:
        p.emitGroups = m.arg<uint8_t>(j);
        break;
      case P_COLOR_CHANGE_GROUPS:
        p.colorChangeGroups = m.arg<uint8_t>(j);
        break;
    }
}

void parseParams(EmitParams &p, const OscMessage &m) {
  for (uint8_t i=0; i<m.size() / 2; i++) {
    EmitParam param = static_cast<EmitParam>(m.arg<uint8_t>(i*2));
    uint8_t j = i*2+1;
    parseParam(p, m, param, j);
  }
}

void onCommand(const OscMessage& m) {
  String command = m.arg<String>(0);
  for (uint8_t i=0; i<command.length(); i++) {
    doCommand(command.charAt(i));
  }
}

void onEmit(const OscMessage& m) {
  EmitParams params;
  parseParams(params, m);
  doEmit(params);
}

void onNoteOn(const OscMessage& m) {
  EmitParams params;
  params.duration = INFINITE_DURATION;
  parseParams(params, m);
  doEmit(params);
}

void onNoteOff(const OscMessage& m) {
  if (m.size() > 0) {
    uint16_t noteId = m.arg<uint16_t>(0);
    state->stopNote(noteId);
  }
  else {
    state->stopAll();
  }
}

void onNotesSet(const OscMessage& m) {
  EmitParams notesSet[MAX_NOTES_SET];
  for (uint8_t i=0; i<m.size() / 3; i++) {
    uint16_t noteId = m.arg<uint16_t>(i*3);
    uint8_t k = 0;
    for (k; k<MAX_NOTES_SET; k++) {
      if (notesSet[k].noteId == 0 || notesSet[k].noteId == noteId) {
        notesSet[k].noteId = noteId;
        break;
      }
    }
    EmitParam param = static_cast<EmitParam>(m.arg<uint8_t>(i*3+1));
    uint8_t j = i*3+2;
    parseParam(notesSet[k], m, param, j);
  }
  for (uint8_t i=0; i<MAX_NOTES_SET; i++) {
    if (notesSet[i].noteId > 0) {
      doEmit(notesSet[i]);
    }
  }
}

void onPalette(const OscMessage& m) {
  if (m.size() > 0) {
    state->currentPalette = m.arg<uint8_t>(0);
  }
}

void onColor(const OscMessage &m) {
  if (m.size() > 0) {
    uint8_t i = m.arg<uint8_t>(0);

    // If we have multiple colors, create a gradient
    if (m.size() > 2) {
      std::vector<ColorRGB> gradient;

      // Start from index 1 and collect all color arguments
      for (int j = 1; j < m.size(); j++) {
        ColorRGB color;
        color.set(m.arg<uint32_t>(j));
        gradient.push_back(color);
      }

      // Apply the gradient
      state->lightLists[i]->palette.setColors(gradient);
    }
    // Single color case
    else if (m.size() > 1) {
      ColorRGB color;
      color.set(m.arg<uint32_t>(1));
      state->lightLists[i]->palette.setColors({color});
    }
    // Random color case
    else {
      ColorRGB color;
      color.setRandom();
      state->lightLists[i]->palette.setColors({color});
    }
  }
  else {
    state->colorAll();
  }
}

void onSplit(const OscMessage &m) {
  if (m.size() > 0) {
    uint8_t i = m.arg<uint8_t>(0);
    state->lightLists[i]->split();
  }
  else {
    state->splitAll();
  }
}

void onAuto(const OscMessage &m) {
  state->autoEnabled = !state->autoEnabled;
}

void setupOSC() {
  if (!wifiConnected || !oscEnabled) {
    if (!wifiConnected) {
      LP_LOGLN("OSC setup skipped (no WiFi connection)");
    } else {
      LP_LOGLN("OSC setup skipped (disabled in settings)");
    }
    return;
  }
  OscWiFi.subscribe(oscPort, "/emit", onEmit);
  OscWiFi.subscribe(oscPort, "/note_on", onNoteOn);
  OscWiFi.subscribe(oscPort, "/note_off", onNoteOff);
  OscWiFi.subscribe(oscPort, "/notes_set", onNotesSet);
  OscWiFi.subscribe(oscPort, "/palette", onPalette);
  OscWiFi.subscribe(oscPort, "/color", onColor);
  OscWiFi.subscribe(oscPort, "/split", onSplit);
  OscWiFi.subscribe(oscPort, "/auto", onAuto);
  OscWiFi.subscribe(oscPort, "/command", onCommand);
  LP_LOGF("OSC setup complete on port %d\n", oscPort);
}
