import React, { createContext, useContext } from 'react';

const DeviceContext = createContext();

export const DeviceProvider = ({ children, selectedDevice }) => {
    const deviceFetch = async (url, options = {}) => {
        if (!selectedDevice) {
            throw new Error('No device selected');
        }
        
        // Ensure URL starts with /
        const cleanUrl = url.startsWith('/') ? url : `/${url}`;
        const fullUrl = `http://${selectedDevice}${cleanUrl}`;
        
        return fetch(fullUrl, options);
    };

    const value = {
        selectedDevice,
        deviceFetch
    };

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