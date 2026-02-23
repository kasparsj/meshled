import {useCallback, useEffect, useState} from "react";
import {useDevice} from "../contexts/DeviceContext.jsx";
import { parseLayersResponse } from "../models/apiModels";

const useLayers = () => {
    const [layers, setLayers] = useState([]);
    const [loading, setLoading] = useState(true);
    const [error, setError] = useState(null);
    const { deviceFetch, selectedDevice } = useDevice();

    useEffect(() => {
        if (!selectedDevice) {
            setLayers([]);
            setLoading(false);
            setError('No device selected');
            return;
        }

        const fetchLayers = async () => {
            try {
                setLoading(true);
                setError(null);
                const response = await deviceFetch('/get_layers');
                const data = parseLayersResponse(await response.json());
                setLayers(data);
            } catch (err) {
                setError("Failed to fetch layers");
                console.error(err);
            } finally {
                setLoading(false);
            }
        };

        fetchLayers();
    }, [selectedDevice, deviceFetch]);

    const toggleLayerVisibility = useCallback(async (layerId) => {
        const layer = layers.find(l => l.id === layerId);
        if (!layer) return;
        const newVisibility = !layer.visible;

        setLayers(prev =>
            prev.map(layer =>
                layer.id === layerId ? { ...layer, visible: newVisibility } : layer
            )
        );

        const formData = new FormData();
        formData.append('layer', layerId.toString());
        formData.append('visible', newVisibility ? 'true' : 'false');
        await deviceFetch('/toggle_visible', {
            method: 'POST',
            body: formData
        });
    }, [deviceFetch, layers]);

    const updateLayerBrightness = useCallback(async (layerId, brightness) => {
        setLayers(prev =>
            prev.map(layer =>
                layer.id === layerId ? { ...layer, brightness } : layer
            )
        );

        const formData = new FormData();
        formData.append('layer', layerId.toString());
        formData.append('value', brightness.toString());
        await deviceFetch('/update_layer_brightness', {
            method: 'POST',
            body: formData
        });
    }, [deviceFetch]);

    const updateLayerSpeed = useCallback(async (layerId, speed) => {
        setLayers(prev =>
            prev.map(layer =>
                layer.id === layerId ? { ...layer, speed } : layer
            )
        );

        const formData = new FormData();
        formData.append('layer', layerId.toString());
        formData.append('value', speed.toString());
        await deviceFetch('/update_speed', {
            method: 'POST',
            body: formData
        });
    }, [deviceFetch]);

    const updateLayerOffset = useCallback(async (layerId, offset) => {
        setLayers(prev =>
            prev.map(layer =>
                layer.id === layerId ? { ...layer, offset } : layer
            )
        );

        const formData = new FormData();
        formData.append('layer', layerId.toString());
        formData.append('offset', offset.toString());
        await deviceFetch('/update_layer_offset', {
            method: 'POST',
            body: formData
        });
    }, [deviceFetch]);

    const removeLayer = useCallback(async (layerId) => {
        try {
            const formData = new FormData();
            formData.append('layer', layerId.toString());
            await deviceFetch('/remove_layer', {
                method: 'POST',
                body: formData
            });

            // Remove layer from state after successful API call
            setLayers(prev => prev.filter(layer => layer.id !== layerId));
        } catch (err) {
            console.error('Failed to remove layer:', err);
            setError('Failed to remove layer');
        }
    }, [deviceFetch]);

    const updatePalette = useCallback(async (layerId, paletteData) => {
        const layer = layers.find(l => l.id === layerId);
        if (!layer) return;
        
        try {
            // Prepare form data for the request
            const formData = new FormData();
            formData.append('layer', layerId.toString());
            
            // Add individual palette properties if provided
            if (paletteData.colorRule !== undefined) {
                formData.append('colorRule', paletteData.colorRule.toString());
            }
            if (paletteData.interMode !== undefined) {
                formData.append('interMode', paletteData.interMode.toString());
            }
            if (paletteData.wrapMode !== undefined) {
                formData.append('wrapMode', paletteData.wrapMode.toString());
            }
            if (paletteData.segmentation !== undefined) {
                formData.append('segmentation', paletteData.segmentation.toString());
            }
            
            // Add colors and positions as JSON if provided
            if (paletteData.colors && paletteData.colors.length > 0) {
                formData.append('colors', JSON.stringify(paletteData.colors));
                if (paletteData.positions && paletteData.positions.length === paletteData.colors.length) {
                    formData.append('positions', JSON.stringify(paletteData.positions));
                }
            }
            
            const response = await deviceFetch('/update_palette', {
                method: 'POST',
                body: formData
            });
            
            if (response.ok) {
                // Update the layer's palette in local state
                setLayers(prev => 
                    prev.map(layer => 
                        layer.id === layerId 
                            ? { 
                                ...layer, 
                                palette: {
                                    ...layer.palette,
                                    ...paletteData
                                }
                            }
                            : layer
                    )
                );
                
                console.log("Palette updated successfully");
            } else {
                const errorText = await response.text();
                console.error("Failed to update palette:", errorText);
                setError("Failed to update palette: " + errorText);
            }
        } catch (err) {
            console.error("Error updating palette:", err);
            setError("Error updating palette");
        }
    }, [deviceFetch, layers]);

    const savePalette = useCallback(async (layerId) => {
        const layer = layers.find(l => l.id === layerId);
        if (!layer || !layer.palette) return;

        const paletteName = prompt("Enter a name for this palette:");
        if (!paletteName || paletteName.trim() === "") return;

        const palette = layer.palette;
        const paletteData = {
            name: paletteName.trim(),
            colors: palette.colors || ['#FF0000'],
            positions: palette.positions || [0],
            colorRule: palette.colorRule || -1,
            interMode: palette.interMode || 1,
            wrapMode: palette.wrapMode || 0,
            segmentation: palette.segmentation || 0
        };

        try {
            const response = await deviceFetch('/save_palette', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify(paletteData)
            });

            if (response.ok) {
                const result = await response.json();
                console.log("Palette saved successfully:", result.message);
                // TODO: Show success message to user
            } else {
                const error = await response.json();
                console.error("Failed to save palette:", error.error);
                setError("Failed to save palette: " + error.error);
            }
        } catch (err) {
            console.error("Error saving palette:", err);
            setError("Error saving palette");
        }
    }, [deviceFetch, layers]);

    const loadPredefinedPalette = useCallback(async (layerId, paletteIndex) => {
        if (!paletteIndex || paletteIndex === "") return;
        
        try {
            const response = await deviceFetch(`/get_palette_colors?index=${paletteIndex}&layer=${layerId}`);
            
            if (response.ok) {
                const paletteData = await response.json();
                
                // Use updatePalette to sync with server and update local state
                await updatePalette(layerId, {
                    colors: paletteData.colors,
                    positions: paletteData.positions,
                    colorRule: paletteData.colorRule,
                    interMode: paletteData.interMode,
                    wrapMode: paletteData.wrapMode,
                    segmentation: paletteData.segmentation
                });
                
                console.log("Loaded predefined palette:", paletteIndex);
            } else {
                const error = await response.json();
                console.error("Failed to load palette:", error.error);
                setError("Failed to load palette: " + error.error);
            }
        } catch (err) {
            console.error("Error loading palette:", err);
            setError("Error loading palette");
        }
    }, [deviceFetch, updatePalette]);

    const addLayer = useCallback(async () => {
        try {
            const response = await deviceFetch('/add_layer', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
            });
            
            if (response.ok) {
                console.log("Layer added successfully");
                // Refresh the layers list to include the new layer
                const layersResponse = await deviceFetch('/get_layers');
                const data = parseLayersResponse(await layersResponse.json());
                setLayers(data);
            } else {
                const errorText = await response.text();
                console.error("Failed to add layer:", errorText);
                setError("Failed to add layer: " + errorText);
            }
        } catch (err) {
            console.error("Error adding layer:", err);
            setError("Error adding layer");
        }
    }, [deviceFetch]);

    const addColorToLayer = useCallback(async (layerId) => {
        const layer = layers.find(l => l.id === layerId);
        if (!layer) return;
        
        const currentPalette = layer.palette || { colors: ['#FF0000'], positions: [0] };
        const currentColors = [...currentPalette.colors];
        const currentPositions = [...currentPalette.positions];
        
        // Add a new red color
        const newColor = '#FF0000';
        currentColors.push(newColor);
        
        // Calculate position for the new color (append at the end)
        const newPosition = currentColors.length === 1 ? 0.0 : 1.0;
        currentPositions.push(newPosition);
        
        // Recalculate positions to be evenly distributed if we have multiple colors
        if (currentColors.length > 1) {
            for (let i = 0; i < currentColors.length; i++) {
                currentPositions[i] = i / (currentColors.length - 1);
            }
        }
        
        // Update the palette via the server
        await updatePalette(layerId, {
            colors: currentColors,
            positions: currentPositions
        });
        
        console.log("Added color to layer", layerId);
    }, [layers, updatePalette]);

    const removeColorFromLayer = useCallback(async (layerId, colorIndex) => {
        const layer = layers.find(l => l.id === layerId);
        if (!layer) return;
        
        const currentPalette = layer.palette || { colors: ['#FF0000'], positions: [0] };
        const currentColors = [...currentPalette.colors];
        const currentPositions = [...currentPalette.positions];
        
        // Don't allow removing the last color
        if (currentColors.length <= 1) {
            console.warn("Cannot remove the last color from palette");
            return;
        }
        
        // Remove the color and position at the specified index
        if (colorIndex >= 0 && colorIndex < currentColors.length) {
            currentColors.splice(colorIndex, 1);
            currentPositions.splice(colorIndex, 1);
            
            // Recalculate positions to be evenly distributed
            if (currentColors.length > 1) {
                for (let i = 0; i < currentColors.length; i++) {
                    currentPositions[i] = i / (currentColors.length - 1);
                }
            } else {
                // Single color should be at position 0
                currentPositions[0] = 0.0;
            }
            
            // Update the palette via the server
            await updatePalette(layerId, {
                colors: currentColors,
                positions: currentPositions
            });
            
            console.log("Removed color from layer", layerId, "at index", colorIndex);
        }
    }, [layers, updatePalette]);

    const updateLayerColor = useCallback(async (layerId, colorIndex, newColor) => {
        const layer = layers.find(l => l.id === layerId);
        if (!layer) return;
        
        const currentPalette = layer.palette || { colors: ['#FF0000'], positions: [0] };
        const currentColors = [...currentPalette.colors];
        const currentPositions = [...currentPalette.positions];
        
        // Update the color at the specified index
        if (colorIndex >= 0 && colorIndex < currentColors.length) {
            currentColors[colorIndex] = newColor;
            
            // Update the palette via the server
            await updatePalette(layerId, {
                colors: currentColors,
                positions: currentPositions
            });
            
            console.log("Updated color in layer", layerId, "at index", colorIndex, "to", newColor);
        }
    }, [layers, updatePalette]);

    const updateColorPosition = useCallback(async (layerId, colorIndex, newPosition) => {
        const layer = layers.find(l => l.id === layerId);
        if (!layer) return;
        
        const currentPalette = layer.palette || { colors: ['#FF0000'], positions: [0] };
        const currentColors = [...currentPalette.colors];
        const currentPositions = [...currentPalette.positions];
        
        // Validate and clamp position between 0 and 1
        const clampedPosition = Math.max(0, Math.min(1, parseFloat(newPosition) || 0));
        
        // Update the position at the specified index
        if (colorIndex >= 0 && colorIndex < currentPositions.length) {
            currentPositions[colorIndex] = clampedPosition;
            
            // Sort colors and positions based on positions to maintain proper order
            const colorPositionPairs = currentColors.map((color, index) => ({
                color,
                position: currentPositions[index]
            }));
            
            // Sort by position
            colorPositionPairs.sort((a, b) => a.position - b.position);
            
            // Extract sorted colors and positions
            const sortedColors = colorPositionPairs.map(pair => pair.color);
            const sortedPositions = colorPositionPairs.map(pair => pair.position);
            
            // Update the palette via the server
            await updatePalette(layerId, {
                colors: sortedColors,
                positions: sortedPositions
            });
            
            console.log("Updated position in layer", layerId, "at index", colorIndex, "to", clampedPosition);
        }
    }, [layers, updatePalette]);

    const updateEasing = useCallback(async (layerId, easing) => {
        try {
            const formData = new FormData();
            formData.append('layer', layerId.toString());
            formData.append('ease', easing.toString());
            const response = await deviceFetch('/update_ease', {
                method: 'POST',
                body: formData,
            });
            
            if (response.ok) {
                // Update the layer's easing in local state
                setLayers(prev => 
                    prev.map(layer => 
                        layer.id === layerId 
                            ? { ...layer, easing }
                            : layer
                    )
                );
                
                console.log("Easing updated successfully");
            } else {
                const errorText = await response.text();
                console.error("Failed to update easing:", errorText);
                setError("Failed to update easing: " + errorText);
            }
        } catch (err) {
            console.error("Error updating easing:", err);
            setError("Error updating easing");
        }
    }, [deviceFetch]);

    const updateBehaviourFlags = useCallback(async (layerId, flags) => {
        try {
            const formData = new FormData();
            formData.append('layer', layerId.toString());
            formData.append('flags', flags.toString());
            const response = await deviceFetch('/update_behaviour_flags', {
                method: 'POST',
                body: formData,
            });
            
            if (response.ok) {
                // Update the layer's behaviour flags in local state
                setLayers(prev => 
                    prev.map(layer => 
                        layer.id === layerId 
                            ? { ...layer, behaviourFlags: flags }
                            : layer
                    )
                );
                
                console.log("Behaviour flags updated successfully");
            } else {
                const errorText = await response.text();
                console.error("Failed to update behaviour flags:", errorText);
                setError("Failed to update behaviour flags: " + errorText);
            }
        } catch (err) {
            console.error("Error updating behaviour flags:", err);
            setError("Error updating behaviour flags");
        }
    }, [deviceFetch]);

    const resetLayer = useCallback(async (layerId) => {
        try {
            const formData = new FormData();
            formData.append('layer', layerId.toString());
            const response = await deviceFetch('/reset_layer', {
                method: 'POST',
                body: formData
            });
            
            if (response.ok) {
                // Refresh layers data to get the reset state
                const layersResponse = await deviceFetch('/get_layers');
                const data = parseLayersResponse(await layersResponse.json());
                setLayers(data);
                console.log(`Layer ${layerId} reset successfully`);
            } else {
                const errorText = await response.text();
                console.error("Failed to reset layer:", errorText);
                setError("Failed to reset layer: " + errorText);
            }
        } catch (err) {
            console.error("Error resetting layer:", err);
            setError("Error resetting layer");
        }
    }, [deviceFetch]);

    return {
        layers,
        loading,
        error,
        toggleLayerVisibility,
        updateLayerBrightness,
        updateLayerSpeed,
        updateLayerOffset,
        removeLayer,
        updatePalette,
        savePalette,
        loadPredefinedPalette,
        addLayer,
        addColorToLayer,
        removeColorFromLayer,
        updateLayerColor,
        updateColorPosition,
        updateEasing,
        updateBehaviourFlags,
        resetLayer,
    };
};

export default useLayers;
