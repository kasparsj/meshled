import { useState, useCallback, useEffect } from 'react';
import { useDevice } from '../contexts/DeviceContext.jsx';
import { parseColorsResponse } from '../models/apiModels';

export const useColors = () => {
    const [colors, setColors] = useState([]);
    const [allPixels, setAllPixels] = useState([]);
    const [loading, setLoading] = useState(false);
    const [error, setError] = useState(null);
    const { deviceFetch, selectedDevice } = useDevice();

    const fetchColors = useCallback(async () => {
        if (!selectedDevice) {
            return { colors: [], step: 1, totalPixels: 0 };
        }
        try {
            const response = await deviceFetch('/get_colors');
            return parseColorsResponse(await response.json());
        } catch (error) {
            console.error('Failed to fetch colors:', error);
            return { colors: [], step: 1, totalPixels: 0 };
        }
    }, [selectedDevice, deviceFetch]);

    const expandPixels = useCallback((colorData) => {
        const { colors, step, totalPixels } = colorData;
        const expandedPixels = [];
        
        if (colors.length > 0 && totalPixels > 0) {
            for (let i = 0; i < totalPixels; i++) {
                const colorIndex = Math.floor(i / step);
                const color = colors[Math.min(colorIndex, colors.length - 1)];
                expandedPixels.push(color);
            }
        }
        
        return expandedPixels;
    }, []);

    const loadColors = useCallback(async () => {
        setLoading(true);
        try {
            const colorData = await fetchColors();
            setColors(colorData.colors);
            const expandedPixels = expandPixels(colorData);
            setAllPixels(expandedPixels);
        } finally {
            setLoading(false);
        }
    }, [fetchColors, expandPixels]);

    const refreshColors = useCallback(() => {
        return loadColors();
    }, [loadColors]);

    // Auto-load colors when device changes
    useEffect(() => {
        if (!selectedDevice) {
            setColors([]);
            setLoading(false);
            setError('No device selected');
            return;
        }

        loadColors();
    }, [selectedDevice, loadColors]);

    return {
        colors,
        allPixels,
        loading,
        error,
        refreshColors,
        fetchColors,
        expandPixels
    };
};
