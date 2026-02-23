// General utility functions
function postForm(path, params = {}) {
  const formData = new FormData();
  Object.entries(params).forEach(([key, value]) => {
    formData.append(key, String(value));
  });
  return fetch(path, {
    method: 'POST',
    body: formData
  });
}

function updateBrightness(value) {
  document.getElementById('brightness-value').textContent = value;
  postForm('/update_brightness', { value })
    .then(response => {
      if (!response.ok) {
        console.error('Failed to update brightness');
      }
    })
    .catch(error => console.error('Error updating brightness:', error));
}

function updateLayerBrightness(layer, value) {
  document.getElementById('bg-brightness-value_' + layer).textContent = value;
  postForm('/update_layer_brightness', { layer, value })
    .then(response => {
      if (!response.ok) {
        console.error('Failed to update brightness');
      }
    })
    .catch(error => console.error('Error updating brightness:', error));
  // Update the preview after changing brightness
  setTimeout(() => updateLayerPreview(layer), 500);
}

function toggleLayerVisibility(layer, isVisible) {
  postForm('/toggle_visible', { layer, visible: isVisible })
    .then(response => {
      if (!response.ok) {
        console.error('Failed to toggle layer visibility');
      } else {
        // Update the preview after toggling visibility
        updateLayerPreview(layer);
      }
    })
    .catch(error => console.error('Error toggling visibility:', error));
}

function updateLayerPreview(layer) {
  const previewEl = document.getElementById('layer-preview-' + layer);
  if (!previewEl) return;
  // Show loading indicator
  previewEl.style.background = 'linear-gradient(to right, #333, #555, #333)';
  
  // Fetch the colors for this layer
  fetch('/get_colors?layer=' + layer + '&maxColors=300')
    .then(response => response.json())
    .then(data => {
      // Create the gradient preview
      if (data.colors && data.colors.length > 0) {
        // Create a flex container for the colors
        const colorContainer = previewEl.querySelector('.color-container');
        
        // Clear existing preview
        colorContainer.innerHTML = '';
        
        // Add color segments
        data.colors.forEach(color => {
          const segment = document.createElement('div');
          segment.style.flex = '1';
          segment.style.height = '100%';
          segment.style.backgroundColor = `rgb(${color.r}, ${color.g}, ${color.b})`;
          colorContainer.appendChild(segment);
        });
        
        previewEl.style.backgroundColor = 'transparent';
      }
    })
    .catch(error => {
      console.error('Error fetching layer preview:', error);
      previewEl.style.backgroundColor = '#933'; // Error color
    });
}

// Palette management functions
function updateLedColor(layer, value, index = 0) {
  const colorInput = document.querySelector(`.led_color[data-index='${index}']`);
  if (colorInput) {
    colorInput.value = value;
  }
  
  // Apply the color change immediately
  applyColorGradient(layer);
}

function updateColorCount(layer) {
  const container = document.querySelector(`#color-inputs-container-${layer}`);
  const colorGroups = container.querySelectorAll('.color-input-group');
  const colorCount = colorGroups.length;
  document.getElementById('color-count-' + layer).textContent = '(' + colorCount + ')';
}

function getColorPositionData(layer) {
  const container = document.querySelector(`#color-inputs-container-${layer}`);
  const colorGroups = container.querySelectorAll('.color-input-group');
  let colorPairs = [];
  
  // Build array of color-position pairs
  colorGroups.forEach(group => {
    const colorInput = group.querySelector('.led_color');
    const positionInput = group.querySelector('.led_position');
    
    if (colorInput) {
      // Get position value and ensure it's in the correct range
      let position = 0;
      if (positionInput) {
        position = parseFloat(positionInput.value);
        position = Math.min(1, Math.max(0, position));
      }
      
      colorPairs.push({
        color: colorInput.value,
        position: position
      });
    }
  });
  
  // Sort by position
  colorPairs.sort((a, b) => a.position - b.position);
  
  // Separate into color and position arrays
  const colorData = colorPairs.map(pair => pair.color);
  const positionData = colorPairs.map(pair => pair.position);
  return {colorData, positionData};
}

function applyColorGradient(layer) {
  const { colorData, positionData } = getColorPositionData(layer);
  const formData = new FormData();
  formData.append('colors', JSON.stringify(colorData));
  formData.append('positions', JSON.stringify(positionData));
  formData.append('layer', layer);
  updatePalette(formData);
}

function updatePalette(formData) {
  // Get the layer ID from the form data
  const layer = formData.get('layer');
  
  // Send using POST for larger data
  fetch('/update_palette', {
    method: 'POST',
    body: formData
  })
    .then(response => {
      if (!response.ok) {
        console.error('Failed to update colors');
      } else {
        // Update the layer preview after palette changes
        setTimeout(() => updateLayerPreview(layer), 300);
      }
    });
}

