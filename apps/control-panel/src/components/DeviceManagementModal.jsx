import React, { useState, useEffect } from 'react';
import { X, Plus, Trash2, Search, KeyRound, Eye, EyeOff } from 'lucide-react';
import { useDevice } from '../contexts/DeviceContext.jsx';
import { fetchDeviceJsonWithTimeout, sanitizeHost } from '../utils/deviceRequest';

const sanitizeAddress = (value) => sanitizeHost(value).replace(/\s+/g, '');

const isSeedHostAllowed = (value) => {
    const host = sanitizeAddress(value).toLowerCase();
    return host.length > 0 && host !== 'localhost' && host !== '127.0.0.1' && host !== '::1';
};

const probeDeviceInfo = async (address, apiToken) => {
    const cleanAddress = sanitizeAddress(address);
    if (!cleanAddress) {
        return null;
    }

    try {
        const info = await fetchDeviceJsonWithTimeout(cleanAddress, '/device_info', {
            timeoutMs: 1600,
            apiToken,
        });
        return sanitizeAddress(info?.ip) || cleanAddress;
    } catch {
        try {
            const info = await fetchDeviceJsonWithTimeout(cleanAddress, '/json/info', {
                timeoutMs: 1600,
                apiToken,
            });
            return sanitizeAddress(info?.ip) || cleanAddress;
        } catch {
            return null;
        }
    }
};

const sortDevices = (devices) =>
    [...devices].sort((a, b) => a.localeCompare(b, undefined, { numeric: true, sensitivity: 'base' }));

