#pragma once

#include "FirmwareContext.h"

inline void registerLayerRoutes(WebServer& web, FirmwareContext& context) {
  (void)context;
  web.on("/get_layers", HTTP_GET, handleGetLayers);
  web.on("/get_layers", HTTP_OPTIONS, allowCORS("GET"));
  web.on("/toggle_visible", HTTP_POST, guardMutatingRoute(handleToggleVisible));
  web.on("/toggle_visible", HTTP_OPTIONS, allowCORS("POST"));
  web.on("/update_palette", HTTP_POST, guardMutatingRoute(handleUpdatePalette));
  web.on("/update_palette", HTTP_OPTIONS, handleCORS);
  web.on("/update_layer_brightness", HTTP_POST, guardMutatingRoute(handleUpdateLayerBrightness));
  web.on("/update_layer_brightness", HTTP_OPTIONS, allowCORS("POST"));
  web.on("/get_palette_colors", HTTP_GET, handleGetPaletteColors);
  web.on("/get_palette_colors", HTTP_OPTIONS, allowCORS("GET"));
  web.on("/save_palette", HTTP_POST, guardMutatingRoute(handleSavePalette));
  web.on("/save_palette", HTTP_OPTIONS, allowCORS("POST"));
  web.on("/add_layer", HTTP_POST, guardMutatingRoute(handleAddLayer));
  web.on("/add_layer", HTTP_OPTIONS, allowCORS("POST"));
  web.on("/remove_layer", HTTP_POST, guardMutatingRoute(handleRemoveLayer));
  web.on("/remove_layer", HTTP_OPTIONS, allowCORS("POST"));
  web.on("/update_blend_mode", HTTP_POST, guardMutatingRoute(handleUpdateBlendMode));
  web.on("/update_speed", HTTP_POST, guardMutatingRoute(handleUpdateSpeed));
  web.on("/update_speed", HTTP_OPTIONS, allowCORS("POST"));
  web.on("/update_ease", HTTP_POST, guardMutatingRoute(handleUpdateEase));
  web.on("/update_ease", HTTP_OPTIONS, allowCORS("POST"));
  web.on("/update_fade_speed", HTTP_POST, guardMutatingRoute(handleUpdateFadeSpeed));
  web.on("/update_fade_speed", HTTP_OPTIONS, allowCORS("POST"));
  web.on("/update_behaviour_flags", HTTP_POST, guardMutatingRoute(handleUpdateBehaviourFlags));
  web.on("/update_behaviour_flags", HTTP_OPTIONS, allowCORS("POST"));
  web.on("/update_layer_offset", HTTP_POST, guardMutatingRoute(handleUpdateLayerOffset));
  web.on("/update_layer_offset", HTTP_OPTIONS, allowCORS("POST"));
  web.on("/reset_layer", HTTP_POST, guardMutatingRoute(handleResetLayer));
  web.on("/reset_layer", HTTP_OPTIONS, allowCORS("POST"));
}

inline void registerPaletteRoutes(WebServer& web, FirmwareContext& context) {
  (void)context;
  web.on("/delete_palette", HTTP_POST, guardMutatingRoute(handleDeletePalette));
  web.on("/delete_palette", HTTP_OPTIONS, allowCORS("POST"));
  web.on("/sync_palettes", HTTP_POST, guardMutatingRoute(handleSyncPalettes));
  web.on("/get_palettes", HTTP_GET, handleGetPalettes);
  web.on("/get_palettes", HTTP_OPTIONS, allowCORS("GET"));
}

inline void registerEmitterRoutes(WebServer& web, FirmwareContext& context) {
  (void)context;
#ifdef WEBSERVER_EMITTER
  web.on("/toggle_auto", HTTP_POST, guardMutatingRoute(handleToggleAuto));
  web.on("/update_emitter_min_speed", HTTP_POST, guardMutatingRoute(handleUpdateEmitterMinSpeed));
  web.on("/update_emitter_max_speed", HTTP_POST, guardMutatingRoute(handleUpdateEmitterMaxSpeed));
  web.on("/update_emitter_min_dur", HTTP_POST, guardMutatingRoute(handleUpdateEmitterMinDuration));
  web.on("/update_emitter_max_dur", HTTP_POST, guardMutatingRoute(handleUpdateEmitterMaxDuration));
  web.on("/update_emitter_min_sat", HTTP_POST, guardMutatingRoute(handleUpdateEmitterMinSat));
  web.on("/update_emitter_max_sat", HTTP_POST, guardMutatingRoute(handleUpdateEmitterMaxSat));
  web.on("/update_emitter_min_val", HTTP_POST, guardMutatingRoute(handleUpdateEmitterMinVal));
  web.on("/update_emitter_max_val", HTTP_POST, guardMutatingRoute(handleUpdateEmitterMaxVal));
  web.on("/update_emitter_min_next", HTTP_POST, guardMutatingRoute(handleUpdateEmitterMinNext));
  web.on("/update_emitter_max_next", HTTP_POST, guardMutatingRoute(handleUpdateEmitterMaxNext));
  web.on("/update_emitter_from", HTTP_POST, guardMutatingRoute(handleUpdateEmitterFrom));
#endif
}

