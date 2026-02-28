import React, { createContext, useCallback, useContext, useEffect, useMemo, useState } from 'react';
import { buildAuthHeaders, buildDeviceRequestUrl } from '../utils/deviceRequest';

const DeviceContext = createContext();

const PROTECTED_ROUTE_PREFIXES = [
    '/update_',
    '/add_',
    '/remove_',
    '/toggle_',
    '/save_',
    '/sync_',
    '/restart',
    '/get_settings',
    '/cross_device/',
];

const isProtectedRoute = (path) => {
    return PROTECTED_ROUTE_PREFIXES.some((prefix) => path.startsWith(prefix));
};

const initialToken = () => {
    if (typeof window === 'undefined') {
        return '';
    }
    return String(window.localStorage.getItem('ledController_apiToken') || '').trim();
};

export const DeviceProvider = ({ children, selectedDevice }) => {
    const [apiToken, setApiTokenState] = useState(initialToken);
    const [authRequired, setAuthRequired] = useState(false);
    const [authError, setAuthError] = useState('');

    useEffect(() => {
        setAuthRequired(false);
        setAuthError('');
    }, [selectedDevice]);

    const setApiToken = useCallback((nextToken) => {
        const trimmed = String(nextToken || '').trim();
        setApiTokenState(trimmed);
        if (typeof window !== 'undefined') {
            if (trimmed.length > 0) {
                window.localStorage.setItem('ledController_apiToken', trimmed);
                setAuthError('');
            } else {
                window.localStorage.removeItem('ledController_apiToken');
            }
        }
    }, []);

    const clearApiToken = useCallback(() => {
        setApiTokenState('');
        if (typeof window !== 'undefined') {
            window.localStorage.removeItem('ledController_apiToken');
        }
        setAuthError('');
    }, []);

    const deviceFetch = useCallback(async (url, options = {}) => {
        if (!selectedDevice) {
            throw new Error('No device selected');
        }

        const cleanUrl = url.startsWith('/') ? url : `/${url}`;
        if (authRequired && apiToken.length === 0 && isProtectedRoute(cleanUrl)) {
            const message = 'API token required for protected device routes.';
            setAuthError(message);
            throw new Error(message);
        }

        let fullUrl;
        try {
            fullUrl = buildDeviceRequestUrl(selectedDevice, cleanUrl);
        } catch (error) {
            setAuthError(error.message);
            throw error;
        }

        const headers = buildAuthHeaders(options.headers, apiToken);
        const response = await fetch(fullUrl, {
            ...options,
            headers,
        });

        if (response.status === 401) {
            setAuthRequired(true);
            if (apiToken.length > 0) {
                setAuthError('Stored API token was rejected by device. Update the token and retry.');
            } else {
                setAuthError('API token required by selected device.');
            }
        }

        if (response.ok && apiToken.length > 0) {
            setAuthError('');
        }

        return response;
    }, [selectedDevice, authRequired, apiToken]);

    const value = useMemo(() => ({
        selectedDevice,
        deviceFetch,
        apiToken,
        hasApiToken: apiToken.length > 0,
        setApiToken,
        clearApiToken,
        authRequired,
        authError,
    }), [selectedDevice, deviceFetch, apiToken, setApiToken, clearApiToken, authRequired, authError]);

    return (
        <DeviceContext.Provider value={value}>
            {children}
        </DeviceContext.Provider>
    );
};

export const useDevice = () => {
    const context = useContext(DeviceContext);
    if (!context) {
        throw new Error('useDevice must be used within a DeviceProvider');
    }
    return context;
};
