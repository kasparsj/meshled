import React, { useState, useEffect } from 'react';
import { X, Plus, Trash2 } from 'lucide-react';

const DeviceManagementModal = ({ isOpen, onClose, devices, setDevices }) => {
    const [newDeviceIP, setNewDeviceIP] = useState('');

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

    const addDevice = () => {
        if (newDeviceIP && !devices.includes(newDeviceIP)) {
            setDevices([...devices, newDeviceIP]);
            setNewDeviceIP('');
        }
    };

    const removeDevice = (ip) => {
        setDevices(devices.filter(device => device !== ip));
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
                            onKeyPress={(e) => e.key === 'Enter' && addDevice()}
                        />
                        <button
                            onClick={addDevice}
                            className="bg-green-600 hover:bg-green-700 px-4 py-2 rounded flex items-center gap-2"
                        >
                            <Plus size={16} />
                            Add
                        </button>
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