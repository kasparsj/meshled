import { useState, useCallback } from 'react';
import { useDevice } from '../contexts/DeviceContext';

const useSettings = () => {
    const { deviceFetch } = useDevice();
    const [loading, setLoading] = useState(false);
    const [error, setError] = useState(null);

    // Constants to match C++ enums
    const LED_TYPES = {
        0: 'WS2812',
        1: 'WS2811'
    };

    const COLOR_ORDERS = {
        0: 'RGB',
        4: 'GRB',
        155: 'RGBW',
        203: 'GRBW'
    };

    const LED_LIBRARIES = {
        0: 'NeoPixelBus',
        1: 'FastLED'
    };

    const OBJECT_TYPES = {
        0: 'Heptagon919',
        1: 'Line',
        2: 'Triangle', 
        3: 'Heptagon3024',
        4: 'Custom (Imported Topology)'
    };

    const getSettings = useCallback(async () => {
        setLoading(true);
        setError(null);
        
        try {
            const response = await deviceFetch('/get_settings');
            if (!response.ok) {
                throw new Error('Failed to fetch settings');
            }
            const data = await response.json();
            return data;
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
            
            // Map React field names to C++ parameter names
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
                apiAuthToken: 'api_auth_token'
            };

            Object.entries(settings).forEach(([key, value]) => {
                const paramName = fieldMapping[key] || key;
                // Convert boolean to 1/0 for C++ compatibility
                const paramValue = typeof value === 'boolean' ? (value ? '1' : '0') : value.toString();
                formData.append(paramName, paramValue);
            });

            const response = await deviceFetch('/update_settings', {
                method: 'POST',
                body: formData
            });

            if (!response.ok) {
                throw new Error('Failed to save settings');
            }

            return true;
        } catch (err) {
            setError(err.message);
            throw err;
        } finally {
            setLoading(false);
        }
    }, [deviceFetch]);

    const updateWifi = useCallback(async (ssid, password) => {
        setLoading(true);
        setError(null);
        
        try {
            const formData = new FormData();
            formData.append('ssid', ssid);
            formData.append('password', password);

            const response = await deviceFetch('/update_wifi', {
                method: 'POST',
                body: formData
            });

            if (!response.ok) {
                throw new Error('Failed to update WiFi settings');
            }

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
            await deviceFetch('/restart', {
                method: 'POST'
            });
            return true;
        } catch (err) {
            setError(err.message);
            throw err;
        }
    }, [deviceFetch]);

    return {
        getSettings,
        saveSettings,
        updateWifi,
        restartDevice,
        loading,
        error,
        constants: {
            LED_TYPES,
            COLOR_ORDERS,
            LED_LIBRARIES,
            OBJECT_TYPES
        }
    };
};

export default useSettings;