function savePalette(layer) {
  // Get current palette data
  const colorGroups = document.querySelectorAll('.color-input-group');
  let colorPairs = [];
  
  colorGroups.forEach(group => {
    const colorInput = group.querySelector('.led_color');
    const positionInput = group.querySelector('.led_position');
    
    if (colorInput) {
      let position = 0;
      if (positionInput) {
        position = parseFloat(positionInput.value);
        position = Math.min(1, Math.max(0, position));
      }
      
      colorPairs.push({
        color: colorInput.value,
        position: position
      });
    }
  });
  
  // Check if we have any colors to save
  if (colorPairs.length === 0) {
    alert('Please add at least one color to the palette before saving.');
    return;
  }
  
  // Sort by position
  colorPairs.sort((a, b) => a.position - b.position);
  
  // Prompt for palette name
  const paletteName = prompt('Enter a name for this palette:', '');
  if (!paletteName || paletteName.trim() === '') {
    // User cancelled or entered empty name
    return;
  }
  
  // Prepare the data
  const paletteData = {
    name: paletteName.trim(),
    colors: colorPairs.map(pair => pair.color),
    positions: colorPairs.map(pair => pair.position),
    colorRule: document.getElementById('color_rule_' + layer).value,
    interMode: document.getElementById('inter_mode_' + layer).value,
    wrapMode: document.getElementById('wrap_mode_' + layer).value,
    segmentation: document.getElementById('segmentation_' + layer).value
  };

  syncPalette(paletteData)
  .then(data => {
    alert(`Palette '${paletteName}' saved successfully!`);
    // Add a delay before reloading to give SPIFFS time to complete the write
    setTimeout(() => {
      // Reload the predefined palette dropdown to include the new palette
      location.reload();
    }, 1000); // 1 second delay
  })
  .catch(error => {
    alert('Error saving palette: ' + error.message);
  });
}

function syncPalette(paletteData) {
  return fetch('/save_palette', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json'
    },
    body: JSON.stringify(paletteData)
  })
      .then(response => {
        if (response.ok) {
          return response.json();
        }
        throw new Error('Failed to save palette');
      })
}

function selectPredefinedPalette(layer, value) {
  if (value === '-1') {
    // Custom colors, do nothing
    return;
  }
  
  // Fetch palette colors from server
  fetch('/get_palette_colors?index=' + value + '&layer=' + layer)
    .then(response => response.json())
    .then(data => {
      // Clear existing colors
      const container = document.getElementById('color-inputs-container-' + layer);
      container.innerHTML = '';
      
      // Add colors from the palette
      data.colors.forEach((color, index) => {
        // Create a new color input group
        const newGroup = document.createElement('div');
        newGroup.className = 'color-input-group';
        newGroup.style = 'display: flex; align-items: center; margin-bottom: 10px;';
        
        // Create color input
        const colorInput = document.createElement('input');
        colorInput.type = 'color';
        colorInput.className = 'led_color';
        colorInput.setAttribute('data-index', index);
        colorInput.value = color;
        colorInput.style = 'flex: 0.7; height: 40px;';
        
        // Create position input
        const positionInput = document.createElement('input');
        positionInput.type = 'number';
        positionInput.className = 'led_position';
        positionInput.setAttribute('data-index', index);
        positionInput.min = 0;
        positionInput.max = 1;
        positionInput.step = 0.01;
        positionInput.value = data.positions[index];
        positionInput.style = 'flex: 0.3; margin-left: 10px;';
        
        // Create remove button
        const removeButton = document.createElement('a');
        removeButton.href = '#';
        removeButton.className = 'remove-color';
        removeButton.style = 'margin-left: 10px; color: white; background: #cc3300; padding: 5px 10px; text-decoration: none; border-radius: 3px;';
        removeButton.innerHTML = '&times;';
        removeButton.addEventListener('click', function(e) {
          e.preventDefault();
          removeColorInput(newGroup);
        });
        
        // Add elements to group
        newGroup.appendChild(colorInput);
        newGroup.appendChild(positionInput);
        newGroup.appendChild(removeButton);
        
        // Add group to container
        container.appendChild(newGroup);
        
        // Add color input event listener
        colorInput.addEventListener('input', function() {
          updateLedColor(layer, this.value, index);
        });
        
        // Add position input event listener
        positionInput.addEventListener('change', function() {
          applyColorGradient(layer);
        });
      });
      
      // Update color rule, interpolation mode, and wrap mode if provided
      if (data.colorRule !== undefined) {
        const colorRuleSelect = document.getElementById('color_rule_' + layer);
        if (colorRuleSelect) {
          colorRuleSelect.value = data.colorRule;
        }
      }
      
      if (data.interMode !== undefined) {
        const interModeSelect = document.getElementById('inter_mode_' + layer);
        if (interModeSelect) {
          interModeSelect.value = data.interMode;
        }
      }
      
      if (data.wrapMode !== undefined) {
        const wrapModeSelect = document.getElementById('wrap_mode_' + layer);
        if (wrapModeSelect) {
          wrapModeSelect.value = data.wrapMode;
        }
      }
      
      // Interpolation multiplier was removed
      
      if (data.segmentation !== undefined) {
        const segmentationInput = document.getElementById('segmentation_' + layer);
        if (segmentationInput) {
          segmentationInput.value = Number(data.segmentation).toFixed(1);
        }
      }
      
      // Create form data and apply the new colors
      const { colorData, positionData } = getColorPositionData(layer);
      const formData = new FormData();
      formData.append('colors', JSON.stringify(colorData));
      formData.append('positions', JSON.stringify(positionData));
      formData.append('layer', layer);
      
      // Also include other palette properties if they were updated
      if (data.colorRule !== undefined) {
        formData.append('colorRule', data.colorRule);
      }
      if (data.interMode !== undefined) {
        formData.append('interMode', data.interMode);
      }
      if (data.wrapMode !== undefined) {
        formData.append('wrapMode', data.wrapMode);
      }
      if (data.segmentation !== undefined) {
        formData.append('segmentation', data.segmentation);
      }
      
      // Send update to server
      updatePalette(formData);
      
      // Update color count
      updateColorCount(layer);
    })
    .catch(error => console.error('Error loading palette:', error));
}

