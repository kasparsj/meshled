import { useEffect, useState } from 'react';
import { useDevice } from '../contexts/DeviceContext';
import { normalizeDeviceInfo } from '../utils/deviceInfo';

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
        // Ignore parse errors and use fallback message.
    }

    return `${fallbackMessage} (${response.status})`;
};

const useDeviceInfo = () => {
    const [deviceInfo, setDeviceInfo] = useState(null);
    const [loading, setLoading] = useState(true);
    const [error, setError] = useState(null);
    const { deviceFetch, selectedDevice } = useDevice();

    useEffect(() => {
        if (!selectedDevice) {
            setDeviceInfo(null);
            setLoading(false);
            setError('No device selected');
            return;
        }

        const controller = new AbortController();
        let cancelled = false;

        const fetchDeviceInfo = async () => {
            try {
                setLoading(true);
                setError(null);
                const response = await deviceFetch('/device_info', {
                    signal: controller.signal,
                });
                if (!response.ok) {
                    throw new Error(await parseResponseError(response, 'Failed to load device info'));
                }
                const data = await response.json();
                if (!cancelled) {
                    setDeviceInfo(normalizeDeviceInfo(data));
                }
            } catch (err) {
                if (cancelled || isAbortError(err)) {
                    return;
                }
                setError(err.message || 'Failed to load device info.');
            } finally {
                if (!cancelled) {
                    setLoading(false);
                }
            }
        };

        fetchDeviceInfo();

        return () => {
            cancelled = true;
            controller.abort();
        };
    }, [selectedDevice, deviceFetch]);

    return { deviceInfo, loading, error };
};

export default useDeviceInfo;
