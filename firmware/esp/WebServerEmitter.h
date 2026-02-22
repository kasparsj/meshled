#pragma once

void streamEmitter(WiFiClient &client) {
  // Emitter controls
  client.println("<div style='margin: 20px 0; padding: 15px; background: #555; border-radius: 5px;'>");
  client.println("<h3 style='margin-top: 0;'>Emitter Control</h3>");

  // Auto emitter toggle
  client.println("<div style='margin: 20px 0; display: flex; gap: 10px; flex-wrap: wrap;'>");
  client.printf("<a href='/toggle_auto' style='flex: 1; display: block; text-align: center; background: %s; color: white; text-decoration: none; padding: 10px 0; border-radius: 5px;'>",
             state->autoEnabled ? "#0044cc" : "#0066ff");
  client.printf("%s</a>", state->autoEnabled ? "Disable Auto Emitter" : "Enable Auto Emitter");
  client.println("</div>");

  // Speed controls
  client.printf("<label for='emitter_min_speed'>Min Speed: <span id='min-speed-value'>%f</span></label>", emitterMinSpeed);
  client.printf("<input type='range' id='emitter_min_speed' name='emitter_min_speed' min='0.1' max='5' step='0.1' value='%f'>", emitterMinSpeed);
  client.printf("<label for='emitter_max_speed'>Max Speed: <span id='max-speed-value'>%f</span></label>", emitterMaxSpeed);
  client.printf("<input type='range' id='emitter_max_speed' name='emitter_max_speed' min='1' max='20' step='1' value='%f'>", emitterMaxSpeed);

  yield();

  // Duration controls
  client.printf("<label for='emitter_min_dur'>Min Duration, ms: <span id='min-duration-value'>%d</span></label>", emitterMinDur);
  client.printf("<input type='range' id='emitter_min_dur' name='emitter_min_dur' min='100' max='5000' step='100' value='%d'>", emitterMinDur);
  client.printf("<label for='emitter_max_dur'>Max Duration, ms: <span id='max-duration-value'>%d</span></label>", emitterMaxDur);
  client.printf("<input type='range' id='emitter_max_dur' name='emitter_max_dur' min='1000' max='50000' step='1000' value='%d'>", emitterMaxDur);

  yield();

  // Color controls
  client.printf("<label for='emitter_min_sat'>Min Saturation: <span id='emitter-min-sat-value'>%d</span></label>", emitterMinSat);
  client.printf("<input type='range' id='emitter_min_sat' name='emitter_min_sat' min='1' max='255' step='1' value='%d'>", emitterMinSat);
  client.printf("<label for='emitter_max_sat'>Max Saturation: <span id='emitter-max-sat-value'>%d</span></label>", emitterMaxSat);
  client.printf("<input type='range' id='emitter_max_sat' name='emitter_max_sat' min='50' max='255' step='1' value='%d'>", emitterMaxSat);

  yield();

  client.printf("<label for='emitter_min_val'>Min Value: <span id='emitter-min-val-value'>%d</span></label>", emitterMinVal);
  client.printf("<input type='range' id='emitter_min_val' name='emitter_min_val' min='1' max='255' step='1' value='%d'>", emitterMinVal);
  client.printf("<label for='emitter_max_val'>Max Value: <span id='emitter-max-val-value'>%d</span></label>", emitterMaxVal);
  client.printf("<input type='range' id='emitter_max_val' name='emitter_max_val' min='50' max='255' step='1' value='%d'>", emitterMaxVal);

  yield();

  // Timing controls
  client.printf("<label for='emitter_min_next'>Min Time Between Emits (ms): <span id='min-next-value'>%d</span></label>", emitterMinNext);
  client.printf("<input type='range' id='emitter_min_next' name='emitter_min_next' min='100' max='5000' step='100' value='%d'>", emitterMinNext);
  client.printf("<label for='emitter_max_next'>Max Time Between Emits (ms): <span id='max-next-value'>%d</span></label>", emitterMaxNext);
  client.printf("<input type='range' id='emitter_max_next' name='emitter_max_next' min='1000' max='30000' step='1000' value='%d'>", emitterMaxNext);
  client.printf("<label for='emitter_from'>Emitter From Index: <span id='emitter-from-value'>%d</span></label>", emitterFrom);
  client.printf("<input type='number' id='emitter_from' name='emitter_from' min='-1' max='255' value='%d'>", emitterFrom);
  client.println("</div>");

  // JavaScript is now loaded in the header from scripts.js
}

