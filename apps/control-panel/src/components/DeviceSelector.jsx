import React from 'react';
import { Settings } from 'lucide-react';

const DeviceSelector = ({ devices, selectedDevice, onDeviceSelect, onManageDevices }) => {
    return (
        <div className="bg-zinc-800 rounded-lg p-4">
            {devices.length === 0 ? (
                <div className="text-center py-4">
                    <p className="text-zinc-500 text-sm mb-2">No devices configured</p>
                    <button
                        onClick={onManageDevices}
                        className="text-sky-400 hover:text-sky-300 text-sm underline"
                    >
                        Add devices
                    </button>
                </div>
            ) : (
                <>
                    <select
                        value={selectedDevice || ''}
                        onChange={(e) => onDeviceSelect(e.target.value)}
                        className="w-full bg-zinc-700 border border-zinc-600 rounded px-3 py-2 text-white text-sm focus:border-sky-500 focus:outline-none mb-2"
                    >
                        <option value="">Choose device...</option>
                        {devices.map((ip) => (
                            <option key={ip} value={ip}>
                                {ip}
                            </option>
                        ))}
                    </select>
                </>
            )}
            <div className="flex items-center justify-between">
                <button
                    onClick={onManageDevices}
                    className="text-zinc-400 hover:text-sky-400 text-xs flex items-center gap-1"
                    title="Manage Devices"
                >
                    <Settings size={14} />
                    Manage
                </button>
            </div>
        </div>
    );
};

export default DeviceSelector;