function addColorInput(layer) {
  const container = document.querySelector(`#color-inputs-container-${layer}`);
  const colorInputGroups = container.querySelectorAll('.color-input-group');
  const newIndex = colorInputGroups.length;
  
  // Create new color input group
  const newGroup = document.createElement('div');
  newGroup.className = 'color-input-group';
  newGroup.style = 'display: flex; align-items: center; margin-bottom: 10px;';
  
  // Create color input - always start with black (unset) color
  const colorInput = document.createElement('input');
  colorInput.type = 'color';
  colorInput.className = 'led_color';
  colorInput.setAttribute('data-index', newIndex);
  colorInput.value = '#000000'; // Start with black (unset)
  colorInput.style = 'height: 40px;';
  
  // Create position input - default position based on index
  const positionInput = document.createElement('input');
  positionInput.type = 'number';
  positionInput.className = 'led_position';
  positionInput.setAttribute('data-index', newIndex);
  positionInput.min = 0;
  positionInput.max = 1;
  positionInput.step = 0.01;
  // If colors already exist, position at the end
  positionInput.value = colorInputGroups.length > 0 ? 1.0 : 0.0;
  positionInput.style = 'flex: 1; margin-left: 10px;';
  
  // Create remove button
  const removeButton = document.createElement('a');
  removeButton.href = '#';
  removeButton.className = 'remove-color';
  removeButton.style = 'margin-left: 10px; color: white; background: #cc3300; padding: 5px 10px; text-decoration: none; border-radius: 3px;';
  removeButton.innerHTML = '&times;';
  removeButton.addEventListener('click', function(e) {
    e.preventDefault();
    removeColorInput(newGroup);
  });
  
  // Add elements to group
  newGroup.appendChild(colorInput);
  newGroup.appendChild(positionInput);
  newGroup.appendChild(removeButton);
  
  // Add group to container
  container.appendChild(newGroup);
  
  // Add color input event listener
  colorInput.addEventListener('input', function() {
    const colorInput = document.querySelector(`.led_color[data-index='${newIndex}']`);
    if (colorInput) {
      colorInput.value = this.value;
    }
    
    applyColorGradient(layer);
  });
  
  // Add position input event listener
  positionInput.addEventListener('change', function() {
    applyColorGradient(layer);
  });
  applyColorGradient(layer);
}

function removeColorInput(group) {
  const container = group.closest('.color-inputs-container');
  const allGroups = container.querySelectorAll('.color-input-group');
  const layer = parseInt(container.getAttribute('data-layer'));
  
  // Check if we already have only 1 input - if so, don't remove
  if (allGroups.length <= 1) {
    // Don't allow removing the last color
    return;
  }
  
  group.remove();
  
  // Re-index the remaining color inputs
  const inputs = document.querySelectorAll('.led_color');
  inputs.forEach((input, index) => {
    input.setAttribute('data-index', index);
  });
  
  // Apply the changes immediately
  applyColorGradient(layer);
  
  // Update the color count
  updateColorCount(layer);
}

function addLayer() {
  fetch('/add_layer', {
    method: 'POST'
  })
    .then(response => {
      if (response.ok) {
        // Reload the page to show the new layer
        location.reload();
      } else {
        console.error('Failed to add layer');
        alert('Failed to add layer');
      }
    })
    .catch(error => {
      console.error('Error adding layer:', error);
      alert('Error adding layer: ' + error.message);
    });
}

function removeLayer(layer) {
  if (confirm('Are you sure you want to remove Layer ' + (layer + 1) + '?')) {
    postForm('/remove_layer', { layer })
      .then(response => {
        if (response.ok) {
          // Reload the page to show the updated layers
          location.reload();
        } else {
          console.error('Failed to remove layer');
          alert('Failed to remove layer');
        }
      })
      .catch(error => {
        console.error('Error removing layer:', error);
        alert('Error removing layer: ' + error.message);
      });
  }
}

