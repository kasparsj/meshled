import React, { useState, useEffect } from 'react';
import { X, Plus, Trash2, Search } from 'lucide-react';

const sanitizeAddress = (value) => {
    if (!value) {
        return '';
    }

    return String(value)
        .trim()
        .replace(/^https?:\/\//i, '')
        .replace(/\/.*$/, '')
        .replace(/\s+/g, '');
};

const isSeedHostAllowed = (value) => {
    const host = sanitizeAddress(value).toLowerCase();
    return host.length > 0 && host !== 'localhost' && host !== '127.0.0.1' && host !== '::1';
};

const fetchJsonWithTimeout = async (url, timeoutMs = 2000) => {
    const controller = new AbortController();
    const timeoutId = window.setTimeout(() => controller.abort(), timeoutMs);

    try {
        const response = await fetch(url, { signal: controller.signal });
        if (!response.ok) {
            throw new Error(`HTTP ${response.status}`);
        }
        return await response.json();
    } finally {
        window.clearTimeout(timeoutId);
    }
};

const probeDeviceInfo = async (address) => {
    const cleanAddress = sanitizeAddress(address);
    if (!cleanAddress) {
        return null;
    }

    try {
        const info = await fetchJsonWithTimeout(`http://${cleanAddress}/device_info`, 1600);
        return sanitizeAddress(info?.ip) || cleanAddress;
    } catch {
        try {
            const info = await fetchJsonWithTimeout(`http://${cleanAddress}/json/info`, 1600);
            return sanitizeAddress(info?.ip) || cleanAddress;
        } catch {
            return null;
        }
    }
};

const sortDevices = (devices) =>
    [...devices].sort((a, b) => a.localeCompare(b, undefined, { numeric: true, sensitivity: 'base' }));

const DeviceManagementModal = ({ isOpen, onClose, devices, setDevices }) => {
    const [newDeviceIP, setNewDeviceIP] = useState('');
    const [isDiscovering, setIsDiscovering] = useState(false);
    const [discoveryMessage, setDiscoveryMessage] = useState('');

    // Handle ESC key to close modal
    useEffect(() => {
        const handleKeyDown = (event) => {
            if (event.key === 'Escape' && isOpen) {
                onClose();
            }
        };

        if (isOpen) {
            document.addEventListener('keydown', handleKeyDown);
            return () => document.removeEventListener('keydown', handleKeyDown);
        }
    }, [isOpen, onClose]);

    useEffect(() => {
        if (!isOpen) {
            return;
        }
        setDiscoveryMessage('');
    }, [isOpen]);

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
                        const discovered = await fetchJsonWithTimeout(`http://${seed}/get_devices`, 2200);
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
                Array.from(candidates).map((candidate) => probeDeviceInfo(candidate))
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
            <div className="bg-zinc-800 rounded-lg p-6 w-full max-w-md mx-4">
                <div className="flex items-center justify-between mb-4">
                    <h2 className="text-xl font-bold">Manage Devices</h2>
                    <button
                        onClick={onClose}
                        className="text-zinc-400 hover:text-white"
                    >
                        <X size={20} />
                    </button>
                </div>

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

                <div className="flex justify-end">
                    <button
                        onClick={onClose}
                        className="bg-zinc-600 hover:bg-zinc-700 px-4 py-2 rounded text-white"
                    >
                        Close
                    </button>
                </div>
            </div>
        </div>
    );
};

export default DeviceManagementModal;
