import { useCallback, useEffect, useState } from 'react';
import { useDevice } from '../contexts/DeviceContext.jsx';
import { parseLayersResponse } from '../models/apiModels';

const isAbortError = (error) => error?.name === 'AbortError';

const parseResponseError = async (response, fallbackMessage) => {
    const contentType = response.headers.get('content-type') || '';
    try {
        if (contentType.includes('application/json')) {
            const payload = await response.json();
            if (typeof payload?.error === 'string' && payload.error.trim().length > 0) {
                return payload.error;
            }
            if (typeof payload?.message === 'string' && payload.message.trim().length > 0) {
                return payload.message;
            }
        }

        const text = await response.text();
        if (text.trim().length > 0) {
            return text;
        }
    } catch {
        // Ignore parse errors and fall back to status code.
    }

    return `${fallbackMessage} (${response.status})`;
};

const useLayers = () => {
    const [layers, setLayers] = useState([]);
    const [loading, setLoading] = useState(true);
    const [error, setError] = useState(null);
    const { deviceFetch, selectedDevice } = useDevice();

    const ensureOk = useCallback(async (response, fallbackMessage) => {
        if (!response.ok) {
            throw new Error(await parseResponseError(response, fallbackMessage));
        }
        return response;
    }, []);

    const fetchLayers = useCallback(async (signal) => {
        const response = await deviceFetch('/get_layers', { signal });
        await ensureOk(response, 'Failed to fetch layers');
        return parseLayersResponse(await response.json());
    }, [deviceFetch, ensureOk]);

    const refreshLayers = useCallback(async () => {
        if (!selectedDevice) {
            setLayers([]);
            setError('No device selected');
            return [];
        }

        const data = await fetchLayers();
        setLayers(data);
        setError(null);
        return data;
    }, [selectedDevice, fetchLayers]);

    useEffect(() => {
        if (!selectedDevice) {
            setLayers([]);
            setLoading(false);
            setError('No device selected');
            return;
        }

        const controller = new AbortController();
        let cancelled = false;

        const load = async () => {
            setLoading(true);
            setError(null);
            try {
                const data = await fetchLayers(controller.signal);
                if (!cancelled) {
                    setLayers(data);
                }
            } catch (err) {
                if (cancelled || isAbortError(err)) {
                    return;
                }
                setError(err.message || 'Failed to fetch layers');
            } finally {
                if (!cancelled) {
                    setLoading(false);
                }
            }
        };

        load();

        return () => {
            cancelled = true;
            controller.abort();
        };
    }, [selectedDevice, fetchLayers]);

    const toggleLayerVisibility = useCallback(async (layerId) => {
        const target = layers.find((layer) => layer.id === layerId);
        if (!target) return;

        const previous = target.visible;
        const nextVisibility = !previous;

        setLayers((prev) => prev.map((layer) => (
            layer.id === layerId ? { ...layer, visible: nextVisibility } : layer
        )));

        try {
            const formData = new FormData();
            formData.append('layer', layerId.toString());
            formData.append('visible', nextVisibility ? 'true' : 'false');
            const response = await deviceFetch('/toggle_visible', {
                method: 'POST',
                body: formData,
            });
            await ensureOk(response, 'Failed to update layer visibility');
            setError(null);
        } catch (err) {
            setLayers((prev) => prev.map((layer) => (
                layer.id === layerId ? { ...layer, visible: previous } : layer
            )));
            setError(err.message || 'Failed to update layer visibility');
            throw err;
        }
    }, [deviceFetch, ensureOk, layers]);

    const updateLayerBrightness = useCallback(async (layerId, brightness) => {
        const target = layers.find((layer) => layer.id === layerId);
        if (!target) return;

        const nextValue = Number.isFinite(brightness) ? brightness : target.brightness;
        const previous = target.brightness;

        setLayers((prev) => prev.map((layer) => (
            layer.id === layerId ? { ...layer, brightness: nextValue } : layer
        )));

        try {
            const formData = new FormData();
            formData.append('layer', layerId.toString());
            formData.append('value', nextValue.toString());
            const response = await deviceFetch('/update_layer_brightness', {
                method: 'POST',
                body: formData,
            });
            await ensureOk(response, 'Failed to update layer brightness');
            setError(null);
        } catch (err) {
            setLayers((prev) => prev.map((layer) => (
                layer.id === layerId ? { ...layer, brightness: previous } : layer
            )));
            setError(err.message || 'Failed to update layer brightness');
            throw err;
        }
    }, [deviceFetch, ensureOk, layers]);

    const updateLayerSpeed = useCallback(async (layerId, speed) => {
        const target = layers.find((layer) => layer.id === layerId);
        if (!target) return;

        const nextValue = Number.isFinite(speed) ? speed : target.speed;
        const previous = target.speed;

        setLayers((prev) => prev.map((layer) => (
            layer.id === layerId ? { ...layer, speed: nextValue } : layer
        )));

        try {
            const formData = new FormData();
            formData.append('layer', layerId.toString());
            formData.append('value', nextValue.toString());
            const response = await deviceFetch('/update_speed', {
                method: 'POST',
                body: formData,
            });
            await ensureOk(response, 'Failed to update layer speed');
            setError(null);
        } catch (err) {
            setLayers((prev) => prev.map((layer) => (
                layer.id === layerId ? { ...layer, speed: previous } : layer
            )));
            setError(err.message || 'Failed to update layer speed');
            throw err;
        }
    }, [deviceFetch, ensureOk, layers]);

    const updateLayerOffset = useCallback(async (layerId, offset) => {
        const target = layers.find((layer) => layer.id === layerId);
        if (!target) return;

        const nextValue = Number.isFinite(offset) ? offset : target.offset;
        const previous = target.offset;

        setLayers((prev) => prev.map((layer) => (
            layer.id === layerId ? { ...layer, offset: nextValue } : layer
        )));

        try {
            const formData = new FormData();
            formData.append('layer', layerId.toString());
            formData.append('offset', nextValue.toString());
            const response = await deviceFetch('/update_layer_offset', {
                method: 'POST',
                body: formData,
            });
            await ensureOk(response, 'Failed to update layer offset');
            setError(null);
        } catch (err) {
            setLayers((prev) => prev.map((layer) => (
                layer.id === layerId ? { ...layer, offset: previous } : layer
            )));
            setError(err.message || 'Failed to update layer offset');
            throw err;
        }
    }, [deviceFetch, ensureOk, layers]);

    const removeLayer = useCallback(async (layerId) => {
        try {
            const formData = new FormData();
            formData.append('layer', layerId.toString());
            const response = await deviceFetch('/remove_layer', {
                method: 'POST',
                body: formData,
            });
            await ensureOk(response, 'Failed to remove layer');
            setLayers((prev) => prev.filter((layer) => layer.id !== layerId));
            setError(null);
        } catch (err) {
            setError(err.message || 'Failed to remove layer');
            throw err;
        }
    }, [deviceFetch, ensureOk]);

    const updatePalette = useCallback(async (layerId, paletteData) => {
        const layer = layers.find((entry) => entry.id === layerId);
        if (!layer) return;

        const previousPalette = layer.palette;
        const mergedPalette = {
            ...previousPalette,
            ...paletteData,
        };

        setLayers((prev) => prev.map((entry) => (
            entry.id === layerId ? { ...entry, palette: mergedPalette } : entry
        )));

        try {
            const formData = new FormData();
            formData.append('layer', layerId.toString());

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
            if (Array.isArray(paletteData.colors) && paletteData.colors.length > 0) {
                formData.append('colors', JSON.stringify(paletteData.colors));
                if (Array.isArray(paletteData.positions) && paletteData.positions.length === paletteData.colors.length) {
                    formData.append('positions', JSON.stringify(paletteData.positions));
                }
            }

            const response = await deviceFetch('/update_palette', {
                method: 'POST',
                body: formData,
            });
            await ensureOk(response, 'Failed to update palette');
            setError(null);
        } catch (err) {
            setLayers((prev) => prev.map((entry) => (
                entry.id === layerId ? { ...entry, palette: previousPalette } : entry
            )));
            setError(err.message || 'Failed to update palette');
            throw err;
        }
    }, [deviceFetch, ensureOk, layers]);

    const savePalette = useCallback(async (layerId, paletteName) => {
        const layer = layers.find((entry) => entry.id === layerId);
        if (!layer || !layer.palette) return null;

        const trimmedName = String(paletteName || '').trim();
        if (!trimmedName) {
            throw new Error('Palette name is required.');
        }

        const palette = layer.palette;
        const payload = {
            name: trimmedName,
            colors: palette.colors || ['#FF0000'],
            positions: palette.positions || [0],
            colorRule: palette.colorRule ?? -1,
            interMode: palette.interMode ?? 1,
            wrapMode: palette.wrapMode ?? 0,
            segmentation: palette.segmentation ?? 0,
        };

        const response = await deviceFetch('/save_palette', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify(payload),
        });
        await ensureOk(response, 'Failed to save palette');
        setError(null);
        return await response.json().catch(() => ({ success: true }));
    }, [deviceFetch, ensureOk, layers]);

    const loadPredefinedPalette = useCallback(async (layerId, paletteIndex) => {
        if (!paletteIndex && paletteIndex !== 0 && paletteIndex !== '0') {
            return;
        }

        const response = await deviceFetch(`/get_palette_colors?index=${paletteIndex}&layer=${layerId}`);
        await ensureOk(response, 'Failed to load palette');
        const paletteData = await response.json();

        await updatePalette(layerId, {
            colors: paletteData.colors,
            positions: paletteData.positions,
            colorRule: paletteData.colorRule,
            interMode: paletteData.interMode,
            wrapMode: paletteData.wrapMode,
            segmentation: paletteData.segmentation,
        });
    }, [deviceFetch, ensureOk, updatePalette]);

    const addLayer = useCallback(async () => {
        const response = await deviceFetch('/add_layer', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
        });

        await ensureOk(response, 'Failed to add layer');
        await refreshLayers();
        setError(null);
    }, [deviceFetch, ensureOk, refreshLayers]);

    const addColorToLayer = useCallback(async (layerId) => {
        const layer = layers.find((entry) => entry.id === layerId);
        if (!layer) return;

        const currentPalette = layer.palette || { colors: ['#FF0000'], positions: [0] };
        const nextColors = [...currentPalette.colors, '#FF0000'];
        const nextPositions = nextColors.map((_, index) => (
            nextColors.length === 1 ? 0 : index / (nextColors.length - 1)
        ));

        await updatePalette(layerId, {
            colors: nextColors,
            positions: nextPositions,
        });
    }, [layers, updatePalette]);

    const removeColorFromLayer = useCallback(async (layerId, colorIndex) => {
        const layer = layers.find((entry) => entry.id === layerId);
        if (!layer) return;

        const currentPalette = layer.palette || { colors: ['#FF0000'], positions: [0] };
        if (currentPalette.colors.length <= 1) {
            throw new Error('Cannot remove the last color from a palette.');
        }

        const nextColors = currentPalette.colors.filter((_, index) => index !== colorIndex);
        const nextPositions = nextColors.map((_, index) => (
            nextColors.length === 1 ? 0 : index / (nextColors.length - 1)
        ));

        await updatePalette(layerId, {
            colors: nextColors,
            positions: nextPositions,
        });
    }, [layers, updatePalette]);

    const updateLayerColor = useCallback(async (layerId, colorIndex, newColor) => {
        const layer = layers.find((entry) => entry.id === layerId);
        if (!layer) return;

        const currentPalette = layer.palette || { colors: ['#FF0000'], positions: [0] };
        if (colorIndex < 0 || colorIndex >= currentPalette.colors.length) {
            return;
        }

        const nextColors = [...currentPalette.colors];
        nextColors[colorIndex] = newColor;

        await updatePalette(layerId, {
            colors: nextColors,
            positions: [...currentPalette.positions],
        });
    }, [layers, updatePalette]);

    const updateColorPosition = useCallback(async (layerId, colorIndex, newPosition) => {
        const layer = layers.find((entry) => entry.id === layerId);
        if (!layer) return;

        const currentPalette = layer.palette || { colors: ['#FF0000'], positions: [0] };
        if (colorIndex < 0 || colorIndex >= currentPalette.positions.length) {
            return;
        }

        const clamped = Math.max(0, Math.min(1, Number(newPosition) || 0));
        const nextPositions = [...currentPalette.positions];
        nextPositions[colorIndex] = clamped;

        const sorted = currentPalette.colors
            .map((color, index) => ({ color, position: nextPositions[index] }))
            .sort((a, b) => a.position - b.position);

        await updatePalette(layerId, {
            colors: sorted.map((entry) => entry.color),
            positions: sorted.map((entry) => entry.position),
        });
    }, [layers, updatePalette]);

    const updateEasing = useCallback(async (layerId, easing) => {
        const layer = layers.find((entry) => entry.id === layerId);
        if (!layer) return;

        const previous = layer.easing;
        setLayers((prev) => prev.map((entry) => (
            entry.id === layerId ? { ...entry, easing } : entry
        )));

        try {
            const formData = new FormData();
            formData.append('layer', layerId.toString());
            formData.append('ease', easing.toString());
            const response = await deviceFetch('/update_ease', {
                method: 'POST',
                body: formData,
            });
            await ensureOk(response, 'Failed to update easing');
            setError(null);
        } catch (err) {
            setLayers((prev) => prev.map((entry) => (
                entry.id === layerId ? { ...entry, easing: previous } : entry
            )));
            setError(err.message || 'Failed to update easing');
            throw err;
        }
    }, [deviceFetch, ensureOk, layers]);

    const updateBlendMode = useCallback(async (layerId, blendMode) => {
        const layer = layers.find((entry) => entry.id === layerId);
        if (!layer) return;

        const previous = layer.blendMode;
        setLayers((prev) => prev.map((entry) => (
            entry.id === layerId ? { ...entry, blendMode } : entry
        )));

        try {
            const formData = new FormData();
            formData.append('layer', layerId.toString());
            formData.append('mode', blendMode.toString());
            const response = await deviceFetch('/update_blend_mode', {
                method: 'POST',
                body: formData,
            });
            await ensureOk(response, 'Failed to update blend mode');
            setError(null);
        } catch (err) {
            setLayers((prev) => prev.map((entry) => (
                entry.id === layerId ? { ...entry, blendMode: previous } : entry
            )));
            setError(err.message || 'Failed to update blend mode');
            throw err;
        }
    }, [deviceFetch, ensureOk, layers]);

    const updateFadeSpeed = useCallback(async (layerId, fadeSpeed) => {
        const layer = layers.find((entry) => entry.id === layerId);
        if (!layer) return;

        const previous = layer.fadeSpeed;
        setLayers((prev) => prev.map((entry) => (
            entry.id === layerId ? { ...entry, fadeSpeed } : entry
        )));

        try {
            const formData = new FormData();
            formData.append('layer', layerId.toString());
            formData.append('value', fadeSpeed.toString());
            const response = await deviceFetch('/update_fade_speed', {
                method: 'POST',
                body: formData,
            });
            await ensureOk(response, 'Failed to update fade speed');
            setError(null);
        } catch (err) {
            setLayers((prev) => prev.map((entry) => (
                entry.id === layerId ? { ...entry, fadeSpeed: previous } : entry
            )));
            setError(err.message || 'Failed to update fade speed');
            throw err;
        }
    }, [deviceFetch, ensureOk, layers]);

    const updateBehaviourFlags = useCallback(async (layerId, flags) => {
        const layer = layers.find((entry) => entry.id === layerId);
        if (!layer) return;

        const previous = layer.behaviourFlags;
        setLayers((prev) => prev.map((entry) => (
            entry.id === layerId ? { ...entry, behaviourFlags: flags } : entry
        )));

        try {
            const formData = new FormData();
            formData.append('layer', layerId.toString());
            formData.append('flags', flags.toString());
            const response = await deviceFetch('/update_behaviour_flags', {
                method: 'POST',
                body: formData,
            });
            await ensureOk(response, 'Failed to update behaviour flags');
            setError(null);
        } catch (err) {
            setLayers((prev) => prev.map((entry) => (
                entry.id === layerId ? { ...entry, behaviourFlags: previous } : entry
            )));
            setError(err.message || 'Failed to update behaviour flags');
            throw err;
        }
    }, [deviceFetch, ensureOk, layers]);

    const resetLayer = useCallback(async (layerId) => {
        const formData = new FormData();
        formData.append('layer', layerId.toString());
        const response = await deviceFetch('/reset_layer', {
            method: 'POST',
            body: formData,
        });

        await ensureOk(response, 'Failed to reset layer');
        await refreshLayers();
        setError(null);
    }, [deviceFetch, ensureOk, refreshLayers]);

    return {
        layers,
        loading,
        error,
        refreshLayers,
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
        updateFadeSpeed,
        updateBlendMode,
        updateBehaviourFlags,
        resetLayer,
    };
};

export default useLayers;