// Visual page functions
function getRGBColor(colorObj) {
  return `rgb(${colorObj.r}, ${colorObj.g}, ${colorObj.b})`;
}

function createLayerControls() {
  const container = document.createElement('div');
  container.style.display = 'flex';
  container.style.marginTop = '10px';
  container.style.gap = '10px';
  
  // Layer selector
  const layerSelect = document.createElement('select');
  layerSelect.id = 'layer-select';
  layerSelect.style.padding = '5px';
  layerSelect.style.flexGrow = '1';
  
  // Add 'All layers' option
  const allOption = document.createElement('option');
  allOption.value = '-1';
  allOption.textContent = 'All layers';
  allOption.selected = true;
  layerSelect.appendChild(allOption);
  
  // Add options for each layer (max 8 layers)
  for (let i = 0; i < 8; i++) {
    const option = document.createElement('option');
    option.value = i;
    option.textContent = `Layer ${i+1}`;
    layerSelect.appendChild(option);
  }
  
  container.appendChild(layerSelect);
  
  return container;
}

function fetchLEDColors(maxColors = 0, layer = -1) {
  document.getElementById('led-strip').innerHTML = '<div class="loading-indicator" style="margin: auto; color: #888;">Loading LED colors...</div>';
  document.getElementById('led-info').innerHTML = 'Hover over LEDs to see their RGBW values.';
  let url = '/get_colors';
  let params = [];
  if (maxColors > 0) { params.push('maxColors=' + maxColors); }
  if (layer >= 0) { params.push('layer=' + layer); }
  if (params.length > 0) { url += '?' + params.join('&'); }
  
  fetch(url)
    .then(response => response.json())
    .then(data => {
      const stripContainer = document.getElementById('led-strip');
      stripContainer.innerHTML = '';
      
      data.colors.forEach((colorObj, index) => {
        const led = document.createElement('div');
        led.style.backgroundColor = getRGBColor(colorObj);
        led.style.height = '100%';
        led.style.flexGrow = '1';
        led.style.minWidth = '10px';
        led.style.margin = '0 1px';
        led.style.borderRadius = '2px';
        
        if (colorObj.w > 100) {
          led.style.border = '2px solid white';
          led.style.boxSizing = 'border-box';
        }
        
        led.title = `LED ${index}: R:${colorObj.r} G:${colorObj.g} B:${colorObj.b} W:${colorObj.w}`;
        stripContainer.appendChild(led);
      });
      
      document.getElementById('led-info').innerHTML = 
        'Hover over LEDs to see their RGBW values. <br>';
    })
    .catch(error => {
      console.error('Error fetching LED colors:', error);
      document.getElementById('led-strip').innerHTML = '<div style="margin: auto; color: #f88;">Error loading LED colors</div>';
    });
}

// Palette management functions from WebServerPalettes.h
function deletePalette() {
  const paletteSelect = document.getElementById('user_palette');
  if (!paletteSelect || !paletteSelect.value.startsWith('u')) {
    alert('Please select a user palette to delete');
    return;
  }
  
  const userPaletteIndex = paletteSelect.value.substring(1);
  const selectedOption = paletteSelect.options[paletteSelect.selectedIndex];
  const paletteName = selectedOption.text.replace('User: ', '');
  
  // Confirm deletion
  if (!confirm(`Are you sure you want to delete the palette "${paletteName}"?`)) {
    return;
  }
  
  // Send delete request to server
  postForm('/delete_palette', { index: userPaletteIndex })
    .then(response => {
      if (response.ok) {
        return response.json();
      }
      throw new Error('Failed to delete palette');
    })
    .then(data => {
      alert(`Palette '${paletteName}' deleted successfully!`);
      // Add a delay before reloading to ensure the palette file is updated
      setTimeout(() => {
        // Reload the page to update the palette dropdown
        location.reload();
      }, 1000); // 1 second delay
    })
    .catch(error => {
      alert('Error deleting palette: ' + error.message);
    });
}

