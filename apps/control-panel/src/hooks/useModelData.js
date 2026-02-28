import { useState, useCallback, useEffect } from 'react';
import { useDevice } from '../contexts/DeviceContext.jsx';
import { parseModelDataResponse } from '../models/apiModels';

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
        // ignore and use fallback below
    }

    return `${fallbackMessage} (${response.status})`;
};

export const useModelData = () => {
    const [modelData, setModelData] = useState(null);
    const [loading, setLoading] = useState(false);
    const [error, setError] = useState(null);
    const { deviceFetch, selectedDevice } = useDevice();

    const fetchModelData = useCallback(async (signal) => {
        if (!selectedDevice) {
            return null;
        }

        const response = await deviceFetch('/get_model', { signal });
        if (!response.ok) {
            throw new Error(await parseResponseError(response, 'Failed to fetch model data'));
        }

        return parseModelDataResponse(await response.json());
    }, [selectedDevice, deviceFetch]);

    const loadModelData = useCallback(async (signal) => {
        setLoading(true);
        setError(null);
        try {
            const data = await fetchModelData(signal);
            setModelData(data);
            return data;
        } catch (err) {
            if (isAbortError(err)) {
                return null;
            }
            setError(err.message || 'Failed to fetch model data');
            throw err;
        } finally {
            setLoading(false);
        }
    }, [fetchModelData]);

    const refreshModelData = useCallback(() => {
        return loadModelData();
    }, [loadModelData]);

    useEffect(() => {
        if (!selectedDevice) {
            setModelData(null);
            setLoading(false);
            setError('No device selected');
            return;
        }

        const controller = new AbortController();
        loadModelData(controller.signal).catch((err) => {
            if (!isAbortError(err)) {
                console.error('Failed to fetch model data:', err);
            }
        });

        return () => {
            controller.abort();
        };
    }, [selectedDevice, loadModelData]);

    return {
        modelData,
        loading,
        error,
        refreshModelData,
        fetchModelData,
    };
};