// Handle toggle auto emitter
void handleToggleAuto() {
  state->autoEnabled = !state->autoEnabled;

  // Update the global emitterEnabled variable to match state->autoEnabled
  emitterEnabled = state->autoEnabled;

  LP_LOGLN("Toggle autoEmitter: " + String(state->autoEnabled ? "ON" : "OFF"));

  // Save the updated setting
  #ifdef SPIFFS_ENABLED
  saveSettings();
  #endif

  // Redirect back to homepage
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}

// Update LPRandom constants with global settings
void updateLPRandomConstants() {
  LPRandom::MIN_SPEED = emitterMinSpeed;
  LPRandom::MAX_SPEED = emitterMaxSpeed;
  LPRandom::MIN_DURATION = emitterMinDur;
  LPRandom::MAX_DURATION = emitterMaxDur;
  LPRandom::MIN_SATURATION = emitterMinSat;
  LPRandom::MAX_SATURATION = emitterMaxSat;
  LPRandom::MIN_VALUE = emitterMinVal;
  LPRandom::MAX_VALUE = emitterMaxVal;
  LPRandom::MIN_NEXT = emitterMinNext;
  LPRandom::MAX_NEXT = emitterMaxNext;
  LP_LOGLN("LPRandom constants updated");
}

// Handle AJAX min speed update
void handleUpdateEmitterMinSpeed() {
  if (server.hasArg("value")) {
    float newemitterMinSpeed = server.arg("value").toFloat();
    if (newemitterMinSpeed > 0 && newemitterMinSpeed < emitterMaxSpeed) {
      emitterMinSpeed = newemitterMinSpeed;
      LP_LOGLN("Updated min speed via AJAX: " + String(emitterMinSpeed));

      // Update LPRandom constant
      updateLPRandomConstants();

      #ifdef SPIFFS_ENABLED
      saveSettings();
      #endif

      server.send(200, "text/plain", "OK");
      return;
    }
  }

  server.send(400, "text/plain", "Invalid min speed value");
}

// Handle AJAX max speed update
void handleUpdateEmitterMaxSpeed() {
  if (server.hasArg("value")) {
    float newMaxSpeed = server.arg("value").toFloat();
    if (newMaxSpeed > emitterMinSpeed) {
      emitterMaxSpeed = newMaxSpeed;
      LP_LOGLN("Updated max speed via AJAX: " + String(emitterMaxSpeed));

      // Update LPRandom constant
      updateLPRandomConstants();

      #ifdef SPIFFS_ENABLED
      saveSettings();
      #endif

      server.send(200, "text/plain", "OK");
      return;
    }
  }

  server.send(400, "text/plain", "Invalid max speed value");
}

// Handle AJAX min duration update
void handleUpdateEmitterMinDuration() {
  if (server.hasArg("value")) {
    uint32_t newMinDuration = server.arg("value").toInt();
    if (newMinDuration > 0 && newMinDuration < emitterMaxDur) {
      emitterMinDur = newMinDuration;
      LP_LOGLN("Updated min duration via AJAX: " + String(emitterMinDur));

      // Update LPRandom constant
      updateLPRandomConstants();

      #ifdef SPIFFS_ENABLED
      saveSettings();
      #endif

      server.send(200, "text/plain", "OK");
      return;
    }
  }

  server.send(400, "text/plain", "Invalid min duration value");
}

// Handle AJAX max duration update
void handleUpdateEmitterMaxDuration() {
  if (server.hasArg("value")) {
    uint32_t newMaxDuration = server.arg("value").toInt();
    if (newMaxDuration > emitterMinDur) {
      emitterMaxDur = newMaxDuration;
      LP_LOGLN("Updated max duration via AJAX: " + String(emitterMaxDur));

      // Update LPRandom constant
      updateLPRandomConstants();

      #ifdef SPIFFS_ENABLED
      saveSettings();
      #endif

      server.send(200, "text/plain", "OK");
      return;
    }
  }

  server.send(400, "text/plain", "Invalid max duration value");
}

