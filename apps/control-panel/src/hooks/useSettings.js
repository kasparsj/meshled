import { useState, useCallback } from 'react';
import { useDevice } from '../contexts/DeviceContext';

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
        // Ignore parse errors and use status fallback.
    }

    return `${fallbackMessage} (${response.status})`;
};

const ensureOk = async (response, fallbackMessage) => {
    if (!response.ok) {
        throw new Error(await parseResponseError(response, fallbackMessage));
    }
    return response;
};

const toParamString = (value, key) => {
    if (typeof value === 'boolean') {
        return value ? '1' : '0';
    }

    if (typeof value === 'number') {
        if (!Number.isFinite(value)) {
            throw new Error(`Invalid numeric value for ${key}`);
        }
        return `${value}`;
    }

    return String(value ?? '');
};

const useSettings = () => {
    const { deviceFetch } = useDevice();
    const [loading, setLoading] = useState(false);
    const [error, setError] = useState(null);

    const LED_TYPES = {
        0: 'WS2812',
        1: 'WS2811',
        2: 'WS2815',
        3: 'WS2813',
        4: 'WS2816',
        5: 'SK6812',
        6: 'TM1829',
        7: 'APA106',
        8: 'WS2814',
        9: 'TM1814',
        10: 'TM1914',
        11: 'TM1812',
        12: 'TM1809',
        13: 'TM1804',
        14: 'TM1803',
        15: 'UCS1903',
        16: 'UCS1903B',
        17: 'UCS1904',
        18: 'UCS2903',
        19: 'SK6822',
        20: 'APA104',
        21: 'WS2811_400',
        22: 'GS1903',
        23: 'GW6205',
        24: 'GW6205_400',
        25: 'LPD1886',
        26: 'LPD1886_8BIT',
        27: 'PL9823',
        28: 'UCS1912',
        29: 'SM16703',
        30: 'SM16824E',
    };

    const COLOR_ORDERS = {
        6: 'RGB',
        18: 'GRB',
        155: 'RGBW',
        203: 'GRBW',
    };

    const LED_LIBRARIES = {
        0: 'NeoPixelBus',
        1: 'FastLED',
    };

    const OBJECT_TYPES = {
        0: 'Heptagon919',
        1: 'Line',
        2: 'Triangle',
        3: 'Heptagon3024',
        4: 'Custom (Imported Topology)',
    };

    const getSettings = useCallback(async () => {
        setLoading(true);
        setError(null);

        try {
            const response = await deviceFetch('/get_settings');
            await ensureOk(response, 'Failed to fetch settings');
            return await response.json();
        } catch (err) {
            setError(err.message);
            throw err;
        } finally {
            setLoading(false);
        }
    }, [deviceFetch]);

    const saveSettings = useCallback(async (settings) => {
        setLoading(true);
        setError(null);

        try {
            const formData = new FormData();

            const fieldMapping = {
                maxBrightness: 'max_brightness',
                deviceHostname: 'hostname',
                pixelCount1: 'pixel_count1',
                pixelCount2: 'pixel_count2',
                pixelPin1: 'pixel_pin1',
                pixelPin2: 'pixel_pin2',
                pixelDensity: 'pixel_density',
                ledType: 'led_type',
                colorOrder: 'color_order',
                ledLibrary: 'led_library',
                objectType: 'object_type',
                oscEnabled: 'osc_enabled',
                oscPort: 'osc_port',
                otaEnabled: 'ota_enabled',
                otaPort: 'ota_port',
                otaPassword: 'ota_password',
                apiAuthEnabled: 'api_auth_enabled',
                apiAuthToken: 'api_auth_token',
            };

            const optionalSecretFields = new Set(['otaPassword', 'apiAuthToken']);

            for (const [key, value] of Object.entries(settings)) {
                if (optionalSecretFields.has(key)) {
                    const raw = value == null ? '' : String(value).trim();
                    if (raw.length === 0) {
                        continue;
                    }
                }

                const paramName = fieldMapping[key] || key;
                formData.append(paramName, toParamString(value, key));
            }

            const response = await deviceFetch('/update_settings', {
                method: 'POST',
                body: formData,
            });

            await ensureOk(response, 'Failed to save settings');
            return true;
        } catch (err) {
            setError(err.message);
            throw err;
        } finally {
            setLoading(false);
        }
    }, [deviceFetch]);

    const updateBrightness = useCallback(async (brightness) => {
        try {
            const formData = new FormData();
            formData.append('value', toParamString(brightness, 'maxBrightness'));

            const response = await deviceFetch('/update_brightness', {
                method: 'POST',
                body: formData,
            });

            await ensureOk(response, 'Failed to update brightness');
            return true;
        } catch (err) {
            setError(err.message);
            throw err;
        }
    }, [deviceFetch]);

    const updateWifi = useCallback(async (ssid, password) => {
        setLoading(true);
        setError(null);

        try {
            const formData = new FormData();
            formData.append('ssid', String(ssid || ''));
            formData.append('password', String(password || ''));

            const response = await deviceFetch('/update_wifi', {
                method: 'POST',
                body: formData,
            });

            await ensureOk(response, 'Failed to update WiFi settings');
            return true;
        } catch (err) {
            setError(err.message);
            throw err;
        } finally {
            setLoading(false);
        }
    }, [deviceFetch]);

    const restartDevice = useCallback(async () => {
        try {
            const response = await deviceFetch('/restart', {
                method: 'POST',
            });
            await ensureOk(response, 'Failed to restart device');
            return true;
        } catch (err) {
            setError(err.message);
            throw err;
        }
    }, [deviceFetch]);

    return {
        getSettings,
        saveSettings,
        updateBrightness,
        updateWifi,
        restartDevice,
        loading,
        error,
        constants: {
            LED_TYPES,
            COLOR_ORDERS,
            LED_LIBRARIES,
            OBJECT_TYPES,
        },
    };
};

export default useSettings;
