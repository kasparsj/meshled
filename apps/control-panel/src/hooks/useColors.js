import { useState, useCallback, useEffect } from 'react';
import { useDevice } from '../contexts/DeviceContext.jsx';
import { parseColorsResponse } from '../models/apiModels';

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
        // Ignore and fall through to fallback.
    }

    return `${fallbackMessage} (${response.status})`;
};

export const useColors = () => {
    const [colors, setColors] = useState([]);
    const [allPixels, setAllPixels] = useState([]);
    const [loading, setLoading] = useState(false);
    const [error, setError] = useState(null);
    const { deviceFetch, selectedDevice } = useDevice();

    const fetchColors = useCallback(async (signal) => {
        if (!selectedDevice) {
            return { colors: [], step: 1, totalPixels: 0 };
        }

        const response = await deviceFetch('/get_colors', { signal });
        if (!response.ok) {
            throw new Error(await parseResponseError(response, 'Failed to fetch colors'));
        }

        return parseColorsResponse(await response.json());
    }, [selectedDevice, deviceFetch]);

    const expandPixels = useCallback((colorData) => {
        const { colors: colorList, step, totalPixels } = colorData;
        const expandedPixels = [];

        if (colorList.length > 0 && totalPixels > 0) {
            for (let i = 0; i < totalPixels; i++) {
                const colorIndex = Math.floor(i / step);
                const color = colorList[Math.min(colorIndex, colorList.length - 1)];
                expandedPixels.push(color);
            }
        }

        return expandedPixels;
    }, []);

    const loadColors = useCallback(async (signal) => {
        setLoading(true);
        setError(null);
        try {
            const colorData = await fetchColors(signal);
            setColors(colorData.colors);
            setAllPixels(expandPixels(colorData));
            return colorData;
        } catch (err) {
            if (isAbortError(err)) {
                return { colors: [], step: 1, totalPixels: 0 };
            }
            setError(err.message || 'Failed to fetch colors');
            throw err;
        } finally {
            setLoading(false);
        }
    }, [fetchColors, expandPixels]);

    const refreshColors = useCallback(() => {
        return loadColors();
    }, [loadColors]);

    useEffect(() => {
        if (!selectedDevice) {
            setColors([]);
            setAllPixels([]);
            setLoading(false);
            setError('No device selected');
            return;
        }

        const controller = new AbortController();
        loadColors(controller.signal).catch((err) => {
            if (!isAbortError(err)) {
                console.error('Failed to fetch colors:', err);
            }
        });

        return () => {
            controller.abort();
        };
    }, [selectedDevice, loadColors]);

    return {
        colors,
        allPixels,
        loading,
        error,
        refreshColors,
        fetchColors,
        expandPixels,
    };
};