void handleUpdateEmitterMinSat() {
  if (server.hasArg("value")) {
    uint8_t newMinSat = server.arg("value").toInt();
    if (newMinSat >= 1 && newMinSat <= emitterMaxSat) {
      emitterMinSat = newMinSat;
      LP_LOGLN("Updated emitter min saturation via AJAX: " + String(emitterMinSat));

      // Update LPRandom constant
      updateLPRandomConstants();

      #ifdef SPIFFS_ENABLED
      saveSettings();
      #endif

      server.send(200, "text/plain", "OK");
      return;
    }
  }

  server.send(400, "text/plain", "Invalid emitter min saturation value");
}

void handleUpdateEmitterMaxSat() {
  if (server.hasArg("value")) {
    uint8_t newMaxSat = server.arg("value").toInt();
    if (newMaxSat > emitterMinSat && newMaxSat <= 255) {
      emitterMaxSat = newMaxSat;
      LP_LOGLN("Updated emitter max saturation via AJAX: " + String(emitterMaxSat));

      // Update LPRandom constant
      updateLPRandomConstants();

      #ifdef SPIFFS_ENABLED
      saveSettings();
      #endif

      server.send(200, "text/plain", "OK");
      return;
    }
  }

  server.send(400, "text/plain", "Invalid emitter max saturation value");
}

void handleUpdateEmitterMinVal() {
  if (server.hasArg("value")) {
    uint8_t newMinVal = server.arg("value").toInt();
    if (newMinVal >= 1 && newMinVal <= emitterMaxVal) {
      emitterMinVal = newMinVal;
      LP_LOGLN("Updated emitter min value via AJAX: " + String(emitterMinVal));

      // Update LPRandom constant
      updateLPRandomConstants();

      #ifdef SPIFFS_ENABLED
      saveSettings();
      #endif

      server.send(200, "text/plain", "OK");
      return;
    }
  }

  server.send(400, "text/plain", "Invalid emitter min value");
}

void handleUpdateEmitterMaxVal() {
  if (server.hasArg("value")) {
    uint8_t newMaxVal = server.arg("value").toInt();
    if (newMaxVal > emitterMinVal && newMaxVal <= 255) {
      emitterMaxVal = newMaxVal;
      LP_LOGLN("Updated emitter max value via AJAX: " + String(emitterMaxVal));

      // Update LPRandom constant
      updateLPRandomConstants();

      #ifdef SPIFFS_ENABLED
      saveSettings();
      #endif

      server.send(200, "text/plain", "OK");
      return;
    }
  }

  server.send(400, "text/plain", "Invalid emitter max value");
}

// Handle AJAX min next update
void handleUpdateEmitterMinNext() {
  if (server.hasArg("value")) {
    uint16_t newMinNext = server.arg("value").toInt();
    if (newMinNext > 0 && newMinNext < emitterMaxNext) {
      emitterMinNext = newMinNext;
      LP_LOGLN("Updated min time between emits via AJAX: " + String(emitterMinNext));

      // Update LPRandom constant
      updateLPRandomConstants();

      #ifdef SPIFFS_ENABLED
      saveSettings();
      #endif

      server.send(200, "text/plain", "OK");
      return;
    }
  }

  server.send(400, "text/plain", "Invalid min time between emits value");
}

// Handle AJAX max next update
void handleUpdateEmitterMaxNext() {
  if (server.hasArg("value")) {
    uint16_t newMaxNext = server.arg("value").toInt();
    if (newMaxNext > emitterMinNext) {
      emitterMaxNext = newMaxNext;
      LP_LOGLN("Updated max time between emits via AJAX: " + String(emitterMaxNext));

      // Update LPRandom constant
      updateLPRandomConstants();

      #ifdef SPIFFS_ENABLED
      saveSettings();
      #endif

      server.send(200, "text/plain", "OK");
      return;
    }
  }

  server.send(400, "text/plain", "Invalid max time between emits value");
}

// Handle AJAX emitter from update
void handleUpdateEmitterFrom() {
  if (server.hasArg("value")) {
    int16_t newFrom = server.arg("value").toInt();

    // -1 means random start position, or any non-negative index
    if (newFrom >= -1) {
      emitterFrom = newFrom;

      // Update the State's autoParams
      State::autoParams.from = emitterFrom;

      LP_LOGLN("Updated emitter from index via AJAX: " + String(emitterFrom));

      #ifdef SPIFFS_ENABLED
      saveSettings();
      #endif

      server.send(200, "text/plain", "OK");
      return;
    }
  }

  server.send(400, "text/plain", "Invalid emitter from index value");
}