function syncPalettes(push, pull) {
  const syncStatus = document.getElementById('sync-status');
  if (syncStatus) {
    syncStatus.innerHTML = 'Syncing... <div class="spinner" style="width: 16px; height: 16px; border: 3px solid #f3f3f3; border-top: 3px solid #3498db; border-radius: 50%; animation: spin 1s linear infinite; margin-left: 8px;"></div>';
    
    // Add spin animation if not already in stylesheet
    if (!document.getElementById('spin-animation')) {
      const style = document.createElement('style');
      style.id = 'spin-animation';
      style.innerHTML = '@keyframes spin { 0% { transform: rotate(0deg); } 100% { transform: rotate(360deg); } }';
      document.head.appendChild(style);
    }
  }
  
  // First fetch our own palette names to send to other devices
  fetch('/get_palettes?v=true')
    .then(response => {
      if (response.ok) {
        return response.json();
      }
      throw new Error('Failed to get local palettes');
    })
    .then(palettes => {
      // Send sync request to server with our palette list
      return fetch('/get_devices')
          .then(response => {
            if (response.ok) {
              return response.json();
            }
            throw new Error('Failed to get local devices');
          })
          .then(async devices => {
            let added = 0;

            for (const ip of devices) {
              await fetch(`http://${ip}/sync_palettes?push=${push}&pull=${pull}`, {
                method: 'POST',
                mode: 'cors',
                body: JSON.stringify(palettes),
              }).then(response => {
                if (response.ok) {
                  return response.json();
                }
              }).then(newPalettes => {
                for (let i = 0; i < palettes; i++) {
                  syncPalette(newPalettes[i]);
                  added++;
                }
              });
            }

            if (syncStatus) {
              if (devices && devices.length > 0) {
                syncStatus.innerHTML = `<span style="color: #2ecc71;">✓ Synced with ${devices.length} device(s). Pulled ${added} new palettes to this device.</span>`;
              } else {
                syncStatus.innerHTML = '<span style="color: #f39c12;">No other devices found</span>';
              }
            }

            // If we received new palettes, reload the page after 2 seconds
            if (added > 0) {
              setTimeout(() => {
                location.reload();
              }, 2000);
            }
          });
    })
    .catch(error => {
      if (syncStatus) {
        syncStatus.innerHTML = '<span style="color: #e74c3c;">Sync failed</span>';
      }
      console.error('Error syncing palettes:', error);
    });
}

function pushPalettes() {
  syncPalettes(true, false);
}

function pullPalettes() {
  syncPalettes(false, true);
}

function toggleDeleteButton(selectValue) {
  const deleteButton = document.getElementById('delete-palette-btn');
  if (deleteButton) {
    // Show delete button only for user palettes (value starts with 'u')
    if (selectValue && selectValue.startsWith('u')) {
      deleteButton.style.display = 'block';
    } else {
      deleteButton.style.display = 'none';
    }
  }
}

function selectUserPalette(value) {
  // Toggle delete button visibility
  toggleDeleteButton(value);
  
  // Show/hide palette details based on selection
  const paletteDetails = document.getElementById('palette_details');
  if (paletteDetails) {
    paletteDetails.style.display = (value === '-1') ? 'none' : 'block';
  }
  
  if (value === '-1') {
    // No palette selected, just hide details
    return;
  }
  
  // Fetch palette colors from server
  fetch('/get_palette_colors?index=' + value)
    .then(response => response.json())
    .then(data => {
      // Clear existing colors
      const container = document.getElementById('color-inputs-container');
      container.innerHTML = '';
      
      // Add colors from the palette
      data.colors.forEach((color, index) => {
        // Create a new color input group
        const newGroup = document.createElement('div');
        newGroup.className = 'color-input-group';
        newGroup.style = 'display: flex; align-items: center; margin-bottom: 10px;';
        
        // Create color input
        const colorInput = document.createElement('input');
        colorInput.type = 'color';
        colorInput.className = 'led_color';
        colorInput.setAttribute('data-index', index);
        colorInput.value = color;
        colorInput.style = 'flex: 0.7; height: 40px;';
        
        // Create position input
        const positionInput = document.createElement('input');
        positionInput.type = 'number';
        positionInput.className = 'led_position';
        positionInput.setAttribute('data-index', index);
        positionInput.min = 0;
        positionInput.max = 1;
        positionInput.step = 0.01;
        positionInput.value = data.positions[index];
        positionInput.style = 'flex: 0.3; margin-left: 10px;';
        
        // Add elements to group
        newGroup.appendChild(colorInput);
        newGroup.appendChild(positionInput);
        
        // Add group to container
        container.appendChild(newGroup);
      });
      
      // Update color rule, interpolation mode, and wrap mode if provided
      if (data.colorRule !== undefined) {
        const colorRuleSelect = document.getElementById('color_rule');
        if (colorRuleSelect) {
          colorRuleSelect.value = data.colorRule;
        }
      }
      
      if (data.interMode !== undefined) {
        const interModeSelect = document.getElementById('inter_mode');
        if (interModeSelect) {
          interModeSelect.value = data.interMode;
        }
      }
      
      if (data.wrapMode !== undefined) {
        const wrapModeSelect = document.getElementById('wrap_mode');
        if (wrapModeSelect) {
          wrapModeSelect.value = data.wrapMode;
        }
      }
      
      // Interpolation multiplier was removed
      
      if (data.segmentation !== undefined) {
        const segmentationInput = document.getElementById('segmentation');
        if (segmentationInput) {
          segmentationInput.value = Number(data.segmentation).toFixed(1);
        }
      }
      
      // Update color count
      updateColorCount();
    })
    .catch(error => console.error('Error loading palette:', error));
}

