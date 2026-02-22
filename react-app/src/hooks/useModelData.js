import { useState, useCallback, useEffect } from 'react';
import { useDevice } from '../contexts/DeviceContext.jsx';

export const useModelData = () => {
    const [modelData, setModelData] = useState(null);
    const [loading, setLoading] = useState(false);
    const [error, setError] = useState(null);
    const { deviceFetch, selectedDevice } = useDevice();

    const fetchModelData = useCallback(async () => {
        if (!selectedDevice) {
            return null;
        }
        try {
            const response = await deviceFetch('/get_model');
            const data = await response.json();
            return data;
        } catch (error) {
            console.error('Failed to fetch model data:', error);
            return null;
        }
    }, [selectedDevice, deviceFetch]);

    const loadModelData = useCallback(async () => {
        setLoading(true);
        try {
            const data = await fetchModelData();
            setModelData(data);
        } finally {
            setLoading(false);
        }
    }, [fetchModelData]);

    const refreshModelData = useCallback(() => {
        return loadModelData();
    }, [loadModelData]);

    // Auto-load model data when device changes
    useEffect(() => {
        if (!selectedDevice) {
            setModelData(null);
            setLoading(false);
            setError('No device selected');
            return;
        }

        loadModelData();
    }, [selectedDevice, loadModelData]);

    return {
        modelData,
        loading,
        error,
        refreshModelData,
        fetchModelData
    };
};