const DeviceManagementModal = ({
    isOpen,
    onClose,
    devices,
    setDevices,
    isDirectDeviceMode = false,
    requireToken = false,
}) => {
    const {
        selectedDevice,
        apiToken,
        setApiToken,
        clearApiToken,
        authError,
    } = useDevice();
    const [newDeviceIP, setNewDeviceIP] = useState('');
    const [isDiscovering, setIsDiscovering] = useState(false);
    const [discoveryMessage, setDiscoveryMessage] = useState('');
    const [token, setToken] = useState(apiToken || '');
    const [showToken, setShowToken] = useState(false);
    const [tokenError, setTokenError] = useState('');
    const blocking = requireToken && String(apiToken || '').trim().length === 0;
    const hasApiToken = String(apiToken || '').trim().length > 0;

    // Handle ESC key to close modal
    useEffect(() => {
        const handleKeyDown = (event) => {
            if (event.key === 'Escape' && isOpen && !blocking) {
                onClose();
            }
        };

        if (isOpen) {
            document.addEventListener('keydown', handleKeyDown);
            return () => document.removeEventListener('keydown', handleKeyDown);
        }
    }, [isOpen, onClose, blocking]);

    useEffect(() => {
        if (!isOpen) {
            return;
        }
        setToken(apiToken || '');
        setShowToken(false);
        setTokenError('');
        setDiscoveryMessage('');
    }, [isOpen, apiToken]);

    const saveToken = () => {
        const trimmed = token.trim();
        if (trimmed.length === 0) {
            setTokenError('Token cannot be empty.');
            return;
        }
        setApiToken(trimmed);
        setTokenError('');
    };

    const clearToken = () => {
        clearApiToken();
        setToken('');
        setTokenError('');
    };

    const addDevice = () => {
        const cleanAddress = sanitizeAddress(newDeviceIP);
        if (cleanAddress && !devices.includes(cleanAddress)) {
            setDevices(sortDevices([...devices, cleanAddress]));
            setNewDeviceIP('');
        }
    };

    const removeDevice = (ip) => {
        setDevices(devices.filter(device => device !== ip));
    };

    const discoverDevices = async () => {
        const seedDevices = new Set(devices.map(sanitizeAddress).filter(Boolean));
        if (isSeedHostAllowed(window.location.hostname)) {
            seedDevices.add(sanitizeAddress(window.location.hostname));
        }

        if (seedDevices.size === 0) {
            setDiscoveryMessage('Add one device first, or open the UI from a device IP.');
            return;
        }

        setIsDiscovering(true);
        setDiscoveryMessage(`Scanning from ${seedDevices.size} seed device(s)...`);

        try {
            const candidates = new Set(seedDevices);

            await Promise.all(
                Array.from(seedDevices).map(async (seed) => {
                    try {
                        const discovered = await fetchDeviceJsonWithTimeout(seed, '/get_devices', {
                            timeoutMs: 2200,
                            apiToken,
                        });
                        if (!Array.isArray(discovered)) {
                            return;
                        }

                        discovered
                            .map(sanitizeAddress)
                            .filter(Boolean)
                            .forEach((address) => candidates.add(address));
                    } catch {
                        // Continue discovery from other seeds even if one seed fails.
                    }
                })
            );

            setDiscoveryMessage(`Validating ${candidates.size} discovered endpoint(s)...`);
            const validatedDevices = await Promise.all(
                Array.from(candidates).map((candidate) => probeDeviceInfo(candidate, apiToken))
            );

            const discoveredDevices = sortDevices(Array.from(new Set(validatedDevices.filter(Boolean))));

            if (discoveredDevices.length === 0) {
                setDiscoveryMessage('No devices detected.');
                return;
            }

            const mergedDevices = sortDevices(Array.from(new Set([...devices, ...discoveredDevices])));
            setDevices(mergedDevices);
            setDiscoveryMessage(`Detected ${discoveredDevices.length} device(s).`);
        } catch (error) {
            const message = error instanceof Error ? error.message : 'Discovery failed';
            setDiscoveryMessage(`Discovery failed: ${message}`);
        } finally {
            setIsDiscovering(false);
        }
    };

    if (!isOpen) return null;

    return (
        <div className="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50">
            <div className="bg-zinc-800 rounded-lg p-6 w-full max-w-lg mx-4">
                <div className="flex items-center justify-between mb-4">
                    <h2 className="text-xl font-bold">Manage Devices</h2>
                    <button
                        onClick={onClose}
                        disabled={blocking}
                        className="text-zinc-400 hover:text-white disabled:opacity-50"
                    >
                        <X size={20} />
                    </button>
                </div>

                {!isDirectDeviceMode ? (
                    <>
                        <div className="mb-6">
                            <label className="block text-sm font-medium text-zinc-300 mb-2">
                                Add Device IP
                            </label>
                            <div className="flex gap-2">
                                <input
                                    type="text"
                                    value={newDeviceIP}
                                    onChange={(e) => setNewDeviceIP(e.target.value)}
                                    placeholder="192.168.1.100"
                                    className="flex-1 bg-zinc-700 border border-zinc-600 rounded px-3 py-2 text-white placeholder-zinc-400 focus:border-sky-500 focus:outline-none"
                                    onKeyDown={(e) => e.key === 'Enter' && addDevice()}
                                />
                                <button
                                    onClick={addDevice}
                                    className="bg-green-600 hover:bg-green-700 px-4 py-2 rounded flex items-center gap-2"
                                >
                                    <Plus size={16} />
                                    Add
                                </button>
                            </div>
                            <div className="mt-3 flex items-center justify-between gap-2">
                                <button
                                    onClick={discoverDevices}
                                    disabled={isDiscovering}
                                    className="bg-sky-600 hover:bg-sky-700 disabled:bg-zinc-600 disabled:cursor-not-allowed px-4 py-2 rounded flex items-center gap-2 text-sm"
                                >
                                    <Search size={16} />
                                    {isDiscovering ? 'Scanning...' : 'Auto-detect'}
                                </button>
                                {discoveryMessage && (
                                    <p className="text-xs text-zinc-400 text-right">{discoveryMessage}</p>
                                )}
                            </div>
                        </div>

                        <div className="mb-6">
                            <h3 className="text-sm font-medium text-zinc-300 mb-3">
                                Configured Devices ({devices.length})
                            </h3>
                            {devices.length === 0 ? (
                                <div className="text-zinc-500 text-center py-4">
                                    No devices configured
                                </div>
                            ) : (
                                <div className="space-y-2 max-h-60 overflow-y-auto">
                                    {devices.map((ip) => (
                                        <div
                                            key={ip}
                                            className="flex items-center justify-between bg-zinc-700 rounded px-3 py-2"
                                        >
                                            <span className="text-sky-400 font-mono">{ip}</span>
                                            <button
                                                onClick={() => removeDevice(ip)}
                                                className="text-red-400 hover:text-red-300 p-1"
                                                title="Remove device"
                                            >
                                                <Trash2 size={16} />
                                            </button>
                                        </div>
                                    ))}
                                </div>
                            )}
                        </div>
                    </>
                ) : (
                    <div className="mb-6 bg-zinc-700/40 border border-zinc-700 rounded px-3 py-2">
                        <p className="text-sm text-zinc-300">
                            Direct-device mode is active.
                        </p>
                        <p className="text-xs text-zinc-400 mt-1">
                            Active device: <span className="text-sky-400 font-mono">{selectedDevice || 'unknown'}</span>
                        </p>
                    </div>
                )}

                <div className="mb-6 border-t border-zinc-700 pt-4">
                    <h3 className="text-sm font-medium text-zinc-300 mb-2 flex items-center gap-2">
                        <KeyRound size={16} />
                        API Token
                    </h3>
                    <p className="text-xs text-zinc-400 mb-3">
                        {blocking
                            ? 'A token is required by this device before protected routes can be used.'
                            : 'Store a bearer token used for protected firmware routes.'}
                    </p>
                    <div className="relative">
                        <input
                            type={showToken ? 'text' : 'password'}
                            value={token}
                            onChange={(event) => {
                                setToken(event.target.value);
                                if (tokenError) {
                                    setTokenError('');
                                }
                            }}
                            onKeyDown={(event) => {
                                if (event.key === 'Enter') {
                                    saveToken();
                                }
                            }}
                            className="w-full bg-zinc-700 border border-zinc-600 rounded px-3 py-2 pr-10 text-white placeholder-zinc-400 focus:border-sky-500 focus:outline-none"
                            placeholder="Enter bearer token"
                        />
                        <button
                            type="button"
                            className="absolute right-2 top-1/2 -translate-y-1/2 text-zinc-400 hover:text-white"
                            onClick={() => setShowToken((prev) => !prev)}
                        >
                            {showToken ? <EyeOff size={16} /> : <Eye size={16} />}
                        </button>
                    </div>
                    {(tokenError || authError) && (
                        <div className="mt-3 bg-red-900/40 border border-red-500/60 text-red-200 px-3 py-2 rounded text-sm">
                            {tokenError || authError}
                        </div>
                    )}
                    {hasApiToken && !tokenError && !authError && (
                        <p className="mt-3 text-xs text-zinc-400">Token is configured.</p>
                    )}
                    <div className="mt-4 flex justify-end gap-3">
                        <button
                            onClick={clearToken}
                            className="px-4 py-2 rounded bg-zinc-700 hover:bg-zinc-600 text-zinc-100 flex items-center gap-2 disabled:opacity-50"
                            disabled={!hasApiToken && token.trim().length === 0}
                        >
                            <Trash2 size={16} />
                            Clear
                        </button>
                        <button
                            onClick={saveToken}
                            className="px-5 py-2 rounded bg-sky-600 hover:bg-sky-700 text-white"
                        >
                            Save Token
                        </button>
                    </div>
                </div>

                <div className="flex justify-end">
                    <button
                        onClick={onClose}
                        disabled={blocking}
                        className="bg-zinc-600 hover:bg-zinc-700 px-4 py-2 rounded text-white disabled:opacity-50"
                    >
                        Close
                    </button>
                </div>
            </div>
        </div>
    );
};

export default DeviceManagementModal;