// Emitter control functions from WebServerEmitter.h
function updateemitterMinSpeed(value) {
  document.getElementById('min-speed-value').textContent = value;
  postForm('/update_emitter_min_speed', { value })
    .then(response => {
      if (!response.ok) {
        console.error('Failed to update min speed');
      }
    })
    .catch(error => console.error('Error updating min speed:', error));
}

function updateMaxSpeed(value) {
  document.getElementById('max-speed-value').textContent = value;
  postForm('/update_emitter_max_speed', { value })
    .then(response => {
      if (!response.ok) {
        console.error('Failed to update max speed');
      }
    })
    .catch(error => console.error('Error updating max speed:', error));
}

function updateMinDuration(value) {
  document.getElementById('min-duration-value').textContent = value;
  postForm('/update_emitter_min_dur', { value });
}

function updateMaxDuration(value) {
  document.getElementById('max-duration-value').textContent = value;
  postForm('/update_emitter_max_dur', { value });
}

function updateEmitterMinSat(value) {
  document.getElementById('emitter-min-sat-value').textContent = value;
  postForm('/update_emitter_min_sat', { value });
}

function updateEmitterMaxSat(value) {
  document.getElementById('emitter-max-sat-value').textContent = value;
  postForm('/update_emitter_max_sat', { value });
}

function updateEmitterMinVal(value) {
  document.getElementById('emitter-min-val-value').textContent = value;
  postForm('/update_emitter_min_val', { value });
}

function updateEmitterMaxVal(value) {
  document.getElementById('emitter-max-val-value').textContent = value;
  postForm('/update_emitter_max_val', { value });
}

function updateMinNext(value) {
  document.getElementById('min-next-value').textContent = value;
  postForm('/update_emitter_min_next', { value });
}

function updateMaxNext(value) {
  document.getElementById('max-next-value').textContent = value;
  postForm('/update_emitter_max_next', { value });
}

function updateEmitterFrom(value) {
  document.getElementById('emitter-from-value').textContent = value;
  postForm('/update_emitter_from', { value });
}

// Initialize event listeners when document is loaded
document.addEventListener('DOMContentLoaded', function() {
  // Settings page brightness slider
  const brightnessSlider = document.getElementById('max_brightness');
  if (brightnessSlider) {
    brightnessSlider.addEventListener('input', function() {
      document.getElementById('brightness-value').textContent = this.value;
    });
    brightnessSlider.addEventListener('change', function() {
      updateBrightness(this.value);
    });
  }
  
  // Initialize layer color previews
  document.querySelectorAll('.layer-preview').forEach(preview => {
    const layerId = parseInt(preview.id.split('-').pop());
    updateLayerPreview(layerId);
    
    // Add click event to toggle layer fields visibility
    preview.addEventListener('click', function() {
      const layerId = this.getAttribute('data-layer');
      const fieldsContainer = document.getElementById(`layer-fields-${layerId}`);
      if (fieldsContainer) {
        const isHidden = fieldsContainer.style.display === 'none';
        fieldsContainer.style.display = isHidden ? 'block' : 'none';
        
        // Update visual indicator of expanded state
        this.style.border = isHidden ? '1px solid white' : 'none';
        // Update the arrow indicator
        const arrow = this.querySelector('.arrow');
        if (arrow) {
          arrow.innerHTML = isHidden ? '&#x25BC;' : '&#x25B6;';
        }
      }
    });
  });
  
  // Add Layer button event listener
  const addLayerBtn = document.getElementById('add-layer-btn');
  if (addLayerBtn) {
    addLayerBtn.addEventListener('click', addLayer);
  }
  
  // Remove Layer button event listeners
  const removeLayerBtns = document.querySelectorAll('.remove-layer-btn');
  removeLayerBtns.forEach(btn => {
    btn.addEventListener('click', function() {
      const layer = parseInt(this.getAttribute('data-layer'));
      removeLayer(layer);
    });
  });
  
  // Initialize the remove buttons for all color groups
  const allRemoveButtons = document.querySelectorAll('.remove-color');
  allRemoveButtons.forEach(button => {
    button.addEventListener('click', function(e) {
      e.preventDefault();
      const group = this.closest('.color-input-group');
      const container = group.closest('.color-inputs-container');
      const layerId = parseInt(container.id.split('-').pop());
      if (group) {
        removeColorInput(group, layerId);
      }
    });
  });
  
  // Initialize the color count for all layers
  document.querySelectorAll('.color-inputs-container').forEach(container => {
    const layerId = container.getAttribute('data-layer');
    updateColorCount(layerId);
  });
  
  // Add event listeners for Add Color buttons
  document.querySelectorAll('.add-color-btn').forEach(btn => {
    btn.addEventListener('click', function() {
      const layerId = this.getAttribute('data-layer');
      addColorInput(layerId);
    });
  });
  
  // Color input handling - initialize all color inputs
  document.querySelectorAll('.led_color').forEach(input => {
    input.addEventListener('input', function() {
      const container = this.closest('.color-inputs-container');
      const layerId = container.getAttribute('data-layer');
      const index = parseInt(this.getAttribute('data-index'));
      updateLedColor(layerId, this.value, index);
    });
  });
  
  // Position input handling - initialize all position inputs
  document.querySelectorAll('.led_position').forEach(input => {
    input.addEventListener('change', function() {
      const container = this.closest('.color-inputs-container');
      const layerId = container.getAttribute('data-layer');
      applyColorGradient(layerId);
    });
  });
  
  // Color rule selects
  document.querySelectorAll('.color-rule').forEach(select => {
    select.addEventListener('change', function() {
      const formData = new FormData();
      formData.append('layer', this.getAttribute('data-layer'));
      formData.append('colorRule', this.value);
      updatePalette(formData);
    });
  });
  
  // Interpolation mode selects
  document.querySelectorAll('.inter-mode').forEach(select => {
    select.addEventListener('change', function() {
      const formData = new FormData();
      formData.append('layer', this.getAttribute('data-layer'));
      formData.append('interMode', this.value);
      updatePalette(formData);
    });
  });
  
  // Wrap mode selects
  document.querySelectorAll('.wrap-mode').forEach(select => {
    select.addEventListener('change', function() {
      const formData = new FormData();
      formData.append('layer', this.getAttribute('data-layer'));
      formData.append('wrapMode', this.value);
      updatePalette(formData);
    });
  });
  
  // Blend mode selects
  document.querySelectorAll('.blend-mode').forEach(select => {
    select.addEventListener('change', function() {
      const layerId = this.getAttribute('data-layer');
      postForm('/update_blend_mode', { layer: layerId, mode: this.value })
        .then(response => {
          if (!response.ok) {
            console.error('Failed to update blend mode');
          } else {
            // Update the layer preview after blend mode changes
            setTimeout(() => updateLayerPreview(layerId), 300);
          }
        });
    });
  });
  
  // Behaviour flags selects
  document.querySelectorAll('.behaviour-flags').forEach(select => {
    select.addEventListener('change', function() {
      const layerId = this.getAttribute('data-layer');
      postForm('/update_behaviour_flags', { layer: layerId, flags: this.value })
        .then(response => {
          if (!response.ok) {
            console.error('Failed to update behaviour flags');
          } else {
            // Update the layer preview after behaviour flags changes
            setTimeout(() => updateLayerPreview(layerId), 300);
          }
        });
    });
  });
  
  // Segmentation inputs
  document.querySelectorAll('.segmentation').forEach(input => {
    input.addEventListener('change', function() {
      const formData = new FormData();
      formData.append('layer', this.getAttribute('data-layer'));
      formData.append('segmentation', this.value);
      updatePalette(formData);
    });
  });
  
  // Predefined palette selects
  document.querySelectorAll('[id^="predefined_palette_"]').forEach(select => {
    select.addEventListener('change', function() {
      const layerId = this.getAttribute('data-layer');
      selectPredefinedPalette(layerId, this.value);
    });
  });
  
  // Save palette buttons
  document.querySelectorAll('.save-palette-btn').forEach(btn => {
    btn.addEventListener('click', function() {
      const layerId = this.getAttribute('data-layer');
      savePalette(layerId);
    });
  });
  
  // Layer visibility checkboxes
  document.querySelectorAll('.layer-visibility').forEach(checkbox => {
    checkbox.addEventListener('change', function() {
      const layerId = this.getAttribute('data-layer');
      toggleLayerVisibility(layerId, this.checked);
    });
  });
  
  // Function to update layer speed
function updateLayerSpeed(layer, value) {
  postForm('/update_speed', { layer, value })
    .then(response => {
      if (!response.ok) {
        console.error('Failed to update speed');
      }
    })
    .catch(error => console.error('Error updating speed:', error));
  // Update the preview after changing speed
  setTimeout(() => updateLayerPreview(layer), 500);
}

// Function to update layer easing
function updateLayerEase(layer, value) {
  postForm('/update_ease', { layer, ease: value })
    .then(response => {
      if (!response.ok) {
        console.error('Failed to update easing');
      }
    })
    .catch(error => console.error('Error updating easing:', error));
  // Update the preview after changing easing
  setTimeout(() => updateLayerPreview(layer), 500);
}

// Function to update layer fade speed
function updateLayerFadeSpeed(layer, value) {
  postForm('/update_fade_speed', { layer, value })
    .then(response => {
      if (!response.ok) {
        console.error('Failed to update fade speed');
      }
    })
    .catch(error => console.error('Error updating fade speed:', error));
  // Update the preview after changing fade speed
  setTimeout(() => updateLayerPreview(layer), 500);
}

// Layer brightness sliders
  document.querySelectorAll('.layer-brightness').forEach(slider => {
    slider.addEventListener('input', function() {
      const layerId = this.getAttribute('data-layer');
      document.getElementById('bg-brightness-value_' + layerId).textContent = this.value;
    });
    slider.addEventListener('change', function() {
      const layerId = this.getAttribute('data-layer');
      updateLayerBrightness(layerId, this.value);
    });
  });
  
  // Layer speed sliders
  document.querySelectorAll('.layer-speed').forEach(slider => {
    slider.addEventListener('change', function() {
      const layerId = this.getAttribute('data-layer');
      updateLayerSpeed(layerId, this.value);
    });
  });
  
  // Layer ease selects
  document.querySelectorAll('.layer-ease').forEach(select => {
    select.addEventListener('change', function() {
      const layerId = this.getAttribute('data-layer');
      updateLayerEase(layerId, this.value);
    });
  });
  
  // Layer fade speed inputs
  document.querySelectorAll('.fade-speed').forEach(input => {
    input.addEventListener('change', function() {
      const layerId = this.getAttribute('data-layer');
      updateLayerFadeSpeed(layerId, this.value);
    });
  });
  
  // Visual page initialization
  const controlsContainer = document.getElementById('visualization-controls');
  if (controlsContainer) {
    controlsContainer.appendChild(createLayerControls());
    // Initialize colors on page load
    fetchLEDColors();
    
    // Add refresh button click handler
    document.getElementById('refresh-leds').addEventListener('click', function() {
      const layerValue = parseInt(document.getElementById('layer-select').value);
      fetchLEDColors(0, layerValue);
    });
    
    // Add layer selector change handler
    document.getElementById('layer-select').addEventListener('change', function() {
      const layerValue = parseInt(this.value);
      fetchLEDColors(0, layerValue);
    });
  }
  
  // Palettes page initialization
  const userPaletteSelect = document.getElementById('user_palette');
  if (userPaletteSelect) {
    userPaletteSelect.addEventListener('change', function() {
      selectUserPalette(this.value);
    });
    
    // Initialize Delete Palette button
    const deletePaletteBtn = document.getElementById('delete-palette-btn');
    if (deletePaletteBtn) {
      deletePaletteBtn.addEventListener('click', function() {
        deletePalette();
      });
    }
    
    const pushPalettesBtn = document.getElementById('push-palettes-btn');
    if (pushPalettesBtn) {
      pushPalettesBtn.addEventListener('click', function() {
        pushPalettes();
      });
    }

    const pullPalettesBtn = document.getElementById('pull-palettes-btn');
    if (pullPalettesBtn) {
      pullPalettesBtn.addEventListener('click', function() {
        pullPalettes();
      });
    }
    
    // Set initial state of delete button based on selection
    toggleDeleteButton(userPaletteSelect.value);
  }
  
  // Emitter page initialization
  const emitterMinSpeedSlider = document.getElementById('emitter_min_speed');
  if (emitterMinSpeedSlider) {
    emitterMinSpeedSlider.addEventListener('change', function() {
      updateemitterMinSpeed(this.value);
    });
    
    const emitterMaxSpeedSlider = document.getElementById('emitter_max_speed');
    emitterMaxSpeedSlider.addEventListener('change', function() {
      updateMaxSpeed(this.value);
    });
    
    const emitterMinDurSlider = document.getElementById('emitter_min_dur');
    emitterMinDurSlider.addEventListener('change', function() {
      updateMinDuration(this.value);
    });
    
    const emitterMaxDurSlider = document.getElementById('emitter_max_dur');
    emitterMaxDurSlider.addEventListener('change', function() {
      updateMaxDuration(this.value);
    });
    
    // Add listeners for other emitter controls as well
    const emitterMinSatSlider = document.getElementById('emitter_min_sat');
    if (emitterMinSatSlider) {
      emitterMinSatSlider.addEventListener('change', function() {
        updateEmitterMinSat(this.value);
      });
    }
    
    const emitterMaxSatSlider = document.getElementById('emitter_max_sat');
    if (emitterMaxSatSlider) {
      emitterMaxSatSlider.addEventListener('change', function() {
        updateEmitterMaxSat(this.value);
      });
    }
    
    const emitterMinValSlider = document.getElementById('emitter_min_val');
    if (emitterMinValSlider) {
      emitterMinValSlider.addEventListener('change', function() {
        updateEmitterMinVal(this.value);
      });
    }
    
    const emitterMaxValSlider = document.getElementById('emitter_max_val');
    if (emitterMaxValSlider) {
      emitterMaxValSlider.addEventListener('change', function() {
        updateEmitterMaxVal(this.value);
      });
    }
    
    const emitterMinNextSlider = document.getElementById('emitter_min_next');
    if (emitterMinNextSlider) {
      emitterMinNextSlider.addEventListener('change', function() {
        updateMinNext(this.value);
      });
    }
    
    const emitterMaxNextSlider = document.getElementById('emitter_max_next');
    if (emitterMaxNextSlider) {
      emitterMaxNextSlider.addEventListener('change', function() {
        updateMaxNext(this.value);
      });
    }
    
    const emitterFromInput = document.getElementById('emitter_from');
    if (emitterFromInput) {
      emitterFromInput.addEventListener('change', function() {
        updateEmitterFrom(this.value);
      });
    }
  }
});
